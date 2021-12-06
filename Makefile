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

BINS = $(BIN_BINPACKING) $(BIN_FLOWSHOP) $(BIN_GRAPHCOLORING) $(BIN_JOBSHOP) $(BIN_KNAPSACK) $(BIN_TRAVELLINGSALESMAN)

NINJA_OUTPUTS = .ninja_deps .ninja_log

RM = rm -rf


.PHONY:				list all clean purge


all:				Base BinPacking FlowShop GraphColoring JobShop KnapSack TravellingSalesman

Base:
					@$(MAKE) -C $(PATH_BASE)

BinPacking:			Base
					@$(MAKE) -C $(PATH_BINPACKING)

FlowShop:			Base
					@$(MAKE) -C $(PATH_FLOWSHOP)

GraphColoring:		Base
					@$(MAKE) -C $(PATH_GRAPHCOLORING)
					
JobShop:			Base
					@$(MAKE) -C $(PATH_JOBSHOP)

KnapSack:			Base
					@$(MAKE) -C $(PATH_KNAPSACK)

TravellingSalesman:	Base
					@$(MAKE) -C $(PATH_TRAVELLINGSALESMAN)
					
list:
					@LC_ALL=C $(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$'

clean:				COMMAND=clean
clean:				--delete

purge:				COMMAND=purge
purge:				--delete
					@$(RM) $(NINJA_OUTPUTS)

--delete:
					@$(MAKE) -s $(COMMAND) -C $(PATH_BASE)
					@$(MAKE) -s $(COMMAND) -C $(PATH_BINPACKING)
					@$(MAKE) -s $(COMMAND) -C $(PATH_FLOWSHOP)
					@$(MAKE) -s $(COMMAND) -C $(PATH_GRAPHCOLORING)
					@$(MAKE) -s $(COMMAND) -C $(PATH_JOBSHOP)
					@$(MAKE) -s $(COMMAND) -C $(PATH_KNAPSACK)
					@$(MAKE) -s $(COMMAND) -C $(PATH_TRAVELLINGSALESMAN)
					@$(RM) $(BINS)
