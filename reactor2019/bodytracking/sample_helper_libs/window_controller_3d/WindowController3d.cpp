// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "WindowController3d.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <thread>
#include <limits>

#include "ViewControl.h"
#include "Helpers.h"

using namespace linmath;
using namespace Visualization;

class GLFWEnvironmentSingleton
{
  private:
    GLFWEnvironmentSingleton()
    {
        if (!glfwInit())
        {
            exit(EXIT_FAILURE);
        }
        glfwSetErrorCallback(GLFWEnvironmentSingleton::GLFWErrorCallback);
    }

    GLFWEnvironmentSingleton(const GLFWEnvironmentSingleton &) = delete;
    GLFWEnvironmentSingleton &operator=(const GLFWEnvironmentSingleton &) = delete;

  public:
    ~GLFWEnvironmentSingleton()
    {
        glfwTerminate();
    }

  public:
    // This function initializes the GLFW library for the WindowController3d rendering. You have to run this function
    // before creating the WindowController3d object.
    //
    // This function must be called from the main thread.
    static void InitGLFW()
    {
        static GLFWEnvironmentSingleton singleton;
    }

    static void GLFWErrorCallback(int error, const char* description)
    {
        Fail("GLFW Error %d: %s\n", error, description);
    }
};

WindowController3d::WindowController3d()
{
    m_leftViewControl.SetViewPoint(ViewPoint::LeftView);
    m_rightViewControl.SetViewPoint(ViewPoint::RightView);
    m_topViewControl.SetViewPoint(ViewPoint::TopView);
}

void WindowController3d::Create(const char* name, bool showWindow, int width, int height, bool fullscreen)
{
    CheckAssert(!m_initialized);
    m_initialized = true;

    // Should be called in main
    GLFWEnvironmentSingleton::InitGLFW();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    if (!showWindow)
    {
        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    }

    const GLFWvidmode* displayInfo = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (width <= 0 || height <= 0)
    {
        m_windowWidth = static_cast<int>(displayInfo->width * m_defaultWindowWidthRatio);
        m_windowHeight = static_cast<int>(displayInfo->height * m_defaultWindowHeightRatio);
    }
    else
    {
        m_windowWidth = width;
        m_windowHeight = height;
    }

    m_windowStartPositionX = (displayInfo->width - m_windowWidth) / 2;
    m_windowStartPositionY = (displayInfo->height - m_windowHeight) / 2;

    // Get monitor for full screen
    GLFWmonitor* monitor = nullptr;
    if (fullscreen)
    {
        monitor = glfwGetPrimaryMonitor();
        int modesCount = 0, bestMode = 0;
        auto modes = glfwGetVideoModes(monitor, &modesCount);

        for (int i = 0; i < modesCount; i++)
        {
            if (modes[i].width >= modes[bestMode].width
                || modes[i].height >= modes[bestMode].height
                || modes[i].refreshRate >= modes[bestMode].refreshRate
                || (modes[i].blueBits + modes[i].greenBits + modes[i].redBits) >= (modes[bestMode].blueBits + modes[bestMode].greenBits + modes[bestMode].redBits))
            {
                bestMode = i;
            }
        }
        m_windowWidth = modes[bestMode].width;
        m_windowHeight = modes[bestMode].height;
    }

    // Create window
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, name, monitor, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetWindowPos(m_window, m_windowStartPositionX, m_windowStartPositionY);

    // In to use the member function as callback, set the current class as the Window User Pointer
    glfwSetWindowUserPointer(m_window, this);

    // Set all callbacks
    auto windowCloseCallback = [](GLFWwindow *window) {
        static_cast<WindowController3d *>(glfwGetWindowUserPointer(window))->
            WindowCloseCallback(window);
    };
    glfwSetWindowCloseCallback(m_window, windowCloseCallback);

    auto windowResizeCallback = [](GLFWwindow *window, int w, int h) {
        static_cast<WindowController3d *>(glfwGetWindowUserPointer(window))->
            FrameBufferSizeCallback(window, w, h);
    };
    glfwSetFramebufferSizeCallback(m_window, windowResizeCallback);

    auto keyPressCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        static_cast<WindowController3d *>(glfwGetWindowUserPointer(window))->
            KeyPressCallback(window, key, scancode, action, mods);
    };
    glfwSetKeyCallback(m_window, keyPressCallback);

    auto mouseMovementCallback = [](GLFWwindow *window, double xpos, double ypos) {
        static_cast<WindowController3d *>(glfwGetWindowUserPointer(window))->
            MouseMovementCallback(window, xpos, ypos);
    };
    glfwSetCursorPosCallback(m_window, mouseMovementCallback);

    auto mouseScrollCallback = [](GLFWwindow *window, double xoffset, double yoffset) {
        static_cast<WindowController3d *>(glfwGetWindowUserPointer(window))->
            MouseScrollCallback(window, xoffset, yoffset);
    };
    glfwSetScrollCallback(m_window, mouseScrollCallback);

    auto mouseButtonCallback = [](GLFWwindow *window, int button, int action, int mods) {
        static_cast<WindowController3d *>(glfwGetWindowUserPointer(window))->
            MouseButtonCallback(window, button, action, mods);
    };
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);


    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSwapInterval(showWindow ? 1 : 0);

    // Context Settings
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClearDepth(1.0f);

    m_pointCloudRenderer.Create(m_window);
    m_skeletonRenderer.Create(m_window);
}

