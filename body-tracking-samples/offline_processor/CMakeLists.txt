# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_executable(offline_processor
      main.cpp
)

target_link_libraries(offline_processor PRIVATE
    k4a
    k4arecord
    k4abt
    sample_helper_includes
    nlohmann::json
)
