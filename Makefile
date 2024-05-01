THIS_FILE := $(lastword $(MAKEFILE_LIST))

include ./Makefile_Base

PATH_ROOT = ./

PROJ =


.PHONY:=			Ateams Base BinPacking FlowShop GraphColoring JobShop KnapSack TravellingSalesman list all base clean purge

.DEFAULT_GOAL:=		Ateams


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


all:				Ateams


clean:				COMMAND=clean
clean:				--delete
					$(RM) $(BUILD_STAT_BIN_FILE)

purge:				COMMAND=purge
purge:				--delete
					$(RM) $(NINJA_OUTPUTS_FILE)
					$(RM) $(BUILD_STAT_BIN_FILE)
					$(RM) $(PROFILE_OUTPUT_FILE)

--delete:
					$(MAKE) -s $(COMMAND) -C $(PATH_BASE)
					$(MAKE) -s $(COMMAND) -C $(PATH_BINPACKING)
					$(MAKE) -s $(COMMAND) -C $(PATH_FLOWSHOP)
					$(MAKE) -s $(COMMAND) -C $(PATH_GRAPHCOLORING)
					$(MAKE) -s $(COMMAND) -C $(PATH_JOBSHOP)
					$(MAKE) -s $(COMMAND) -C $(PATH_KNAPSACK)
					$(MAKE) -s $(COMMAND) -C $(PATH_TRAVELLINGSALESMAN)
