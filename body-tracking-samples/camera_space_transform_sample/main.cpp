// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <assert.h>
#include <iostream>

#include <k4a/k4a.h>
#include <k4abt.h>

#define VERIFY(result, error)                                                                            \
    if(result != K4A_RESULT_SUCCEEDED)                                                                   \
    {                                                                                                    \
        printf("%s \n - (File: %s, Function: %s, Line: %d)\n", error, __FILE__, __FUNCTION__, __LINE__); \
        exit(1);                                                                                         \
    }                                                                                                    \

void print_body_index_map_middle_line(k4a_image_t body_index_map)
{
    uint8_t* body_index_map_buffer = k4a_image_get_buffer(body_index_map);

    // Given body_index_map pixel type should be uint8, the stride_byte should be the same as width
    // TODO: Since there is no API to query the byte-per-pixel information, we have to compare the width and stride to
    // know the information. We should replace this assert with proper byte-per-pixel query once the API is provided by
    // K4A SDK.
    assert(k4a_image_get_stride_bytes(body_index_map) == k4a_image_get_width_pixels(body_index_map));

    int middle_line_num = k4a_image_get_height_pixels(body_index_map) / 2;
    body_index_map_buffer = body_index_map_buffer + middle_line_num * k4a_image_get_width_pixels(body_index_map);

    printf("BodyIndexMap at Line %d:\n", middle_line_num);
    for (int i = 0; i < k4a_image_get_width_pixels(body_index_map); i++)
    {
        printf("%u, ", *body_index_map_buffer);
        body_index_map_buffer++;
    }
    printf("\n");
}

// Transform skeleton results from 3d depth space to 2d color image space
inline bool transform_joint_from_depth_3d_to_color_2d(
    const k4a_calibration_t* calibration, 
    k4a_float3_t joint_in_depth_space, 
    k4a_float2_t& joint_in_color_2d)
{
    int valid;
    VERIFY(k4a_calibration_3d_to_2d(
        calibration, 
        &joint_in_depth_space, 
        K4A_CALIBRATION_TYPE_DEPTH, 
        K4A_CALIBRATION_TYPE_COLOR, 
        &joint_in_color_2d, 
        &valid), "Failed to project 3d joint from depth space to 2d color image space!");

    return valid != 0;
}

// Transform body index map results from depth space to color space
void transform_body_index_map_from_depth_to_color(
    k4a_transformation_t transformation_handle,
    const k4a_image_t depth_image,
    const k4a_image_t body_index_map_in_depth_space, 
    k4a_image_t depth_image_in_color_space,
    k4a_image_t body_index_map_in_color_space)
{
    // Note:
    // 1. Depth image - In order to transform the body index map to color space, the corresponding depth image is 
    //    required to help perform this transformation in 3d.
    // 2. Interpolation type - Each pixel value for the body index map represents the body index. It is not interpolatable.
    //    The interpolation method has to be set to K4A_TRANSFORMATION_INTERPOLATION_TYPE_NEAREST.
    // 3. Invalid custom value - Because there is disparity between the depth camera and color camera. There might be 
    //    invalid values during the transform. We want this invalid value to be set to K4ABT_BODY_INDEX_MAP_BACKGROUND.

    VERIFY(k4a_transformation_depth_image_to_color_camera_custom(
        transformation_handle,
        depth_image,
        body_index_map_in_depth_space,
        depth_image_in_color_space,
        body_index_map_in_color_space,
        K4A_TRANSFORMATION_INTERPOLATION_TYPE_NEAREST,
        K4ABT_BODY_INDEX_MAP_BACKGROUND), "Failed to transform body index map to color space!");
}