void WindowController3d::Delete()
{
    m_initialized = false;
    m_pointCloudRenderer.Delete();
    m_skeletonRenderer.Delete();

    if (m_enableFloorRendering)
    {
        m_floorRenderer.Delete();
        m_enableFloorRendering = false;
    }

    glfwDestroyWindow(m_window);
    m_window = nullptr;
}

void WindowController3d::SetWindowPosition(int xPos, int yPos)
{
    if (m_window != nullptr)
    {
        glfwSetWindowPos(m_window, xPos, yPos);
    }
}

bool WindowController3d::InitializePointCloudRenderer(
    bool enableShading,
    const float* depthXyTableInterleaved,
    int width, int height)
{
    m_pointCloudRenderer.SetShading(enableShading);
    if (enableShading)
    {
        if (depthXyTableInterleaved == nullptr)
        {
            return false;
        }

        m_pointCloudRenderer.InitializeDepthXYTable(depthXyTableInterleaved, width, height);
    }

    return true;
}

void WindowController3d::UpdatePointClouds(
    const PointCloudVertex* point3d,
    uint32_t numPoints,
    const uint16_t* depthFrame,
    uint32_t width, uint32_t height,
    bool useTestPointClouds)
{
    m_pointCloudRenderer.UpdatePointClouds(m_window, point3d, numPoints, depthFrame, width, height, useTestPointClouds);
}

void WindowController3d::CleanJointsAndBones()
{
    m_skeletonRenderer.CleanJointsAndBones();
}

void WindowController3d::AddJoint(const Visualization::Joint& joint)
{
    m_skeletonRenderer.AddJoint(joint);
}

void WindowController3d::AddBone(const Visualization::Bone& bone)
{
    m_skeletonRenderer.AddBone(bone);
}

void WindowController3d::RenderScene(ViewControl& viewControl, Viewport viewport)
{
    // Assign viewport to viewControl.
    viewControl.SetViewport(viewport);

    // Change view port
    glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

    // Update view/projection matrix
    linmath::mat4x4 projection;
    linmath::mat4x4 view;

    viewControl.GetPerspectiveMatrix(projection);
    viewControl.GetViewMatrix(view);

    UpdateRenderersViewProjection(view, projection);

    if (m_enableFloorRendering)
    {
        m_floorRenderer.Render();
    }

    if (m_skeletonRenderMode == SkeletonRenderMode::SkeletonOverlayWithJointFrame)
    {
        m_skeletonRenderer.EnableJointCoordinateAxes(true);
        m_skeletonRenderer.EnableSkeletons(true);
    }
    else
    {
        m_skeletonRenderer.EnableJointCoordinateAxes(false);
        m_skeletonRenderer.EnableSkeletons(true);
    }

    if (m_skeletonRenderMode == SkeletonRenderMode::SkeletonOverlay ||
        m_skeletonRenderMode == SkeletonRenderMode::SkeletonOverlayWithJointFrame)
    {
        m_pointCloudRenderer.Render(viewport.width, viewport.height);

        glClear(GL_DEPTH_BUFFER_BIT);
        m_skeletonRenderer.Render();
    }
    else
    {
        m_skeletonRenderer.Render();
        m_pointCloudRenderer.Render(viewport.width, viewport.height);
    }

    // Render Camera Pivot Point when interacting with the view control.

    const bool ctrl = glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL);
    if (m_cameraPivotPointRenderCount > 0 || m_mouseButtonLeftPressed || m_mouseButtonRightPressed || ctrl)
    {
        m_cameraPivotPointRenderCount = std::max(0, m_cameraPivotPointRenderCount - 1);

        vec3 targetPos;
        m_viewControl.GetTargetPosition(targetPos);
        // Render Camera Pivot Point the shape of a joint, but red.
        vec4 red = { 1.0f, 0.0f, 0.0f, 1.0f };
        m_skeletonRenderer.RenderJoint(targetPos, red);
    }
}

