import subprocess as sp
import platform


def build_exe(soruces: list[str], output: str, debug: bool):
    if platform.system() == "Windows":
        args = ["cl", "/EHsc", "/std:c++20", "/O2", "/Fe" + output + ".exe", *soruces]
        if debug:
            args.insert(1, "/Zi")
    else:
        args = ["c++", "-std=c++20", "-O2", *soruces, "-o", output + ".bin"]
        if debug:
            args.insert(1, "-g")

    sp.run(args)


build_exe(["apps/sample_surface.cpp", "libs/read_stl.cpp"], "sample_surface", False)
build_exe(["apps/bvh.cpp", "libs/read_stl.cpp"], "bvh", False)
