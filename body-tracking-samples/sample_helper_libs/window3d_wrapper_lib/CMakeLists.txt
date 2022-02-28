add_library(window3d_wrapper_lib STATIC
    Window3dWrapper.cpp
)

target_include_directories(window3d_wrapper_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Dependencies of this library
target_link_libraries(window3d_wrapper_lib PUBLIC
    k4a
    k4abt
    sample_helper_includes
    glfw::glfw
    window_controller_3d::window_controller_3d
)

# Define alias for window3d_wrapper_lib
add_library(k4abt::window3d_wrapper_lib ALIAS window3d_wrapper_lib)
