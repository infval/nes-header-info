# Emscripten makefile
CC = emcc

OPTLEVEL = -g
CFLAGS = $(OPTLEVEL)
CFLAGS += -Wall -Wextra -Wpedantic

CSRCS = \
    ../nesinfo.c \
    ../hash/md5.c \
    ../hash/sha1.c
#    $(wildcard ../*.c)

COBJS := $(patsubst %.c,%.o,$(CSRCS))

EXPORTED_FUNCTIONS = \
    '_PrintNESInfo', \
    '_malloc', \
    '_free'

EFLAGS = \
    -s "EXPORTED_RUNTIME_METHODS=['ccall','cwrap']" \
    -s "EXPORTED_FUNCTIONS=[$(EXPORTED_FUNCTIONS)]" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -o index.html --shell-file nesinfo_shell.html

.PHONY: nesinfo clean clean2 release all default

nesinfo: $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) $(EFLAGS)

clean clean2:
	-$(RM) $(COBJS)

release: OPTLEVEL = -O3
release: clean nesinfo clean2

all: nesinfo

default: all