int main()
{
    k4a_device_configuration_t device_config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    device_config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    device_config.color_resolution = K4A_COLOR_RESOLUTION_720P;

    k4a_device_t device;
    VERIFY(k4a_device_open(0, &device), "Open K4A Device failed!");
    VERIFY(k4a_device_start_cameras(device, &device_config), "Start K4A cameras failed!");

    // Make sure to pass in the correct device config for both depth camera and color camera to get the correct sensor calibration
    k4a_calibration_t sensor_calibration;
    VERIFY(k4a_device_get_calibration(device, device_config.depth_mode, device_config.color_resolution, &sensor_calibration),
        "Get depth camera calibration failed!");

    // Create transformation handle to perform the body index map space transform
    k4a_transformation_t transformation = NULL;
    transformation = k4a_transformation_create(&sensor_calibration);
    if (transformation == NULL)
    {
        printf("Failed to create transformation from sensor calibration!");
        exit(1);
    }

    k4abt_tracker_t tracker = NULL;
    k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
    VERIFY(k4abt_tracker_create(&sensor_calibration, tracker_config, &tracker), "Body tracker initialization failed!");

    // Preallocated the buffers to hold the depth image in color space and the body index map in color space
    int color_image_width_pixels = sensor_calibration.color_camera_calibration.resolution_width;
    int color_image_height_pixels = sensor_calibration.color_camera_calibration.resolution_height;

    k4a_image_t depth_image_in_color_space = NULL;
    VERIFY(k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16,
        color_image_width_pixels,
        color_image_height_pixels,
        color_image_width_pixels * (int)sizeof(uint16_t),
        &depth_image_in_color_space), "Failed to create empty image for the depth image in color space");

    k4a_image_t body_index_map_in_color_space = NULL;
    VERIFY(k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM8,
        color_image_width_pixels,
        color_image_height_pixels,
        color_image_width_pixels * (int)sizeof(uint8_t),
        &body_index_map_in_color_space), "Failed to create empty image for the body index map in color space");

    int frame_count = 0;
    do
    {
        k4a_capture_t sensor_capture;
        k4a_wait_result_t get_capture_result = k4a_device_get_capture(device, &sensor_capture, K4A_WAIT_INFINITE);
        if (get_capture_result == K4A_WAIT_RESULT_SUCCEEDED)
        {
            frame_count++;

            printf("Start processing frame %d\n", frame_count);

            k4a_wait_result_t queue_capture_result = k4abt_tracker_enqueue_capture(tracker, sensor_capture, K4A_WAIT_INFINITE);

            k4a_capture_release(sensor_capture);
            if (queue_capture_result == K4A_WAIT_RESULT_TIMEOUT)
            {
                // It should never hit timeout when K4A_WAIT_INFINITE is set.
                printf("Error! Add capture to tracker process queue timeout!\n");
                break;
            }
            else if (queue_capture_result == K4A_WAIT_RESULT_FAILED)
            {
                printf("Error! Add capture to tracker process queue failed!\n");
                break;
            }

            k4abt_frame_t body_frame = NULL;
            k4a_wait_result_t pop_frame_result = k4abt_tracker_pop_result(tracker, &body_frame, K4A_WAIT_INFINITE);
            if (pop_frame_result == K4A_WAIT_RESULT_SUCCEEDED)
            {
                uint32_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
                printf("%u bodies are detected!\n", num_bodies);

                // Transform each 3d joints from 3d depth space to 2d color image space
                for (uint32_t i = 0; i < num_bodies; i++)
                {
                    printf("Person[%u]:\n", i);
                    k4abt_skeleton_t skeleton;
                    VERIFY(k4abt_frame_get_body_skeleton(body_frame, i, &skeleton), "Get body from body frame failed!");
                    for (int joint_id = 0; joint_id < (int)K4ABT_JOINT_COUNT; joint_id++)
                    {
                        k4a_float2_t joint_in_color_2d;
                        bool valid = transform_joint_from_depth_3d_to_color_2d(
                            &sensor_calibration, 
                            skeleton.joints[joint_id].position,
                            joint_in_color_2d);
                        if (valid)
                        {
                            printf("Joint[%d]: Pixel Location at Color Image ( %f, %f) \n",
                                joint_id, joint_in_color_2d.v[0], joint_in_color_2d.v[1]);
                        }
                        else
                        {
                            printf("Joint[%d]: Invalid Pixel Location \n", joint_id);
                        }
                    }
                }

                // Transform the body index map from the depth space to color space
                k4a_image_t body_index_map_in_depth_space = k4abt_frame_get_body_index_map(body_frame);
                if (body_index_map_in_depth_space != NULL)
                {
                    // Depth image is needed in order to perform the body index map space transform
                    k4a_image_t depth_image = k4a_capture_get_depth_image(sensor_capture);

                    transform_body_index_map_from_depth_to_color(
                        transformation,
                        depth_image,
                        body_index_map_in_depth_space,
                        depth_image_in_color_space,
                        body_index_map_in_color_space);

                    print_body_index_map_middle_line(body_index_map_in_color_space);
                    k4a_image_release(body_index_map_in_depth_space);
                    k4a_image_release(depth_image);
                }
                else
                {
                    printf("Error: Fail to generate bodyindex map!\n");
                }

                k4abt_frame_release(body_frame);
            }
            else if (pop_frame_result == K4A_WAIT_RESULT_TIMEOUT)
            {
                //  It should never hit timeout when K4A_WAIT_INFINITE is set.
                printf("Error! Pop body frame result timeout!\n");
                break;
            }
            else
            {
                printf("Pop body frame result failed!\n");
                break;
            }
        }
        else if (get_capture_result == K4A_WAIT_RESULT_TIMEOUT)
        {
            // It should never hit time out when K4A_WAIT_INFINITE is set.
            printf("Error! Get depth frame time out!\n");
            break;
        }
        else
        {
            printf("Get depth capture returned error: %d\n", get_capture_result);
            break;
        }

    } while (frame_count < 100);

    printf("Finished body tracking processing!\n");

    k4a_image_release(depth_image_in_color_space);
    k4a_image_release(body_index_map_in_color_space);

    k4a_transformation_destroy(transformation);
    k4abt_tracker_shutdown(tracker);
    k4abt_tracker_destroy(tracker);
    k4a_device_stop_cameras(device);
    k4a_device_close(device);

    return 0;
}
