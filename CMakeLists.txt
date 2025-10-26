cmake_minimum_required(VERSION 3.16)
project(GeoProc CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(read_stl libs/read_stl.cpp)

add_executable(sample_surface apps/sample_surface.cpp)
target_link_libraries(sample_surface read_stl)

add_executable(bvh apps/bvh.cpp libs/bvh.cpp)
target_link_libraries(bvh read_stl)
