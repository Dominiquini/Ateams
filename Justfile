Build: Build_All

Build_All:
	./Ateams.py build

Build_BinPacking:
	./Ateams.py build -a BinPacking

Build_FlowShop:
	./Ateams.py build -a FlowShop

Build_GraphColoring:
	./Ateams.py build -a GraphColoring

Build_JobShop:
	./Ateams.py build -a JobShop

Build_KnapSack:
	./Ateams.py build -a KnapSack

Build_TravellingSalesman:
	./Ateams.py build -a TravellingSalesman


Run_BinPacking: Build_BinPacking
	./Ateams.py run -a BinPacking

Run_FlowShop: Build_FlowShop
	./Ateams.py run -a FlowShop

Run_GraphColoring: Build_GraphColoring
	./Ateams.py run -a GraphColoring

Run_JobShop: Build_JobShop
	./Ateams.py run -a JobShop

Run_KnapSack: Build_KnapSack
	./Ateams.py run -a KnapSack

Run_TravellingSalesman: Build_TravellingSalesman
	./Ateams.py run -a TravellingSalesman


Clean:
	./Ateams.py build --clean
