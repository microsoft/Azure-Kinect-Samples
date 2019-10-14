// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <assert.h>
#include <iostream>

#include <k4a/k4a.hpp>
#include <k4abt.hpp>

void print_body_information(k4abt_body_t body)
{
    std::cout << "Body ID: " << body.id << std::endl;
    for (int i = 0; i < (int)K4ABT_JOINT_COUNT; i++)
    {
        k4a_float3_t position = body.skeleton.joints[i].position;
        k4a_quaternion_t orientation = body.skeleton.joints[i].orientation;
        k4abt_joint_confidence_level_t confidence_level = body.skeleton.joints[i].confidence_level;
        printf("Joint[%d]: Position[mm] ( %f, %f, %f ); Orientation ( %f, %f, %f, %f); Confidence Level (%d)  \n",
            i, position.v[0], position.v[1], position.v[2], orientation.v[0], orientation.v[1], orientation.v[2], orientation.v[3], confidence_level);
    }
}

void print_body_index_map_middle_line(k4a::image body_index_map)
{
    uint8_t* body_index_map_buffer = body_index_map.get_buffer();

    // Given body_index_map pixel type should be uint8, the stride_byte should be the same as width
    // TODO: Since there is no API to query the byte-per-pixel information, we have to compare the width and stride to
    // know the information. We should replace this assert with proper byte-per-pixel query once the API is provided by
    // K4A SDK.
    assert(body_index_map.get_stride_bytes() == body_index_map.get_width_pixels());

    int middle_line_num = body_index_map.get_height_pixels() / 2;
    body_index_map_buffer = body_index_map_buffer + middle_line_num * body_index_map.get_width_pixels();

    std::cout << "BodyIndexMap at Line " << middle_line_num << ":" << std::endl;
    for (int i = 0; i < body_index_map.get_width_pixels(); i++)
    {
        std::cout << (int)*body_index_map_buffer << ", ";
        body_index_map_buffer++;
    }
    std::cout << std::endl;
}

int main()
{
    try
    {
        k4a_device_configuration_t device_config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
        device_config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;

        k4a::device device = k4a::device::open(0);
        device.start_cameras(&device_config);

        k4a::calibration sensor_calibration = device.get_calibration(device_config.depth_mode, device_config.color_resolution);

        k4abt::tracker tracker = k4abt::tracker::create(sensor_calibration);

        int frame_count = 0;
        do
        {
            k4a::capture sensor_capture;
            if (device.get_capture(&sensor_capture, std::chrono::milliseconds(K4A_WAIT_INFINITE)))
            {
                frame_count++;

                std::cout << "Start processing frame " << frame_count << std::endl;

                if (!tracker.enqueue_capture(sensor_capture))
                {
                    // It should never hit timeout when K4A_WAIT_INFINITE is set.
                    std::cout << "Error! Add capture to tracker process queue timeout!" << std::endl;
                    break;
                }

                k4abt::frame body_frame = tracker.pop_result();
                if (body_frame != nullptr)
                {
                    size_t num_bodies = body_frame.get_num_bodies();
                    std::cout << num_bodies << " bodies are detected!" << std::endl;

                    for (size_t i = 0; i < num_bodies; i++)
                    {
                        k4abt_body_t body = body_frame.get_body(i);
                        print_body_information(body);
                    }

                    k4a::image body_index_map = body_frame.get_body_index_map();
                    if (body_index_map != nullptr)
                    {
                        print_body_index_map_middle_line(body_index_map);
                    }
                    else
                    {
                        std::cout << "Error: Failed to generate bodyindex map!" << std::endl;
                    }
                }
                else
                {
                    //  It should never hit timeout when K4A_WAIT_INFINITE is set.
                    std::cout << "Error! Pop body frame result time out!" << std::endl;
                    break;
                }
            }
            else
            {
                // It should never hit time out when K4A_WAIT_INFINITE is set.
                std::cout << "Error! Get depth frame time out!" << std::endl;
                break;
            }
        } while (frame_count < 100);
        std::cout << "Finished body tracking processing!" << std::endl;

    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed with exception:" << std::endl
            << "    " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
