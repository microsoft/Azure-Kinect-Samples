# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_executable(offline_processor
      main.cpp
)

target_include_directories(offline_processor PRIVATE ../sample_helper_includes)

target_link_libraries(offline_processor PRIVATE
    k4a
    k4abt
    k4arecord
    nlohmann::json
)
