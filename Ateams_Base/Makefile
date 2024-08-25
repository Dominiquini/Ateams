include ../Makefile_Base


PATH_ROOT 	= ../


.PHONY: 				Base $(ALL) $(CLEAN) $(PURGE)

.SECONDARY:				$(BASE_OBJS)


Base:					$(PATH_BASE_BIN) $(BASE_LIB)


$(PATH_BASE_BIN):
							$(MKDIR) $(PATH_BASE_BIN)

$(PATTERN_BASE_OBJS):	$(PATTERN_BASE_SRCS)# $(PATTERN_BASE_HEADERS)
							$(CC) $(CXXFLAGS) $(C_DEPS) -c $< -o $@

$(BASE_LIB):			$(BASE_OBJS)
							$(AR) $(ARFLAGS) $@ $?


$(ALL):					Base


$(CLEAN):
						$(RM) $(BASE_DEPS) $(BASE_OBJS) $(BASE_LIB) $(PATH_BASE_BIN)

$(PURGE):			clean


-include $(BASE_DEPS)
