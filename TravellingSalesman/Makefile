CC = g++
CFLAGS = -lpthread -Wall -pedantic -O3 -march=native

PATH_DADOS = ./dados/
PATH_PARAM = ./parametros/
PATH_PROJ = ./
PATH_BIN = ./bin/
PATH_SRC = ./src/
PATH_RES = ./resultados/
PATH_CORE = ../Ateams_Base/
PATH_CORE_SRC = ../Ateams_Base/src/
PATH_CORE_BIN = ../Ateams_Base/bin/

BIN = TravellingSalesman
SRCS =  $(PATH_SRC)TravellingSalesman.cpp $(PATH_CORE_SRC)Ateams.cpp $(PATH_CORE_SRC)Controle.cpp $(PATH_CORE_SRC)Tabu.cpp $(PATH_CORE_SRC)Genetico.cpp $(PATH_CORE_SRC)Annealing.cpp
SRCS_BASE = $(PATH_CORE_SRC)Ateams.cpp $(PATH_CORE_SRC)Controle.cpp $(PATH_CORE_SRC)Tabu.cpp $(PATH_CORE_SRC)Genetico.cpp $(PATH_CORE_SRC)Annealing.cpp
OBJS = $(PATH_BIN)TravellingSalesman.o
INCL = $(SRCS:.cpp=.h) $(PATH_CORE_SRC)Problema.h $(PATH_CORE_SRC)Heuristica.h
INCL_BASE = $(SRCS_BASE:.cpp=.h) $(PATH_CORE_SRC)Problema.h $(PATH_CORE_SRC)Heuristica.h
EXEC = $(PATH_BIN)$(BIN)

RUN_PARAM = -i $(PATH_DADOS)br17.prb -p $(PATH_PARAM)DEFAULT.param -r $(PATH_RES)br17.res

RM = rm -rf

.PHONY: all base install compile run auto clean purge

all:		base
		$(MAKE) compile

install:	all
		@ln -fs $(EXEC) $(PATH_PROJ)$(BIN)

base:		$(SRCS_BASE) $(INCL_BASE)
		@cd $(PATH_CORE) && $(MAKE)

debug:
		@echo "$(CC) $(PATH_CORE_SRC)*.cpp $(PATH_SRC)*.cpp -lpthread -g3 -O0 -o $(BIN)"
		@$(CC) $(SRCS) -g3 -O0 -lpthread -o $(BIN)

compile:	$(PATH_BIN) $(EXEC)

$(PATH_BIN):
		@mkdir $(PATH_BIN)


$(EXEC):	$(OBJS)
		@$(CC) $(CFLAGS) $(PATH_CORE_BIN)*.o $^ -o $@
		@echo "$(CC) $(CFLAGS) $(PATH_BIN)*.o -o $@"

$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.h
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(CC) $(CFLAGS) -c $< -o $@"

run:		install
		$(EXEC) $(RUN_PARAM)

clean:
		@$(RM) $(OBJS) $(BIN) $(EXEC) $(PATH_BIN)
		@echo "$(RM) $(BIN) $(OBJS) $(EXEC) $(PATH_BIN)"

purge:		clean
		@$(RM) $(PATH_RES) $(PATH_BIN)
		@cd $(PATH_CORE) && $(MAKE) $@
		@echo "$(RM) $(PATH_RES)*.res $(PATH_BIN)"
