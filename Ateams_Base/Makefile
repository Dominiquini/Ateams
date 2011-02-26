CC = g++
CFLAGS = -lpthread -Wall -pedantic -O3 -march=native
LDFLAGS = -lglut -lGL -lGLU

PATH_PROJ = ./
PATH_BIN = ./bin/
PATH_SRC = ./src/

SRCS = $(PATH_SRC)Ateams.cpp $(PATH_SRC)Controle.cpp $(PATH_SRC)Tabu.cpp $(PATH_SRC)Genetico.cpp $(PATH_SRC)Annealing.cpp
OBJS = $(PATH_BIN)Ateams.o $(PATH_BIN)Controle.o $(PATH_BIN)Tabu.o $(PATH_BIN)Genetico.o $(PATH_BIN)Annealing.o
INCL = $(SRCS:.cpp=.h) $(PATH_SRC)Problema.h $(PATH_SRC)Heuristica.h

RM = rm -rf

.PHONY: all install clean purge

all:		$(PATH_BIN) $(OBJS)

$(PATH_BIN):
		@mkdir $(PATH_BIN)


$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.h
		@$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@
		@echo "$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@"

clean:
		@$(RM) $(OBJS) $(PATH_BIN)
		@echo "$(RM) $(PATH_BIN)* $(PATH_BIN)"

purge:		clean