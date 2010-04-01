CC = gcc
CFLAGS = -g -std=c99
SOURCES = ateams.c io.c genetico.c tabu.c makespan.c lista.c memoria.c politicasMemoria.c solucoes.c
OBJECTS = $(SOURCES:.c=.o)
EXEC = ateams

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS)   $(LDFLAGS) $^ -o $@

run:	$(EXEC)
	./$^ -i dados/la01.prb

clean:
	rm -rf $(OBJECTS) $(EXEC)
