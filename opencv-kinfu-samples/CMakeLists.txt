cmake_minimum_required(VERSION 3.10)
add_executable(kinfu-example main.cpp)

find_package(k4a 1.3.0 QUIET)
include_directories(${K4A_INCLUDE_DIRS})

find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(kinfu-example PRIVATE k4a::k4a opencv_rgbd opencv_viz)
