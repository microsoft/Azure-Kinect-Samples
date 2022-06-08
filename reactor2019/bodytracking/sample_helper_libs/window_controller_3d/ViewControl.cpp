// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "ViewControl.h"

#include <algorithm>
#include <stdio.h>

using namespace linmath;

#ifndef M_PI
#define M_PI       3.14159265358979323846   // pi
#endif

static inline float DegreesToRadians(float angleInDegrees)
{
    return (float)(angleInDegrees / 180.f * M_PI);
}

// Default camera values
const float kDefaultSensitivity = 0.2f;
const float kDefaultFOV = 65.0f;                // FOV (degrees)

const float kFullPerspectiveFactor = 1.0f;      // Perspective projection
const float kLeastPerspectiveFactor = 0.1f;     // Orthographic projection
const float kDefaulPerspectiveFactor = kFullPerspectiveFactor;
const float kObserverPerspectiveFactor = 0.3f;

// K4A coordinate system uses OpenCV camera convention: X-right, Y-down, Z-forward
const float kTargetDepth = 1.5f;

const ViewParameters kDefaultView(
    0.f, 0.f, kTargetDepth,                     // Target depth (meters)
    0.f, -1.f, 0.f,                             // WorldUp
    0.f, 0.f);                                  // Yaw and Pitch (degrees)

const ViewParameters kRightView(
    0.f, 0.f, kTargetDepth,                     // Target depth (meters)
    0.f, -1.f, 0.f,                             // WorldUp
    75.f, 10.0f);                               // Yaw and Pitch (degrees)

const ViewParameters kBackView(
    0.f, 0.f, kTargetDepth,                     // Target depth (meters)
    0.f, -1.f, 0.f,                             // WorldUp
    180.f, 0.f);                                // Yaw and Pitch (degrees)

const ViewParameters kLeftView(
    0.f, 0.f, kTargetDepth,                     // Target depth (meters)
    0.f, -1.f, 0.f,                             // WorldUp
    -75.0f, 10.0f);                             // Yaw and Pitch (degrees)

const ViewParameters kTopView(
    0.f, 0.f, kTargetDepth,                     // Target depth (meters)
    0.f, -1.f, 0.f,                             // WorldUp
    0.f, 45.0f);                                // Yaw and Pitch (degrees)

ViewParameters::ViewParameters(const ViewParameters &v)
{
    vec3_copy(targetPos, v.targetPos);
    targetDepth = v.targetDepth;
    yaw = v.yaw;
    pitch = v.pitch;
    vec3_copy(front, v.front);
    vec3_copy(up, v.up);
    vec3_copy(right, v.right);
    vec3_copy(worldUp, v.worldUp);
}

ViewParameters::ViewParameters(
    float targetX, float targetY, float targetZ,
    float upX, float upY, float upZ,
    float _yaw, float _pitch)
{
    vec3_set(targetPos, targetX, targetY, targetZ);
    targetDepth = vec3_len(targetPos);
    vec3_set(worldUp, upX, upY, upZ);
    yaw = _yaw;
    pitch = _pitch;
    UpdateRotationVectors();
}

// Update camera position and rotation vectors from Euler angles and target point.
// It needs to be called every time after updating the primary camera attributes.
void ViewParameters::UpdateRotationVectors()
{
    // Calculate the new front vector (relative to +Z)
    vec3 frontTemp;
    frontTemp[0] = (float)(sin(DegreesToRadians(yaw)) * cos(DegreesToRadians(pitch)));
    frontTemp[1] = (float)(sin(DegreesToRadians(pitch)));
    frontTemp[2] = (float)(cos(DegreesToRadians(yaw)) * cos(DegreesToRadians(pitch)));

    vec3_norm(front, frontTemp);

    // Also re-calculate the Right and Up vector
    vec3 rightTemp;
    vec3_mul_cross(rightTemp, front, worldUp);
    vec3_norm(right, rightTemp); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.

    vec3 upTemp;
    vec3_mul_cross(upTemp, right, front);
    vec3_norm(up, upTemp);
}

void ViewParameters::PrintViewInfo()
{
    printf("---------------------------------------\n");
    printf("TargetPos: %f, %f, %f;\n", targetPos[0], targetPos[1], targetPos[2]);
    printf("Front: %f, %f, %f;\n", front[0], front[1], front[2]);
    printf("Right: %f, %f, %f;\n", right[0], right[1], right[2]);
    printf("Up: %f, %f, %f;\n", up[0], up[1], up[2]);
    printf("WorldUp: %f, %f, %f;\n", worldUp[0], worldUp[1], worldUp[2]);
    printf("Yaw: %f; Pitch: %f;\n", yaw, pitch);
}

ViewControl::ViewControl()
    : m_viewParams(kDefaultView)
    , m_mouseSensitivity(kDefaultSensitivity)
    , m_defaultFOV(kDefaultFOV)
    , m_perspectiveFactor(kDefaulPerspectiveFactor)
{
}

