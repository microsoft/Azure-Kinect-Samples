// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "Cylinder.h"

#include <cmath>

#include "Helpers.h"

// Shader Header
#include "MonoObjectShaders.h"

using namespace linmath;
using namespace Visualization;

static const int MIN_SECTOR_COUNT = 3;

Cylinder::Cylinder(float baseRadius, float height, int sectorCount)
    : m_baseRadius(baseRadius)
    , m_height(height)
    , m_sectorCount(sectorCount)
{
    if (m_sectorCount < MIN_SECTOR_COUNT)
    {
        m_sectorCount = MIN_SECTOR_COUNT;
    }

    BuildVertices();

    mat4x4_identity(m_view);
    mat4x4_identity(m_projection);
}

void Cylinder::SetBaseRadius(float baseRadius)
{
    if (baseRadius <= 0.f)
    {
        return;
    }

    float scale = baseRadius / m_baseRadius;

    for (size_t i = 0; i < m_vertices.size(); i++)
    {
        m_vertices[i].Position[0] *= scale;
        m_vertices[i].Position[1] *= scale;
    }

    m_baseRadius = baseRadius;

    if (m_initialized)
    {
        UpdateVAO();
    }
}

void Cylinder::SetHeight(float height)
{
    if (height <= 0.f)
    {
        return;
    }

    float scale = height / m_height;

    for (size_t i = 0; i < m_vertices.size(); i++)
    {
        m_vertices[i].Position[2] *= scale;
    }

    m_height = height;

    if (m_initialized)
    {
        UpdateVAO();
    }
}

void Cylinder::Create(GLFWwindow * window)
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

    // **************** Generate Sphere VAO ****************
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

void Cylinder::Delete()
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

void Cylinder::Render()
{
    mat4x4 model;
    mat4x4_identity(model);
    vec4 color;
    vec4_set(color, 1.f, 1.f, 1.f, 1.f);

    Render(model, color);
}

void Cylinder::Render(const mat4x4 model, const linmath::vec4 color)
{
    glUseProgram(m_shaderProgram);

    // Update model/view/projective matrices in shader
    glUniformMatrix4fv(m_viewIndex, 1, GL_FALSE, (const GLfloat*)m_view);
    glUniformMatrix4fv(m_projectionIndex, 1, GL_FALSE, (const GLfloat*)m_projection);
    glUniformMatrix4fv(m_modelIndex, 1, GL_FALSE, (const GLfloat*)model);
    glUniform4f(m_colorIndex, color[0], color[1], color[2], color[3]);

    glBindVertexArray(m_vertexArrayObject); // Bind Sphere VAO
    glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, NULL);
}

void Cylinder::Render(const linmath::vec3 start, const linmath::vec3 end, const linmath::vec4 color)
{
    vec3 centralAxis;
    vec3_sub(centralAxis, start, end);
    float length = vec3_len(centralAxis);

    vec3 centerPosition;
    vec3_add(centerPosition, start, end);
    vec3_scale(centerPosition, centerPosition, 0.5f);

    mat4x4 model, translation, rotation;
    mat4x4_translate(translation, centerPosition[0], centerPosition[1], centerPosition[2]);

    vec3 zAxis;
    vec3_set(zAxis, 0.f, 0.f, 1.f);

    ComputeRotationBetweenVectors(rotation, zAxis, centralAxis);
    mat4x4_mul(model, translation, rotation);

    SetHeight(length);
    Render(model, color);
}

void Cylinder::ComputeRotationBetweenVectors(mat4x4 rotation, const vec3 v0, const vec3 v1)
{
    vec3 u0;
    vec3_norm(u0, v0);

    vec3 u1;
    vec3_norm(u1, v1);

    vec3 v;
    vec3_mul_cross(v, u0, u1);

    float sinTheta = vec3_len(v);
    if (sinTheta < 0.00001f)
    {
        mat4x4_identity(rotation);
        return;
    }

    float cosTheta = vec3_mul_inner(u0, u1);
    float scale = 1.f / (1.f + cosTheta);

    mat4x4 vx =
    {
        {0    , v[2] , -v[1], 0  },
        {-v[2], 0    , v[0] , 0  },
        {v[1] , -v[0], 0    , 0  },
        {0    , 0    , 0    , 1.f}
    };

    mat4x4 vx2, vx2Scaled;
    mat4x4_mul(vx2, vx, vx);
    mat4x4_scale(vx2Scaled, vx2, scale);

    mat4x4_identity(rotation);
    mat4x4_add(rotation, rotation, vx);
    mat4x4_add(rotation, rotation, vx2Scaled);

    rotation[3][3] = 1.f;
}

// build vertices of Cylinder with smooth shading using parametric equation
// x = r * cos(v)
// y = r * sin(v)
// z = h/2 or -h/2
// where v: sector(longitude) angle (0 <= v <= 360)
//       h: height
void Cylinder::BuildVertices()
{
    const float PI = 3.1415926f;

    // clear memory of prev arrays
    m_vertices.clear();
    m_indices.clear();

    float radiusInv = 1.0f / m_baseRadius;    // normal

    float sectorStep = 2 * PI / m_sectorCount;
    float sectorAngle = 0;

    float x = 0.f, y = 0.f, z = 0.f;
    float nx = 0.f, ny = 0.f;

    for (int circleIndex = 0; circleIndex < 2; circleIndex++)
    {
        z = circleIndex == 0 ? m_height / 2 : -m_height / 2;

        for (int j = 0; j <= m_sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position
            x = m_baseRadius * std::cos(sectorAngle);             // r * cos(v)
            y = m_baseRadius * std::sin(sectorAngle);             // r * sin(v)

            // normalized vertex normal
            nx = x * radiusInv;
            ny = y * radiusInv;

            m_vertices.push_back({ {x, y, z}, {nx, ny, 0.f} });
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    //
    // Only side surface is needed for Skeletal Tracking
    uint32_t k1 = 0;                         // beginning of bottom circle
    uint32_t k2 = k1 + m_sectorCount + 1;    // beginning of top circle

    for (int j = 0; j < m_sectorCount; ++j, ++k1, ++k2)
    {
        AddIndices(k1, k2, k1 + 1);   // k1---k2---k1+1
        AddIndices(k1 + 1, k2, k2 + 1); // k1+1---k2---k2+1
    }
}

void Cylinder::UpdateVAO()
{
    glBindVertexArray(m_vertexArrayObject);
    // Create buffers and bind the geometry
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(MonoVertex), m_vertices.data(), GL_STATIC_DRAW);

    // Create buffers and bind the indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);
}

void Cylinder::AddIndices(uint32_t i1, uint32_t i2, uint32_t i3)
{
    m_indices.push_back(i1);
    m_indices.push_back(i2);
    m_indices.push_back(i3);
}
