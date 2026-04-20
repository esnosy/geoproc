cmake_minimum_required(VERSION 3.16)
project(geoproc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(stl_io STATIC libs/stl_io.cpp)
add_library(ply_io STATIC libs/ply_io.cpp)

add_executable(sample_surface apps/sample_surface.cpp)
target_link_libraries(sample_surface PUBLIC stl_io ply_io)

add_executable(bvh apps/bvh.cpp)
target_link_libraries(bvh PUBLIC stl_io)

add_executable(vox_to_stl apps/vox_to_stl.cpp)
target_link_libraries(vox_to_stl PUBLIC stl_io ply_io)