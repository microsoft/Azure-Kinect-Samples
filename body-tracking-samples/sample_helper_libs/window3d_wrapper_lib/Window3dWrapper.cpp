// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "Window3dWrapper.h"

#include <array>
#include <k4a/k4a.h>
#include <k4abt.h>

#include "Utilities.h"

const float MillimeterToMeter = 0.001f;

void ConvertMillimeterToMeter(k4a_float3_t positionInMM, linmath::vec3 outPositionInMeter)
{
    outPositionInMeter[0] = positionInMM.v[0] * MillimeterToMeter;
    outPositionInMeter[1] = positionInMM.v[1] * MillimeterToMeter;
    outPositionInMeter[2] = positionInMM.v[2] * MillimeterToMeter;
}

Window3dWrapper::~Window3dWrapper()
{
    Delete();
}

void Window3dWrapper::Create(
    const char* name,
    k4a_depth_mode_t depthMode,
    int windowWidth,
    int windowHeight)
{
    m_window3d.Create(name, true, windowWidth, windowHeight);
    m_window3d.SetMirrorMode(true);

    switch (depthMode)
    {
    case K4A_DEPTH_MODE_WFOV_UNBINNED:
    case K4A_DEPTH_MODE_WFOV_2X2BINNED:
        m_window3d.SetDefaultVerticalFOV(120.0f);
        break;
    case K4A_DEPTH_MODE_NFOV_2X2BINNED:
    case K4A_DEPTH_MODE_NFOV_UNBINNED:
    default:
        m_window3d.SetDefaultVerticalFOV(65.0f);
        break;
    }
}

void Window3dWrapper::Create(
    const char* name,
    const k4a_calibration_t& sensorCalibration)
{
    Create(name, sensorCalibration.depth_mode);
    InitializeCalibration(sensorCalibration);
}

void Window3dWrapper::SetCloseCallback(
    Visualization::CloseCallbackType closeCallback,
    void* closeCallbackContext)
{
    m_window3d.SetCloseCallback(closeCallback, closeCallbackContext);
}

void Window3dWrapper::SetKeyCallback(
    Visualization::KeyCallbackType keyCallback,
    void* keyCallbackContext)
{
    m_window3d.SetKeyCallback(keyCallback, keyCallbackContext);
}

void Window3dWrapper::Delete()
{
    m_window3d.Delete();

    if (m_transformationHandle != nullptr)
    {
        k4a_transformation_destroy(m_transformationHandle);
        m_transformationHandle = nullptr;
    }

    if (m_pointCloudImage != nullptr)
    {
        k4a_image_release(m_pointCloudImage);
        m_pointCloudImage = nullptr;
    }
}

void Window3dWrapper::UpdatePointClouds(k4a_image_t depthImage, std::vector<Color> pointCloudColors)
{
    m_pointCloudUpdated = true;
    VERIFY(k4a_transformation_depth_image_to_point_cloud(m_transformationHandle,
        depthImage,
        K4A_CALIBRATION_TYPE_DEPTH,
        m_pointCloudImage), "Transform depth image to point clouds failed!");

    int width = k4a_image_get_width_pixels(m_pointCloudImage);
    int height = k4a_image_get_height_pixels(m_pointCloudImage);

    int16_t* pointCloudImageBuffer = (int16_t*)k4a_image_get_buffer(m_pointCloudImage);

    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            int pixelIndex = h * width + w;
            k4a_float3_t position = {
                static_cast<float>(pointCloudImageBuffer[3 * pixelIndex + 0]),
                static_cast<float>(pointCloudImageBuffer[3 * pixelIndex + 1]),
                static_cast<float>(pointCloudImageBuffer[3 * pixelIndex + 2]) };

            // When the point cloud is invalid, the z-depth value is 0.
            if (position.v[2] == 0)
            {
                continue;
            }

            linmath::vec4 color = { 0.8f, 0.8f, 0.8f, 0.6f };
            linmath::ivec2 pixelLocation = { w, h };

            if (pointCloudColors.size() > 0)
            {
                BlendBodyColor(color, pointCloudColors[pixelIndex]);
            }

            linmath::vec3 positionInMeter;
            ConvertMillimeterToMeter(position, positionInMeter);
            Visualization::PointCloudVertex pointCloud;
            linmath::vec3_copy(pointCloud.Position, positionInMeter);
            linmath::vec4_copy(pointCloud.Color, color);
            pointCloud.PixelLocation[0] = pixelLocation[0];
            pointCloud.PixelLocation[1] = pixelLocation[1];

            m_pointClouds.push_back(pointCloud);
        }
    }

    UpdateDepthBuffer(depthImage);
}