// Trigger camera pivot point rendering for a few frames.
void WindowController3d::TriggerCameraPivotPointRendering()
{
    m_cameraPivotPointRenderCount = 5;
}

void WindowController3d::Render(std::vector<uint8_t>* renderedPixelsBgr, int* pixelsWidth, int* pixelsHeight)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    glfwMakeContextCurrent(m_window);

    // Per-frame time logic
    double currentFrame = glfwGetTime();
    m_deltaTime = (float)(currentFrame - m_lastFrame);
    m_lastFrame = currentFrame;

    // General Render clean up
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render window
    int windowWidth = m_windowWidth;
    int windowHeight = m_windowHeight;

    // NOTE: Viewport placement is relative to the lower-left corner of the window content area.
    switch (m_layout3d)
    {
    default:
    case Layout3d::OnlyMainView:
        RenderScene(m_viewControl, Viewport{0, 0, windowWidth, windowHeight});
        break;
    case Layout3d::FourViews:
        RenderScene(m_leftViewControl, Viewport{0, 0, windowWidth / 2, windowHeight / 2});
        RenderScene(m_rightViewControl, Viewport{windowWidth / 2, 0, windowWidth / 2, windowHeight / 2});
        RenderScene(m_viewControl, Viewport{0, m_windowHeight / 2, windowWidth / 2, windowHeight / 2});
        RenderScene(m_topViewControl, Viewport{windowWidth / 2, windowHeight / 2, windowWidth / 2, windowHeight / 2});
        break;
    }

    // Copy rendered pixels if needed
    if (renderedPixelsBgr != nullptr)
    {
        renderedPixelsBgr->resize(windowWidth * windowHeight * 3);
        glReadPixels(0, 0, windowWidth, windowHeight, GL_BGR, GL_UNSIGNED_BYTE, renderedPixelsBgr->data());
    }
    if (pixelsWidth != nullptr)
    {
        *pixelsWidth = windowWidth;
    }
    if (pixelsHeight != nullptr)
    {
        *pixelsHeight = windowHeight;
    }

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void WindowController3d::SetPointCloudShading(bool enableShading)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_pointCloudRenderer.SetShading(enableShading);
}

void WindowController3d::SetDefaultVerticalFOV(float degrees)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto v : m_allViewControls)
    {
        v->SetDefaultVerticalFOV(degrees);
    }
}

void WindowController3d::SetMirrorMode(bool enableMirrorMode)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto v : m_allViewControls)
    {
        v->SetMirrorMode(enableMirrorMode);
    }
}

void WindowController3d::SetSkeletonRenderMode(SkeletonRenderMode skeletonRenderMode)
{
    m_skeletonRenderMode = skeletonRenderMode;
}

void WindowController3d::SetLayout3d(Layout3d layout3d)
{
    m_layout3d = layout3d;
}

void WindowController3d::ChangePointCloudSize(float pointCloudSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_pointCloudRenderer.ChangePointCloudSize(pointCloudSize);
}

void WindowController3d::SetFloorRendering(bool enableFloorRendering, linmath::vec3 floorPosition, linmath::quaternion floorOrientation)
{
    if (!m_enableFloorRendering && enableFloorRendering)
    {
        m_floorRenderer.Create(m_window);
    }
    else if (m_enableFloorRendering && !enableFloorRendering)
    {
        m_floorRenderer.Delete();
    }
    m_enableFloorRendering = enableFloorRendering;

    if (m_enableFloorRendering)
    {
        m_floorRenderer.SetFloorPlacement(floorPosition, floorOrientation);
    }
}

void WindowController3d::SetCloseCallback(CloseCallbackType callback, void* context)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_closeCallback = callback;
    m_closeCallbackContext = context;
}

void WindowController3d::SetKeyCallback(KeyCallbackType callback, void* context)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_keyCallback = callback;
    m_keyCallbackContext = context;
}

// Callback functions
void WindowController3d::FrameBufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
}

void WindowController3d::GetCursorPosInScreenCoordinates(GLFWwindow* window, linmath::vec2 outScreenPos)
{
    // NOTE: Cursor coordinates are relative to the upper-left corner of the window content area.
    double cursorPosX, cursorPosY;
    glfwGetCursorPos(window, &cursorPosX, &cursorPosY);

    // Convert cursor coordinates to OpenGL screen coordinates.
    // NOTE: OpenGL screen coordinates are relative to the lower-left corner of the window content area.
    GetCursorPosInScreenCoordinates(cursorPosX, cursorPosY, outScreenPos);
}

