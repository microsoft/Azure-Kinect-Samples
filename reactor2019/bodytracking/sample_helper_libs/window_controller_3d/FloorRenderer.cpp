// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "FloorRenderer.h"

#include <cmath>

#include "Helpers.h"

// Shader Header
#include "MonoObjectShaders.h"

using namespace linmath;
using namespace Visualization;

FloorRenderer::FloorRenderer(float length, float width)
    : m_length(length)
    , m_width(width)
{
    BuildVertices();

    mat4x4_identity(m_model);
    mat4x4_identity(m_view);
    mat4x4_identity(m_projection);
}

void FloorRenderer::SetFloorPlacement(const linmath::vec3 position, const linmath::quaternion orientation)
{
    mat4x4 translation, rotation;
    mat4x4_translate(translation, position[0], position[1], position[2]);
    quaternion_to_mat4x4(rotation, orientation);
    mat4x4_mul(m_model, translation, rotation);
}

void FloorRenderer::Create(GLFWwindow * window)
{
    CheckAssert(!m_initialized);
    m_initialized = true;

    m_window = window;
    glfwMakeContextCurrent(window);

    // Context Settings
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertexShaderSources[] = { glslShaderVersion, glslMonoObjectVertexShader };
    int numVertexShaderSources = sizeof(vertexShaderSources) / sizeof(*vertexShaderSources);
    glShaderSource(m_vertexShader, numVertexShaderSources, vertexShaderSources, NULL);
    glCompileShader(m_vertexShader);
    ValidateShader(m_vertexShader);

    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragmentShaderSources[] = { glslShaderVersion, glslMonoObjectFragmentShader };
    int numFragmentShaderSources = sizeof(fragmentShaderSources) / sizeof(*fragmentShaderSources);
    glShaderSource(m_fragmentShader, numFragmentShaderSources, fragmentShaderSources, NULL);
    glCompileShader(m_fragmentShader);
    ValidateShader(m_fragmentShader);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);
    ValidateProgram(m_shaderProgram);

    // Get shader index
    m_modelIndex = glGetUniformLocation(m_shaderProgram, "model");
    m_viewIndex = glGetUniformLocation(m_shaderProgram, "view");
    m_projectionIndex = glGetUniformLocation(m_shaderProgram, "projection");
    m_colorIndex = glGetUniformLocation(m_shaderProgram, "color");

    // **************** Generate FloorRenderer VAO ****************
    glGenVertexArrays(1, &m_vertexArrayObject);
    glBindVertexArray(m_vertexArrayObject);

    // Create buffers and bind the geometry
    glGenBuffers(1, &m_vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(MonoVertex), m_vertices.data(), GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MonoVertex), (void*)0);

    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MonoVertex), (void*)offsetof(MonoVertex, Normal));

    // Create buffers and bind the indices
    glGenBuffers(1, &m_elementBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);

    // **************** Unbind VAO ****************
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void FloorRenderer::Delete()
{
    if (!m_initialized)
    {
        return;
    }

    m_initialized = false;
    glDeleteBuffers(1, &m_vertexBufferObject);
    glDeleteBuffers(1, &m_elementBufferObject);

    glDeleteShader(m_vertexShader);
    glDeleteShader(m_fragmentShader);
    glDeleteProgram(m_shaderProgram);
}

void FloorRenderer::Render()
{
    vec4 color;
    vec4_set(color, 1.f, 1.f, 1.f, 1.f);

    glUseProgram(m_shaderProgram);

    // Update model/view/projective matrices in shader
    glUniformMatrix4fv(m_viewIndex, 1, GL_FALSE, (const GLfloat*)m_view);
    glUniformMatrix4fv(m_projectionIndex, 1, GL_FALSE, (const GLfloat*)m_projection);
    glUniformMatrix4fv(m_modelIndex, 1, GL_FALSE, (const GLfloat*)m_model);
    glUniform4f(m_colorIndex, color[0], color[1], color[2], color[3]);

    glBindVertexArray(m_vertexArrayObject); // Bind FloorRenderer VAO
    glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, NULL);
}

void FloorRenderer::BuildVertices()
{
    // clear memory of prev arrays
    m_vertices.clear();
    m_indices.clear();

    m_vertices.push_back({ {-m_length / 2, 0, -m_width / 2}, {0, 1.f, 0} });
    m_vertices.push_back({ {-m_length / 2, 0, m_width / 2}, {0, 1.f, 0} });
    m_vertices.push_back({ {m_length / 2, 0, m_width / 2}, {0, 1.f, 0} });
    m_vertices.push_back({ {m_length / 2, 0, -m_width / 2}, {0, 1.f, 0} });

    AddIndices(0, 1, 2);
    AddIndices(2, 3, 0);
}

void FloorRenderer::UpdateVAO()
{
    glBindVertexArray(m_vertexArrayObject);
    // Create buffers and bind the geometry
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(MonoVertex), m_vertices.data(), GL_STATIC_DRAW);

    // Create buffers and bind the indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);
}

void FloorRenderer::AddIndices(uint32_t i1, uint32_t i2, uint32_t i3)
{
    m_indices.push_back(i1);
    m_indices.push_back(i2);
    m_indices.push_back(i3);
}