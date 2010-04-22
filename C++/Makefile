CC = g++
CFLAGS = -g
SOURCES = Ateams.cpp Controle.cpp JobShop.cpp 
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
