CC = g++
CFLAGS = -lm -lpthread -Wall -pedantic -fopenmp -O3 -march=native

PATH_SCRIPT = scripts/
PATH_DADOS = dados/
PATH_PARAM = parametros/
PATH_PROJ = ./
PATH_BIN = bin/
PATH_SRC = src/
PATH_RES = resultados/

BIN = Ateams
SRCS = $(PATH_SRC)Ateams.cpp $(PATH_SRC)Controle.cpp $(PATH_SRC)JobShop.cpp $(PATH_SRC)Tabu.cpp $(PATH_SRC)Genetico.cpp $(PATH_SRC)Annealing.cpp
OBJS = $(PATH_BIN)Ateams.o $(PATH_BIN)Controle.o $(PATH_BIN)JobShop.o $(PATH_BIN)Tabu.o $(PATH_BIN)Genetico.o $(PATH_BIN)Annealing.o
INCL = $(SRCS:.cpp=.h) $(PATH_SRC)Problema.h $(PATH_SRC)Heuristica.h
EXEC = $(PATH_BIN)$(BIN)

SCRIPT = exec_makespan.sh

RUN_PARAM = -i $(PATH_DADOS)la01.prb -p $(PATH_PARAM)default.param -r $(PATH_RES)la01.res
RUN_SCRIPT = $(PATH_SCRIPT)$(SCRIPT) 01 40 1

RM = rm -rf

.PHONY: all install run auto clean purge

all:		$(PATH_BIN) $(EXEC)

install:	all
		@cp $(EXEC) $(PATH_PROJ)

$(PATH_BIN):	
		@mkdir $(PATH_BIN)


$(EXEC):	$(OBJS)
		@$(CC) $(CFLAGS) $^ -o $@
		@echo "$(CC) $(CFLAGS) $(PATH_BIN)*.o -o $@"

$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.h
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(CC) $(CFLAGS) -c $< -o $@"

run:		$(EXEC)
		./$^ $(RUN_PARAM)

auto:		$(EXEC)
		$(RUN_SCRIPT)

clean:
		@$(RM) $(OBJS) $(EXEC) $(PATH_PROJ)$(BIN)
		@echo "$(RM) $(PATH_BIN)* $(PATH_PROJ)$(BIN)"

purge:		clean
		@$(RM) $(PATH_RES)*.res $(PATH_BIN)
		@echo "$(RM) $(PATH_RES)*.res $(PATH_BIN)"