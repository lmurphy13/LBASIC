CC = gcc
LEX = flex
YACC = bison
YFLAGS = -d

SRCDIR = src

LEXER = $(SRCDIR)/lexer.l
PARSER = $(SRCDIR)/parser.y

SOURCES = $(wildcard $(SRCDIR)/*.c)
#OBJECTS = $(SRCDIR)/error.o $(SRCDIR)/lexer.o $(SRCDIR)/token.o $(SRCDIR)/parser.o $(SRCDIR)/main.o
OBJECTS = $(SOURCES:.c=.o) $(lexer.o) $(parser.o)
CFLAGS = -g -O0 -DDEBUG

lbasic: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -ll -o $@

parser.o: $(SRCDIR)/parser.tab.c
	$(CC) $(CFLAGS) -o $@ -c $^

parser.tab.c: $(SRCDIR)/parser.y
	bison $(PARSER)
	mv parser.tab.c $(SRCDIR)/parser.tab.c

lexer.o: $(SRCDIR)/lexer.c
	$(CC) $(CFLAGS) -o $@ -c $^

lexer.c: $(SRCDIR)/lexer.l
	$(LEX) $(LEXER)

clean:
	rm -rf $(SRCDIR)/*.o
	rm $(SRCDIR)/parser.tab.c
	rm $(SRCDIR)/lexer.c
	rm lbasic

format:
	clang-format-11 -i src/*.c
	clang-format-11 -i src/*.h
