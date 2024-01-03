#!/usr/bin/env python
# -*- coding: utf-8 -*-

import collections
import subprocess
import pathlib
import pickle
import signal
import timeit
import glob
import math
import sys
import os

import click
import ninja_syntax

PLATFORM = collections.namedtuple('PlatformInfo', ['windows_key', 'linux_key', 'is_windows', 'is_linux', 'system'])(windows_key := "WINDOWS", linux_key := "LINUX", is_windows := sys.platform == 'win32', is_linux := not is_windows, system := windows_key if is_windows else linux_key)

PATH_SEPARATOR = os.path.sep

ROOT_FOLDER = os.path.dirname(os.path.abspath(__file__))

THIS_FILE = os.path.basename(__file__)

LAST_BUILD_INFO_FILE = os.path.basename(f"{ROOT_FOLDER}{PATH_SEPARATOR}.build_info")

NINJA_BUILD_FILE = os.path.basename(f"{ROOT_FOLDER}{PATH_SEPARATOR}build.ninja")

MULTITHREADING_BUILDING_ENABLED = True

BUILD_TOOLS = collections.namedtuple('BuildTools', ['tools', 'default'])(tools := ["make", "ninja"], default := tools[0])

BUILDING_MODES = collections.namedtuple('BuildModes', ['modes', 'default'])(modes := ["RELEASE", "DEBUG", "PROFILE"], default := modes[0])

CXXFLAGS = {BUILDING_MODES.modes[0]: "-std=c++17 -static-libstdc++ -pthread -O3 -ffast-math -march=native -mtune=native", BUILDING_MODES.modes[1]: "-std=c++17 -static-libstdc++ -pthread -O0 -g3 -no-pie -march=native -mtune=native", BUILDING_MODES.modes[2]: "-std=c++17 -static-libstdc++ -pthread -O0 -g3 -pg -no-pie -march=native -mtune=native"}

LDFLAGS = {PLATFORM.windows_key: "-lopengl32 -lGLU32 -lfreeglut", PLATFORM.linux_key: "-lGL -lGLU -lglut"}

ARFLAGS = "-crs"

POOL_ASYNC = collections.namedtuple('NinjaPool', ['name', 'depth'])("threaded", 16)

BIN_EXT = ".exe" if PLATFORM.is_windows else ""

BASE_SRCS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.cpp") if cpp.find("Ateams_Base") != -1]
BASE_OBJS = [src.replace("src", "bin").replace(".cpp", ".o") for src in BASE_SRCS]
BASE_LIB = f"{ROOT_FOLDER}{PATH_SEPARATOR}Ateams_Base{PATH_SEPARATOR}bin{PATH_SEPARATOR}Ateams_Base.a".replace(ROOT_FOLDER, ".")

PROJ_SRCS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.cpp") if cpp.find("Ateams_Base") == -1]
PROJ_OBJS = [src.replace("src", "bin").replace(".cpp", ".o") for src in PROJ_SRCS]
PROJ_BINS = [obj.replace(".o", BIN_EXT) for obj in PROJ_OBJS]

GDB_COMMAND = "gdb --args {cmd}"

GPROF_COMMAND = "{cmd} && gprof {bin} gmon.out > profile.txt"

VALGRIND_COMMANDS = {"memcheck": "valgrind --tool=memcheck --leak-check=full -s {cmd}", "callgrind": "valgrind --tool=callgrind -s {cmd}", "cachegrind": "valgrind --tool=cachegrind -s {cmd}", "helgrind": "valgrind --tool=helgrind -s {cmd}", "drd": "valgrind --tool=drd -s {cmd}"}

ALGORITHMS = ['BinPacking', 'FlowShop', 'GraphColoring', 'JobShop', 'KnapSack', 'TravellingSalesman']

BUILD_ALL_KEYWORD = 'all'

