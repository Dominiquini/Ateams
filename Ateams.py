#!/usr/bin/env python
# -*- coding: utf-8 -*-

import dataclasses
import collections
import subprocess
import pathlib
import shutil
import pickle
import signal
import timeit
import shutil
import glob
import math
import json
import sys
import os

import ninja

try:
    import rich_click as click
except ImportError:
    import click

try:
    from click_prompt import choice_option, confirm_option, filepath_option, auto_complete_option
except ImportError:
    choice_option = confirm_option = filepath_option = auto_complete_option = click.option


PLATFORM = collections.namedtuple('PlatformInfo', ['windows_key', 'linux_key', 'is_windows', 'is_linux', 'system'])(windows_key := "WINDOWS", linux_key := "LINUX", is_windows := sys.platform == 'win32', is_linux := not is_windows, system := windows_key if is_windows else linux_key)

PATH_SEPARATOR = os.path.sep

ROOT_FOLDER = os.path.dirname(os.path.abspath(__file__))

LAST_BUILD_INFO_FILE = os.path.basename(f"{ROOT_FOLDER}{PATH_SEPARATOR}.build_info")

MULTITHREADING_BUILDING_ENABLED = True

BUILDERS = collections.namedtuple('Builders', ['builders', 'default', 'binary'])(builders := ["make", "remake", "colormake", "colormake-short", "ninja", "samu"], default := builders[0], binary := lambda x : x)

COMPILERS = collections.namedtuple('Compilers', ['compilers', 'default', 'binary'])(compilers := ["g++", "clang++"], default := compilers[0], binary := lambda x : x)

LINKERS = collections.namedtuple('Linkers', ['linkers', 'default', 'binary'])(linkers := ["ld", "bfd", "gold", "mold", "lld"], default := linkers[0], binary := lambda x : x if x == "ld" else "ld" + '.' + x)

ARCHIVERS = collections.namedtuple('Archivers', ['archivers', 'default', 'binary'])(archivers := ["ar", "gcc-ar", "llvm-ar"], default := archivers[0], binary := lambda x : x)

BUILDING_MODES = collections.namedtuple('BuildingModes', ['modes', 'default'])(modes := ["RELEASE", "DEBUG", "PROFILE"], default := modes[0])

COMPILER_CACHE_SYSTEMS = {"ccache": "ccache {cmd}", "sccache": "sccache {cmd}", "buildcache": "buildcache {cmd}"}

DEBUGGER_COMMANDS = {"none": "{cmd}", "gdb": "gdb --args {cmd}", "lldb": "lldb -- {cmd}"}

PROFILER_COMMANDS = {"none": "{cmd}", "gprof": "{cmd} && gprof {bin} gmon.out > profile.gprof", "perf": "perf record --call-graph dwarf --quiet {bin} -- {args}", "strace": "strace -fttTyyy -s 1024 -o strace.log {bin} -- {args} "}

VALGRIND_COMMANDS = {"none": "{cmd}", "memcheck": "valgrind --tool=memcheck --leak-check=full -s {cmd}", "callgrind": "valgrind --tool=callgrind -s {cmd}", "cachegrind": "valgrind --tool=cachegrind -s {cmd}", "helgrind": "valgrind --tool=helgrind -s {cmd}", "drd": "valgrind --tool=drd -s {cmd}"}

ALLOC_OPTIONS = {"default": "{cmd}", "jemalloc": "LD_PRELOAD=/usr/lib/libjemalloc.so {cmd}", "mimalloc": "LD_PRELOAD=/usr/lib/libmimalloc.so {cmd}", "mimalloc": "LD_PRELOAD=/usr/lib/libmimalloc.so {cmd}", "snmalloc": "LD_PRELOAD=/usr/lib/libsnmallocshim.so {cmd}", "tcmalloc": "LD_PRELOAD=/usr/lib/libtcmalloc.so {cmd}"}

ALGORITHMS = ['BinPacking', 'FlowShop', 'GraphColoring', 'JobShop', 'KnapSack', 'TravellingSalesman']

