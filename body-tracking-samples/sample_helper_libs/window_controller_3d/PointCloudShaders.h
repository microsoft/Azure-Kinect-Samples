// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "GlShaderDefs.h"

// ************** Point Cloud Vertex Shader **************
static const char* const glslPointCloudVertexShader = GLSL_STRING(

    layout(location = 0) in vec3 vertexPosition;
    layout(location = 1) in vec4 vertexColor;
    layout(location = 2) in ivec2 pixelLocation;

    out vec4 fragmentColor;

    uniform mat4 view;
    uniform mat4 projection;
    uniform bool enableShading;

    layout(rg32f, binding = 0) restrict readonly uniform image2D xyTable;
    layout(r16ui, binding = 1) restrict readonly uniform uimage2D depth;

    vec3 ComputePoint3d(ivec2 pixelId)
    {
        float depthInMeter = float(imageLoad(depth, pixelId).x) /  1000.f;

        // Calculate depth 3d point
        vec3 point3d = vec3(imageLoad(xyTable, pixelId).xy, 1) * depthInMeter;

        // If both x is 0 and y is 0, it means:
        // Either the xyTable at pixelId is invalid, or depthInMeter == 0.
        // Both cases mean the point3d result is invalid
        if (point3d.x == 0 && point3d.y == 0)
        {
            return vec3(0, 0, 0);
        }

        // Y, Z direction for OpenGL camera coordinate is opposite to the Kinect camera coordinate
        return vec3(point3d.x, -point3d.y, -point3d.z);
    }

    vec3 ComputeNormal(ivec2 pixelId)
    {
        vec3 pointLeft = ComputePoint3d(ivec2(pixelId.x - 1, pixelId.y));
        vec3 pointRight = ComputePoint3d(ivec2(pixelId.x + 1, pixelId.y));
        vec3 pointUp = ComputePoint3d(ivec2(pixelId.x, pixelId.y - 1));
        vec3 pointDown = ComputePoint3d(ivec2(pixelId.x, pixelId.y + 1));

        pointLeft = pointLeft.z == 0 ? vertexPosition : pointLeft;
        pointRight = pointRight.z == 0 ? vertexPosition : pointRight;
        pointUp = pointUp.z == 0 ? vertexPosition : pointUp;
        pointDown = pointDown.z == 0 ? vertexPosition : pointDown;

        vec3 xDirection = pointRight - pointLeft;
        vec3 yDirection = pointUp - pointDown;

        vec3 normal = cross(xDirection, yDirection);

        return normal;
    }

    void main()
    {
        gl_Position = projection * view * vec4(vertexPosition, 1);

        if (enableShading)
        {
            const vec3 lightPosition = vec3(0, 0, 0);
            vec3 vertexNormal = ComputeNormal(pixelLocation);
            float diffuse = 0.f;
            if (dot(vertexNormal, vertexNormal) != 0.f)
            {
                vec3 lightDirection = normalize(lightPosition - vertexPosition);
                // Use mix function to reduce the strength of the diffuse effect
                float defuseRatio = 0.7f;
                diffuse = mix(1.0f, abs(dot(normalize(vertexNormal), lightDirection)), defuseRatio);
            }

            float distance = length(lightPosition - vertexPosition);
            // Attenuation term for light source that covers distance up to 50 meters
            // http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

            fragmentColor = vec4(attenuation * diffuse * vertexColor.rgb, vertexColor.a);
        }
        else
        {
            fragmentColor = vertexColor;
        }
    }

);  // GLSL_STRING


// ************** Point Cloud Fragment Shader **************
static const char* const glslPointCloudFragmentShader = GLSL_STRING(

    out vec4 fragColor;
    in vec4 fragmentColor;

    void main()
    {
        fragColor = fragmentColor;
    }

);  // GLSL_STRING