FILE_BINS = {A: f"{ROOT_FOLDER}{PATH_SEPARATOR}{A}{PATH_SEPARATOR}bin{PATH_SEPARATOR}{A}{BIN_EXT}" for A in ALGORITHMS}
FILE_PARAMS = glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}Ateams_Base{PATH_SEPARATOR}parameters{PATH_SEPARATOR}*.xml")
FILE_DEFAULT_PARAM = f"{ROOT_FOLDER}{PATH_SEPARATOR}Ateams_Base{PATH_SEPARATOR}parameters{PATH_SEPARATOR}DEFAULT.xml"
FILE_INPUTS = {A: glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}{A}{PATH_SEPARATOR}inputs{PATH_SEPARATOR}*.prb") for A in ALGORITHMS}
FILE_DEFAULT_INPUTS = {A: f"{ROOT_FOLDER}{PATH_SEPARATOR}{A}{PATH_SEPARATOR}inputs{PATH_SEPARATOR}{I}" for (A, I) in zip(ALGORITHMS, ['binpack1_01.prb', 'car1.prb', 'jean.prb', 'la01.prb', 'mk_gk01.prb', 'br17.prb'])}
PATH_DEFAULT_OUTPUTS = {A: f"{ROOT_FOLDER}{PATH_SEPARATOR}{A}{PATH_SEPARATOR}results{PATH_SEPARATOR}" for A in ALGORITHMS}


class NinjaFileWriter(ninja_syntax.Writer):

    def __init__(self, path=NINJA_BUILD_FILE, mode="w", width=512):
        super().__init__(output=open(path, mode), width=width)

    def __enter__(self):
        return self

    def __exit__(self, *exception):
        super().close()


class BuildInfo:

    def __init__(self, build_tool, build_mode):
        self.platform = PLATFORM.system
        self.build_tool = build_tool
        self.build_mode = build_mode

    def __str__(self):
        return f"({self.platform}, {self.build_tool}, {self.build_mode})"

    def write_on_file(self, file):
        with open(file, 'wb') as info_file:
            pickle.dump(self, info_file, pickle.DEFAULT_PROTOCOL)

        return self

    @classmethod
    def load_from_file(cls, file):
        if not os.path.exists(file):
            return None
        else:
            with open(file, 'rb') as info_file: return pickle.load(info_file)

    @staticmethod
    def compare(info1, info2):
        return info1 is None or info2 is None or info1.platform != info2.platform or info1.build_tool != info2.build_tool or info1.build_mode != info2.build_mode


def normalize_choice(options, choice, default=None, preprocessor=lambda e: e.casefold()):
    return default if choice is None else next((opt for opt in options if preprocessor(choice) in preprocessor(opt)), default)


def truncate_number(number, decimals=0):
    factor = 10.0 ** decimals
    return math.trunc(number) if decimals <= 0 else math.trunc(number * factor) / factor


def validate_path(ctx: click.core.Context=None, param: click.core.Option=None, value: click.Path=None):
    try: os.makedirs(os.path.dirname(value))
    finally: return value


click.pass_config = click.make_pass_decorator(Config := collections.namedtuple('LauncherConfig', ['execute', 'verbose', 'clear', 'pause']))


@click.group(context_settings=dict(ignore_unknown_options=True, help_option_names=['-h', '--help']))
@click.option('--execute/--no-execute', default=True, help='Execute Selected Command')
@click.option('--verbose/--no-verbose', default=True, help='Print Execution Info')
@click.option('--clear/--no-clear', default=False, help='Clear Terminal')
@click.option('--pause/--no-pause', default=False, help='Pause Terminal')
@click.pass_context
def ateams(ctx, execute, verbose, clear, pause):
    ctx.obj = Config(execute, verbose, clear, pause)