void Window3dWrapper::CleanJointsAndBones()
{
    m_window3d.CleanJointsAndBones();
}

void Window3dWrapper::AddJoint(k4a_float3_t position, k4a_quaternion_t orientation, Color color)
{
    linmath::vec3 jointPositionInMeter;
    ConvertMillimeterToMeter(position, jointPositionInMeter);
    m_window3d.AddJoint({
        {jointPositionInMeter[0], jointPositionInMeter[1], jointPositionInMeter[2]},
        {orientation.v[0], orientation.v[1], orientation.v[2], orientation.v[3]},
        {color.r, color.g, color.b, color.a} });
}

void Window3dWrapper::AddBone(k4a_float3_t joint1Position, k4a_float3_t joint2Position, Color color)
{
    linmath::vec3 joint1PositionInMeter;
    ConvertMillimeterToMeter(joint1Position, joint1PositionInMeter);
    linmath::vec3 joint2PositionInMeter;
    ConvertMillimeterToMeter(joint2Position, joint2PositionInMeter);
    Visualization::Bone bone;
    linmath::vec4_copy(bone.Joint1Position, joint1PositionInMeter);
    linmath::vec4_copy(bone.Joint2Position, joint2PositionInMeter);
    bone.Color[0] = color.r;
    bone.Color[1] = color.g;
    bone.Color[2] = color.b;
    bone.Color[3] = color.a;

    m_window3d.AddBone(bone);
}

void Window3dWrapper::AddBody(const k4abt_body_t& body, Color color)
{
    Color lowConfidenceColor = color;
    lowConfidenceColor.a = color.a / 4;

    for (int joint = 0; joint < static_cast<int>(K4ABT_JOINT_COUNT); joint++)
    {
        if (body.skeleton.joints[joint].confidence_level >= K4ABT_JOINT_CONFIDENCE_LOW)
        {
            const k4a_float3_t& jointPosition = body.skeleton.joints[joint].position;
            const k4a_quaternion_t& jointOrientation = body.skeleton.joints[joint].orientation;

            AddJoint(
                jointPosition,
                jointOrientation,
                body.skeleton.joints[joint].confidence_level >= K4ABT_JOINT_CONFIDENCE_MEDIUM ? color : lowConfidenceColor);
        }
    }

    for (size_t boneIdx = 0; boneIdx < g_boneList.size(); boneIdx++)
    {
        k4abt_joint_id_t joint1 = g_boneList[boneIdx].first;
        k4abt_joint_id_t joint2 = g_boneList[boneIdx].second;

        if (body.skeleton.joints[joint1].confidence_level >= K4ABT_JOINT_CONFIDENCE_LOW &&
            body.skeleton.joints[joint2].confidence_level >= K4ABT_JOINT_CONFIDENCE_LOW)
        {
            bool confidentBone = body.skeleton.joints[joint1].confidence_level >= K4ABT_JOINT_CONFIDENCE_MEDIUM &&
                body.skeleton.joints[joint2].confidence_level >= K4ABT_JOINT_CONFIDENCE_MEDIUM;
            const k4a_float3_t& joint1Position = body.skeleton.joints[joint1].position;
            const k4a_float3_t& joint2Position = body.skeleton.joints[joint2].position;

            AddBone(joint1Position, joint2Position, confidentBone ? color : lowConfidenceColor);
        }
    }
}

void Window3dWrapper::Render()
{
    if (m_pointCloudUpdated || m_pointClouds.size() != 0)
    {
        m_window3d.UpdatePointClouds(m_pointClouds.data(), (uint32_t)m_pointClouds.size(), m_depthBuffer.data(), m_depthWidth, m_depthHeight);
        m_pointClouds.clear();
        m_pointCloudUpdated = false;
    }

    m_window3d.Render();
}

void Window3dWrapper::SetWindowPosition(int xPos, int yPos)
{
    m_window3d.SetWindowPosition(xPos, yPos);
}


void Window3dWrapper::SetLayout3d(Visualization::Layout3d layout3d)
{
    m_window3d.SetLayout3d(layout3d);
}

void Window3dWrapper::SetJointFrameVisualization(bool enableJointFrameVisualization)
{
    Visualization::SkeletonRenderMode skeletonRenderMode = enableJointFrameVisualization ?
        Visualization::SkeletonRenderMode::SkeletonOverlayWithJointFrame : Visualization::SkeletonRenderMode::SkeletonOverlay;

    m_window3d.SetSkeletonRenderMode(skeletonRenderMode);
}

