THIS_FILE := $(lastword $(MAKEFILE_LIST))

include ./Makefile_Base

PATH_ROOT = ./

PROJ =


.PHONY: 			Ateams Base BinPacking FlowShop GraphColoring JobShop KnapSack TravellingSalesman $(ALL) $(CLEAN) $(PURGE)


Ateams:				Base BinPacking FlowShop GraphColoring JobShop KnapSack TravellingSalesman


Base:
						$(MAKE) $@ -C $(PATH_BASE)

BinPacking:			Base
						$(MAKE) $@ -C $(PATH_BINPACKING)

FlowShop:			Base
						$(MAKE) $@ -C $(PATH_FLOWSHOP)

GraphColoring:		Base
						$(MAKE) $@ -C $(PATH_GRAPHCOLORING)

JobShop:			Base
						$(MAKE) $@ -C $(PATH_JOBSHOP)

KnapSack:			Base
						$(MAKE) $@ -C $(PATH_KNAPSACK)

TravellingSalesman:	Base
						$(MAKE) $@ -C $(PATH_TRAVELLINGSALESMAN)


$(ALL):				Ateams

$(CLEAN):			COMMAND=clean
$(CLEAN):			--delete
						$(RM) $(BUILD_STAT_BIN_FILE)

$(PURGE):			COMMAND=purge
$(PURGE):			--delete
						$(RM) $(NINJA_OUTPUTS_FILE)
						$(RM) $(BUILD_STAT_BIN_FILE)
						$(RM) $(PROFILE_OUTPUT_FILE)


--delete:
						$(MAKE) $(COMMAND) -C $(PATH_BASE)
						$(MAKE) $(COMMAND) -C $(PATH_BINPACKING)
						$(MAKE) $(COMMAND) -C $(PATH_FLOWSHOP)
						$(MAKE) $(COMMAND) -C $(PATH_GRAPHCOLORING)
						$(MAKE) $(COMMAND) -C $(PATH_JOBSHOP)
						$(MAKE) $(COMMAND) -C $(PATH_KNAPSACK)
						$(MAKE) $(COMMAND) -C $(PATH_TRAVELLINGSALESMAN)
