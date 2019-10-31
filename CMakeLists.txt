# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.9.0)

cmake_policy(SET CMP0048 NEW)

project(Azure-Kinect-Samples LANGUAGES C CXX
    VERSION 1.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(body-tracking-samples)
