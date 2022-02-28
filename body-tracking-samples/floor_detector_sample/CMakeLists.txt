# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_executable(floor_detector_sample
    main.cpp
)

# Dependencies of this library
target_link_libraries(floor_detector_sample PRIVATE
    k4a
    sample_shared_lib
    window3d_wrapper_lib
)
