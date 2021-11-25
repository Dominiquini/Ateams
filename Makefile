PATH_BASE 				= ./Ateams_Base/

PATH_BINPACKING 		= ./BinPacking/
PATH_FLOWSHOP 			= ./FlowShop/
PATH_GRAPHCOLORING 		= ./GraphColoring/
PATH_JOBSHOP 			= ./JobShop/
PATH_KNAPSACK 			= ./KnapSack/
PATH_TRAVELLINGSALESMAN = ./TravellingSalesman/

BIN_BINPACKING 			= $(PATH_BINPACKING)bin/BinPacking
BIN_FLOWSHOP 			= $(PATH_FLOWSHOP)bin/FlowShop
BIN_GRAPHCOLORING 		= $(PATH_GRAPHCOLORING)bin/GraphColoring
BIN_JOBSHOP 			= $(PATH_JOBSHOP)bin/JobShop
BIN_KNAPSACK 			= $(PATH_KNAPSACK)bin/KnapSack
BIN_TRAVELLINGSALESMAN	= $(PATH_TRAVELLINGSALESMAN)bin/TravellingSalesman

EXEC_BINPACKING 		= Ateams_BinPacking
EXEC_FLOWSHOP 			= Ateams_FlowShop
EXEC_GRAPHCOLORING 		= Ateams_GraphColoring
EXEC_JOBSHOP 			= Ateams_JobShop
EXEC_KNAPSACK 			= Ateams_KnapSack
EXEC_TRAVELLINGSALESMAN	= Ateams_TravellingSalesman

EXECS =	$(EXEC_BINPACKING) $(EXEC_FLOWSHOP) $(EXEC_GRAPHCOLORING) $(EXEC_JOBSHOP) $(EXEC_KNAPSACK) $(EXEC_TRAVELLINGSALESMAN)

BINS = $(BIN_BINPACKING) $(BIN_FLOWSHOP) $(BIN_GRAPHCOLORING) $(BIN_JOBSHOP) $(BIN_KNAPSACK) $(BIN_TRAVELLINGSALESMAN)

ifeq ($(OS), Windows_NT)
  LN = ln -sf
  RM = rm -rf
else
  LN = cp
  RM = rm -rf
endif


.PHONY:				all install clean purge $(EXECS)


all:				Base
					@$(MAKE) -s BinPacking 
					@$(MAKE) -s FlowShop 
					@$(MAKE) -s GraphColoring 
					@$(MAKE) -s JobShop 
					@$(MAKE) -s KnapSack 
					@$(MAKE) -s TravellingSalesman
					@$(RM) $(EXECS)

install:			all
					@$(LN) $(BIN_BINPACKING) $(EXEC_BINPACKING)
					@$(LN) $(BIN_FLOWSHOP) $(EXEC_FLOWSHOP)
					@$(LN) $(BIN_GRAPHCOLORING) $(EXEC_GRAPHCOLORING)
					@$(LN) $(BIN_JOBSHOP) $(EXEC_JOBSHOP)
					@$(LN) $(BIN_KNAPSACK) $(EXEC_KNAPSACK)
					@$(LN) $(BIN_TRAVELLINGSALESMAN) $(EXEC_TRAVELLINGSALESMAN)

Base:
					@$(MAKE) -s -C $(PATH_BASE)

BinPacking:			$(EXEC_BINPACKING)
					@$(MAKE) -s -C $(PATH_BINPACKING)
					@$(LN) $(PATH_BINPACKING)bin/$@ $<

FlowShop:			$(EXEC_FLOWSHOP)
					@$(MAKE) -s -C $(PATH_FLOWSHOP)
					@$(LN) $(PATH_FLOWSHOP)bin/$@ $<

GraphColoring:		$(EXEC_GRAPHCOLORING)
					@$(MAKE) -s -C $(PATH_GRAPHCOLORING)
					@$(LN) $(PATH_GRAPHCOLORING)bin/$@ $<

JobShop:			$(EXEC_JOBSHOP)
					@$(MAKE) -s -C $(PATH_JOBSHOP)
					@$(LN) $(PATH_JOBSHOP)bin/$@ $<

KnapSack:			$(EXEC_KNAPSACK)
					@$(MAKE) -s -C $(PATH_KNAPSACK)
					@$(LN) $(PATH_KNAPSACK)bin/$@ $<

TravellingSalesman:	$(EXEC_TRAVELLINGSALESMAN)
					@$(MAKE) -s -C $(PATH_TRAVELLINGSALESMAN)
					@$(LN) $(PATH_TRAVELLINGSALESMAN)bin/$@ $<

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
