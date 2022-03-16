# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_executable(camera_space_transform_sample
    main.cpp
)

target_link_libraries(camera_space_transform_sample PRIVATE
    k4a
    k4abt
)
