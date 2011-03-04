PATH_BASE = ./Ateams_Base/

PATH_BINPACKING 	= ./BinPacking/
PATH_FLOWSHOP 		= ./FlowShop/
PATH_GRAPHCOLORING 	= ./GraphColoring/
PATH_JOBSHOP 		= ./JobShop/
PATH_KNAPSACK 		= ./KnapSack/
PATH_TRAVELLINGSALESMAN = ./TravellingSalesman/

BIN_BINPACKING 		= $(PATH_BINPACKING)bin/BinPacking
BIN_FLOWSHOP 		= $(PATH_FLOWSHOP)bin/FlowShop
BIN_GRAPHCOLORING 	= $(PATH_GRAPHCOLORING)bin/GraphColoring
BIN_JOBSHOP 		= $(PATH_JOBSHOP)bin/JobShop
BIN_KNAPSACK 		= $(PATH_KNAPSACK)bin/KnapSack
BIN_TRAVELLINGSALESMAN	= $(PATH_TRAVELLINGSALESMAN)bin/TravellingSalesman

EXEC_BINPACKING 	= Ateams_BinPacking
EXEC_FLOWSHOP 		= Ateams_FlowShop
EXEC_GRAPHCOLORING 	= Ateams_GraphColoring
EXEC_JOBSHOP 		= Ateams_JobShop
EXEC_KNAPSACK 		= Ateams_KnapSack
EXEC_TRAVELLINGSALESMAN	= Ateams_TravellingSalesman

EXECS =	$(EXEC_BINPACKING) $(EXEC_FLOWSHOP) $(EXEC_GRAPHCOLORING) $(EXEC_JOBSHOP) $(EXEC_KNAPSACK) $(EXEC_TRAVELLINGSALESMAN)

BINS = $(BIN_BINPACKING) $(BIN_FLOWSHOP) $(BIN_GRAPHCOLORING) $(BIN_JOBSHOP) $(BIN_KNAPSACK) $(BIN_TRAVELLINGSALESMAN)

RM = rm -f


.PHONY:			all install clean purge $(EXECS)


all:			Base
			@$(MAKE) -s BinPacking 
			@$(MAKE) -s FlowShop 
			@$(MAKE) -s GraphColoring 
			@$(MAKE) -s JobShop 
			@$(MAKE) -s KnapSack 
			@$(MAKE) -s TravellingSalesman
			@$(RM) $(EXECS)

install:		all
			@ln -fs $(BIN_BINPACKING) $(EXEC_BINPACKING)
			@ln -fs $(BIN_FLOWSHOP) $(EXEC_FLOWSHOP)
			@ln -fs $(BIN_GRAPHCOLORING) $(EXEC_GRAPHCOLORING)
			@ln -fs $(BIN_JOBSHOP) $(EXEC_JOBSHOP)
			@ln -fs $(BIN_KNAPSACK) $(EXEC_KNAPSACK)
			@ln -fs $(BIN_TRAVELLINGSALESMAN) $(EXEC_TRAVELLINGSALESMAN)

Base:
			@$(MAKE) -s -C $(PATH_BASE)

BinPacking:		$(EXEC_BINPACKING)
			@$(MAKE) -s -C $(PATH_BINPACKING)
			@ln -fs $(PATH_BINPACKING)bin/$@ $<

FlowShop:		$(EXEC_FLOWSHOP)
			@$(MAKE) -s -C $(PATH_FLOWSHOP)
			@ln -fs $(PATH_FLOWSHOP)bin/$@ $<

GraphColoring:		$(EXEC_GRAPHCOLORING)
			@$(MAKE) -s -C $(PATH_GRAPHCOLORING)
			@ln -fs $(PATH_GRAPHCOLORING)bin/$@ $<

JobShop:		$(EXEC_JOBSHOP)
			@$(MAKE) -s -C $(PATH_JOBSHOP)
			@ln -fs $(PATH_JOBSHOP)bin/$@ $<

KnapSack:		$(EXEC_KNAPSACK)
			@$(MAKE) -s -C $(PATH_KNAPSACK)
			@ln -fs $(PATH_KNAPSACK)bin/$@ $<

TravellingSalesman:	$(EXEC_TRAVELLINGSALESMAN)
			@$(MAKE) -s -C $(PATH_TRAVELLINGSALESMAN)
			@ln -fs $(PATH_TRAVELLINGSALESMAN)bin/$@ $<

clean:
			@$(MAKE) -s $@ -C $(PATH_BASE)
			@$(MAKE) -s $@ -C $(PATH_BINPACKING)
			@$(MAKE) -s $@ -C $(PATH_FLOWSHOP)
			@$(MAKE) -s $@ -C $(PATH_GRAPHCOLORING)
			@$(MAKE) -s $@ -C $(PATH_JOBSHOP)
			@$(MAKE) -s $@ -C $(PATH_KNAPSACK)
			@$(MAKE) -s $@ -C $(PATH_TRAVELLINGSALESMAN)
			@$(RM) $(EXECS)

purge:
			@$(MAKE) -s $@ -C $(PATH_BASE)
			@$(MAKE) -s $@ -C $(PATH_BINPACKING)
			@$(MAKE) -s $@ -C $(PATH_FLOWSHOP)
			@$(MAKE) -s $@ -C $(PATH_GRAPHCOLORING)
			@$(MAKE) -s $@ -C $(PATH_JOBSHOP)
			@$(MAKE) -s $@ -C $(PATH_KNAPSACK)
			@$(MAKE) -s $@ -C $(PATH_TRAVELLINGSALESMAN)
			@$(RM) $(EXECS)