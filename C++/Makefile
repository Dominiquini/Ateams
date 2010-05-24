CC = g++
CFLAGS = -O3 -lpthread -Wall -pedantic
SOURCES = Ateams.cpp Controle.cpp JobShop.cpp Tabu.cpp Genetico.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXEC = Ateams

all:	$(SOURCES) $(EXEC)

$(EXEC):$(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

run:	$(EXEC)
	./$^

clean:
	rm -rf $(OBJECTS) $(EXEC)

purge:	clean
	rm -rf resultados/*.res
