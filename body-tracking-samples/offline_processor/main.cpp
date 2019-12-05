// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#include <k4a/k4a.h>
#include <k4arecord/playback.h>
#include <k4abt.h>
#include <nlohmann/json.hpp>

#include <BodyTrackingHelpers.h>
#include <Utilities.h>

using namespace std;
using namespace nlohmann;

bool predict_joints(json &frames_json, int frame_count, k4abt_tracker_t tracker, k4a_capture_t capture_handle)
{
    k4a_wait_result_t queue_capture_result = k4abt_tracker_enqueue_capture(tracker, capture_handle, K4A_WAIT_INFINITE);
    if (queue_capture_result != K4A_WAIT_RESULT_SUCCEEDED)
    {
        cerr << "Error! Adding capture to tracker process queue failed!" << endl;
        return false;
    }

    k4abt_frame_t body_frame = nullptr;
    k4a_wait_result_t pop_frame_result = k4abt_tracker_pop_result(tracker, &body_frame, K4A_WAIT_INFINITE);
    if (pop_frame_result != K4A_WAIT_RESULT_SUCCEEDED)
    {
        cerr << "Error! Popping body tracking result failed!" << endl;
        return false;
    }

    uint32_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
    uint64_t timestamp = k4abt_frame_get_device_timestamp_usec(body_frame);

    json frame_result_json;
    frame_result_json["timestamp_usec"] = timestamp;
    frame_result_json["frame_id"] = frame_count;
    frame_result_json["num_bodies"] = num_bodies;
    frame_result_json["bodies"] = json::array();
    for (uint32_t i = 0; i < num_bodies; i++)
    {
        k4abt_skeleton_t skeleton;
        VERIFY(k4abt_frame_get_body_skeleton(body_frame, i, &skeleton), "Get body from body frame failed!");
        json body_result_json;
        int body_id = k4abt_frame_get_body_id(body_frame, i);
        body_result_json["body_id"] = body_id;

        for (int j = 0; j < (int)K4ABT_JOINT_COUNT; j++)
        {
            body_result_json["joint_positions"].push_back( {    skeleton.joints[j].position.xyz.x,
                                                                skeleton.joints[j].position.xyz.y,
                                                                skeleton.joints[j].position.xyz.z });

            body_result_json["joint_orientations"].push_back({  skeleton.joints[j].orientation.wxyz.w,
                                                                skeleton.joints[j].orientation.wxyz.x,
                                                                skeleton.joints[j].orientation.wxyz.y,
                                                                skeleton.joints[j].orientation.wxyz.z });
        }
        frame_result_json["bodies"].push_back(body_result_json);
    }
    frames_json.push_back(frame_result_json);
    k4abt_frame_release(body_frame);

    return true;
}

bool check_depth_image_exists(k4a_capture_t capture)
{
    k4a_image_t depth = k4a_capture_get_depth_image(capture);
    if (depth != nullptr)
    {
        k4a_image_release(depth);
        return true;
    }
    else
    {
        return false;
    }
}

bool process_mkv_offline(const char* input_path, const char* output_path)
{
    k4a_playback_t playback_handle = nullptr;
    k4a_result_t result = k4a_playback_open(input_path, &playback_handle);
    if (result != K4A_RESULT_SUCCEEDED)
    {
        cerr << "Cannot open recording at " << input_path << endl;
        return false;
    }

    k4a_calibration_t calibration;
    result = k4a_playback_get_calibration(playback_handle, &calibration);
    if (result != K4A_RESULT_SUCCEEDED)
    {
        cerr << "Failed to get calibration" << endl;
        return false;
    }

    k4abt_tracker_t tracker = NULL;
    k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
    if (K4A_RESULT_SUCCEEDED != k4abt_tracker_create(&calibration, tracker_config, &tracker))
    {
        cerr << "Body tracker initialization failed!" << endl;
        return false;
    }

    json json_output;
    json_output["k4abt_sdk_version"] = K4ABT_VERSION_STR;
    json_output["source_file"] = input_path;

    // Store all joint names to the json
    json_output["joint_names"] = json::array();
    for (int i = 0; i < (int)K4ABT_JOINT_COUNT; i++)
    {
        json_output["joint_names"].push_back(g_jointNames.find((k4abt_joint_id_t)i)->second);
    }

    // Store all bone linkings to the json
    json_output["bone_list"] = json::array();
    for (int i = 0; i < (int)g_boneList.size(); i++)
    {
        json_output["bone_list"].push_back({ g_jointNames.find(g_boneList[i].first)->second,
                                             g_jointNames.find(g_boneList[i].second)->second });
    }

    cout << "Tracking " << input_path << endl;

    int frame_count = 0;
    json frames_json = json::array();
    bool success = true;
    while (true)
    {
        k4a_capture_t capture_handle = nullptr;
        k4a_stream_result_t stream_result = k4a_playback_get_next_capture(playback_handle, &capture_handle);
        if (stream_result == K4A_STREAM_RESULT_EOF)
        {
            break;
        }

        cout << "frame " << frame_count << '\r';
        if (stream_result == K4A_STREAM_RESULT_SUCCEEDED)
        {
            // Only try to predict joints when capture contains depth image
            if (check_depth_image_exists(capture_handle))
            {
                success = predict_joints(frames_json, frame_count, tracker, capture_handle);
                k4a_capture_release(capture_handle);
                if (!success)
                {
                    cerr << "Predict joints failed for clip at frame " << frame_count << endl;
                    break;
                }
            }
        }
        else
        {
            success = false;
            cerr << "Stream error for clip at frame " << frame_count << endl;
            break;
        }

        frame_count++;
    }

    if (success)
    {
        json_output["frames"] = frames_json;
        cout << endl << "DONE " << endl;

        cout << "Total read " << frame_count << " frames" << endl;
        std::ofstream output_file(output_path);
        output_file << std::setw(4) << json_output << std::endl;
        cout << "Results saved in " << output_path;
    }

    k4a_playback_close(playback_handle);

    return success;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cout << "Usage: k4abt_offline_processor.exe <input_mkv_file> <output_json_file>" << endl;
        return -1;
    }

    return process_mkv_offline(argv[1], argv[2]) ? 0 : -1;
}
