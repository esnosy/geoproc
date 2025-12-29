

add_library(read_stl_lib libs/read_stl.cpp)
add_library(bvh_lib libs/bvh.cpp)

add_executable(sample_surface_app apps/sample_surface.cpp)
target_link_libraries(sample_surface_app read_stl_lib)

add_executable(bvh_demo_app apps/bvh_demo.cpp)
target_link_libraries(bvh_demo_app read_stl_lib bvh_lib)


include(CheckIPOSupported)
check_ipo_supported(RESULT is_ipo_supported)
if(is_ipo_supported)
  set_target_properties(sample_surface_app PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
  set_target_properties(bvh_demo_app PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
