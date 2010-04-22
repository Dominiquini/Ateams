CC = g++
CFLAGS = -g
SOURCES = Controle.cpp JobShop.cpp 
OBJECTS = $(SOURCES:.c=.o)
EXEC = Ateams

all:	$(SOURCES) $(EXEC)

$(EXEC):$(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

run:	$(EXEC)
	./$^ -i dados/la01.prb

clean:
	rm -rf $(OBJECTS) $(EXEC)

purge:	clean
	rm -rf resultados/*.res
