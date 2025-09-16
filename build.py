import subprocess as sp
import platform
import sys

debug = False

if len(sys.argv) == 2 and sys.argv[1] == "--debug":
    debug = True
elif len(sys.argv) == 1:
    pass
else:
    print("Usage: python build.py [--debug]")
    sys.exit(1)


def build_exe(soruces: list[str], output: str):
    if platform.system() == "Windows":
        args = ["cl", "/EHsc", "/std:c++20", "/O2", "/Fe" + output + ".exe", *soruces]
        if debug:
            args.insert(1, "/Zi")
    else:
        args = ["c++", "-std=c++20", "-O2", *soruces, "-o", output + ".bin"]
        if debug:
            args.insert(1, "-g")

    sp.run(args)


build_exe(["apps/sample_surface.cpp", "libs/read_stl.cpp"], "sample_surface")
build_exe(["apps/bvh.cpp", "libs/read_stl.cpp", "libs/bvh_lib.cpp"], "bvh")
