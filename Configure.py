#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import glob
import platform
import ninja_syntax

IS_WINDOWS = sys.platform == 'win32'
IS_LINUX = not IS_WINDOWS

PATH_SEPARATOR = os.path.sep

ROOT_FOLDER = os.path.dirname(os.path.abspath(__file__))

NINJA_BUILD_FILE = f"{ROOT_FOLDER}/build.ninja"

CXXFLAGS = "-Wall -pedantic -O3 -march=native -mtune=native -masm=intel"

LDFLAGS = "-lpthread -lopengl32 -lGLU32 -lfreeglut -lxerces-c" if IS_WINDOWS else "-lpthread -lGL -lGLU -lglut -lxerces-c"

BIN_EXT = ".exe" if IS_WINDOWS else ""

POOL = "default_pool", 16

BASE_SOURCES = list(map(lambda src: src.replace(ROOT_FOLDER, "."), [src for src in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.cpp") if src.find("Ateams_Base") != -1]))

PROJ_SOURCES = list(map(lambda src: src.replace(ROOT_FOLDER, "."), [src for src in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.cpp") if src.find("Ateams_Base") == -1]))

BASE_LIB = f"{ROOT_FOLDER}{PATH_SEPARATOR}Ateams_Base{PATH_SEPARATOR}bin{PATH_SEPARATOR}Ateams_Base.a"

def generate_ninja_file(ninja_file_path=NINJA_BUILD_FILE):
    ninja = ninja_syntax.Writer(open(NINJA_BUILD_FILE, "w"))

    ninja.width = 512

    ninja.comment(f"PLATFORM: {platform.system()}")
    
    ninja.newline()
    
    ninja.variable("CXXFLAGS", CXXFLAGS)
    ninja.variable("LDFLAGS", LDFLAGS)

    ninja.newline()

    ninja.pool(*POOL)

    ninja.newline()

    ninja.rule(name="compile", command="g++ -MMD -MF $out.d $CXXFLAGS -c -o $out $in $LDFLAGS", description="COMPILE $out", deps="gcc", depfile="$out.d", pool=POOL[0])

    ninja.newline()

    ninja.rule(name="link", command="g++ $CXXFLAGS -o $out $in $LDFLAGS", description="LINK $out", pool=POOL[0])

    ninja.newline()

    ninja.rule(name="archive", command="ar -rsc $out $in", description="ARCHIVE $out", pool=POOL[0])

    ninja.newline()

    for source in BASE_SOURCES:
        ninja.build(outputs=source.replace("src", "bin").replace(".cpp", ".o"), rule="compile", inputs=source)

    ninja.newline()

    ninja.build(outputs=BASE_LIB.replace(ROOT_FOLDER, "."), rule="archive", inputs=list(map(lambda src: src.replace(ROOT_FOLDER, ".").replace("src", "bin").replace(".cpp", ".o"), BASE_SOURCES)))

    ninja.newline()

    for source in PROJ_SOURCES:
        ninja.build(outputs=source.replace("src", "bin").replace(".cpp", ".o"), rule="compile", inputs=source)

    ninja.newline()

    for source in PROJ_SOURCES:
        ninja.build(outputs=source.replace("src", "bin").replace(".cpp", BIN_EXT), rule="link", inputs=[BASE_LIB.replace(ROOT_FOLDER, "."), source.replace("src", "bin").replace(".cpp", ".o")])

    ninja.newline()

    ninja.default(list(map(lambda src: src.replace(ROOT_FOLDER, ".").replace("src", "bin").replace(".cpp", BIN_EXT), PROJ_SOURCES)))

    ninja.close()

if __name__ == '__main__':   
    generate_ninja_file()
