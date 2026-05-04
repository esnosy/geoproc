cmake_minimum_required(VERSION 3.16)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS" FORCE)
project(geoproc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(stl_io STATIC libs/stl_io.cpp)
add_library(ply_io STATIC libs/ply_io.cpp)

add_executable(sample_surface apps/sample_surface.cpp)
target_link_libraries(sample_surface PUBLIC stl_io ply_io)

add_executable(bvh_demo apps/bvh_demo.cpp)
target_link_libraries(bvh_demo PUBLIC stl_io)

add_executable(vox_to_surface apps/vox_to_surface.cpp)
target_link_libraries(vox_to_surface PUBLIC stl_io ply_io)

add_executable(poke_faces apps/poke_faces.cpp)
target_link_libraries(poke_faces PUBLIC stl_io)

add_executable(smooth_vertices apps/smooth_vertices.cpp)
target_link_libraries(smooth_vertices PUBLIC stl_io)

add_executable(shrink_fatten apps/shrink_fatten.cpp)
target_link_libraries(shrink_fatten PUBLIC stl_io)

add_executable(subdivide apps/subdivide.cpp)
target_link_libraries(subdivide PUBLIC stl_io)

add_executable(sample_volume apps/sample_volume.cpp)
target_link_libraries(sample_volume PUBLIC stl_io ply_io)

add_executable(separate_islands apps/separate_islands.cpp)
target_link_libraries(separate_islands PUBLIC stl_io)