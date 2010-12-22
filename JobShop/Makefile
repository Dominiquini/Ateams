CC = g++
CFLAGS = -lpthread -Wall -pedantic -O3 -march=native

PATH_DADOS = ./dados/
PATH_PARAM = ./parametros/
PATH_PROJ = ./
PATH_BIN = ./bin/
PATH_SRC = ./src/
PATH_RES = ./resultados/
PATH_CORE = ../Core/
PATH_CORE_SRC = ../Core/src/
PATH_CORE_BIN = ../Core/bin/

BIN = JobShop
SRCS =  $(PATH_SRC)JobShop.cpp $(PATH_CORE_SRC)Ateams.cpp $(PATH_CORE_SRC)Controle.cpp$ (PATH_CORE_SRC)Tabu.cpp $(PATH_CORE_SRC)Genetico.cpp $(PATH_CORE_SRC)Annealing.cpp
OBJS = $(PATH_BIN)JobShop.o
INCL = $(SRCS:.cpp=.h) $(PATH_CORE_SRC)Problema.h $(PATH_CORE_SRC)Heuristica.h
EXEC = $(PATH_BIN)$(BIN)

RUN_PARAM = -i $(PATH_DADOS)car1.prb -p $(PATH_PARAM)default.param -r $(PATH_RES)la01.res

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
		@$(CC) $(CFLAGS) $(PATH_CORE_BIN)*.o $^ -o $@
		@echo "$(CC) $(CFLAGS) $(PATH_BIN)*.o -o $@"

$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.h
		@cd $(PATH_CORE) && $(MAKE)
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(CC) $(CFLAGS) -c $< -o $@"

run:		install
		$(EXEC) $(RUN_PARAM)

clean:
		@$(RM) $(OBJS) $(EXEC) $(PATH_PROJ)$(BIN)
		@echo "$(RM) $(PATH_BIN)* $(PATH_PROJ)$(BIN)"

purge:		clean
		@$(RM) $(PATH_RES)*.res $(PATH_BIN)
		@cd $(PATH_CORE) && $(MAKE) $@
		@echo "$(RM) $(PATH_RES)*.res $(PATH_BIN)"
