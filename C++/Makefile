CC = g++
CFLAGS = -O3
SOURCES = Ateams.cpp Controle.cpp JobShop.cpp Tabu.cpp Genetico.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXEC = Ateams

all:	$(SOURCES) $(EXEC)

$(EXEC):$(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

run:	$(EXEC)
	./$^ -i ./dados/la01.prb

clean:
	rm -rf $(OBJECTS) $(EXEC)

purge:	clean
	rm -rf resultados/*.res
