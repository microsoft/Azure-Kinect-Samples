// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <k4abttypes.h>
#include <BodyTrackingHelpers.h>

#include "WindowController3d.h"



// This is a wrapper library that convert the types from the k4abt types to the window3d visualization library types
class Window3dWrapper
{
public:
    LIB_EXPORT ~Window3dWrapper();

    // Create Window3d wrapper without point cloud shading
    LIB_EXPORT void Create(
        const char* name,
        k4a_depth_mode_t depthMode,
        int windowWidth = -1,
        int windowHeight = -1);

    // Create Window3d wrapper with point cloud shading
    LIB_EXPORT void Create(
        const char* name,
        const k4a_calibration_t& sensorCalibration);

    LIB_EXPORT void SetCloseCallback(
        Visualization::CloseCallbackType closeCallback,
        void* closeCallbackContext = nullptr);

    LIB_EXPORT void SetKeyCallback(
        Visualization::KeyCallbackType keyCallback,
        void* keyCallbackContext = nullptr);

    LIB_EXPORT void Delete();

    LIB_EXPORT void UpdatePointClouds(k4a_image_t depthImage, std::vector<Color> pointCloudColors = std::vector<Color>());

    LIB_EXPORT void CleanJointsAndBones();

    LIB_EXPORT void AddJoint(k4a_float3_t position, k4a_quaternion_t orientation, Color color);

    LIB_EXPORT void AddBone(k4a_float3_t joint1Position, k4a_float3_t joint2Position, Color color);

    // Helper function to directly add the whole body for rendering instead of adding separate joints and bones
    LIB_EXPORT void AddBody(const k4abt_body_t& body, Color color);

    LIB_EXPORT void Render();

    // Window Configuration Functions
    LIB_EXPORT void SetFloorRendering(bool enableFloorRendering, float floorPositionX, float floorPositionY, float floorPositionZ);

    LIB_EXPORT void SetWindowPosition(int xPos, int yPos);

    // Render Setting Functions
    LIB_EXPORT void SetLayout3d(Visualization::Layout3d layout3d);
    LIB_EXPORT void SetJointFrameVisualization(bool enableJointFrameVisualization);

private:
    void InitializeCalibration(const k4a_calibration_t& sensorCalibration);

    void BlendBodyColor(linmath::vec4 color, Color bodyColor);

    void UpdateDepthBuffer(k4a_image_t depthImage);

    bool CreateXYDepthTable(const k4a_calibration_t& sensorCalibration);

private:
    Visualization::WindowController3d m_window3d;

    bool m_pointCloudUpdated = false;
    std::vector<uint16_t> m_depthBuffer;
    std::vector<Visualization::PointCloudVertex> m_pointClouds;

    struct XY
    {
        float x;
        float y;
    };
    std::vector<XY> m_xyDepthTable;
    uint32_t m_depthWidth = 0;
    uint32_t m_depthHeight = 0;
    k4a_transformation_t m_transformationHandle = nullptr;
    k4a_image_t m_pointCloudImage = nullptr;
};