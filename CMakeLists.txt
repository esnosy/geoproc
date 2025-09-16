cmake_minimum_required(VERSION 3.16)
project(GeoProc CXX)

add_executable(sample_surface apps/sample_surface.cpp libs/read_stl.cpp)
target_compile_features(sample_surface PRIVATE cxx_std_20)

add_executable(bvh apps/bvh.cpp libs/bvh.cpp libs/read_stl.cpp)
target_compile_features(bvh PRIVATE cxx_std_20)
