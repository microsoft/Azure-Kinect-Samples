// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "linmath.h"

namespace Visualization
{
    struct PointCloudVertex
    {
        linmath::vec3 Position;         // The position of the point cloud vertex specified in meters
        linmath::vec4 Color;
        linmath::ivec2 PixelLocation;   // Pixel location of point cloud in the depth map (w, h)
    };

    struct MonoVertex
    {
        linmath::vec3 Position;         // The position of the mono vertex specified in meters
        linmath::vec3 Normal;
    };

    struct ColorVertex
    {
        linmath::vec3 Position;         // The position of the color vertex specified in meters
        linmath::vec3 Normal;
        linmath::vec4 Color;
    };

    struct Joint
    {
        linmath::vec3 Position;          // The position of the joint specified in meters
        linmath::quaternion Orientation; // The orientation of the joint specified in normalized quaternion
        linmath::vec4 Color;
    };

    struct Bone
    {
        linmath::vec3 Joint1Position;
        linmath::vec3 Joint2Position;
        linmath::vec4 Color;
    };
}
