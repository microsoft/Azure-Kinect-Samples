// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <unordered_map>
#include <k4abttypes.h>

// Define the bone list based on the documentation
const std::array<std::pair<k4abt_joint_id_t, k4abt_joint_id_t>, 31> g_boneList =
{
    std::make_pair(K4ABT_JOINT_SPINE_CHEST, K4ABT_JOINT_SPINE_NAVEL),
    std::make_pair(K4ABT_JOINT_SPINE_NAVEL, K4ABT_JOINT_PELVIS),
    std::make_pair(K4ABT_JOINT_SPINE_CHEST, K4ABT_JOINT_NECK),
    std::make_pair(K4ABT_JOINT_NECK, K4ABT_JOINT_HEAD),
    std::make_pair(K4ABT_JOINT_HEAD, K4ABT_JOINT_NOSE),

    std::make_pair(K4ABT_JOINT_SPINE_CHEST, K4ABT_JOINT_CLAVICLE_LEFT),
    std::make_pair(K4ABT_JOINT_CLAVICLE_LEFT, K4ABT_JOINT_SHOULDER_LEFT),
    std::make_pair(K4ABT_JOINT_SHOULDER_LEFT, K4ABT_JOINT_ELBOW_LEFT),
    std::make_pair(K4ABT_JOINT_ELBOW_LEFT, K4ABT_JOINT_WRIST_LEFT),
    std::make_pair(K4ABT_JOINT_WRIST_LEFT, K4ABT_JOINT_HAND_LEFT),
    std::make_pair(K4ABT_JOINT_HAND_LEFT, K4ABT_JOINT_HANDTIP_LEFT),
    std::make_pair(K4ABT_JOINT_WRIST_LEFT, K4ABT_JOINT_THUMB_LEFT),
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
    std::make_pair(K4ABT_JOINT_WRIST_RIGHT, K4ABT_JOINT_HAND_RIGHT),
    std::make_pair(K4ABT_JOINT_HAND_RIGHT, K4ABT_JOINT_HANDTIP_RIGHT),
    std::make_pair(K4ABT_JOINT_WRIST_RIGHT, K4ABT_JOINT_THUMB_RIGHT),
    std::make_pair(K4ABT_JOINT_PELVIS, K4ABT_JOINT_HIP_RIGHT),
    std::make_pair(K4ABT_JOINT_HIP_RIGHT, K4ABT_JOINT_KNEE_RIGHT),
    std::make_pair(K4ABT_JOINT_KNEE_RIGHT, K4ABT_JOINT_ANKLE_RIGHT),
    std::make_pair(K4ABT_JOINT_ANKLE_RIGHT, K4ABT_JOINT_FOOT_RIGHT),
    std::make_pair(K4ABT_JOINT_NOSE, K4ABT_JOINT_EYE_RIGHT),
    std::make_pair(K4ABT_JOINT_EYE_RIGHT, K4ABT_JOINT_EAR_RIGHT)
};

// Define the joint string names
const std::unordered_map<k4abt_joint_id_t, std::string> g_jointNames =
{
    std::make_pair(K4ABT_JOINT_PELVIS,        "PELVIS"),
    std::make_pair(K4ABT_JOINT_SPINE_NAVEL,   "SPINE_NAVEL"),
    std::make_pair(K4ABT_JOINT_SPINE_CHEST,   "SPINE_CHEST"),
    std::make_pair(K4ABT_JOINT_NECK,          "NECK"),
    std::make_pair(K4ABT_JOINT_CLAVICLE_LEFT, "CLAVICLE_LEFT"),
    std::make_pair(K4ABT_JOINT_SHOULDER_LEFT, "SHOULDER_LEFT"),
    std::make_pair(K4ABT_JOINT_ELBOW_LEFT,    "ELBOW_LEFT"),
    std::make_pair(K4ABT_JOINT_WRIST_LEFT,    "WRIST_LEFT"),
    std::make_pair(K4ABT_JOINT_HAND_LEFT,     "HAND_LEFT"),
    std::make_pair(K4ABT_JOINT_HANDTIP_LEFT,  "HANDTIP_LEFT"),
    std::make_pair(K4ABT_JOINT_THUMB_LEFT,    "THUMB_LEFT"),
    std::make_pair(K4ABT_JOINT_CLAVICLE_RIGHT,"CLAVICLE_RIGHT"),
    std::make_pair(K4ABT_JOINT_SHOULDER_RIGHT,"SHOULDER_RIGHT"),
    std::make_pair(K4ABT_JOINT_ELBOW_RIGHT,   "ELBOW_RIGHT"),
    std::make_pair(K4ABT_JOINT_WRIST_RIGHT,   "WRIST_RIGHT"),
    std::make_pair(K4ABT_JOINT_HAND_RIGHT,    "HAND_RIGHT"),
    std::make_pair(K4ABT_JOINT_HANDTIP_RIGHT, "HANDTIP_RIGHT"),
    std::make_pair(K4ABT_JOINT_THUMB_RIGHT,   "THUMB_RIGHT"),
    std::make_pair(K4ABT_JOINT_HIP_LEFT,      "HIP_LEFT"),
    std::make_pair(K4ABT_JOINT_KNEE_LEFT,     "KNEE_LEFT"),
    std::make_pair(K4ABT_JOINT_ANKLE_LEFT,    "ANKLE_LEFT"),
    std::make_pair(K4ABT_JOINT_FOOT_LEFT,     "FOOT_LEFT"),
    std::make_pair(K4ABT_JOINT_HIP_RIGHT,     "HIP_RIGHT"),
    std::make_pair(K4ABT_JOINT_KNEE_RIGHT,    "KNEE_RIGHT"),
    std::make_pair(K4ABT_JOINT_ANKLE_RIGHT,   "ANKLE_RIGHT"),
    std::make_pair(K4ABT_JOINT_FOOT_RIGHT,    "FOOT_RIGHT"),
    std::make_pair(K4ABT_JOINT_HEAD,          "HEAD"),
    std::make_pair(K4ABT_JOINT_NOSE,          "NOSE"),
    std::make_pair(K4ABT_JOINT_EYE_LEFT,      "EYE_LEFT"),
    std::make_pair(K4ABT_JOINT_EAR_LEFT,      "EAR_LEFT"),
    std::make_pair(K4ABT_JOINT_EYE_RIGHT,     "EYE_RIGHT"),
    std::make_pair(K4ABT_JOINT_EAR_RIGHT,     "EAR_RIGHT")
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
