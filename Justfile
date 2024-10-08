[group('Tool')]
Default: Choose

[group('Tool')]
Choose:
	@just --choose --unsorted

[group('Tool')]
List:
	@just --list --unsorted


[group('Build')]
Build ALGORITHM:
	./Ateams.py build -a {{ALGORITHM}}

[group('Build')]
Build_BinPacking: (Build "BinPacking")

[group('Build')]
Build_FlowShop: (Build "FlowShop")

[group('Build')]
Build_GraphColoring: (Build "GraphColoring")

[group('Build')]
Build_JobShop: (Build "JobShop")

[group('Build')]
Build_KnapSack: (Build "KnapSack")

[group('Build')]
Build_TravellingSalesman: (Build "TravellingSalesman")

[group('Build')]
Build_All: (Build "All")

[group('Build')]
Clean:
	./Ateams.py clean

alias Purge := Clean


[group('Run')]
Run ALGORITHM: (Build ALGORITHM)
	./Ateams.py run -a {{ALGORITHM}} -p ./Ateams_Base/parameters/DEFAULT.xml -i ./Ateams_Base/inputs/{{ALGORITHM}}.prb

[group('Run')]
Run_BinPacking: (Run "BinPacking")

[group('Run')]
Run_FlowShop: (Run "FlowShop")

[group('Run')]
Run_GraphColoring: (Run "GraphColoring")

[group('Run')]
Run_JobShop: (Run "JobShop")

[group('Run')]
Run_KnapSack: (Run "KnapSack")

[group('Run')]
Run_TravellingSalesman: (Run "TravellingSalesman")
