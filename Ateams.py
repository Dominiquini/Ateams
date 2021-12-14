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
def Ateams(algorithm, parameters, input, result, log, show_info):

    def mkdir(file):
        try:
            os.makedirs(os.path.dirname(file))
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    
    def buildCmd(algorithm, parameters, input, result, log, show_info):
        commandLine = f"{os.getcwd()}/{algorithm}/bin/{algorithm} "

        commandLine += f"-p {parameters} -i {input} "

        if(result != None):
            mkdir(result)
            commandLine += f"-r {result} "

        if(log != None):
            mkdir(log)
            commandLine += f"-l {log} "

        if(show_info):
            commandLine += f"-g "
        
        return commandLine

    
    def execute(cmd, times=1):
        return timeit.timeit(stmt=lambda: os.system(cmd), number=times)

    cmd = buildCmd(algorithm, parameters, input, result, log, show_info)

    click.echo(f"\n*** Command: '{cmd}' ***\n")

    time = execute(cmd)

    click.echo(f"\n*** Execution Time: {time} ***\n")


if __name__ == '__main__':
    Ateams()
