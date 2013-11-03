SUBDIRS = nmea

include Common.mk

.PHONY: all
.PHONY: clean
.PHONY: depend
.PHONY: doc
.PHONY: tags
.PHONY: force

all: depend tags proj

#
# Build list of sources and objects to build
SRCS := $(wildcard *.c)
$(foreach subdir,$(SUBDIRS), \
	$(eval SRCS := $(SRCS) $(wildcard $(subdir)/*.c)) \
)
OBJS := $(patsubst %.c,%.o,$(SRCS))

#
# Dependencies rules
depend: proj.dep

proj.dep: $(SRCS)
	@echo "Generating dependencies.."
	@touch $@
	@makedepend $(INCLUDES) -Y -f $@ $^ &> /dev/null
	@rm -f $@.bak

#
# Append specific CFLAGS/LDFLAGS
DEBUG := $(shell grep "^\#define CONFIG_DEBUG" config.h)
ifeq ($(DEBUG),)
TARGET	:= RELEASE
CFLAGS	+= $(CFLAGS_REL)
LDFLAGS	+= $(LDFLAGS_REL)
else
TARGET	:= DEBUG
CFLAGS	+= $(CFLAGS_DBG)
LDFLAGS	+= $(LDFLAGS_DBG)
endif

# rebuild if CFLAGS changed, as suggested in:
# http://stackoverflow.com/questions/3236145/force-gnu-make-to-rebuild-objects-affected-by-compiler-definition/3237349#3237349
proj.cflags: force
	@echo "$(CFLAGS)" | cmp -s - $@ || echo "$(CFLAGS)" > $@

$(OBJS): proj.cflags
#
# Top rules

proj: $(OBJS)
	@echo -e "\n>> Building $@ as target $(TARGET)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ $+	

modinit.o: modinit.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -Wno-implicit-function-declaration \
		$(INCLUDES) -c $< -o $@

%.o: %.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

tags: $(SRCS)
	@echo "Generating tags .."
	@exuberant-ctags -R

clean: $(SUBDIRS)
	@for subdir in $(SUBDIRS); do \
		echo "Cleaning $$subdir .."; rm -f $$subdir/*.o; \
	done
	@rm -f *.o proj.{elf,txt,cflags,dep} output.map proj

doc:
	rm -rf doc/*
	doxygen Doxyfile
	
-include proj.dep
