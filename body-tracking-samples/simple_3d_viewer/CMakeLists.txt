
add_executable(simple_3d_viewer main.cpp)

target_include_directories(simple_3d_viewer PRIVATE ../sample_helper_includes ../sample_helper_libs/window_controller_3d)

# Dependencies of this library
target_link_libraries(simple_3d_viewer PRIVATE 
    k4a
    k4abt
    window_controller_3d
    glfw::glfw
    )
