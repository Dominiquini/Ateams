#!/usr/bin/env python
# -*- coding: utf-8 -*-

import collections
import glob
import sys
import os

import ninja_syntax

try:
    import rich_click as click
except ImportError:
    import click


THIS_FILE = os.path.basename(__file__)

POOL_ASYNC = collections.namedtuple('NinjaPool', ['name', 'depth'])("threaded", 16)

PLATFORM = collections.namedtuple('PlatformInfo', ['windows_key', 'linux_key', 'is_windows', 'is_linux', 'system'])(windows_key := "WINDOWS", linux_key := "LINUX", is_windows := sys.platform == 'win32', is_linux := not is_windows, system := windows_key if is_windows else linux_key)

PATH_SEPARATOR = os.path.sep

ROOT_FOLDER = os.path.dirname(os.path.abspath(__file__))

NINJA_BUILD_FILE = os.path.basename(f"{ROOT_FOLDER}{PATH_SEPARATOR}build.ninja")

COMPILER = "g++"

LINKER = "ld"

ARCHIVER = 'ar'

BUILDING_MODE = "RELEASE"

CXX_GLOBAL_FLAGS = "-std=c++20 -pthread -march=native -mtune=native -fno-pie -fdiagnostics-color=always"

CXX_FLAGS = {"RELEASE": f"{CXX_GLOBAL_FLAGS} -O3 -flto=auto -ffast-math", "DEBUG": f"{CXX_GLOBAL_FLAGS} -O0 -fno-lto -g3 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer", "PROFILE": f"{CXX_GLOBAL_FLAGS} -O0 -fno-lto -g3 -pg -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer"}

LD_GLOBAL_FLAGS = "-static-libgcc -static-libstdc++ -no-pie"

LD_FLAGS = {PLATFORM.windows_key: f"{LD_GLOBAL_FLAGS} -lopengl32 -lGLU32 -lfreeglut", PLATFORM.linux_key: f"{LD_GLOBAL_FLAGS} -lGL -lGLU -lglut"}

LD_OPTIONAL_FLAGS_LINKER = "-fuse-ld={linker}"

AR_FLAGS = "-crs"

BIN_EXT = ".exe" if PLATFORM.is_windows else ""

BASE_SRCS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.cpp") if cpp.find("Ateams_Base") != -1]
BASE_HDRS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.hpp") if cpp.find("Ateams_Base") != -1]
BASE_OBJS = [src.replace("src", "bin").replace(".cpp", ".o") for src in BASE_SRCS]
BASE_LIB = f"{ROOT_FOLDER}{PATH_SEPARATOR}Ateams_Base{PATH_SEPARATOR}bin{PATH_SEPARATOR}Ateams_Base.a".replace(ROOT_FOLDER, ".")

PROJ_SRCS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.cpp") if cpp.find("Ateams_Base") == -1]
PROJ_HDRS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.hpp") if cpp.find("Ateams_Base") == -1]
PROJ_OBJS = [src.replace("src", "bin").replace(".cpp", ".o") for src in PROJ_SRCS]
PROJ_BINS = [obj.replace(".o", BIN_EXT) for obj in PROJ_OBJS]

BUILD_ALL_KEYWORD = 'All'

