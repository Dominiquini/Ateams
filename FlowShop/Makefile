CC = g++
CFLAGS = -lpthread -Wall -pedantic -O3 -march=native

PATH_SCRIPT = ./scripts/
PATH_DADOS = ./dados/
PATH_PARAM = ./parametros/
PATH_PROJ = ./
PATH_BIN = ./bin/
PATH_SRC = ./src/
PATH_RES = ./resultados/

BIN = FlowShop
SRCS = $(PATH_SRC)Ateams.cpp $(PATH_SRC)Controle.cpp $(PATH_SRC)FlowShop.cpp $(PATH_SRC)Tabu.cpp $(PATH_SRC)Genetico.cpp $(PATH_SRC)Annealing.cpp
OBJS = $(PATH_BIN)Ateams.o $(PATH_BIN)Controle.o $(PATH_BIN)FlowShop.o $(PATH_BIN)Tabu.o $(PATH_BIN)Genetico.o $(PATH_BIN)Annealing.o
INCL = $(SRCS:.cpp=.h) $(PATH_SRC)Problema.h $(PATH_SRC)Heuristica.h
EXEC = $(PATH_BIN)$(BIN)

SCRIPT = exec_makespan.sh

RUN_PARAM = -i $(PATH_DADOS)car1.prb -p $(PATH_PARAM)default.param -r $(PATH_RES)la01.res
RUN_SCRIPT = $(PATH_SCRIPT)$(SCRIPT) 01 40 1

RM = rm -rf

.PHONY: all install run auto clean purge

all:		$(PATH_BIN) $(EXEC)

install:	all
		@ln -fs $(EXEC) $(PATH_PROJ)$(BIN)

debug:
		@$(CC) $(SRCS) -g3 -O0 -lpthread -o $(BIN)
		@echo "$(CC) $(PATH_SRC)*.cpp -lpthread -g3 -O0 -o $(BIN)"

$(PATH_BIN):
		@mkdir $(PATH_BIN)


$(EXEC):	$(OBJS)
		@$(CC) $(CFLAGS) $^ -o $@
		@echo "$(CC) $(CFLAGS) $(PATH_BIN)*.o -o $@"

$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.h
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(CC) $(CFLAGS) -c $< -o $@"

run:		install
		$(EXEC) $(RUN_PARAM)

auto:		install
		$(RUN_SCRIPT)

clean:
		@$(RM) $(OBJS) $(EXEC) $(PATH_PROJ)$(BIN)
		@echo "$(RM) $(PATH_BIN)* $(PATH_PROJ)$(BIN)"

purge:		clean
		@$(RM) $(PATH_RES)*.res $(PATH_BIN)
		@echo "$(RM) $(PATH_RES)*.res $(PATH_BIN)"
