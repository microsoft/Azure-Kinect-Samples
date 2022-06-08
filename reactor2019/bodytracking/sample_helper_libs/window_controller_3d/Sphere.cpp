// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "Sphere.h"

#include <cmath>

#include "Helpers.h"

// Shader Header
#include "MonoObjectShaders.h"

using namespace linmath;
using namespace Visualization;

static const int MIN_SECTOR_COUNT = 3;
static const int MIN_STACK_COUNT = 2;

Sphere::Sphere(float radius, int sectorCount, int stackCount)
    : m_radius(radius)
    , m_sectorCount(sectorCount)
    , m_stackCount(stackCount)
{
    if (m_sectorCount < MIN_SECTOR_COUNT)
    {
        m_sectorCount = MIN_SECTOR_COUNT;
    }

    if (m_stackCount < MIN_STACK_COUNT)
    {
        m_stackCount = MIN_STACK_COUNT;
    }

    BuildVertices();

    mat4x4_identity(m_view);
    mat4x4_identity(m_projection);
}

void Sphere::SetRadius(float radius)
{
    if (radius <= 0.f)
    {
        return;
    }

    float scale = radius / m_radius;

    for (size_t i = 0; i < m_vertices.size(); i++)
    {
        m_vertices[i].Position[0] *= scale;
        m_vertices[i].Position[1] *= scale;
        m_vertices[i].Position[2] *= scale;
    }

    m_radius = radius;

    if (m_initialized)
    {
        UpdateVAO();
    }
}

void Sphere::Create(GLFWwindow * window)
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

void Sphere::Delete()
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

void Sphere::Render()
{
    mat4x4 model;
    mat4x4_identity(model);
    vec4 color;
    vec4_set(color, 1.f, 1.f, 1.f, 1.f);

    Render(model, color);
}

void Sphere::Render(const linmath::mat4x4 model, const linmath::vec4 color)
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

void Sphere::Render(const linmath::vec3 p, const linmath::vec4 color)
{
    mat4x4 model;
    mat4x4_translate(model, p[0], p[1], p[2]);
    Render(model, color);
}

// build vertices of sphere with smooth shading using parametric equation
// x = r * cos(u) * cos(v)
// y = r * cos(u) * sin(v)
// z = r * sin(u)
// where u: stack(latitude) angle (-90 <= u <= 90)
//       v: sector(longitude) angle (0 <= v <= 360)
void Sphere::BuildVertices()
{
    const float PI = 3.1415926f;

    // clear memory of prev arrays
    m_vertices.clear();
    m_indices.clear();

    float radiusInv = 1.0f / m_radius;    // normal

    float sectorStep = 2 * PI / m_sectorCount;
    float stackStep = PI / m_stackCount;
    float sectorAngle = 0, stackAngle = 0;

    float x = 0.f, y = 0.f, z = 0.f, xy = 0.f;
    float nx = 0.f, ny = 0.f, nz = 0.f;

    for (int i = 0; i <= m_stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = m_radius * std::cos(stackAngle);             // r * cos(u)
        z = m_radius * std::sin(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        for (int j = 0; j <= m_sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position
            x = xy * std::cos(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * std::sin(sectorAngle);             // r * cos(u) * sin(v)

            // normalized vertex normal
            nx = x * radiusInv;
            ny = y * radiusInv;
            nz = z * radiusInv;

            m_vertices.push_back({ {x, y, z}, {nx, ny, nz} });
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    uint32_t k1 = 0, k2 = 0;
    for (int i = 0; i < m_stackCount; ++i)
    {
        k1 = i * (m_sectorCount + 1);     // beginning of current stack
        k2 = k1 + m_sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < m_sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding 1st and last stacks
            if (i != 0)
            {
                AddIndices(k1, k2, k1 + 1);   // k1---k2---k1+1
            }

            if (i != (m_stackCount - 1))
            {
                AddIndices(k1 + 1, k2, k2 + 1); // k1+1---k2---k2+1
            }
        }
    }
}

void Sphere::UpdateVAO()
{
    glBindVertexArray(m_vertexArrayObject);
    // Create buffers and bind the geometry
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(MonoVertex), m_vertices.data(), GL_STATIC_DRAW);

    // Create buffers and bind the indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);
}

void Sphere::AddIndices(uint32_t i1, uint32_t i2, uint32_t i3)
{
    m_indices.push_back(i1);
    m_indices.push_back(i2);
    m_indices.push_back(i3);
}
