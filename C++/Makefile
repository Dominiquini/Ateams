CC = g++
CFLAGS = -O3 -lm -lpthread -Wall -pedantic -fopenmp
SRCS = Ateams.cpp Controle.cpp JobShop.cpp Tabu.cpp Genetico.cpp Annealing.cpp
OBJS = $(SRCS:.cpp=.o)
INCL = $(SRCS:.cpp=.h) Problema.h
EXEC = Ateams

all:		$(EXEC)

$(EXEC):	$(OBJS)
		$(CC) $(CFLAGS) $^ -o $@

%.o:		%.cpp %.h
		$(CC) $(CFLAGS) -c $< -o $@


run:		$(EXEC)
		./$^

clean:
		rm -rf $(OBJS) $(EXEC)

purge:		clean
		rm -rf resultados/*.res
