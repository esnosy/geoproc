cl /EHsc /O2 /std:c++20 /GL .\apps\sample_surface.cpp .\libs\read_stl.cpp /link /LTCG
cl /EHsc /O2 /std:c++20 /GL .\apps\draw_2d.cpp /link /LTCG
cl /EHsc /O2 /std:c++20 /GL .\apps\build_bvh.cpp .\libs\read_stl.cpp /link /LTCG
cl /EHsc /O2 /std:c++20 /GL .\apps\win32_window.cpp /link user32.lib gdi32.lib /LTCG