void WindowController3d::GetCursorPosInScreenCoordinates(double cursorPosX, double cursorPosY, linmath::vec2 outScreenPos)
{
    // NOTE: Cursor coordinates are relative to the upper-left corner of the window content area.

    // Convert cursor coordinates to OpenGL screen coordinates.
    // NOTE: OpenGL screen coordinates are relative to the lower-left corner of the window content area.
    outScreenPos[0] = (float)cursorPosX;
    outScreenPos[1] = (float)(m_windowHeight - cursorPosY - 1.);
}

void WindowController3d::UpdateRenderersViewProjection(linmath::mat4x4 view, linmath::mat4x4 projection)
{
    m_pointCloudRenderer.UpdateViewProjection(view, projection);
    m_skeletonRenderer.UpdateViewProjection(view, projection);

    if (m_enableFloorRendering)
    {
        m_floorRenderer.UpdateViewProjection(view, projection);
    }
}

void WindowController3d::MouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/)
{
    // Keep track of mouse movement for camera rotation/translation when left button is pressed.
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        GetCursorPosInScreenCoordinates(window, m_prevMouseScreenPos);
        m_mouseButtonLeftPressed = action == GLFW_PRESS;
    }

    // Use right button to select a camera pivot point in the scene.
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        m_mouseButtonRightPressed = action == GLFW_PRESS;
        if (action == GLFW_PRESS)
        {
            vec2 screenPos;
            GetCursorPosInScreenCoordinates(window, screenPos);

            if (m_viewControl.GetViewport().ContainsScreenPoint(screenPos))
            {
                ChangeCameraPivotPoint(m_viewControl, screenPos);
            }
        }
    }
}

void WindowController3d::MouseMovementCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Only take mouse position data when mouse left button is pressed.
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
    {
        return;
    }

    vec2 screenPos;
    GetCursorPosInScreenCoordinates(xpos, ypos, screenPos);

    const bool ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
    if (ctrl)
    {
        m_viewControl.ProcessPositionalMovement(m_prevMouseScreenPos, screenPos);
    }
    else
    {
        vec2 offset;
        vec2_sub(offset, screenPos, m_prevMouseScreenPos);

        m_viewControl.ProcessRotationalMovement(offset);
    }

    vec2_copy(m_prevMouseScreenPos, screenPos);
}

void WindowController3d::ChangeCameraPivotPoint(ViewControl& viewControl, linmath::vec2 screenPos)
{
    float minDist = std::numeric_limits<float>::max();
    vec3 selectedPoint;
    const auto &joints = m_skeletonRenderer.GetJoints();
    for (auto j : joints)
    {
        linmath::vec2 screen;
        if (viewControl.ProjectToScreen(screen, j.Position))
        {
            linmath::vec2 delta;
            vec2_sub(delta, screenPos, screen);
            if (vec2_len(delta) < minDist)
            {
                minDist = vec2_len(delta);
                vec3_copy(selectedPoint, j.Position);
            }
        }
    }
    if (minDist != std::numeric_limits<float>::max())
    {
        viewControl.SetViewTarget(selectedPoint);
    }
}

void WindowController3d::MouseScrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset)
{
    m_viewControl.ProcessMouseScroll(window, (float)yoffset);
    TriggerCameraPivotPointRendering();
}

void WindowController3d::WindowCloseCallback(GLFWwindow* /*window*/)
{
    if (m_closeCallback)
    {
        m_closeCallback(m_closeCallbackContext);
    }
}

void WindowController3d::KeyPressCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
    // https://www.glfw.org/docs/latest/group__keys.html
    if (action == GLFW_RELEASE)
    {
        return;
    }

    switch (key)
    {
    case GLFW_KEY_HOME:
        m_viewControl.Reset();
        break;
    case GLFW_KEY_F1:
        m_viewControl.SetViewPoint(ViewPoint::FrontView);
        break;
    case GLFW_KEY_F2:
        m_viewControl.SetViewPoint(ViewPoint::RightView);
        break;
    case GLFW_KEY_F3:
        m_viewControl.SetViewPoint(ViewPoint::BackView);
        break;
    case GLFW_KEY_F4:
        m_viewControl.SetViewPoint(ViewPoint::LeftView);
        break;
    case GLFW_KEY_F5:
        m_viewControl.SetViewPoint(ViewPoint::TopView);
        break;
    default:
        // If not handled, then pass along to external callback.
        if (m_keyCallback)
        {
            m_keyCallback(m_keyCallbackContext, key);
        }
        break;
    }
}