@ateams.command(context_settings=dict(ignore_unknown_options=True, help_option_names=['-h', '--help']))
@click.option('-t', '--tool', type=click.Choice(BUILD_TOOLS.tools, case_sensitive=False), required=True, default=BUILD_TOOLS.default, help='Building Tool')
@click.option('-a', '--algorithm', type=click.Choice([BUILD_ALL_KEYWORD] + ALGORITHMS, case_sensitive=False), required=True, default='all', help='Algorithm')
@click.option('-m', '--mode', type=click.Choice(BUILDING_MODES.modes, case_sensitive=False), required=True, default=BUILDING_MODES.default, help='Building Mode')
@click.option('--rebuild', type=click.BOOL, is_flag=True, help='Force A Rebuild')
@click.option('--clean', type=click.BOOL, is_flag=True, help='Remove Build Files')
@click.argument('extra_args', nargs=-1, type=click.UNPROCESSED)
@click.pass_config
def build(config, tool, algorithm, mode, rebuild, clean, extra_args):
    """A Wrapper For Building ATEAMS With MAKE Or NINJA"""

    tool = normalize_choice(BUILD_TOOLS.tools, tool, BUILD_TOOLS.default, lambda e: e.casefold())
    mode = normalize_choice(BUILDING_MODES.modes, mode, BUILDING_MODES.default, lambda e: e.casefold())

    def __generate_ninja_build_file():
        ninja_file = ROOT_FOLDER + PATH_SEPARATOR + NINJA_BUILD_FILE

        with NinjaFileWriter(ninja_file) as ninja:
            ninja.comment(f"PLATFORM: {PLATFORM.system}")

            ninja.newline()

            ninja.variable("BUILD_MODE", mode)
            ninja.variable("CXXFLAGS", CXXFLAGS[mode])
            ninja.variable("LDFLAGS", LDFLAGS[PLATFORM.system])
            ninja.variable("ARFLAGS", ARFLAGS)

            ninja.newline()

            ninja.pool(**POOL_ASYNC._asdict())

            ninja.newline()

            ninja.rule(name="compile", command="g++ $CXXFLAGS -MMD -MF $out.d -c -o $out $in", description="COMPILE $out", deps="gcc", depfile="$out.d", pool=POOL_ASYNC.name)

            ninja.newline()

            ninja.rule(name="link", command="g++ $CXXFLAGS -o $out $in $LDFLAGS", description="LINK $out", pool=POOL_ASYNC.name)

            ninja.newline()

            ninja.rule(name="archive", command="ar $ARFLAGS $out $in", description="ARCHIVE $out", pool=POOL_ASYNC.name)

            ninja.newline()

            ninja.rule(name="generate", command="python $in --no-execute --no-verbose --no-clear --no-pause build -a ninja -m $BUILD_MODE", description="UPDATE NINJA BUILD FILE", pool=POOL_ASYNC.name, generator=True)

            ninja.newline()

            for src, obj in zip(BASE_SRCS, BASE_OBJS):
                ninja.build(outputs=obj, rule="compile", inputs=src)

            ninja.newline()

            ninja.build(outputs=BASE_LIB, rule="archive", inputs=BASE_OBJS)

            ninja.newline()

            for src, obj in zip(PROJ_SRCS, PROJ_OBJS):
                ninja.build(outputs=obj, rule="compile", inputs=src)

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

    def __update_timestamps_if_needed():
        file = ROOT_FOLDER + PATH_SEPARATOR + LAST_BUILD_INFO_FILE

        current_build = BuildInfo(tool, mode)

        if rebuild or BuildInfo.compare(current_build, BuildInfo.load_from_file(file)):
            for src in BASE_SRCS + PROJ_SRCS:
                pathlib.Path(src).touch()

        current_build.write_on_file(file)

    def __compose_command_line():
        command_line = tool

        if MULTITHREADING_BUILDING_ENABLED and "-j" not in extra_args: command_line += f" -j {os.cpu_count()}"

        command_line += f" {algorithm}" if not clean else f" purge" if tool == "make" else f" -t clean"

        command_line += f" {mode}=true" if tool == "make" else ""

        for arg in extra_args: command_line += f" {arg}"

        return command_line

    def __build_ateams(cmd, change_to_root_folder=True, rebuild_if_needed=True):
        if change_to_root_folder: os.chdir(ROOT_FOLDER)

        if rebuild_if_needed: __update_timestamps_if_needed()

        return timeit.repeat(stmt=lambda: subprocess.run(cmd, shell=True), repeat=1, number=1)[0] if config.execute else 0

    __generate_ninja_build_file()

    if config.clear: click.clear()

    command_line = __compose_command_line()

    if config.verbose: click.echo(f"\n*** Command: '{command_line}' ***\n")

    timer = __build_ateams(command_line)

    if config.verbose: click.echo(f"\n*** Timer: {truncate_number(timer, 2)} ***\n")

    if config.pause: click.prompt("\nPress ENTER To Exit ", prompt_suffix='', hide_input=True, show_choices=False, show_default=False, default='') ; click.echo("\n")


