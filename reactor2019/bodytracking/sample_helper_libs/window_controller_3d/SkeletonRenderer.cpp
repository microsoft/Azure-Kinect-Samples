// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "SkeletonRenderer.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <thread>
#include <math.h>
#include <iostream>
#include "ViewControl.h"
#include "Helpers.h"

using namespace linmath;
using namespace Visualization;

SkeletonRenderer::SkeletonRenderer()
    : m_sphere(m_jointRadius)
    , m_cylinder(m_boneBaseRadius)
    , m_coordinateAxes(m_axisThickness, m_axisLength)
{
    mat4x4_identity(m_view);
    mat4x4_identity(m_projection);
}

SkeletonRenderer::~SkeletonRenderer()
{
    Delete();
}

void SkeletonRenderer::Create(GLFWwindow* window)
{
    CheckAssert(!m_initialized);
    m_initialized = true;

    m_window = window;

    m_sphere.Create(window);
    m_cylinder.Create(window);
    m_coordinateAxes.Create(window);
}

void SkeletonRenderer::Delete()
{
    if (!m_initialized)
    {
        return;
    }

    m_initialized = false;

    CleanJointsAndBones();
    m_sphere.Delete();
    m_cylinder.Delete();
    m_coordinateAxes.Delete();
}

void SkeletonRenderer::CleanJointsAndBones()
{
    m_joints.clear();
    m_bones.clear();
}

void SkeletonRenderer::AddJoint(const Visualization::Joint& joint)
{
    m_joints.push_back(joint);
}

void SkeletonRenderer::AddBone(const Visualization::Bone& bone)
{
    m_bones.push_back(bone);
}

void SkeletonRenderer::UpdateViewProjection(linmath::mat4x4 view, linmath::mat4x4 projection)
{
    RendererBase::UpdateViewProjection(view, projection);
    m_sphere.UpdateViewProjection(view, projection);
    m_cylinder.UpdateViewProjection(view, projection);
    m_coordinateAxes.UpdateViewProjection(view, projection);
}

void SkeletonRenderer::Render()
{
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (m_renderSkeletons)
    {
        // Render Bones
        for (size_t i = 0; i < m_bones.size(); i++)
        {
            m_cylinder.Render(
                m_bones[i].Joint1Position,
                m_bones[i].Joint2Position,
                m_bones[i].Color);
        }

        // Render Joints
        for (size_t i = 0; i < m_joints.size(); i++)
        {
            RenderJoint(m_joints[i].Position, m_joints[i].Color);
        }
    }

    if (m_renderCoordinateAxes)
    {
        // Render Joint Coordinate
        for (size_t i = 0; i < m_joints.size(); i++)
        {
            RenderCoordinateAxes(m_joints[i].Position, m_joints[i].Orientation);
        }
    }
    glBindVertexArray(0);
}

void SkeletonRenderer::RenderJoint(const linmath::vec3 p, const linmath::vec4 color)
{
    m_sphere.Render(p, color);
}

void SkeletonRenderer::RenderCoordinateAxes(const linmath::vec3 p, const linmath::quaternion q)
{
    m_coordinateAxes.Render(p, q);
}
