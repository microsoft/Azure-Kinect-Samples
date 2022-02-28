# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_executable(jump_analysis_sample
    HandRaisedDetector.cpp
    JumpEvaluator.cpp
    main.cpp
)

# Dependencies of this library
target_link_libraries(jump_analysis_sample PRIVATE
    k4a
    k4abt
    sample_shared_lib
    window3d_wrapper_lib
)
