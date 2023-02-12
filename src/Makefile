CC = gcc
LEX = flex
YACC = bison

SOURCES = $(wildcard *.c) lexer.l parser.y
OBJECTS = $(SOURCES:.c=.o)
CFLAGS = -g -O0 -DDEBUG

all: lbasic

parser.tab.c: parser.y
	$(YACC) -d -t -v parser.y

lex.yy.c: parser.y
	$(LEX) lexer.l

lbasic: lex.yy.c parser.tab.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) parser.tab.c lex.yy.c -ll -o $@

#lbasic: lexer.o parser.o $(OBJECTS)
#	$(CC) $(CFLAGS) $(OBJECTS) lexer.o parser.o -ll -o $@


clean:
	rm -rf *.o
	rm parser.tab.*
	rm parser.output
	rm lex.yy.c

format:
	clang-format-11 -i *.c
	clang-format-11 -i *.h

