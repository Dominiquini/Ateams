CC = g++

CXXFLAGS = -Wall -pedantic -fpermissive -O3 -march=native

ifeq ($(OS), Windows_NT)
  LDFLAGS = -lmingw32 -lpthread -lfreeglut -lopengl32 -lGLU32 -lxerces-c
else
  LDFLAGS = -lpthread -lglut -lGL -lGLU -lxerces-c
endif

BASE_PATH = ../Ateams_Base/

BASE_PATH_SRC = ../Ateams_Base/src/
BASE_PATH_BIN = ../Ateams_Base/bin/

BASE_SRCS = $(wildcard $(BASE_PATH_SRC)*.cpp)
BASE_LIB = $(BASE_PATH_BIN)AteamsBase.a

PATH_PARAM 	= ../Ateams_Base/parametros/
PATH_DADOS 	= ./dados/
PATH_BIN 	= ./bin/
PATH_SRC 	= ./src/
PATH_RES 	= ./resultados/

SRCS =  $(wildcard $(PATH_SRC)*.cpp)

OBJS = $(subst $(PATH_SRC),$(PATH_BIN),$(subst .cpp,.o,$(SRCS)))

BIN = GraphColoring

EXEC = $(PATH_BIN)$(BIN)

RUN_PARAM = -i $(PATH_DADOS)jean.prb -p $(PATH_PARAM)DEFAULT.xml -r $(PATH_RES)jean.res $(ARG)

RM = rm -rf


.PHONY: 		all base run clean purge

all:			$(PATH_BIN) $(EXEC)

base:
				@$(MAKE) -C $(BASE_PATH)

$(PATH_BIN):
				@mkdir $(PATH_BIN)

$(PATH_RES):
				@mkdir $(PATH_RES)

$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.hpp
				@$(CC) $(CXXFLAGS) -c $< -o $@

$(EXEC):		$(OBJS) base
				@$(CC) $(CXXFLAGS) $(BASE_LIB) $< -o $@ $(LDFLAGS)

run:			$(PATH_RES) all
				$(EXEC) $(RUN_PARAM)

clean:
				@$(RM) $(OBJS) $(EXEC) $(PATH_BIN)

purge:			clean
				@$(RM) $(PATH_RES)
				@cd $(BASE_PATH) && $(MAKE) $@ -s
