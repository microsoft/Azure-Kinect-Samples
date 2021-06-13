// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "linmath.h" // https://github.com/datenwolf/linmath.h

enum class ViewPoint
{
    FrontView,
    RightView,
    LeftView,
    BackView,
    TopView
};

// Camera placement parameterized around the target point the camera points to.
struct ViewParameters
{
    ViewParameters(const ViewParameters &v);

    ViewParameters(
        float targetX, float targetY, float targetZ,
        float upX, float upY, float upZ,
        float yaw, float pitch);

    // Update the rotation vectors based on the updated yaw and pitch values.
    // It needs to be called every time after updating the yaw and pitch values.
    void UpdateRotationVectors();

    void PrintViewInfo();

    // Primary Camera Attributes
    linmath::vec3 targetPos; // Location the camera points to.
    float targetDepth;
    linmath::vec3 worldUp;
    float yaw;               // Euler Angles relative to forward direction.
    float pitch;

    // Dependent Properties
    linmath::vec3 front;
    linmath::vec3 up;
    linmath::vec3 right;
};


// OpenGL-compatible screen viewport: https://docs.microsoft.com/en-us/windows/desktop/opengl/glviewport
// NOTE: Viewport placement is relative to the lower-left corner of the window content area.
struct Viewport
{
    // Specify the lower left corner of the viewport rectangle, in pixels.
    int x;
    int y;
    // Specify the width and height of the viewport.
    int width;
    int height;

    // Screen coordinates are relative to the lower-left corner of the window content area.
    bool ContainsScreenPoint(linmath::vec2 screenPos) const;
};


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class ViewControl
{
public:
    ViewControl();

    void SetViewport(Viewport viewport) { m_viewport = viewport; }
    const Viewport& GetViewport() const { return m_viewport; }

    void SetDefaultVerticalFOV(float degrees);

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    void GetViewMatrix(linmath::mat4x4 viewMatrix);

    void GetPerspectiveMatrix(linmath::mat4x4 perspectiveMatrix);

    void GetTargetPosition(linmath::vec3 targetPos);

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessRotationalMovement(const linmath::vec2 screenOffset);

    // Processes input received from a mouse input system for camera translation.
    void ProcessPositionalMovement(const linmath::vec2 startScreenPos, const linmath::vec2 endScreenPos);

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(GLFWwindow* window, float yoffset);

    // Set the location the camera points to.
    void SetViewTarget(const linmath::vec3 target);

    void Reset();

    void SetViewPoint(ViewPoint viewPoint);

    void SetMirrorMode(bool enableMirrorMode) { m_enableMirrorMode = enableMirrorMode; }

    // Project 3D point to screen coordinates.
    // Screen coordinates are relative to the lower-left corner of the window content area.
    bool ProjectToScreen(linmath::vec2 screen, const linmath::vec3 viewPoint);

    // Convert 2D screen coordinate to 3D camera ray.
    void UnprojectFromScreen(linmath::vec3 ray, const linmath::vec2 screen, float zDepth);

private:
    float PerspectiveTargetDepthForRendering();
    float PerspectiveFOVForRendering();

    ViewParameters m_viewParams;
    Viewport m_viewport;
    bool m_enableMirrorMode = false;

    // Camera options
    float m_mouseSensitivity;
    float m_defaultFOV;
    float m_perspectiveFactor; // 0: Orthographic; 1: Full perspective.
};