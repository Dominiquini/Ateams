#!/usr/bin/env python
# -*- coding: utf-8 -*-

import timeit
import click
import time
import math
import os

class Ateams:

    DEFAULT_PARAM = f"{os.getcwd()}/Ateams_Base/parameters/DEFAULT.xml"

    @staticmethod
    def truncate(number, decimals=0):
        factor = 10.0 ** decimals
        if decimals <= 0:
            return math.trunc(number)
        else:
            return math.trunc(number * factor) / factor

    @classmethod
    def run(cls, ateams, algorithm, print=True, formatter=lambda t: Ateams.truncate(t, 2)):
        ateams.__run(algorithm, print, formatter)

    def __init__(self, parameters, input, result, log, show_info, repeat):
        self.parameters = parameters
        self.input = input
        self.result = result
        self.log = log
        self.show_info = show_info
        self.repeat = repeat

    def __buildCommandLine(self, algorithm):
        commandLine = f"{os.getcwd()}/{algorithm}/bin/{algorithm}"

        commandLine += f" -p {self.parameters} -i {self.input}"

        if(self.result != None): commandLine += f" -r {self.result}"

        if(self.log != None): commandLine += f" -l {self.log}"

        if(self.show_info): commandLine += f" -g"

        return commandLine

    def __executeAteams(self, cmd, repeat=1):
        return timeit.repeat(stmt=lambda: os.system(cmd), repeat=repeat, number=1)

    def __run(self, algorithm, print, formatter):
        commandLine = self.__buildCommandLine(algorithm)

        if(print):  click.echo(f"\n*** Command: '{commandLine}' ***\n")

        timers = self.__executeAteams(commandLine, self.repeat)

        if(print): click.echo(f"\n*** Timers: {[formatter(n) for n in timers]} ***\n")


def validate_path(ctx: click.core.Context, param: click.core.Option, value: click.Path):
    try:
        os.makedirs(os.path.dirname(value))
    finally:
        return value


@click.group(context_settings=dict(help_option_names=['-h', '--help']))
@click.option('-p', '--parameters', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=True, default=Ateams.DEFAULT_PARAM, callback=validate_path, help='Input Parameters File')
@click.option('-i', '--input', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=True, callback=validate_path, help='Input Data File')
@click.option('-r', '--result', type=click.Path(exists=False, dir_okay=False, file_okay=True), callback=validate_path, help='Output Result File')
@click.option('-l', '--log', type=click.Path(exists=False, dir_okay=False, file_okay=True), callback=validate_path, help='Working Log File')
@click.option('-g', '--show_info', type=click.BOOL, is_flag=True, default=False, help='Show Graphical Overview')
@click.option('-n', '--repeat', type=click.INT, default=1, help='Repeat Number Of Times')
@click.pass_context
def base(ctx: click.core.Context, parameters, input, result, log, show_info, repeat):
    ctx.obj = Ateams(parameters, input, result, log, show_info, repeat)

@base.command(name="BinPacking")
@click.pass_context
def bin_packing(ctx: click.core.Context):
    Ateams.run(ctx.obj, ctx.info_name)

@base.command(name="FlowShop")
@click.pass_context
def flow_shop(ctx: click.core.Context):
    Ateams.run(ctx.obj, ctx.info_name)

@base.command(name="GraphColoring")
@click.pass_context
def graph_coloring(ctx: click.core.Context):
    Ateams.run(ctx.obj, ctx.info_name)

@base.command(name="JobShop")
@click.pass_context
def job_shop(ctx: click.core.Context):
    Ateams.run(ctx.obj, ctx.info_name)

@base.command(name="KnapSack")
@click.pass_context
def knap_sack(ctx: click.core.Context):
    Ateams.run(ctx.obj, ctx.info_name)

@base.command(name="TravellingSalesman")
@click.pass_context
def travelling_salesman(ctx: click.core.Context):
    Ateams.run(ctx.obj, ctx.info_name)

if __name__ == '__main__':
    base()