MULTITHREADING_MODE = collections.namedtuple('MultithreadingModes', ['modes', 'default'])(modes := ["EXECUTOR", "THREADS"], default := modes[0])

BUILD_BASE_KEYWORD = 'Base'

BUILD_ALL_KEYWORD = 'All'

BIN_EXT = ".exe" if PLATFORM.is_windows else ""

BASE_SRCS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.cpp") if cpp.find("Ateams_Base") != -1]
BASE_HDRS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.hpp") if cpp.find("Ateams_Base") != -1]
BASE_OBJS = [src.replace("src", "bin").replace(".cpp", ".o") for src in BASE_SRCS]
BASE_LIB = f"{ROOT_FOLDER}{PATH_SEPARATOR}Ateams_Base{PATH_SEPARATOR}bin{PATH_SEPARATOR}Ateams_Base.a".replace(ROOT_FOLDER, ".")

PROJ_SRCS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.cpp") if cpp.find("Ateams_Base") == -1]
PROJ_HDRS = [cpp.replace(ROOT_FOLDER, ".") for cpp in glob.glob(f"{ROOT_FOLDER}{PATH_SEPARATOR}*{PATH_SEPARATOR}src{PATH_SEPARATOR}*.hpp") if cpp.find("Ateams_Base") == -1]
PROJ_OBJS = [src.replace("src", "bin").replace(".cpp", ".o") for src in PROJ_SRCS]
PROJ_BINS = [obj.replace(".o", BIN_EXT) for obj in PROJ_OBJS]

PATH_BINS = {A: f"{A}{PATH_SEPARATOR}bin{PATH_SEPARATOR}" for A in ALGORITHMS}
FILE_BINS = {A: f"{PATH_BINS[A]}{A}{BIN_EXT}" for A in PATH_BINS}
PATH_PARAMS = f"Ateams_Base{PATH_SEPARATOR}parameters{PATH_SEPARATOR}"
FILE_PARAMS = glob.glob(f"{PATH_PARAMS}*.xml")
PATH_DEFAULT_PARAM = f"{PATH_PARAMS}"
FILE_DEFAULT_PARAM = f"{PATH_DEFAULT_PARAM}DEFAULT.xml"
PATH_INPUTS = {A: f"{A}{PATH_SEPARATOR}inputs{PATH_SEPARATOR}" for A in ALGORITHMS}
FILE_INPUTS = {A: glob.glob(f"{PATH_INPUTS[A]}*.prb") for A in PATH_INPUTS}
PATH_DEFAULT_INPUTS = f"Ateams_Base{PATH_SEPARATOR}inputs{PATH_SEPARATOR}"
FILE_DEFAULT_INPUTS = {A: f"{PATH_DEFAULT_INPUTS}{A}.prb" for A in ALGORITHMS}
FILE_DEFAULT_INPUT_LINK = f"{PATH_DEFAULT_INPUTS}Ateams.lnk"
PATH_DEFAULT_OUTPUTS = {A: f"{A}{PATH_SEPARATOR}results{PATH_SEPARATOR}" for A in ALGORITHMS}


def check_binary(binary):
    return shutil.which(binary) is not None


def sanitize_options(**selectors):
    for selector in selectors:
        options = getattr(selectors[selector], selector)
        binary = selectors[selector].binary

        options[:] = list(filter(lambda bin: check_binary(binary(bin)), options))


def normalize_choice(options, choice, default=None, preprocessor=lambda e: e.casefold()):
    return default if choice is None else next((opt for opt in options if preprocessor(choice) in preprocessor(opt)), default)


def create_input_link(algorithm=None):
    input = pathlib.Path(FILE_DEFAULT_INPUTS.get(algorithm, FILE_DEFAULT_INPUT_LINK))
    input_link = pathlib.Path(FILE_DEFAULT_INPUT_LINK)

    input_link.unlink(missing_ok=True)

    if algorithm is not None:
        input_link.symlink_to(target=input.relative_to(input_link.parent))
    else:
        input_link.touch(exist_ok=True)


