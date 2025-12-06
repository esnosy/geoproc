cmake_minimum_required(VERSION 3.16)
project(GeoProc CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(read_stl_lib libs/read_stl.cpp)
add_library(bvh_lib libs/bvh.cpp)

add_executable(sample_surface_app apps/sample_surface.cpp)
target_link_libraries(sample_surface_app read_stl_lib)

add_executable(bvh_demo_app apps/bvh_demo.cpp)
target_link_libraries(bvh_demo_app read_stl_lib bvh_lib)