class NinjaFileWriter(ninja_syntax.Writer):

    def __init__(self, path=NINJA_BUILD_FILE, mode="w", width=512):
        super().__init__(output=open(path, mode), width=width)

    def __enter__(self):
        return self

    def __exit__(self, *exception):
        super().close()
    
    @classmethod
    def generate_ninja_build_file(cls, ninja_file, compiler, linker, archiver, mode):
        with NinjaFileWriter(ninja_file) as ninja:
            ninja.comment(f"PLATFORM: {PLATFORM.system}")

            ninja.newline()

            ninja.variable("BUILD_MODE", mode)

            ninja.newline()

            ninja.variable("CC", compiler)
            ninja.variable("LK", linker)
            ninja.variable("AR", archiver)

            ninja.newline()

            ninja.variable("CXXFLAGS", CXX_FLAGS[mode] if mode in CXX_FLAGS else CXX_GLOBAL_FLAGS)
            ninja.variable("LDFLAGS", LD_FLAGS[PLATFORM.system] + (f" {LD_OPTIONAL_FLAGS_LINKER.format(linker=linker)}" if linker != LINKER else ""))
            ninja.variable("ARFLAGS", AR_FLAGS)

            ninja.newline()

            ninja.pool(**POOL_ASYNC._asdict())

            ninja.newline()

            ninja.rule(name="compile", command="$CC $CXXFLAGS -MMD -MF $out.d -c -o $out $in", description="COMPILE $out", deps="gcc", depfile="$out.d", pool=POOL_ASYNC.name)

            ninja.newline()

            ninja.rule(name="link", command="$CC $CXXFLAGS -o $out $in $LDFLAGS", description="LINK $out", pool=POOL_ASYNC.name)

            ninja.newline()

            ninja.rule(name="archive", command="$AR $ARFLAGS $out $in", description="ARCHIVE $out", pool=POOL_ASYNC.name)

            ninja.newline()

            ninja.rule(name="generate", command="python $in -c $CC -l $LK -a $AR -m $BUILD_MODE", description="UPDATE NINJA BUILD FILE", pool=POOL_ASYNC.name, generator=True)

            ninja.newline()

            for src, obj in zip(BASE_SRCS, BASE_OBJS):
                ninja.build(outputs=obj, rule="compile", inputs=src)

            ninja.newline()

            for src, obj in zip(PROJ_SRCS, PROJ_OBJS):
                ninja.build(outputs=obj, rule="compile", inputs=src)

            ninja.newline()

            ninja.build(outputs=BASE_LIB, rule="archive", inputs=BASE_OBJS)

            ninja.newline()

            for obj, bin in zip(PROJ_OBJS, PROJ_BINS):
                ninja.build(outputs=bin, rule="link", inputs=[BASE_LIB, obj])

            ninja.newline()

            ninja.build(outputs="Ateams", rule="phony", inputs=PROJ_BINS)

            ninja.newline()

            ninja.build(outputs="Base", rule="phony", inputs=BASE_LIB)

            ninja.newline()

            for bin in PROJ_BINS:
                ninja.build(outputs=os.path.splitext(os.path.basename(bin))[0], rule="phony", inputs=bin)

            ninja.newline()

            ninja.build(outputs=BUILD_ALL_KEYWORD, rule="phony", inputs="Ateams")

            ninja.newline()

            ninja.default(BUILD_ALL_KEYWORD)

            ninja.newline()

            ninja.build(outputs="update", rule="generate", inputs=THIS_FILE, implicit=NINJA_BUILD_FILE)


CLICK_CONTEXT_SETTINGS = dict(ignore_unknown_options=True, help_option_names=['-h', '--help'])

@click.command(context_settings=CLICK_CONTEXT_SETTINGS)
@click.option('-c', '--compiler', type=click.STRING, required=False, default=COMPILER, prompt="Compiler", help='Compiler')
@click.option('-l', '--linker', type=click.STRING, required=False, default=LINKER, prompt="Linker", help='Linker')
@click.option('-a', '--archiver', type=click.STRING, required=False, default=ARCHIVER, prompt="Archiver", help='Archiver')
@click.option('-m', '--mode', type=click.STRING, required=False, default=BUILDING_MODE, prompt="Building Mode", help='Building Mode')
def ninja(compiler, linker, archiver, mode):
    """Simple Program That Generates NINJA Build Files"""

    generate_ninja_build_file(compiler, linker, archiver, mode)

def generate_ninja_build_file(compiler = COMPILER, linker = LINKER, archiver = ARCHIVER, mode = BUILDING_MODE):
    NinjaFileWriter.generate_ninja_build_file(ROOT_FOLDER + PATH_SEPARATOR + NINJA_BUILD_FILE, compiler, linker, archiver, mode)

if __name__ == '__main__':
    ninja()
