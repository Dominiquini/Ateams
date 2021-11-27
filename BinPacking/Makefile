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

PATH_PARAM 	= ../Ateams_Base/parametros/
PATH_DADOS 	= ./dados/
PATH_BIN 	= ./bin/
PATH_SRC 	= ./src/
PATH_RES 	= ./resultados/

SRCS =  $(wildcard $(PATH_SRC)*.cpp)

OBJS = $(subst $(PATH_SRC),$(PATH_BIN),$(subst .cpp,.o,$(SRCS)))

BIN = BinPacking

EXEC = $(PATH_BIN)$(BIN)

RUN_PARAM = -i $(PATH_DADOS)binpack1_01.prb -p $(PATH_PARAM)DEFAULT.xml -r $(PATH_RES)binpack1_01.res $(ARG)

ifeq ($(OS), Windows_NT)
  LN = ln -sf
  RM = rm -rf
else
  LN = cp
  RM = rm -rf
endif


.PHONY: 		all install base debug run clean purge

all:			$(PATH_BIN) $(EXEC)

install:		all
				@$(LN) $(EXEC) $(PATH_PROJ)$(BIN)

base:
				@cd $(BASE_PATH) && $(MAKE) -s

$(PATH_BIN):
				@mkdir $(PATH_BIN)

$(PATH_RES):
				@mkdir $(PATH_RES)

$(EXEC):		$(OBJS) base
				@$(CC) $(CXXFLAGS) $(BASE_PATH_BIN)*.o $< -o $@ $(LDFLAGS)

$(PATH_BIN)%.o:	$(PATH_SRC)%.cpp $(PATH_SRC)%.hpp
				@$(CC) $(CXXFLAGS) -c $< -o $@

run:			$(PATH_RES) install
				$(EXEC) $(RUN_PARAM)

clean:
				@$(RM) $(OBJS) $(BIN) $(EXEC) $(PATH_BIN)

purge:			clean
				@$(RM) $(PATH_RES) $(PATH_BIN)
				@cd $(BASE_PATH) && $(MAKE) $@ -s
