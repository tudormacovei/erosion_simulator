cmake_minimum_required(VERSION 3.27.7)
project(erosion_sim)
include(CTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        OFF)

add_executable(${PROJECT_NAME} erosion_simulator.cpp)
add_library(lodepng lodepng.cpp)

target_link_libraries(${PROJECT_NAME} lodepng)

enable_testing()
add_test(NAME first_test COMMAND erosion_sim ${CMAKE_SOURCE_DIR}/heightmap_64.png ${CMAKE_SOURCE_DIR}/out_64.png)
add_test(NAME second_test COMMAND erosion_sim ${CMAKE_SOURCE_DIR}/heightmap_128.png ${CMAKE_SOURCE_DIR}/out_128.png)    