cmake_minimum_required(VERSION 3.16)
project(GeoProc CXX)

add_library(read_stl libs/read_stl.cpp)
target_compile_features(read_stl PRIVATE cxx_std_20)

add_executable(sample_surface apps/sample_surface.cpp)
target_link_libraries(sample_surface read_stl)
target_compile_features(sample_surface PRIVATE cxx_std_20)

add_executable(bvh apps/bvh.cpp libs/bvh.cpp)
target_link_libraries(bvh read_stl)
target_compile_features(bvh PRIVATE cxx_std_20)
