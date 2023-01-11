CC = gcc

SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)
#OBJECTS = $(SRCDIR)/error.o $(SRCDIR)/lexer.o $(SRCDIR)/token.o $(SRCDIR)/parser.o $(SRCDIR)/main.o
OBJECTS = $(SOURCES:.c=.o)
CFLAGS = -g -O0 -DDEBUG

lbasic: $(SOURCES) $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

clean:
	rm -rf $(SRCDIR)/*.o
	rm lbasic

format:
	clang-format-11 -i src/*.c
	clang-format-11 -i src/*.h
