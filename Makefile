CC = gcc

SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SRCDIR)/lexer.o $(SRCDIR)/token.o $(SRCDIR)/main.o
CFLAGS = -g -O0

lbasic: $(SOURCES) $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

clean:
	rm -rf $(SRCDIR)/*.o
	rm lbasic
