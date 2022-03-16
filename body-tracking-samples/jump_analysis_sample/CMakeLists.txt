# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_executable(jump_analysis_sample
    DigitalSignalProcessing.cpp
    HandRaisedDetector.cpp
    JumpEvaluator.cpp
    main.cpp
)

target_include_directories(jump_analysis_sample PRIVATE ../sample_helper_includes)

# Dependencies of this library
target_link_libraries(jump_analysis_sample PRIVATE
    k4a
    k4abt
    k4arecord
    window_controller_3d::window_controller_3d
    glfw::glfw
)
