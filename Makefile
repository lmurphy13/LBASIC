# file: Makefile
# Author: Liam M. Murphy

CC = gcc

SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)

# Catchall sources -> objects
OBJECTS = $(SOURCES:.c=.o)

CFLAGS = -g -O0 -DDEBUG

lbasic: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

clean:
	rm -rf $(SRCDIR)/*.o
	rm lbasic

realclean:
	rm -rf $(SRCDIR)/*.o
	rm lbasic
	rm /usr/local/bin/lbasic

format:
	clang-format-11 -i src/*.c
	clang-format-11 -i src/*.h

install:
	sudo cp lbasic /usr/local/bin
