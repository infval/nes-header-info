CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -static -static-libgcc
SOURCES = nesinfo.c hash/md5.c hash/sha1.c

.PHONY: all release debug
all:
	$(CC) -O3 $(CFLAGS) $(SOURCES) -o nesinfo.exe
release:
	$(CC) -O3 $(CFLAGS) $(SOURCES) -o nesinfo.exe -DNDEBUG
debug:
	$(CC) -g3 $(CFLAGS) $(SOURCES) -o nesinfo.exe