void ViewControl::SetDefaultVerticalFOV(float degrees)
{
    m_defaultFOV = degrees;
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
void ViewControl::GetViewMatrix(mat4x4 viewMatrix)
{
    // Update position
    vec3 v;
    vec3_scale(v, m_viewParams.front, PerspectiveTargetDepthForRendering());
    vec3 pos;
    vec3_sub(pos, m_viewParams.targetPos, v);

    mat4x4_look_at(viewMatrix, pos, m_viewParams.targetPos, m_viewParams.up);
}

void ViewControl::GetPerspectiveMatrix(mat4x4 perspectiveMatrix)
{
    mat4x4_perspective(perspectiveMatrix, DegreesToRadians(PerspectiveFOVForRendering()), m_viewport.width / (float)m_viewport.height, 0.1f, 150.f);

    if (m_enableMirrorMode)
    {
        perspectiveMatrix[0][0] = -perspectiveMatrix[0][0];
    }
}

float ViewControl::PerspectiveTargetDepthForRendering()
{
    return m_viewParams.targetDepth / m_perspectiveFactor;
}

float ViewControl::PerspectiveFOVForRendering()
{
    return m_defaultFOV * m_perspectiveFactor;
}

void ViewControl::GetTargetPosition(vec3 targetPos)
{
    vec3_copy(targetPos, m_viewParams.targetPos);
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void ViewControl::ProcessRotationalMovement(const linmath::vec2 screenOffset)
{
    float xoffset = screenOffset[0];
    float yoffset = screenOffset[1];

    if (m_enableMirrorMode)
    {
        xoffset = -xoffset;
    }

    m_viewParams.yaw += xoffset * m_mouseSensitivity;
    m_viewParams.pitch -= yoffset * m_mouseSensitivity;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    m_viewParams.pitch = std::clamp(m_viewParams.pitch, -89.5f, 89.5f);

    // Update m_viewParams.front, right and up Vectors using the updated Euler angles
    m_viewParams.UpdateRotationVectors();
}

void ViewControl::ProcessPositionalMovement(const linmath::vec2 startScreenPos, const linmath::vec2 endScreenPos)
{
    vec3 startRay;
    UnprojectFromScreen(startRay, startScreenPos, PerspectiveTargetDepthForRendering());
    vec3 endRay;
    UnprojectFromScreen(endRay, endScreenPos, PerspectiveTargetDepthForRendering());

    vec3 offset;
    vec3_sub(offset, endRay, startRay);

    vec3 oldTargetPos;
    vec3_copy(oldTargetPos, m_viewParams.targetPos);
    vec3_sub(m_viewParams.targetPos, oldTargetPos, offset);
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void ViewControl::ProcessMouseScroll(GLFWwindow* window, float yoffset)
{
    const bool shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    if (shift)
    {
        m_perspectiveFactor += yoffset * m_mouseSensitivity;
        m_perspectiveFactor = std::clamp(m_perspectiveFactor, kLeastPerspectiveFactor, kFullPerspectiveFactor);
    }
    else
    {
        m_viewParams.targetDepth += yoffset * m_mouseSensitivity;
        m_viewParams.targetDepth = std::clamp(m_viewParams.targetDepth, 0.2f, 10.0f);
    }
}

void ViewControl::SetViewTarget(const linmath::vec3 target)
{
    vec3_copy(m_viewParams.targetPos, target);
}

void ViewControl::Reset()
{
    m_viewParams = kDefaultView;
    m_perspectiveFactor = kDefaulPerspectiveFactor;
}

void ViewControl::SetViewPoint(ViewPoint viewPoint)
{
    switch (viewPoint)
    {
    case ViewPoint::FrontView:
        m_viewParams = kDefaultView;
        m_perspectiveFactor = kDefaulPerspectiveFactor;
        break;
    case ViewPoint::RightView:
        m_viewParams = kRightView;
        m_perspectiveFactor = kObserverPerspectiveFactor;
        break;
    case ViewPoint::LeftView:
        m_viewParams = kLeftView;
        m_perspectiveFactor = kObserverPerspectiveFactor;
        break;
    case ViewPoint::BackView:
        m_viewParams = kBackView;
        m_perspectiveFactor = kObserverPerspectiveFactor;
        break;
    case ViewPoint::TopView:
        m_viewParams = kTopView;
        m_perspectiveFactor = kObserverPerspectiveFactor;
        break;
    }
}

bool ViewControl::ProjectToScreen(linmath::vec2 screen, const linmath::vec3 viewPoint)
{
    vec4 p;
    vec3_copy(p, viewPoint);
    p[3] = 1.0f;

    mat4x4 projectionMatrix;
    GetPerspectiveMatrix(projectionMatrix);
    mat4x4 viewMatrix;
    GetViewMatrix(viewMatrix);
    mat4x4 M;
    mat4x4_mul(M, projectionMatrix, viewMatrix);

    vec4 r;
    mat4x4_mul_vec4(r, M, p);
    if (r[3] == 0)
    {
        return false;
    }

    screen[0] = (1.f + r[0] / r[3]) / 2.f * m_viewport.width + m_viewport.x + 0.5f;
    screen[1] = (1.f + r[1] / r[3]) / 2.f * m_viewport.height + m_viewport.y + 0.5f;
    return true;
}

void ViewControl::UnprojectFromScreen(linmath::vec3 ray, const linmath::vec2 screen, float zDepth)
{
    vec4 s;
    s[0] = (screen[0] - 0.5f - m_viewport.x) / m_viewport.width * 2.f - 1.f;
    s[1] = (screen[1] - 0.5f - m_viewport.y) / m_viewport.height * 2.f - 1.f;
    s[2] = 0.0f;
    s[3] = 1.0f;

    vec4 r;
    vec4_scale(r, s, zDepth);

    mat4x4 projectionMatrix;
    GetPerspectiveMatrix(projectionMatrix);
    mat4x4 viewMatrix;
    GetViewMatrix(viewMatrix);
    mat4x4 M;
    mat4x4_mul(M, projectionMatrix, viewMatrix);
    mat4x4 invM;
    mat4x4_invert(invM, M);

    vec4 p;
    mat4x4_mul_vec4(p, invM, r);
    vec3_copy(ray, p);
}

bool Viewport::ContainsScreenPoint(linmath::vec2 screenPos) const
{
    int vx = (int)floor(screenPos[0]) - x;
    int vy = (int)floor(screenPos[1]) - y;
    return 0 <= vx && vx < width && 0 <= vy && vy < height;
}
