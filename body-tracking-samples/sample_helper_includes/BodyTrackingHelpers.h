// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <k4abttypes.h>

// Define the bone list based on the documentation
const std::array<std::pair<k4abt_joint_id_t, k4abt_joint_id_t>, 25> g_boneList =
{
    std::make_pair(K4ABT_JOINT_SPINE_CHEST, K4ABT_JOINT_SPINE_NAVAL),
    std::make_pair(K4ABT_JOINT_SPINE_NAVAL, K4ABT_JOINT_PELVIS),
    std::make_pair(K4ABT_JOINT_SPINE_CHEST, K4ABT_JOINT_NECK),
    std::make_pair(K4ABT_JOINT_NECK, K4ABT_JOINT_HEAD),
    std::make_pair(K4ABT_JOINT_HEAD, K4ABT_JOINT_NOSE),

    std::make_pair(K4ABT_JOINT_SPINE_CHEST, K4ABT_JOINT_CLAVICLE_LEFT),
    std::make_pair(K4ABT_JOINT_CLAVICLE_LEFT, K4ABT_JOINT_SHOULDER_LEFT),
    std::make_pair(K4ABT_JOINT_SHOULDER_LEFT, K4ABT_JOINT_ELBOW_LEFT),
    std::make_pair(K4ABT_JOINT_ELBOW_LEFT, K4ABT_JOINT_WRIST_LEFT),
    std::make_pair(K4ABT_JOINT_PELVIS, K4ABT_JOINT_HIP_LEFT),
    std::make_pair(K4ABT_JOINT_HIP_LEFT, K4ABT_JOINT_KNEE_LEFT),
    std::make_pair(K4ABT_JOINT_KNEE_LEFT, K4ABT_JOINT_ANKLE_LEFT),
    std::make_pair(K4ABT_JOINT_ANKLE_LEFT, K4ABT_JOINT_FOOT_LEFT),
    std::make_pair(K4ABT_JOINT_NOSE, K4ABT_JOINT_EYE_LEFT),
    std::make_pair(K4ABT_JOINT_EYE_LEFT, K4ABT_JOINT_EAR_LEFT),

    std::make_pair(K4ABT_JOINT_SPINE_CHEST, K4ABT_JOINT_CLAVICLE_RIGHT),
    std::make_pair(K4ABT_JOINT_CLAVICLE_RIGHT, K4ABT_JOINT_SHOULDER_RIGHT),
    std::make_pair(K4ABT_JOINT_SHOULDER_RIGHT, K4ABT_JOINT_ELBOW_RIGHT),
    std::make_pair(K4ABT_JOINT_ELBOW_RIGHT, K4ABT_JOINT_WRIST_RIGHT),
    std::make_pair(K4ABT_JOINT_PELVIS, K4ABT_JOINT_HIP_RIGHT),
    std::make_pair(K4ABT_JOINT_HIP_RIGHT, K4ABT_JOINT_KNEE_RIGHT),
    std::make_pair(K4ABT_JOINT_KNEE_RIGHT, K4ABT_JOINT_ANKLE_RIGHT),
    std::make_pair(K4ABT_JOINT_ANKLE_RIGHT, K4ABT_JOINT_FOOT_RIGHT),
    std::make_pair(K4ABT_JOINT_NOSE, K4ABT_JOINT_EYE_RIGHT),
    std::make_pair(K4ABT_JOINT_EYE_RIGHT, K4ABT_JOINT_EAR_RIGHT)
};

struct Color
{
    float r = 1.f;
    float g = 1.f;
    float b = 1.f;
    float a = 1.f;
};

const std::array<Color, 20> g_bodyColors =
{
    0.00f, 1.00f, 1.00f, 1.00f,
    1.00f, 0.65f, 0.00f, 1.00f,
    0.00f, 0.50f, 0.50f, 1.00f,
    0.85f, 0.44f, 0.84f, 1.00f,
    0.00f, 1.00f, 0.50f, 1.00f,
    0.53f, 0.81f, 0.98f, 1.00f,
    1.00f, 0.39f, 0.28f, 1.00f,
    0.13f, 0.70f, 0.67f, 1.00f,
    0.87f, 0.63f, 0.87f, 1.00f,
    0.00f, 0.98f, 0.60f, 1.00f,
    0.00f, 1.00f, 1.00f, 1.00f,
    1.00f, 0.65f, 0.00f, 1.00f,
    0.00f, 0.50f, 0.50f, 1.00f,
    0.85f, 0.44f, 0.84f, 1.00f,
    0.00f, 1.00f, 0.50f, 1.00f,
    0.53f, 0.81f, 0.98f, 1.00f,
    1.00f, 0.39f, 0.28f, 1.00f,
    0.13f, 0.70f, 0.67f, 1.00f,
    0.87f, 0.63f, 0.87f, 1.00f,
    0.00f, 0.98f, 0.60f, 1.00f
};
