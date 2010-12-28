CC = g++
CFLAGS = -lpthread -Wall -pedantic -O3 -march=native

PATH_DADOS = ./dados/
PATH_PARAM = ./parametros/
PATH_PROJ = ./
PATH_BIN1 = ./bin_1/
PATH_BIN2 = ./bin_2/
PATH_SRC1 = ./src_1/
PATH_SRC2 = ./src_2/
PATH_RES = ./resultados/
PATH_CORE = ../Ateams_Base/
PATH_CORE_SRC = ../Ateams_Base/src/
PATH_CORE_BIN = ../Ateams_Base/bin/

BIN1 = KnapSack_1
BIN2 = KnapSack_2
SRCS1 =  $(PATH_SRC1)KnapSack.cpp $(PATH_CORE_SRC)Ateams.cpp $(PATH_CORE_SRC)Controle.cpp $(PATH_CORE_SRC)Tabu.cpp $(PATH_CORE_SRC)Genetico.cpp $(PATH_CORE_SRC)Annealing.cpp
SRCS2 =  $(PATH_SRC2)KnapSack.cpp $(PATH_CORE_SRC)Ateams.cpp $(PATH_CORE_SRC)Controle.cpp $(PATH_CORE_SRC)Tabu.cpp $(PATH_CORE_SRC)Genetico.cpp $(PATH_CORE_SRC)Annealing.cpp
SRCS_BASE = $(PATH_CORE_SRC)Ateams.cpp $(PATH_CORE_SRC)Controle.cpp $(PATH_CORE_SRC)Tabu.cpp $(PATH_CORE_SRC)Genetico.cpp $(PATH_CORE_SRC)Annealing.cpp
OBJS1 = $(PATH_BIN1)KnapSack.o
OBJS2 = $(PATH_BIN2)KnapSack.o
INCL1 = $(SRCS1:.cpp=.h) $(PATH_CORE_SRC)Problema.h $(PATH_CORE_SRC)Heuristica.h
INCL2 = $(SRCS2:.cpp=.h) $(PATH_CORE_SRC)Problema.h $(PATH_CORE_SRC)Heuristica.h
INCL_BASE = $(SRCS_BASE:.cpp=.h) $(PATH_CORE_SRC)Problema.h $(PATH_CORE_SRC)Heuristica.h
EXEC1 = $(PATH_BIN1)$(BIN1)
EXEC2 = $(PATH_BIN2)$(BIN2)

RUN_PARAM = -i $(PATH_DADOS)car1.prb -p $(PATH_PARAM)default.param -r $(PATH_RES)la01.res

RM = rm -rf

.PHONY: all install run1 run2 auto clean purge

all:		proj_all

proj_all:	proj1 proj2

proj1:		base $(PATH_BIN1) $(EXEC1)

proj2:		base $(PATH_BIN2) $(EXEC2)

install_all:	install_proj1 install_proj2

install_proj1:	proj1
		@ln -fs $(EXEC1) $(PATH_PROJ)$(BIN1)

install_proj2:	proj2
		@ln -fs $(EXEC2) $(PATH_PROJ)$(BIN2)

base:		$(SRCS_BASE) $(INCL_BASE)
		@cd $(PATH_CORE) && $(MAKE)

debug_all:	debug_proj1 debug_proj2

debug_proj1:	
		@echo "$(CC) $(PATH_CORE_SRC)*.cpp $(PATH_SRC1)*.cpp -lpthread -g3 -O0 -o $(BIN1)"
		@$(CC) $(SRCS1) -g3 -O0 -lpthread -o $(BIN1)

debug_proj2:
		@$(CC) $(SRCS2) -g3 -O0 -lpthread -o $(BIN2)
		@echo "$(CC) $(PATH_CORE_SRC)*.cpp $(PATH_SRC2)*.cpp -lpthread -g3 -O0 -o $(BIN2)"

$(PATH_BIN1):
		@mkdir $(PATH_BIN1)

$(PATH_BIN2):
		@mkdir $(PATH_BIN2)

$(EXEC1):	$(OBJS1)
		@$(CC) $(CFLAGS) $(PATH_CORE_BIN)*.o $^ -o $@
		@echo "$(CC) $(CFLAGS) $(PATH_BIN1)*.o -o $@"

$(EXEC2):	$(OBJS2)
		@$(CC) $(CFLAGS) $(PATH_CORE_BIN)*.o $^ -o $@
		@echo "$(CC) $(CFLAGS) $(PATH_BIN2)*.o -o $@"

$(PATH_BIN1)%.o:$(PATH_SRC1)%.cpp $(PATH_SRC1)%.h
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(CC) $(CFLAGS) -c $< -o $@"

$(PATH_BIN2)%.o:$(PATH_SRC2)%.cpp $(PATH_SRC2)%.h
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(CC) $(CFLAGS) -c $< -o $@"

run1:		install
		$(EXEC1) $(RUN_PARAM)

run2:		install
		$(EXEC2) $(RUN_PARAM)

clean:
		@$(RM) $(OBJS1) $(OBJS2) $(BIN1) $(BIN2) $(EXEC1) $(EXEC2) $(PATH_BIN1) $(PATH_BIN2)
		@echo "$(RM) $(BIN1) $(BIN2) $(OBJS1) $(OBJS2) $(EXEC1) $(EXEC2) $(PATH_BIN1) $(PATH_BIN2)"

purge:		clean
		@$(RM) $(PATH_RES) $(PATH_BIN1) $(PATH_BIN2)
		@cd $(PATH_CORE) && $(MAKE) $@
		@echo "$(RM) $(PATH_RES)*.res $(PATH_BIN)"
