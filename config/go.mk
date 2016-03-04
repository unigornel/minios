OBJS += $(GOARCHIVE)

ifneq ($(GOINCLUDE),)
CFLAGS += -iquote $(GOINCLUDE)
endif
