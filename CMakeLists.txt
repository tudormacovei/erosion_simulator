cmake_minimum_required(VERSION 3.27.7)
project(erosion_sim)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        OFF)

add_executable(${PROJECT_NAME} erosion_simulator.cpp)
add_library(lodepng lodepng.cpp)

target_link_libraries(${PROJECT_NAME} lodepng)