void Window3dWrapper::SetFloorRendering(bool enableFloorRendering, float floorPositionX, float floorPositionY, float floorPositionZ)
{
    linmath::vec3 position = { floorPositionX, floorPositionY, floorPositionZ };
    m_window3d.SetFloorRendering(enableFloorRendering, position, {1.f, 0.f, 0.f, 0.f});
}

void Window3dWrapper::SetFloorRendering(bool enableFloorRendering, float floorPositionX, float floorPositionY, float floorPositionZ, float normalX, float normalY, float normalZ)
{
    linmath::vec3 position = { floorPositionX, floorPositionY, floorPositionZ };
    linmath::vec3 n = { normalX , normalY , normalZ };
    linmath::vec3_norm(n,n);
    linmath::vec3 up = { 0, -1, 0 };

    linmath::vec3 ax;
    linmath::vec3_mul_cross(ax, up, n);
    linmath::vec3_norm(ax, ax);

    float ang = acos(linmath::vec3_mul_inner(up, n));
    float hs = sin(ang / 2);
    linmath::quaternion q = { cos(ang / 2), hs * ax[0], hs * ax[1], hs * ax[2] };
    m_window3d.SetFloorRendering(enableFloorRendering, position, q);
}

void Window3dWrapper::InitializeCalibration(const k4a_calibration_t& sensorCalibration)
{

    m_depthWidth = static_cast<uint32_t>(sensorCalibration.depth_camera_calibration.resolution_width);
    m_depthHeight = static_cast<uint32_t>(sensorCalibration.depth_camera_calibration.resolution_height);

    // Cache the 2D to 3D unprojection table
    EXIT_IF(!CreateXYDepthTable(sensorCalibration), "Create XY Depth Table failed!");
    m_window3d.InitializePointCloudRenderer(
        true,   // Enable point cloud shading for better visualization effect
        reinterpret_cast<float*>(m_xyDepthTable.data()),
        m_depthWidth,
        m_depthHeight);

    // Create transformation handle
    if (m_transformationHandle == nullptr)
    {
        m_transformationHandle = k4a_transformation_create(&sensorCalibration);

        if (m_pointCloudImage == nullptr)
        {
            VERIFY(k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
                m_depthWidth,
                m_depthHeight,
                m_depthWidth * 3 * (int)sizeof(int16_t),
                &m_pointCloudImage), "Create Point Cloud Image failed!");
        }
    }
}

void Window3dWrapper::BlendBodyColor(linmath::vec4 color, Color bodyColor)
{
    float darkenRatio = 0.8f;
    float instanceAlpha = 0.8f;

    color[0] = bodyColor.r * instanceAlpha + color[0] * darkenRatio;
    color[1] = bodyColor.g * instanceAlpha + color[1] * darkenRatio;
    color[2] = bodyColor.b * instanceAlpha + color[2] * darkenRatio;
}

void Window3dWrapper::UpdateDepthBuffer(k4a_image_t depthFrame)
{
    int width = k4a_image_get_width_pixels(depthFrame);
    int height = k4a_image_get_height_pixels(depthFrame);
    uint16_t* depthFrameBuffer = (uint16_t*)k4a_image_get_buffer(depthFrame);
    m_depthBuffer.assign(depthFrameBuffer, depthFrameBuffer + width * height);
}

bool Window3dWrapper::CreateXYDepthTable(const k4a_calibration_t & sensorCalibration)
{
    int width = sensorCalibration.depth_camera_calibration.resolution_width;
    int height = sensorCalibration.depth_camera_calibration.resolution_height;

    m_xyDepthTable.resize(width * height);

    auto xyTablePtr = m_xyDepthTable.begin();

    k4a_float3_t pt3;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            k4a_float2_t pt = { static_cast<float>(w), static_cast<float>(h) };
            int valid = 0;
            k4a_result_t result = k4a_calibration_2d_to_3d(&sensorCalibration,
                &pt,
                1.f,
                K4A_CALIBRATION_TYPE_DEPTH,
                K4A_CALIBRATION_TYPE_DEPTH,
                &pt3,
                &valid);
            if (result != K4A_RESULT_SUCCEEDED)
            {
                return false;
            }

            if (valid == 0)
            {
                // Set the invalid xy table to be (0, 0)
                xyTablePtr->x = 0.f;
                xyTablePtr->y = 0.f;
            }
            else
            {
                xyTablePtr->x = pt3.xyz.x;
                xyTablePtr->y = pt3.xyz.y;
            }

            ++xyTablePtr;
        }
    }

    return true;
}