def truncate_number(number, decimals=0):
    factor = 10.0 ** decimals
    return math.trunc(number) if decimals <= 0 else math.trunc(number * factor) / factor


@dataclasses.dataclass(order=True)
class BuildInfo():

    DEFAULT_FILE = ROOT_FOLDER + PATH_SEPARATOR + LAST_BUILD_INFO_FILE

    platform: str = PLATFORM.system
    builder: str = None
    compiler: str = None
    linker: str = None
    archiver: str = None
    mode: str = None

    def generate_ninja_build_file(self):
        ninja.generate_ninja_build_file(self.compiler, self.linker, self.archiver, self.mode)

    def write_on_file(self, file=DEFAULT_FILE):
        with open(file, 'w') as info_file:
            json.dump(dataclasses.asdict(self), info_file, indent=4)

        return self

    @classmethod
    def load_from_file(cls, file=DEFAULT_FILE):
        if not os.path.isfile(file):
            return None
        else:
            with open(file, 'r') as info_file: return BuildInfo(**json.load(info_file))

    @classmethod
    def delete_file(cls, file=DEFAULT_FILE):
        if os.path.isfile(file): os.remove(file)

    @staticmethod
    def generate_default_ninja_build_file():
        ninja.generate_ninja_build_file(COMPILERS.default, LINKERS.default, ARCHIVERS.default, BUILDING_MODES.default)


CLICK_CONTEXT_SETTINGS = dict(ignore_unknown_options=True, help_option_names=['-h', '--help'])

click.pass_config = click.make_pass_decorator(Config := collections.namedtuple('LauncherConfig', ['build_info', 'execute', 'verbose', 'clear', 'pause']))


@click.group(context_settings=CLICK_CONTEXT_SETTINGS, no_args_is_help=True)
@click.option('--execute/--no-execute', default=True, help='Execute Selected Command')
@click.option('--verbose/--no-verbose', default=True, help='Print Execution Info')
@click.option('--clear/--no-clear', default=False, help='Clear Terminal')
@click.option('--pause/--no-pause', default=False, help='Pause Terminal')
@click.pass_context
def ateams(context, execute, verbose, clear, pause):
    """A Wrapper For Interacting With ATEAMS"""

    create_input_link()

    sanitize_options(builders=BUILDERS, compilers=COMPILERS, linkers=LINKERS, archivers=ARCHIVERS)

    context.obj = Config(build_info=BuildInfo.load_from_file(), execute=execute, verbose=verbose, clear=clear, pause=pause)


@ateams.command(context_settings=CLICK_CONTEXT_SETTINGS)
@choice_option('-t', '--tool', type=click.Choice(BUILDERS.builders, case_sensitive=False), required=True, default=BUILDERS.default, prompt="Building Tool", help='Building Tool')
@click.argument('extra_args', nargs=-1, type=click.UNPROCESSED)
@click.pass_config
def clean(config, tool, extra_args):
    """A Wrapper For Cleaning Previously Builded Files"""

    tool = normalize_choice(BUILDERS.builders, tool, BUILDERS.default, lambda e: e.casefold())

    config.build_info.generate_ninja_build_file() if config.build_info is not None else BuildInfo.generate_default_ninja_build_file()

    create_input_link()

    def __compose_command_line():
        command_line = tool

        command_line += f" clean" if "make" in tool else f" -t clean"

        for arg in extra_args: command_line += f" {arg}"

        return command_line

    def __clean_ateams(cmd, change_to_root_folder=True, delete_build_info=True):
        if change_to_root_folder: os.chdir(ROOT_FOLDER)

        if delete_build_info: BuildInfo.delete_file()

        return timeit.repeat(stmt=lambda: subprocess.run(cmd, shell=True), repeat=1, number=1)[0]


    if config.clear: click.clear()

    command_line = __compose_command_line()

    if config.verbose: click.echo(f"\n*** Command: '{command_line}' ***\n")

    if config.execute:
        timer = __clean_ateams(command_line)

        if config.verbose: click.echo(f"\n*** Timer: {truncate_number(timer, 2)} ***\n")

    if config.pause: click.prompt("\nPress ENTER To Exit ", prompt_suffix='', hide_input=True, show_choices=False, show_default=False, default='') ; click.echo("\n")


