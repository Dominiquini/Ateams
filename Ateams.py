#!/usr/bin/env python
# -*- coding: utf-8 -*-

import click
import timeit
import time
import math
import os
    
ALGORITHMS = ['BinPacking', 'FlowShop', 'GraphColoring', 'JobShop', 'KnapSack', 'TravellingSalesman']

DEFAULT_PARAM = f"{os.getcwd()}/Ateams_Base/parameters/DEFAULT.xml"

DEFAULT_INPUTS = {A: f"{os.getcwd()}/{A}/input/{I}" for (A, I) in zip(ALGORITHMS, ['binpack1_01.prb', 'car1.prb', 'jean.prb', 'la01.prb', 'mk_gk01.prb', 'br17.prb'])}

def validate_path(ctx: click.core.Context, param: click.core.Option, value: click.Path):
    try:
        os.makedirs(os.path.dirname(value))
    finally:
        return value

@click.command(context_settings=dict(help_option_names=['-h', '--help']))
@click.option('-a', '--algorithm', type=click.Choice(ALGORITHMS, case_sensitive=False), required=True, prompt="Algorithm: ", help='Algorithm')
@click.option('-p', '--parameters', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=False, default=DEFAULT_PARAM, callback=validate_path, help='Input Parameters File')
@click.option('-i', '--input', type=click.Path(exists=False, dir_okay=False, file_okay=True), required=False, default=None, callback=validate_path, help='Input Data File')
@click.option('-r', '--result', type=click.Path(exists=False, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Output Result File')
@click.option('-l', '--log', type=click.Path(exists=False, dir_okay=False, file_okay=True), required=False, callback=validate_path, help='Working Log File')
@click.option('-g', '--show_info', type=click.BOOL, is_flag=True, default=False, help='Show Graphical Overview')
@click.option('-s', '--show_solution', type=click.BOOL, is_flag=True, default=False, help='Show Solution')
@click.option('-n', '--repeat', type=click.INT, default=1, help='Repeat Number Of Times')
def ateams(algorithm, parameters, input, result, log, show_info, show_solution, repeat):
    
    def __truncate(number, decimals=0):
        factor = 10.0 ** decimals
        if decimals <= 0:
            return math.trunc(number)
        else:
            return math.trunc(number * factor) / factor
    
    def __buildCommandLine():
        commandLine = f"{os.getcwd()}/{algorithm}/bin/{algorithm}"

        commandLine += f" -p {parameters}"
        
        commandLine += f" -i {input if input is not None else DEFAULT_INPUTS[algorithm]}"

        if(result != None): commandLine += f" -r {result}"

        if(log != None): commandLine += f" -l {log}"

        if(show_info): commandLine += f" -g"

        if(show_solution): commandLine += f" -s"

        return commandLine

    def __executeAteams(cmd, repeat=1):
        return timeit.repeat(stmt=lambda: os.system(cmd), repeat=repeat, number=1)


    commandLine = __buildCommandLine()

    if(print):  click.echo(f"\n*** Command: '{commandLine}' ***\n")

    timers = __executeAteams(commandLine, repeat)

    if(print): click.echo(f"\n*** Timers: {[__truncate(n, 2) for n in timers]} ***\n")


if __name__ == '__main__':
    ateams()
