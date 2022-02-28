add_library(sample_shared_lib STATIC
    DigitalSignalProcessing.cpp
    FloorDetector.cpp
    PointCloudGenerator.cpp
)

target_include_directories(sample_shared_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Dependencies of this library
target_link_libraries(sample_shared_lib PUBLIC
    k4a
    k4abt
    sample_helper_includes
)

# Define alias for sample_shared_lib
add_library(k4abt::sample_shared_lib ALIAS sample_shared_lib)
