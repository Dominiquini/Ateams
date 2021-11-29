CC = g++
CXXFLAGS = -Wall -pedantic -fpermissive -O3 -march=native

ifeq ($(OS), Windows_NT)
  LDFLAGS = -lmingw32 -lpthread -lfreeglut -lopengl32 -lGLU32 -lxerces-c
else
  LDFLAGS = -lpthread -lglut -lGL -lGLU -lxerces-c
endif

PATH_BIN = ./bin/
PATH_SRC = ./src/

SRCS = $(wildcard $(PATH_SRC)*.cpp)

OBJS = $(subst $(PATH_SRC),$(PATH_BIN),$(subst .cpp,.o,$(SRCS)))

RM = rm -rf


.PHONY: 		all clean purge

all:			$(PATH_BIN) $(OBJS)

$(PATH_BIN):
				@mkdir $(PATH_BIN)


$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.hpp
				@$(CC) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

clean:
				@$(RM) $(OBJS) $(PATH_BIN)

purge:			clean