@ateams.command(context_settings=dict(ignore_unknown_options=True, help_option_names=['-h', '--help']))
@click.option('-a', '--algorithm', type=click.Choice(ALGORITHMS, case_sensitive=False), required=True, prompt="Algorithm:", help='Algorithm')
@click.option('-p', '--parameters', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Input Parameters File')
@click.option('-i', '--input', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Input Data File')
@click.option('-r', '--result', type=click.Path(exists=False, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Output Result File')
@click.option('-o', '--pop', type=click.Path(exists=False, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Population File')
@click.option('-c', '--show-cmd-info', type=click.IntRange(0, 5), count=True, help='Show Prompt Overview')
@click.option('-g', '--show-graphical-info', type=click.BOOL, is_flag=True, help='Show Graphical Overview')
@click.option('-s', '--show-solution', type=click.BOOL, is_flag=True, help='Show Solution')
@click.option('--executor/--threads', default=True, help='Executor or Threads')
@click.option('-n', '--repeat', type=click.INT, default=1, help='Repeat N Times')
@click.option('--output', type=click.BOOL, is_flag=True, help='Force Write Output Files')
@click.option('--debug', type=click.BOOL, is_flag=True, help='Run With GDB')
@click.option('--profile', type=click.BOOL, is_flag=True, help='Run With GPROF')
@click.option('--valgrind', type=click.Choice(VALGRIND_COMMANDS.keys(), case_sensitive=False), required=False, help='Run With VALGRIND')
@click.argument('extra_args', nargs=-1, type=click.UNPROCESSED)
@click.pass_config
def run(config, algorithm, parameters, input, result, pop, show_cmd_info, show_graphical_info, show_solution, executor, repeat, output, debug, profile, valgrind, extra_args):
    """A Wrapper For Running ATEAMS Algorithms"""

    algorithm = normalize_choice(ALGORITHMS, algorithm, None, lambda e: e.casefold())

    parameters = normalize_choice(FILE_PARAMS, parameters, FILE_DEFAULT_PARAM, lambda e: os.path.basename(e).casefold())
    input = normalize_choice(FILE_INPUTS[algorithm], input, FILE_DEFAULT_INPUTS[algorithm], lambda e: os.path.basename(e).casefold())

    if result is None and output: result = validate_path(value=PATH_DEFAULT_OUTPUTS[algorithm] + pathlib.Path(input).with_suffix(".res").name)
    if pop is None and output: pop = validate_path(value=PATH_DEFAULT_OUTPUTS[algorithm] + pathlib.Path(input).with_suffix(".pop").name)

    def __apply_execution_modifiers(cmd, bin):
        if sum(1 for mode in [debug, profile, valgrind] if mode) > 1: raise Exception("Don't Use GDB, VALGRIND and GPROF At Same Time!")
        if debug: cmd = GDB_COMMAND.format(cmd=cmd)
        if profile: cmd = GPROF_COMMAND.format(cmd=cmd, bin=bin)
        if valgrind: cmd = VALGRIND_COMMANDS[valgrind].format(cmd=cmd)

        return cmd

    def __compose_command_line():
        command_line = FILE_BINS[algorithm]

        if parameters is not None: command_line += f" -p {parameters}"

        if input is not None: command_line += f" -i {input}"

        if result is not None: command_line += f" -r {result}"

        if pop is not None: command_line += f" -o {pop}"

        if show_cmd_info: command_line += f" -{'c' * show_cmd_info}"

        if show_graphical_info: command_line += f" -g"

        if show_solution: command_line += f" -s"

        if not executor: command_line += f" -t"

        for arg in extra_args: command_line += f" {arg}"

        command_line = __apply_execution_modifiers(command_line, FILE_BINS[algorithm])

        return command_line

    def __execute_ateams(cmd, change_to_root_folder=True):
        if change_to_root_folder: os.chdir(ROOT_FOLDER)

        return timeit.repeat(stmt=lambda: subprocess.run(cmd, shell=True), repeat=repeat, number=1) if config.execute else [0] * repeat

    if config.clear: click.clear()

    command_line = __compose_command_line()

    if config.verbose: click.echo(f"\n*** Command: '{command_line}' ***\n")

    timers = __execute_ateams(command_line)
    total = sum(timers)
    average = total / len(timers)

    if config.verbose: click.echo(f"\n*** Timers: {[truncate_number(n, 2) for n in timers]} || Total: {truncate_number(total, 2)} || Average: {truncate_number(average, 2)} ***\n")

    if config.pause: click.prompt("\nPress ENTER To Exit ", prompt_suffix='', hide_input=True, show_choices=False, show_default=False, default='') ; click.echo("\n")


if __name__ == '__main__':
    for sig in [signal.SIGINT, signal.SIGTERM]: signal.signal(sig, lambda *_: None)

    ateams()
