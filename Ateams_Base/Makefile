include ../Makefile.mk

-include $(BASE_DEPS)

PATH_ROOT = ../

PROJ = Ateams_Base


.PHONY: 				all clean purge

all:					$(PATH_BASE_BIN) $(BASE_LIB)

clean:
						$(RM) $(BASE_OBJS) $(BASE_DEPS) $(BASE_LIB) $(PATH_BASE_BIN)

purge:					clean

$(PATH_BASE_BIN):
						$(MKDIR) $(PATH_BASE_BIN)

$(PATTERN_BASE_OBJS):	$(PATTERN_BASE_SRCS) $(BASE_HEADERS)
						$(CC) $(CXXFLAGS) -MMD -MF $(patsubst %.o,%.o.d,$@) -c $< -o $@

$(BASE_LIB):			$(BASE_OBJS)
						$(AR) $(ARFLAGS) $@ $^
						$(RANLIB) $@
