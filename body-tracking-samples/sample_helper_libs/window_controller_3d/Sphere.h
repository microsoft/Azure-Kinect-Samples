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
    class Sphere : public RendererBase
    {
    public:
        Sphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18);

        void SetRadius(float radius);

        const MonoVertex* GetVertices() const { return m_vertices.data(); }
        size_t GetVerticesNum() { return m_vertices.size(); }

        const uint32_t* GetIndices() const { return m_indices.data(); }
        size_t GetIndicesNum() { return m_indices.size(); }

        // Renderer functions
        void Create(GLFWwindow* window) override;
        void Delete() override;

        void Render() override;
        void Render(const linmath::mat4x4 model, const linmath::vec4 color);
        void Render(const linmath::vec3 p, const linmath::vec4 color);

    private:
        void BuildVertices();

        void UpdateVAO();

        void AddIndices(uint32_t i1, uint32_t i2, uint32_t i3);

        // Settings
        float m_radius;
        int m_sectorCount;                        // longitude, # of slices
        int m_stackCount;                         // latitude, # of stacks

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