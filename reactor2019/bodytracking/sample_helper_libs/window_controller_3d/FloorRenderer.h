// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "RendererBase.h"
#include "WindowController3dTypes.h"

namespace Visualization
{
    class FloorRenderer : public RendererBase
    {
    public:
        FloorRenderer(float length = 3.f, float width = 3.f);
        void SetFloorPlacement(const linmath::vec3 position, const linmath::quaternion orientation);

        // Renderer functions
        void Create(GLFWwindow* window) override;
        void Delete() override;

        void Render() override;

    private:
        void BuildVertices();

        void UpdateVAO();

        void AddIndices(uint32_t i1, uint32_t i2, uint32_t i3);

        linmath::mat4x4 m_model;

        // Settings
        float m_length = 0.f;
        float m_width = 0.f;

        // Data buffers
        std::vector<MonoVertex> m_vertices;
        std::vector<uint32_t> m_indices;

        // OpenGL objects
        GLuint m_vertexArrayObject;
        GLuint m_vertexBufferObject;
        GLuint m_elementBufferObject;

        GLuint m_modelIndex;
        GLuint m_viewIndex;
        GLuint m_projectionIndex;

        GLuint m_colorIndex;
    };
}
