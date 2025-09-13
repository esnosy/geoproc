import subprocess as sp
import platform


def build_exe(soruces: list[str], output: str):
    if platform.system() == "Windows":
        sp.run(["cl", "/EHsc", "/std:c++20", "/O2", "/Fe" + output + ".exe", *soruces])
    else:
        sp.run(["c++", "-std=c++20", "-O2", *soruces, "-o", output + ".bin"])


build_exe(["apps/sample_surface.cpp", "libs/read_stl.cpp"], "sample_surface")
build_exe(["apps/bvh.cpp", "libs/read_stl.cpp"], "bvh")
