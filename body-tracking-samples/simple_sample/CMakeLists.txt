
add_executable(simple_sample main.c)

#target_include_directories(simple_sample PRIVATE ../sample_helper_includes ../sample_helper_libs/window_controller_3d)

# Dependencies of this library
target_link_libraries(simple_sample PRIVATE 
    k4a
    k4abt
#    window_controller_3d
#    glfw::glfw
    )


