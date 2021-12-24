.DEFAULT_GOAL :=	all

CC = g++
AR = ar
RANLIB = ranlib

ifdef DEBUG
  CXXFLAGS = -Wall -pedantic -O0 -g3 -march=native -mtune=native
else ifdef PROFILE
  CXXFLAGS = -Wall -pedantic -O2 -pg -march=native -mtune=native
else
  CXXFLAGS = -Wall -pedantic -O3 -march=native -mtune=native
endif

ifeq ($(OS), Windows_NT)
  LDFLAGS = -lpthread -lopengl32 -lGLU32 -lfreeglut -lxerces-c
else
  LDFLAGS = -lpthread -lGL -lGLU -lglut -lxerces-c
endif

ARFLAGS = -rsc

ifeq ($(OS), Windows_NT)
  BIN_EXT = .exe
else
  BIN_EXT =
endif

PROJ = 
PATH_ROOT = ./

PATH_BASE 				= $(PATH_ROOT)Ateams_Base/

PATH_BINPACKING 		= $(PATH_ROOT)BinPacking/
PATH_FLOWSHOP 			= $(PATH_ROOT)FlowShop/
PATH_GRAPHCOLORING 		= $(PATH_ROOT)GraphColoring/
PATH_JOBSHOP 			= $(PATH_ROOT)JobShop/
PATH_KNAPSACK 			= $(PATH_ROOT)KnapSack/
PATH_TRAVELLINGSALESMAN = $(PATH_ROOT)TravellingSalesman/

PATH_BASE_SRC 	= $(PATH_BASE)src/
PATH_BASE_BIN 	= $(PATH_BASE)bin/

PATH_PROJ_PARAM = $(PATH_BASE)parameters/
PATH_PROJ_INPUT = $(PATH_ROOT)$(PROJ)/input/
PATH_PROJ_SRC 	= $(PATH_ROOT)$(PROJ)/src/
PATH_PROJ_BIN 	= $(PATH_ROOT)$(PROJ)/bin/
PATH_PROJ_RES 	= $(PATH_ROOT)$(PROJ)/results/

BASE_SRCS = $(wildcard $(PATH_BASE_SRC)*.cpp)
BASE_HEADERS = $(wildcard $(PATH_BASE_SRC)*.hpp)
BASE_OBJS = $(subst $(PATH_BASE_SRC),$(PATH_BASE_BIN),$(subst .cpp,.o,$(BASE_SRCS)))
BASE_LIB = $(PATH_BASE_BIN)Ateams_Base.a
BASE_DEPS = $(BASE_OBJS:.o=.o.d)

PROJ_SRCS = $(wildcard $(PATH_PROJ_SRC)*.cpp)
PROJ_HEADERS = $(wildcard $(PATH_PROJ_SRC)*.hpp)
PROJ_OBJS = $(subst $(PATH_PROJ_SRC),$(PATH_PROJ_BIN),$(subst .cpp,.o,$(PROJ_SRCS)))
PROJ_BIN = $(PATH_PROJ_BIN)$(PROJ)$(BIN_EXT)
PROJ_DEPS = $(PROJ_OBJS:.o=.o.d)

PATTERN_BASE_SRCS = $(PATH_BASE_SRC)%.cpp
PATTERN_BASE_HEADERS = $(PATH_BASE_SRC)%.hpp
PATTERN_BASE_OBJS = $(PATH_BASE_BIN)%.o

PATTERN_PROJ_SRCS = $(PATH_PROJ_SRC)%.cpp
PATTERN_PROJ_HEADERS = $(PATH_PROJ_SRC)%.hpp
PATTERN_PROJ_OBJS = $(PATH_PROJ_BIN)%.o

RUN_PARAM = -p $(PATH_PROJ_PARAM)DEFAULT.xml

NINJA_OUTPUTS = .ninja_deps .ninja_log

PROFILE_OUTPUT = gmon.out

MKDIR = mkdir -p
RM = rm -rf

debug_vars:
			@echo BASE_SOURCES: $(BASE_SRCS) $(BASE_HEADERS)
			@echo BASE_OBJS: $(BASE_OBJS)
			@echo BASE_LIB: $(BASE_LIB)
			@echo PROJ_SOURCES: $(PROJ_SRCS) $(PROJ_HEADERS)
			@echo PROJ_OBJS: $(PROJ_OBJS)
			@echo PROJ_BIN: $(PROJ_BIN)
