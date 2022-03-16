# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

include(FetchContent)

FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY     https://github.com/nlohmann/json.git
    GIT_TAG             v3.7.0
)
FetchContent_GetProperties(nlohmann_json)
if (NOT nlohmann_json_POPULATED)
    FetchContent_Populate(nlohmann_json)
    add_subdirectory(${nlohmann_json_SOURCE_DIR} ${nlohmann_json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_library(nlohmann::json ALIAS nlohmann_json)
