# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CLANG_DISABLE_ALL_WARNINGS "-Wno-everything")
    add_compile_options(${CLANG_DISABLE_ALL_WARNINGS})
endif()

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build shared libs for GLFW")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Disable building GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Disable building GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Disable building GLFW docs")
set(GLFW_INSTALL OFF CACHE INTERNAL "Disable GLFW install")


add_subdirectory(src)

add_library(glfw::glfw ALIAS glfw)
