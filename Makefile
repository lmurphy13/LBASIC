# LBASIC Top-level Makefile
# file: Makefile
# Author: Liam M. Murphy

CC = gcc

SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)

# Catchall sources -> objects
OBJECTS = $(SOURCES:.c=.o)

CFLAGS = -g -O0 -DDEBUG

# Enable all warnings, except for -Wrestrict and -Wformat-overflow.
# -Wrestrict and -Wformat-overflow are being raised due to using sprintf() in the lexer for a quick and dirty string concatenation
# These warnings are mitigated by snprintf() called in the parser
CFLAGS += -Wall -Wno-restrict -Wno-format-overflow

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
	cp lbasic /usr/local/bin
