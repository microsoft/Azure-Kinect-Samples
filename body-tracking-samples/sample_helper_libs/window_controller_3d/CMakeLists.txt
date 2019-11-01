# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_library(window_controller_3d SHARED
            CoordinateAxes.cpp
            Cylinder.cpp
            FloorRenderer.cpp
            Helpers.cpp
            packages.config
            PointCloudRenderer.cpp
            RendererBase.cpp
            SkeletonRenderer.cpp
            Sphere.cpp
            ViewControl.cpp
            Window3dWrapper.cpp
            WindowController3d.cpp
            glad/glad.c)

target_include_directories(window_controller_3d PRIVATE ../../sample_helper_includes)

# Dependencies of this library
target_link_libraries(window_controller_3d PRIVATE 
    glfw::glfw
    )
