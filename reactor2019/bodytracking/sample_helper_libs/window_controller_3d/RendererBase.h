// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "GLFW/glfw3.h"
#include "linmath.h"

namespace Visualization
{
    class RendererBase
    {
    public:
        virtual ~RendererBase() = default;
        virtual void Create(GLFWwindow* window) = 0;
        virtual void Delete() = 0;

        virtual void UpdateViewProjection(
            linmath::mat4x4 view,
            linmath::mat4x4 projection);

        virtual void Render() = 0;

    protected:
        bool m_initialized = false;

        linmath::mat4x4 m_view;
        linmath::mat4x4 m_projection;

        // Basic OpenGL resources
        GLFWwindow* m_window;
        GLuint m_shaderProgram;
        GLuint m_vertexShader;
        GLuint m_fragmentShader;
    };
}