@ateams.command(context_settings=CLICK_CONTEXT_SETTINGS)
@choice_option('-t', '--tool', type=click.Choice(BUILDERS.builders, case_sensitive=False), required=True, default=BUILDERS.default, prompt="Building Tool", help='Building Tool')
@choice_option('-c', '--compiler', type=click.Choice(COMPILERS.compilers, case_sensitive=False), required=True, default=COMPILERS.default, prompt="Compiler", help='Compiler')
@choice_option('-l', '--linker', type=click.Choice(LINKERS.linkers, case_sensitive=False), required=True, default=LINKERS.default, prompt="Linker", help='Linker')
@choice_option('-x', '--archiver', type=click.Choice(ARCHIVERS.archivers, case_sensitive=False), required=True, default=ARCHIVERS.default, prompt="Archiver", help='Archiver')
@choice_option('-m', '--mode', type=click.Choice(BUILDING_MODES.modes, case_sensitive=False), required=True, default=BUILDING_MODES.default, prompt="Building Mode", help='Building Mode')
@choice_option('-a', '--algorithm', type=click.Choice([BUILD_ALL_KEYWORD, BUILD_BASE_KEYWORD] + ALGORITHMS, case_sensitive=False), required=True, default=BUILD_ALL_KEYWORD, prompt="Algorithm", help='Algorithm')
@confirm_option('--rebuild/--no-rebuild', type=click.BOOL, default=False, prompt="Rebuild", help='Force A Rebuild Of The Entire Project')
@confirm_option('--cache/--no-cache', type=click.BOOL, default=False, prompt="Cache", help='Use A Compiler Cache')
@click.option('--cache-system', type=click.Choice(COMPILER_CACHE_SYSTEMS.keys(), case_sensitive=False), required=False, default=next(iter(COMPILER_CACHE_SYSTEMS)), help='Compiler Cache System')
@click.argument('extra_args', nargs=-1, type=click.UNPROCESSED)
@click.pass_config
def build(config, tool, compiler, linker, archiver, mode, algorithm, rebuild, cache, cache_system, extra_args):
    """A Wrapper For Building ATEAMS With MAKE Or NINJA"""

    tool = normalize_choice(BUILDERS.builders, tool, BUILDERS.default, lambda e: e.casefold())
    compiler = normalize_choice(COMPILERS.compilers, compiler, COMPILERS.default, lambda e: e.casefold())
    linker = normalize_choice(LINKERS.linkers, linker, LINKERS.default, lambda e: e.casefold())
    archiver = normalize_choice(ARCHIVERS.archivers, archiver, ARCHIVERS.default, lambda e: e.casefold())
    mode = normalize_choice(BUILDING_MODES.modes, mode, BUILDING_MODES.default, lambda e: e.casefold())
    algorithm = normalize_choice([BUILD_ALL_KEYWORD, BUILD_BASE_KEYWORD] + ALGORITHMS, algorithm, None, lambda e: e.casefold())

    if cache: compiler = COMPILER_CACHE_SYSTEMS[cache_system].format(cmd=compiler)

    current_build = BuildInfo(builder=tool, compiler=compiler, linker=linker, archiver=archiver, mode=mode)
    current_build.generate_ninja_build_file()
 
    create_input_link(algorithm)
 
    def __update_timestamps_if_needed(current_build):
        if rebuild or current_build != config.build_info:
            for src in BASE_SRCS + PROJ_SRCS:
                pathlib.Path(src).touch()

    def __compose_command_line():
        command_line = tool

        if MULTITHREADING_BUILDING_ENABLED and "-j" not in extra_args: command_line += f" -j {os.cpu_count()}"

        command_line += f" {algorithm}"

        command_line += f" CC='{compiler}' LK='{linker}' AR='{archiver}' {mode}='true'" if "make" in tool else ""

        for arg in extra_args: command_line += f" {arg}"

        return command_line

    def __build_ateams(cmd, change_to_root_folder=True, rebuild_if_needed=True):
        if change_to_root_folder: os.chdir(ROOT_FOLDER)

        if rebuild_if_needed: __update_timestamps_if_needed(current_build)

        return timeit.repeat(stmt=lambda: subprocess.run(cmd, shell=True), repeat=1, number=1)[0]


    if config.clear: click.clear()

    command_line = __compose_command_line()

    if config.verbose: click.echo(f"\n*** Command: '{command_line}' ***\n")

    current_build.write_on_file()

    if config.execute:
        timer = __build_ateams(command_line)

        if config.verbose: click.echo(f"\n*** Timer: {truncate_number(timer, 2)} ***\n")

    if config.pause: click.prompt("\nPress ENTER To Exit ", prompt_suffix='', hide_input=True, show_choices=False, show_default=False, default='') ; click.echo("\n")


