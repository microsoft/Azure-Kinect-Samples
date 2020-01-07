// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <iostream>

#include <k4a/k4a.h>

#include "FloorDetector.h"
#include "PointCloudGenerator.h"
#include "Utilities.h"
#include "Window3dWrapper.h"

void PrintAppUsage()
{
    printf("\n");
    printf(" Basic Navigation:\n\n");
    printf(" Rotate: Rotate the camera by moving the mouse while holding mouse left button\n");
    printf(" Pan: Translate the scene by holding Ctrl key and drag the scene with mouse left button\n");
    printf(" Zoom in/out: Move closer/farther away from the scene center by scrolling the mouse scroll wheel\n");
    printf("\n");
    printf(" Key Shortcuts\n\n");
    printf(" ESC: quit\n");
    printf(" h: help\n");
    printf("\n");
}

// Global State and Key Process Function
bool s_isRunning = true;

int64_t ProcessKey(void* /*context*/, int key)
{
    // https://www.glfw.org/docs/latest/group__keys.html
    switch (key)
    {
        // Quit
    case GLFW_KEY_ESCAPE:
        s_isRunning = false;
        break;
    case GLFW_KEY_H:
        PrintAppUsage();
        break;
    }
    return 1;
}

int64_t CloseCallback(void* /*context*/)
{
    s_isRunning = false;
    return 1;
}

int main()
{
    PrintAppUsage();

    k4a_device_t device = nullptr;
    VERIFY(k4a_device_open(0, &device), "Open K4A Device failed!");

    // Start camera. Make sure depth camera is enabled.
    k4a_device_configuration_t deviceConfig = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    deviceConfig.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    deviceConfig.color_resolution = K4A_COLOR_RESOLUTION_OFF;
    VERIFY(k4a_device_start_cameras(device, &deviceConfig), "Start K4A cameras failed!");

    // Get calibration information.
    k4a_calibration_t sensorCalibration;
    VERIFY(k4a_device_get_calibration(device, deviceConfig.depth_mode, deviceConfig.color_resolution, &sensorCalibration),
        "Get depth camera calibration failed!");

    // Start imu for gravity vector.
    VERIFY(k4a_device_start_imu(device), "Start IMU failed!");

    // Initialize the 3d window controller.
    Window3dWrapper window3d;
    window3d.Create("3D Visualization", sensorCalibration);
    window3d.SetCloseCallback(CloseCallback);
    window3d.SetKeyCallback(ProcessKey);

    // PointCloudGenerator for floor estimation.
    Samples::PointCloudGenerator pointCloudGenerator{ sensorCalibration };
    Samples::FloorDetector floorDetector;

    while (s_isRunning)
    {
        k4a_capture_t sensorCapture = nullptr;
        k4a_wait_result_t getCaptureResult = k4a_device_get_capture(device, &sensorCapture, 0); // timeout_in_ms is set to 0

        if (getCaptureResult == K4A_WAIT_RESULT_SUCCEEDED)
        {
            k4a_image_t depthImage = k4a_capture_get_depth_image(sensorCapture);

            // Capture an IMU sample for sensor orientation.
            k4a_imu_sample_t imu_sample;
            if (k4a_device_get_imu_sample(device, &imu_sample, 0) == K4A_WAIT_RESULT_SUCCEEDED)
            {
                // Update point cloud.
                pointCloudGenerator.Update(depthImage);

                // Get down-sampled cloud points.
                const int downsampleStep = 2;
                const auto& cloudPoints = pointCloudGenerator.GetCloudPoints(downsampleStep);

                // Detect floor plane based on latest visual and inertial observations.
                const size_t minimumFloorPointCount = 1024 / (downsampleStep * downsampleStep);
                const auto& maybeFloorPlane = floorDetector.TryDetectFloorPlane(cloudPoints, imu_sample, sensorCalibration, minimumFloorPointCount);

                // Visualize point cloud.
                window3d.UpdatePointClouds(depthImage);

                // Visualize the floor plane.
                if (maybeFloorPlane.has_value())
                {
                    // For visualization purposes, make floor origin the projection of a point 1.5m in front of the camera.
                    Samples::Vector cameraOrigin = { 0, 0, 0 };
                    Samples::Vector cameraForward = { 0, 0, 1 };

                    auto p = maybeFloorPlane->ProjectPoint(cameraOrigin) + maybeFloorPlane->ProjectVector(cameraForward) * 1.5f;
                    auto n = maybeFloorPlane->Normal;
                    window3d.SetFloorRendering(true, p.X, p.Y, p.Z, n.X, n.Y, n.Z);
                }
                else
                {
                    window3d.SetFloorRendering(false, 0, 0, 0);
                }
            }

            // Release the sensor capture and depth image once they are no longer needed.
            k4a_capture_release(sensorCapture);
            k4a_image_release(depthImage);

        }
        else if (getCaptureResult != K4A_WAIT_RESULT_TIMEOUT)
        {
            std::cout << "Get depth capture returned error: " << getCaptureResult << std::endl;
            break;
        }

        window3d.Render();
    }

    window3d.Delete();

    k4a_device_stop_cameras(device);
    k4a_device_stop_imu(device);
    k4a_device_close(device);

    return 0;
}
