THIS_FILE := $(lastword $(MAKEFILE_LIST))

include ./Makefile.mk

PATH_ROOT = ./


.PHONY:				Ateams Base BinPacking FlowShop GraphColoring JobShop KnapSack TravellingSalesman list all clean purge


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
					@LC_ALL=C $(MAKE) -pRrq -f $(lastword $(THIS_FILE)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$'

all:				Ateams

clean:				COMMAND=clean
clean:				--delete

purge:				COMMAND=purge
purge:				--delete
					$(RM) $(NINJA_OUTPUTS)

--delete:
					$(MAKE) -s $(COMMAND) -C $(PATH_BASE)
					$(MAKE) -s $(COMMAND) -C $(PATH_BINPACKING)
					$(MAKE) -s $(COMMAND) -C $(PATH_FLOWSHOP)
					$(MAKE) -s $(COMMAND) -C $(PATH_GRAPHCOLORING)
					$(MAKE) -s $(COMMAND) -C $(PATH_JOBSHOP)
					$(MAKE) -s $(COMMAND) -C $(PATH_KNAPSACK)
					$(MAKE) -s $(COMMAND) -C $(PATH_TRAVELLINGSALESMAN)
