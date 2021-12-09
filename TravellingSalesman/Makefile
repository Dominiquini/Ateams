include ../Makefile.mk

PATH_ROOT = ../

PROJ = TravellingSalesman

RUN_PARAM = -i $(PATH_PROJ_INPUT)br17.prb -p $(PATH_PROJ_PARAM)DEFAULT.xml -r $(PATH_PROJ_RES)br17.res $(ARG)


.PHONY: 				all base run clean purge


all:					$(PATH_PROJ_BIN) $(PROJ_BIN)

base:					$(BASE_LIB)

run:					$(PATH_PROJ_RES) all
						$(PROJ_BIN) $(RUN_PARAM)

clean:
						$(RM) $(PROJ_OBJS) $(PROJ_BIN) $(PATH_PROJ_BIN)

purge:					clean
						$(RM) $(PATH_PROJ_RES)

$(PATH_PROJ_BIN):
						$(MKDIR) $(PATH_PROJ_BIN)

$(PATH_PROJ_RES):
						$(MKDIR) $(PATH_PROJ_RES)

$(BASE_LIB):			$(BASE_SRCS) $(BASE_HEADERS)
						$(MAKE) -C $(PATH_BASE)

$(PATTERN_PROJ_OBJS):	$(PATTERN_PROJ_SRCS) $(PROJ_HEADERS)
						$(CC) $(CXXFLAGS) -c $< -o $@

$(PROJ_BIN):			$(BASE_LIB) $(PROJ_OBJS)
						$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
