cmake_minimum_required(VERSION 3.16)
project(geoproc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(stl_io STATIC libs/stl_io.cpp)

add_executable(sample_surface apps/sample_surface.cpp)
target_link_libraries(sample_surface PUBLIC stl_io)