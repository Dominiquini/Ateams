THIS_FILE := $(lastword $(MAKEFILE_LIST))

include ./Makefile_Base

PATH_ROOT = ./

PROJ =


.PHONY:				Ateams Base BinPacking FlowShop GraphColoring JobShop KnapSack TravellingSalesman list all base clean purge


Ateams:				Base BinPacking FlowShop GraphColoring JobShop KnapSack TravellingSalesman

Base:
					$(MAKE) -C $(PATH_BASE)

BinPacking:			Base
					$(MAKE) -C $(PATH_BINPACKING)

FlowShop:			Base
					$(MAKE) -C $(PATH_FLOWSHOP)

GraphColoring:		Base
					$(MAKE) -C $(PATH_GRAPHCOLORING)

JobShop:			Base
					$(MAKE) -C $(PATH_JOBSHOP)

KnapSack:			Base
					$(MAKE) -C $(PATH_KNAPSACK)

TravellingSalesman:	Base
					$(MAKE) -C $(PATH_TRAVELLINGSALESMAN)


list:
					@$(ECHO)  -- Ateams "\n" - Base "\n" - BinPacking "\n" - FlowShop "\n" - GraphColoring "\n" - JobShop "\n" - KnapSack "\n" - TravellingSalesman

all:				Ateams

base:				Base

clean:				COMMAND=clean
clean:				--delete

purge:				COMMAND=purge
purge:				--delete
					$(RM) $(NINJA_OUTPUTS)
					$(RM) $(PROFILE_OUTPUT)

--delete:
					$(MAKE) -s $(COMMAND) -C $(PATH_BASE)
					$(MAKE) -s $(COMMAND) -C $(PATH_BINPACKING)
					$(MAKE) -s $(COMMAND) -C $(PATH_FLOWSHOP)
					$(MAKE) -s $(COMMAND) -C $(PATH_GRAPHCOLORING)
					$(MAKE) -s $(COMMAND) -C $(PATH_JOBSHOP)
					$(MAKE) -s $(COMMAND) -C $(PATH_KNAPSACK)
					$(MAKE) -s $(COMMAND) -C $(PATH_TRAVELLINGSALESMAN)
