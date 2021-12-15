#!/usr/bin/env python
# -*- coding: utf-8 -*-

import timeit
import click
import errno
import os

ALGORITHMS = ['BinPacking', 'FlowShop', 'GraphColoring', 'JobShop', 'KnapSack', 'TravellingSalesman']

@click.command()
@click.option('-a', '--algorithm', type=click.Choice(ALGORITHMS, case_sensitive=False), required=True, prompt=True, help='Algorithm')
@click.option('-p', '--parameters', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=True, help='Input Parameters File')
@click.option('-i', '--input', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=True, help='Input Data File')
@click.option('-r', '--result', type=click.Path(exists=False, dir_okay=False, file_okay=True), help='Output Result File')
@click.option('-l', '--log', type=click.Path(exists=False, dir_okay=False, file_okay=True), help='Input/Output Log File')
@click.option('-g', '--show_info', is_flag=True, default=False, help='Show Graphical Overview')
@click.option('-n', '--repeat', type=click.INT, default=1, help='Repeat Number Of Times')
def Ateams(algorithm, parameters, input, result, log, show_info, repeat):

    def mkdir(file):
        try:
            os.makedirs(os.path.dirname(file))
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    
    def buildCmd(algorithm, parameters, input, result, log, show_info):
        commandLine = f"{os.getcwd()}/{algorithm}/bin/{algorithm}"

        commandLine += f" -p {parameters} -i {input}"

        if(result != None):
            mkdir(result)
            commandLine += f" -r {result}"

        if(log != None):
            mkdir(log)
            commandLine += f" -l {log}"

        if(show_info):
            commandLine += f" -g"
        
        return commandLine

    
    def execute(cmd, repeat=1):
        return timeit.repeat(stmt=lambda: os.system(cmd), repeat=repeat, number=1)

    cmd = buildCmd(algorithm, parameters, input, result, log, show_info)

    click.echo(f"\n*** Command: '{cmd}' ***\n")

    time = execute(cmd, repeat)

    click.echo(f"\n*** Timers: {time} ***\n")


if __name__ == '__main__':
    Ateams()
