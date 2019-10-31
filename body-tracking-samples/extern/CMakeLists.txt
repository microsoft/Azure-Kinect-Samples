# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

#apt install libk4abt0.9-dev
#apt install libxi-dev

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#set (X11_Xi_INCLUDE_PATH "/usr/include")
add_subdirectory(glfw)
add_library(glfw::glfw ALIAS glfw)
