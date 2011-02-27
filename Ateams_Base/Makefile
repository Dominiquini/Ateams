CC = g++
CFLAGS = -Wall -pedantic -O3 -march=native
LDFLAGS = -lpthread -lglut -lGL -lGLU

PATH_BIN = ./bin/
PATH_SRC = ./src/

SRCS = $(wildcard $(PATH_SRC)*.cpp)
INCL = $(wildcard $(PATH_SRC)*.h)

OBJS = $(subst $(PATH_SRC),$(PATH_BIN),$(subst .cpp,.o,$(SRCS)))

RM = rm -rf


.PHONY: 	all clean purge

all:		$(PATH_BIN) $(OBJS)


$(PATH_BIN):
		@mkdir $(PATH_BIN)


$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.h
		@$(CC) $(CFLAGS) -c $< -o $@
		@echo "$(CC) $(CFLAGS) -c $< -o $@"

clean:
		@$(RM) $(OBJS) $(PATH_BIN)
		@echo "$(RM) $(OBJS) $(PATH_BIN)"

purge:		clean
