CC = g++
AR = ar
RANLIB = ranlib

CXXFLAGS = -Wall -pedantic -fpermissive -O3 -march=native

ifeq ($(OS), Windows_NT)
  LDFLAGS = -lmingw32 -lpthread -lfreeglut -lopengl32 -lGLU32 -lxerces-c
else
  LDFLAGS = -lpthread -lglut -lGL -lGLU -lxerces-c
endif

ARFLAGS = -rsc

PATH_BIN = ./bin/
PATH_SRC = ./src/

SRCS = $(wildcard $(PATH_SRC)*.cpp)

OBJS = $(subst $(PATH_SRC),$(PATH_BIN),$(subst .cpp,.o,$(SRCS)))

LIB = $(PATH_BIN)AteamsBase.a

RM = rm -rf


.PHONY: 		all clean purge

all:			$(PATH_BIN) $(LIB)

$(PATH_BIN):
				@mkdir $(PATH_BIN)


$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.hpp
				@$(CC) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)
				
$(LIB):			$(OBJS)
				@$(AR) $(ARFLAGS) $@ $?
				@$(RANLIB) $@

clean:
				@$(RM) $(OBJS) $(LIB) $(PATH_BIN)

purge:			clean
