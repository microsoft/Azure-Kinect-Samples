// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <mutex>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "linmath.h"
#include "WindowController3dTypes.h"
#include "RendererBase.h"

#include "CoordinateAxes.h"
#include "Cylinder.h"
#include "Sphere.h"

namespace Visualization
{
    class SkeletonRenderer : public RendererBase
    {
    public:
        SkeletonRenderer();
        ~SkeletonRenderer();
        void Create(GLFWwindow* window) override;
        void Delete() override;

        void CleanJointsAndBones();
        void AddJoint(const Visualization::Joint& joint);
        void AddBone(const Visualization::Bone& bone);

        void UpdateViewProjection(
            linmath::mat4x4 view,
            linmath::mat4x4 projection) override;

        void Render() override;
        void RenderJoint(const linmath::vec3 p, const linmath::vec4 color);
        void RenderCoordinateAxes(const linmath::vec3 p, const linmath::quaternion q);

        void EnableJointCoordinateAxes(bool renderCoordinateAxes) { m_renderCoordinateAxes = renderCoordinateAxes; }
        void EnableSkeletons(bool renderSkeletons) { m_renderSkeletons = renderSkeletons; }

        const std::vector<Joint>& GetJoints() { return m_joints; }

    private:
        // Render settings
        bool m_renderSkeletons = true;
        bool m_renderCoordinateAxes = false;

        float m_boneBaseRadius = 0.012f;
        float m_jointRadius = 0.024f;

        float m_axisThickness = 0.005f;
        float m_axisLength = 0.1f;

        // Render shape object
        Sphere m_sphere;
        Cylinder m_cylinder;
        CoordinateAxes m_coordinateAxes;

        // Skeleton information
        std::vector<Joint> m_joints;
        std::vector<Bone> m_bones;
    };
}