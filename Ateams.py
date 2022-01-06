#!/usr/bin/env python
# -*- coding: utf-8 -*-

import click
import builtins
import pathlib
import signal
import timeit
import time
import math
import os

ATEAMS_PATH_PLACEHOLDER = "#ATEAMS_PATH#"

GDB_COMMAND = "gdb --args"

VALGRIND_COMMAND = "valgrind --leak-check=full -s"

ALGORITHMS = ['BinPacking', 'FlowShop', 'GraphColoring', 'JobShop', 'KnapSack', 'TravellingSalesman']

BINARIES = {A: f"{ATEAMS_PATH_PLACEHOLDER}/{A}/bin/{A}" for A in ALGORITHMS}

DEFAULT_PARAM = f"{ATEAMS_PATH_PLACEHOLDER}/Ateams_Base/parameters/DEFAULT.xml"

DEFAULT_INPUTS = {A: f"{ATEAMS_PATH_PLACEHOLDER}/{A}/input/{I}" for (A, I) in zip(ALGORITHMS, ['binpack1_01.prb', 'car1.prb', 'jean.prb', 'la01.prb', 'mk_gk01.prb', 'br17.prb'])}

DEFAULT_OUTPUTS_PATH = {A: f"{ATEAMS_PATH_PLACEHOLDER}/{A}/results/" for A in ALGORITHMS}

def ateams_path(path=ATEAMS_PATH_PLACEHOLDER, placeholder=ATEAMS_PATH_PLACEHOLDER):
    return path.replace(placeholder, os.path.dirname(os.path.abspath(__file__)))

def validate_path(ctx: click.core.Context=None, param: click.core.Option=None, value: click.Path=None):
    try:
        os.makedirs(os.path.dirname(value))
    finally:
        return value

@click.command(context_settings=dict(ignore_unknown_options=True, help_option_names=['-h', '--help']))
@click.option('-a', '--algorithm', type=click.Choice(ALGORITHMS, case_sensitive=False), required=True, prompt="Algorithm: ", help='Algorithm')
@click.option('-p', '--parameters', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Input Parameters File')
@click.option('-i', '--input', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Input Data File')
@click.option('-r', '--result', type=click.Path(exists=False, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Output Result File')
@click.option('-t', '--pop', type=click.Path(exists=False, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Population File')
@click.option('-o', '--write_output', type=click.BOOL, is_flag=True, help='Force Write Output Files')
@click.option('-c', '--show_cmd_info', type=click.BOOL, is_flag=True, help='Show Prompt Overview')
@click.option('-g', '--show_graphical_info', type=click.BOOL, is_flag=True, help='Show Graphical Overview')
@click.option('-s', '--show_solution', type=click.BOOL, is_flag=True, help='Show Solution')
@click.option('-n', '--repeat', type=click.INT, default=1, help='Repeat N Times')
@click.option('-d', '--debug', type=click.BOOL, is_flag=True, help='Run With GDB')
@click.option('-m', '--memory', type=click.BOOL, is_flag=True, help='Run With Valgrind')
@click.argument('extra_args', nargs=-1, type=click.UNPROCESSED)
def ateams(algorithm, parameters, input, result, pop, write_output, show_cmd_info, show_graphical_info, show_solution, repeat, debug, memory, extra_args):
    """A Wrapper For Ateams"""
    
    def __truncate(number, decimals=0):
        factor = 10.0 ** decimals

        return math.trunc(number) if decimals <= 0 else math.trunc(number * factor) / factor

    def __evaluate_output_files(extension):
        filename = pathlib.Path(input).stem

        return validate_path(value=ateams_path(DEFAULT_OUTPUTS_PATH[algorithm] + filename + '.' + extension))
    
    def __build_command_line():       
        command_line = BINARIES[algorithm]

        if(parameters is not None): command_line += f" -p {parameters}"
        
        if(input is not None): command_line += f" -i {input}"
        
        if(result is not None): command_line += f" -r {result}"

        if(pop is not None): command_line += f" -t {pop}"

        if(show_cmd_info): command_line += f" -c"

        if(show_graphical_info): command_line += f" -g"

        if(show_solution): command_line += f" -s"

        for arg in extra_args:
            command_line += f" {arg}"

        if(debug): command_line = GDB_COMMAND + " " + command_line
        
        if(memory): command_line = VALGRIND_COMMAND + " " + command_line

        return ateams_path(command_line)

    def __execute_ateams(cmd, repeat=1):
        return timeit.repeat(stmt=lambda: os.system(cmd), repeat=repeat, number=1)

    click.clear()

    algorithm = next((a for a in ALGORITHMS if a.casefold() == algorithm.casefold()), algorithm)
    if(parameters is None): parameters = DEFAULT_PARAM
    if(input is None): input = DEFAULT_INPUTS[algorithm]
    if(result is None and write_output): result = __evaluate_output_files("res")
    if(pop is None and write_output): pop = __evaluate_output_files("pop")

    command_line = __build_command_line()

    if(print):  click.echo(f"\n*** Command: '{command_line}' ***\n")

    timers = __execute_ateams(command_line, repeat)
    average = sum(timers) / len(timers)

    if(print): click.echo(f"\n*** Timers: {[__truncate(n, 2) for n in timers]} || Average: {__truncate(average, 2)} ***\n")
    
    click.prompt("\nPress ENTER To Exit", prompt_suffix='', hide_input=True, show_choices=False, show_default=False, default='')
    click.echo("\n")
    

if __name__ == '__main__':
    for sig in [signal.SIGINT, signal.SIGTERM]:
        signal.signal(sig, lambda *_: None)

    ateams()
