add_library(sample_helper_includes INTERFACE)

target_include_directories(sample_helper_includes INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Define alias for sample_helper_includes
add_library(k4abt::sample_helper_includes ALIAS sample_helper_includes)
