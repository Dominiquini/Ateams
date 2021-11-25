CC = g++
CFLAGS = -Wall -pedantic -O3 -march=native

ifeq ($(OS), Windows_NT)
	LDFLAGS = -lmingw32 -lpthread -lfreeglut -lopengl32 -lGLU32 -lxerces-c -fpermissive
else
	LDFLAGS = -lpthread -lglut -lGL -lGLU -lxerces-c -fpermissive
endif

PATH_BIN = ./bin/
PATH_SRC = ./src/

SRCS = $(wildcard $(PATH_SRC)*.cpp)
DEPS = $(PATH_SRC)Problema.hpp $(PATH_SRC)Heuristica.hpp

OBJS = $(subst $(PATH_SRC),$(PATH_BIN),$(subst .cpp,.o,$(SRCS)))

RM = rm -rf


.PHONY: 		all clean purge

all:			$(PATH_BIN) $(OBJS)


$(PATH_BIN):
				@mkdir $(PATH_BIN)


$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.hpp $(DEPS)
				@$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)
				@echo "$(CC) $(CFLAGS) -c $< -o $@  $(LDFLAGS)"

clean:
				@$(RM) $(OBJS) $(PATH_BIN)
				@echo "$(RM) $(OBJS) $(PATH_BIN)"

purge:			clean