@ateams.command(context_settings=CLICK_CONTEXT_SETTINGS)
@choice_option('-a', '--algorithm', type=click.Choice(ALGORITHMS, case_sensitive=False), required=True, prompt="Algorithm:", help='Algorithm')
@filepath_option('-p', '--parameters', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=True, default=FILE_DEFAULT_PARAM, prompt="Input Parameters", help='Input Parameters File')
@filepath_option('-i', '--input', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=True, default=FILE_DEFAULT_INPUT_LINK, prompt="Input Data", help='Input Data File')
@filepath_option('-o', '--result', type=click.Path(exists=False, dir_okay=False, file_okay=True), required=False, default='', prompt="Output Result", help='Output Result File')
@filepath_option('-d', '--population', type=click.Path(exists=False, dir_okay=False, file_okay=True), required=False, default='', prompt="Output Population", help='Population File')
@choice_option('-c', '--show-cmd-info', type=click.Choice([str(c) for c in range(0, 6)]), required=False, default='0', prompt="Prompt Overview", help='Show Prompt Overview')
@confirm_option('-g', '--show-graphical-info/--hide-graphical-info', type=click.BOOL, required=False, default=False, prompt="Graphical Overview", help='Show Graphical Overview')
@confirm_option('-s', '--show-solution/--hide-solution', type=click.BOOL, required=False, default=False, prompt="Show Solution", help='Show Solution')
@confirm_option('-f', '--force-output/--ignore-output', type=click.BOOL, required=False, default=False, prompt="Write Default Output Files", help='Force Write Output Files (Result + Population)')
@choice_option('-m', '--multithreading', type=click.Choice(MULTITHREADING_MODE.modes, case_sensitive=False), required=False, default=MULTITHREADING_MODE.default, prompt="Multithreading Policy", help='Multithreading Policy')
@click.option('--repeat', type=click.INT, default=1, required=False, help='Repeat N Times')
@click.option('--debugger', type=click.Choice(DEBUGGER_COMMANDS.keys(), case_sensitive=False), required=False, help='Run With GDB')
@click.option('--profiler', type=click.Choice(PROFILER_COMMANDS.keys(), case_sensitive=False), required=False, help='Run With GPROF, PERF or STRACE')
@click.option('--valgrind', type=click.Choice(VALGRIND_COMMANDS.keys(), case_sensitive=False), required=False, help='Run With VALGRIND')
@click.option('--allocator', type=click.Choice(ALLOC_OPTIONS.keys(), case_sensitive=False), required=False, help='Run With Custom Allocator')
@click.argument('extra_args', nargs=-1, type=click.UNPROCESSED)
@click.pass_config
def run(config, algorithm, parameters, input, result, population, show_cmd_info, show_graphical_info, show_solution, force_output, multithreading, repeat, debugger, profiler, valgrind, allocator, extra_args):
    """A Wrapper For Running ATEAMS Algorithms"""

    algorithm = normalize_choice(ALGORITHMS, algorithm, None, lambda e: e.casefold())

    parameters = normalize_choice(FILE_PARAMS, parameters, FILE_DEFAULT_PARAM, lambda e: os.path.basename(e).casefold())
    input = normalize_choice([FILE_DEFAULT_INPUT_LINK, FILE_DEFAULT_INPUTS[algorithm]] + FILE_INPUTS[algorithm], input, FILE_DEFAULT_INPUTS[algorithm], lambda e: os.path.basename(e).casefold())

    if force_output:
        if not result: result = PATH_DEFAULT_OUTPUTS[algorithm] + pathlib.Path(input).with_suffix(".res").name
        if not population: population = PATH_DEFAULT_OUTPUTS[algorithm] + pathlib.Path(input).with_suffix(".pop").name

    if result: os.makedirs(os.path.dirname(result) or '.', exist_ok=True)
    if population: os.makedirs(os.path.dirname(population) or '.', exist_ok=True)

    multithreading = normalize_choice(MULTITHREADING_MODE.modes, multithreading, MULTITHREADING_MODE.default, lambda e: e.casefold())

    create_input_link(algorithm)

    def __apply_execution_modifiers(cmd, bin, args):
        if sum(1 for mode in [debugger, profiler, valgrind] if mode) > 1: raise Exception("Don't Use Multiple Tools At Same Time!")
        if debugger: cmd = DEBUGGER_COMMANDS[debugger].format(cmd=cmd, bin=bin, args=args)
        if profiler: cmd = PROFILER_COMMANDS[profiler].format(cmd=cmd, bin=bin, args=args)
        if valgrind: cmd = VALGRIND_COMMANDS[valgrind].format(cmd=cmd, bin=bin, args=args)

        return cmd

    def __run_with_custom_allocator(cmd):
        if allocator: cmd = ALLOC_OPTIONS[allocator].format(cmd=cmd)

        return cmd

    def __compose_command_line():
        executable = FILE_BINS[algorithm]

        command_line = executable

        if str(parameters): command_line += f" -p {parameters}"

        if str(input): command_line += f" -i {input}"

        if str(result): command_line += f" -o {result}"

        if str(population): command_line += f" -d {population}"

        if int(show_cmd_info): command_line += f" -{'c' * int(show_cmd_info)}"

        if bool(show_graphical_info): command_line += f" -g"

        if bool(show_solution): command_line += f" -s"

        if multithreading != MULTITHREADING_MODE.default: command_line += f" -t"

        for arg in extra_args: command_line += f" {arg}"

        command_line = __apply_execution_modifiers(command_line, FILE_BINS[algorithm], command_line.replace(FILE_BINS[algorithm], ''))
        command_line = __run_with_custom_allocator(command_line)

        return command_line

    def __run_ateams(cmd, change_to_root_folder=True):
        if change_to_root_folder: os.chdir(ROOT_FOLDER)

        return timeit.repeat(stmt=lambda: subprocess.run(cmd, shell=True), repeat=repeat, number=1)


    if config.clear: click.clear()

    command_line = __compose_command_line()

    if config.verbose: click.echo(f"\n*** Command: '{command_line}' ***\n")

    if config.execute:
        timers = __run_ateams(command_line)
        total = sum(timers)
        average = total / len(timers)

        if config.verbose: click.echo(f"\n*** Timers: {[truncate_number(n, 2) for n in timers]} || Total: {truncate_number(total, 2)} || Average: {truncate_number(average, 2)} ***\n")

    if config.pause: click.prompt("\nPress ENTER To Exit ", prompt_suffix='', hide_input=True, show_choices=False, show_default=False, default='') ; click.echo("\n")


@ateams.result_callback()
def finish(result=None, **parameters):
    click.echo(f"Finished: {result}")

    create_input_link()


if __name__ == '__main__':
    for sig in [signal.SIGINT, signal.SIGTERM]: signal.signal(sig, lambda *_: finish())

    ateams()
