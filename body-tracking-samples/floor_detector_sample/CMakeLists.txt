# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.


add_executable(floor_detector_sample 
    main.cpp
    FloorDetector.cpp
    PointCloudGenerator.cpp)

target_include_directories(floor_detector_sample PRIVATE ../sample_helper_includes)

# Dependencies of this library
target_link_libraries(floor_detector_sample PRIVATE 
    k4a
    window_controller_3d::window_controller_3d
    )

