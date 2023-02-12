/* A Bison parser, made by GNU Bison 3.7.5.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_AND = 258,                   /* T_AND  */
    T_OR = 259,                    /* T_OR  */
    T_FUNC = 260,                  /* T_FUNC  */
    T_FOR = 261,                   /* T_FOR  */
    T_WHILE = 262,                 /* T_WHILE  */
    T_TO = 263,                    /* T_TO  */
    T_END = 264,                   /* T_END  */
    T_STRUCT = 265,                /* T_STRUCT  */
    T_TRUE = 266,                  /* T_TRUE  */
    T_FALSE = 267,                 /* T_FALSE  */
    T_NIL = 268,                   /* T_NIL  */
    T_INT = 269,                   /* T_INT  */
    T_BOOL = 270,                  /* T_BOOL  */
    T_STRING = 271,                /* T_STRING  */
    T_FLOAT = 272,                 /* T_FLOAT  */
    T_VOID = 273,                  /* T_VOID  */
    T_GOTO = 274,                  /* T_GOTO  */
    T_IF = 275,                    /* T_IF  */
    T_THEN = 276,                  /* T_THEN  */
    T_ELSE = 277,                  /* T_ELSE  */
    T_RETURN = 278,                /* T_RETURN  */
    T_LPAREN = 279,                /* T_LPAREN  */
    T_RPAREN = 280,                /* T_RPAREN  */
    T_ASSIGN = 281,                /* T_ASSIGN  */
    T_COLON = 282,                 /* T_COLON  */
    T_SEMICOLON = 283,             /* T_SEMICOLON  */
    T_COMMA = 284,                 /* T_COMMA  */
    T_DOT = 285,                   /* T_DOT  */
    T_DQUOTE = 286,                /* T_DQUOTE  */
    T_OFTYPE = 287,                /* T_OFTYPE  */
    T_LT = 288,                    /* T_LT  */
    T_GT = 289,                    /* T_GT  */
    T_BANG = 290,                  /* T_BANG  */
    T_EQ = 291,                    /* T_EQ  */
    T_LE = 292,                    /* T_LE  */
    T_GE = 293,                    /* T_GE  */
    T_NE = 294,                    /* T_NE  */
    T_IDENT = 295,                 /* T_IDENT  */
    L_NUM = 296,                   /* L_NUM  */
    L_STR = 297,                   /* L_STR  */
    T_PLUS = 298,                  /* T_PLUS  */
    T_MINUS = 299,                 /* T_MINUS  */
    T_MUL = 300,                   /* T_MUL  */
    T_DIV = 301,                   /* T_DIV  */
    T_MOD = 302,                   /* T_MOD  */
    UMINUS = 303                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 15 "parser.y"

    char *id;
    int num;
    char *str;

#line 118 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
