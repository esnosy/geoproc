import subprocess as sp
import platform


if platform.system() == "Windows":
    sp.run(["cl", "/EHsc", "/std:c++20", "/O2", "apps/sample_surface.cpp", "libs/read_stl.cpp"])
else:
    sp.run(["c++", "-std=c++20", "-O2", "apps/sample_surface.cpp", "libs/read_stl.cpp", "-o", "sample_surface.bin"])
