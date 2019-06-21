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
    class CoordinateAxes : public RendererBase
    {
    public:
        CoordinateAxes(float axisThickness = 1.0f, float axisLength = 1.0f);

        void SetAxisThickness(float axisThickness);
        void SetAxisLength(float axisLength);

        const ColorVertex* GetVertices() const { return m_vertices.data(); }
        size_t GetVerticesNum() { return m_vertices.size(); }

        const uint32_t* GetIndices() const { return m_indices.data(); }
        size_t GetIndicesNum() { return m_indices.size(); }

        // Renderer functions
        void Create(GLFWwindow* window) override;
        void Delete() override;

        void Render() override;
        void Render(const linmath::mat4x4 model);
        void Render(const linmath::vec3 p, const linmath::quaternion q);

    private:
        void BuildVertices();

        void UpdateVAO();

        // Settings
        float m_axisThickness;
        float m_axisLength;

        // Data buffers
        std::vector<ColorVertex> m_vertices;
        std::vector<uint32_t> m_indices;

        // OpenGL objects
        GLuint m_vertexArrayObject;
        GLuint m_vertexBufferObject;
        GLuint m_elementBufferObject;

        GLuint m_modelIndex;
        GLuint m_viewIndex;
        GLuint m_projectionIndex;
    };
}