include ../Makefile_Base

-include $(PROJ_DEPS)

PATH_ROOT = ../

PROJ = JobShop


.PHONY: 		all base clean purge


all:			$(PATH_PROJ_BIN) $(PROJ_BIN)

base:			$(BASE_LIB)

clean:
				$(RM) $(PROJ_OBJS) $(PROJ_DEPS) $(PROJ_BIN) $(PATH_PROJ_BIN)

purge:			clean
				$(RM) $(PATH_PROJ_RES)

$(PATH_PROJ_BIN):
				$(MKDIR) $(PATH_PROJ_BIN)

$(BASE_LIB):		$(BASE_SRCS) $(BASE_HEADERS)
				$(MAKE) -C $(PATH_BASE)

$(PATTERN_PROJ_OBJS):	$(PATTERN_PROJ_SRCS) $(PROJ_HEADERS)
				$(CC) $(CXXFLAGS) -MMD -MF $(patsubst %$(OBJ_EXT),%$(DEPS_EXT),$@) -c $< -o $@

$(PROJ_BIN):		$(BASE_LIB) $(PROJ_OBJS)
				$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
