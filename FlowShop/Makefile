include ../Makefile_Base


PATH_ROOT 	= ../


.PHONY: 				Base $(PROJ) $(ALL) $(CLEAN) $(PURGE)

.SECONDARY:				$(BASE_LIB)


Base:					$(BASE_LIB)


$(BASE_LIB):			$(BASE_SRCS) $(BASE_HEADERS)
							$(MAKE) Base -C $(PATH_BASE)


$(PROJ):				$(PATH_PROJ_BIN) $(PROJ_BIN)


$(PATH_PROJ_BIN):
							$(MKDIR) $(PATH_PROJ_BIN)

$(PATTERN_PROJ_OBJS):	$(PATTERN_PROJ_SRCS)# $(PATTERN_PROJ_HEADERS)
							$(CC) $(CXXFLAGS) $(C_DEPS) -c $< -o $@

$(PROJ_BIN):			$(BASE_LIB) $(PROJ_OBJS)
							$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)


$(ALL):					$(PROJ)


$(CLEAN):
							$(RM) $(PROJ_DEPS) $(PROJ_OBJS) $(PROJ_BIN) $(PATH_PROJ_BIN)

$(PURGE):				clean
							$(RM) $(PATH_PROJ_RES)


-include $(PROJ_DEPS)
