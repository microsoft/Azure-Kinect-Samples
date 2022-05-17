// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "GlShaderDefs.h"

// ************** Mono Object Vertex Shader **************
static const char* const glslMonoObjectVertexShader = GLSL_STRING(

    layout(location = 0) in vec3 vertexPosition;
    layout(location = 1) in vec3 vertexNormal;

    out vec4 fragmentColor;
    out vec3 fragmentPosition;
    out vec3 fragmentNormal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    uniform vec4 color;

    void main()
    {
        fragmentColor = color;
        fragmentPosition = vec3(model * vec4(vertexPosition, 1.0));
        fragmentNormal = mat3(transpose(inverse(model))) * vertexNormal;

        gl_Position = projection * view * model * vec4(vertexPosition, 1);
    }

);  // GLSL_STRING


// ************** Mono Object Fragment Shader **************
static const char* const glslMonoObjectFragmentShader = GLSL_STRING(

    out vec4 fragColor;
    in vec4 fragmentColor;
    in vec3 fragmentPosition;
    in vec3 fragmentNormal;

    void main()
    {
        vec3 lightPosition = vec3(0, 0, 0);

        // diffuse
        vec3 norm = normalize(fragmentNormal);
        vec3 lightDir = normalize(lightPosition - fragmentPosition);
        float diffuse = abs(dot(norm, lightDir));

        fragColor = vec4(fragmentColor.rgb * diffuse, fragmentColor.a);
    }

);  // GLSL_STRING
