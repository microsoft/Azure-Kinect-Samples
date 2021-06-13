// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "CoordinateAxes.h"

#include <cmath>

#include "Cylinder.h"
#include "Helpers.h"

// Shader Header
#include "ColorObjectShaders.h"

using namespace linmath;
using namespace Visualization;

CoordinateAxes::CoordinateAxes(float axisThickness, float axisLength)
    : m_axisThickness(axisThickness)
    , m_axisLength(axisLength)
{
    BuildVertices();

    mat4x4_identity(m_view);
    mat4x4_identity(m_projection);
}

void CoordinateAxes::SetAxisThickness(float axisThickness)
{
    if (axisThickness <= 0.f)
    {
        return;
    }

    m_axisThickness = axisThickness;
    BuildVertices();

    if (m_initialized)
    {
        UpdateVAO();
    }
}

void CoordinateAxes::SetAxisLength(float axisLength)
{
    if (axisLength <= 0.f)
    {
        return;
    }

    m_axisLength = axisLength;
    BuildVertices();

    if (m_initialized)
    {
        UpdateVAO();
    }
}


void CoordinateAxes::Create(GLFWwindow * window)
{
    CheckAssert(!m_initialized);
    m_initialized = true;

    m_window = window;
    glfwMakeContextCurrent(window);

    // Context Settings
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertexShaderSources[] = { glslShaderVersion, glslColorObjectVertexShader };
    int numVertexShaderSources = sizeof(vertexShaderSources) / sizeof(*vertexShaderSources);
    glShaderSource(m_vertexShader, numVertexShaderSources, vertexShaderSources, NULL);
    glCompileShader(m_vertexShader);
    ValidateShader(m_vertexShader);

    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragmentShaderSources[] = { glslShaderVersion, glslColorObjectFragmentShader };
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

    // **************** Generate Sphere VAO ****************
    glGenVertexArrays(1, &m_vertexArrayObject);
    glBindVertexArray(m_vertexArrayObject);

    // Create buffers and bind the geometry
    glGenBuffers(1, &m_vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(ColorVertex), m_vertices.data(), GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)0);

    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)offsetof(ColorVertex, Normal));

    // Vertex Colors
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)offsetof(ColorVertex, Color));

    // Create buffers and bind the indices
    glGenBuffers(1, &m_elementBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);

    // **************** Unbind VAO ****************
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CoordinateAxes::Delete()
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

void CoordinateAxes::Render()
{
    mat4x4 model;
    mat4x4_identity(model);

    Render(model);
}

void CoordinateAxes::Render(const mat4x4 model)
{
    glUseProgram(m_shaderProgram);

    // Update model/view/projective matrices in shader
    glUniformMatrix4fv(m_viewIndex, 1, GL_FALSE, (const GLfloat*)m_view);
    glUniformMatrix4fv(m_projectionIndex, 1, GL_FALSE, (const GLfloat*)m_projection);
    glUniformMatrix4fv(m_modelIndex, 1, GL_FALSE, (const GLfloat*)model);

    glBindVertexArray(m_vertexArrayObject); // Bind Sphere VAO
    glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, NULL);
}

void CoordinateAxes::Render(const linmath::vec3 p, const linmath::quaternion q)
{
    mat4x4 model, translation, rotation;
    mat4x4_translate(translation, p[0], p[1], p[2]);
    quaternion_to_mat4x4(rotation, q);
    mat4x4_mul(model, translation, rotation);

    Render(model);
}

void CoordinateAxes::BuildVertices()
{
    // Cylinder is created along z axis and centered at origin.
    Cylinder axisZCylinder(m_axisThickness, m_axisLength);
    std::vector<MonoVertex> axisZCylinderVertices = axisZCylinder.GetVecticesVector();
    std::vector<uint32_t> axisZCylinderIndices = axisZCylinder.GetIndicesVector();

    // Swapping x with z can make the cylinder place along x.
    // Then add axisLength/2 to its x axis can make it starts at origin.
    for (size_t i = 0; i < axisZCylinderVertices.size(); i++)
    {
        ColorVertex newVertex;
        newVertex.Position[0] = axisZCylinderVertices[i].Position[2];
        newVertex.Position[1] = axisZCylinderVertices[i].Position[1];
        newVertex.Position[2] = axisZCylinderVertices[i].Position[0];
        newVertex.Normal[0] = axisZCylinderVertices[i].Normal[2];
        newVertex.Normal[1] = axisZCylinderVertices[i].Normal[1];
        newVertex.Normal[2] = axisZCylinderVertices[i].Normal[0];

        newVertex.Position[0] += m_axisLength / 2;

        vec4_set(newVertex.Color, 1.f, 0.f, 0.f, 1.f);

        m_vertices.push_back(newVertex);
    }
    for (size_t i = 0; i < axisZCylinderIndices.size(); i++)
    {
        m_indices.push_back(axisZCylinderIndices[i]);
    }

    // Similar operation for y axis
    uint32_t yAxisStartIndex = static_cast<uint32_t>(m_vertices.size());
    for (size_t i = 0; i < axisZCylinderVertices.size(); i++)
    {
        ColorVertex newVertex;
        newVertex.Position[0] = axisZCylinderVertices[i].Position[0];
        newVertex.Position[1] = axisZCylinderVertices[i].Position[2];
        newVertex.Position[2] = axisZCylinderVertices[i].Position[1];
        newVertex.Normal[0] = axisZCylinderVertices[i].Normal[0];
        newVertex.Normal[1] = axisZCylinderVertices[i].Normal[2];
        newVertex.Normal[2] = axisZCylinderVertices[i].Normal[1];

        newVertex.Position[1] += m_axisLength / 2;

        vec4_set(newVertex.Color, 0.f, 1.f, 0.f, 1.f);

        m_vertices.push_back(newVertex);
    }
    for (size_t i = 0; i < axisZCylinderIndices.size(); i++)
    {
        m_indices.push_back(axisZCylinderIndices[i] + yAxisStartIndex);
    }

    // For z axis, only the addition of axisLength/2 is needed to make it starts at origin
    uint32_t zAxisStartIndex = static_cast<uint32_t>(m_vertices.size());
    for (size_t i = 0; i < axisZCylinderVertices.size(); i++)
    {
        ColorVertex newVertex;
        vec3_set(newVertex.Position, axisZCylinderVertices[i].Position);
        vec3_set(newVertex.Normal, axisZCylinderVertices[i].Normal);

        newVertex.Position[2] += m_axisLength / 2;

        vec4_set(newVertex.Color, 0.f, 0.f, 1.f, 1.f);

        m_vertices.push_back(newVertex);
    }
    for (size_t i = 0; i < axisZCylinderIndices.size(); i++)
    {
        m_indices.push_back(axisZCylinderIndices[i] + zAxisStartIndex);
    }
}

void CoordinateAxes::UpdateVAO()
{
    glBindVertexArray(m_vertexArrayObject);
    // Create buffers and bind the geometry
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(ColorVertex), m_vertices.data(), GL_STATIC_DRAW);

    // Create buffers and bind the indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);
}
