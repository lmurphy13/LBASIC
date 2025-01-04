# LBASIC Top-level Makefile
# file: Makefile
# Author: Liam M. Murphy

CC = gcc
CLANG_FORMAT = clang-format

SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)
#SOURCES := $(filter-out $(SRCDIR)/test.c, $(SOURCES))

# Catchall sources -> objects
OBJECTS = $(SOURCES:.c=.o)

#CFLAGS = -g -O0
CFLAGS = -g -O0 -DDEBUG
CFLAGS += -std=c11
# Enable all warnings, except for -Wrestrict and -Wformat-overflow.
# -Wrestrict and -Wformat-overflow are being raised due to using sprintf() in the lexer for a quick and dirty string concatenation
# These warnings are mitigated by snprintf() called in the parser to ensure strings are null-terminated and are a specific length
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
	$(CLANG_FORMAT) -i src/*.c
	$(CLANG_FORMAT) -i src/*.h

install:
	cp lbasic /usr/local/bin
