cmake_minimum_required(VERSION 3.27.7)
project(erosion_sim)
include(CTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        OFF)

add_executable(${PROJECT_NAME} erosion_simulator.cpp)
add_library(lodepng lodepng.cpp)

target_link_libraries(${PROJECT_NAME} lodepng)

# Automatically create a unit test for each .png file in the TestData directory
set(test_data_dir "${CMAKE_SOURCE_DIR}/TestData")
set(test_results_dir "${CMAKE_SOURCE_DIR}/TestOutputs")
FILE(GLOB input_list "${test_data_dir}/*.png")

enable_testing()
foreach(input_name ${input_list})
    string(REGEX REPLACE "${test_data_dir}/"
       "" output_name
       "${input_name}")
    add_test(NAME "test_${output_name}" COMMAND erosion_sim "${input_name}" "${test_results_dir}/${output_name}")
endforeach()
  