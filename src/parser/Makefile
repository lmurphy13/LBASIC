CC = gcc
LEX = flex
YACC = bison
make = make

LEXER = lexer.l
PARSER = parser.y

SOURCES = $(LEXER) $(PARSER)
SOURCES += $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

lexer: $(LEXER)
	$(LEX) $(LEXER)

parser: $(PARSER)
	$(YACC) $(PARSER)

all:
	$(make) lexer
	$(make) parser
	$(CC) parser.tab.c -ll -o parsetest

clean:
	rm parsetest
	rm *.c
