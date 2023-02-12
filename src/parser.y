%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *yytext;


int yylex();
void yyerror(const char *s);

%}

%union {
    char *id;
    int num;
    char *str;
}

%type <id> T_IDENT
%type <num> L_NUM
%type <str> L_STR

%token T_AND T_OR T_FUNC T_FOR T_WHILE T_TO T_END
%token T_STRUCT T_TRUE T_FALSE T_NIL
%token T_INT T_BOOL T_STRING T_FLOAT T_VOID
%token T_GOTO T_IF T_THEN T_ELSE T_RETURN
%token T_LPAREN T_RPAREN T_ASSIGN T_COLON T_SEMICOLON T_COMMA
%token T_DOT T_DQUOTE T_OFTYPE
%token T_LT T_GT T_BANG T_EQ T_LE T_GE T_NE
%token T_IDENT L_NUM L_STR

%left T_AND T_OR
%left T_BANG
%nonassoc T_LT T_GT T_GE T_LE T_EQ

%left T_PLUS T_MINUS
%left T_MUL T_DIV T_MOD

%left UMINUS

%nonassoc T_ELSE

%define parse.error verbose

%%

/* for now, no labels */

program : declarations statements
        ;

declarations : declaration declarations
             | declaration
             ;
declaration : function_decl
            | var_decl
            | struct_decl
            ;

statements : statement statements
           | statement
           ;

statement : for_stmt
          | while_stmt
          | if_then_stmt
          | if_then_else_stmt
          | assign_stmt
          | expression
          ;

function_decl : T_FUNC T_IDENT T_LPAREN formal_list T_RPAREN T_OFTYPE type func_body T_END
              | T_FUNC T_IDENT T_LPAREN T_RPAREN T_OFTYPE type func_body T_END
              | T_FUNC T_IDENT T_LPAREN T_RPAREN T_OFTYPE T_VOID func_body T_END
              ;

func_body : var_decls statements T_RETURN expression T_SEMICOLON
          | var_decls statements
          ;

type : T_INT { printf("type is int\n"); }
     | T_BOOL { printf("type is bool\n"); }
     | T_STRING { printf("type is string\n"); }
     | T_FLOAT { printf("type is float\n"); }
     ;

var_decls : var_decl var_decls
          | var_decl
          ;

var_decl : type T_IDENT T_SEMICOLON
         | type T_IDENT T_ASSIGN expression T_SEMICOLON
         | T_STRUCT T_IDENT T_IDENT T_SEMICOLON
         ;

struct_decl : T_STRUCT T_IDENT member_decls T_END
            ;

member_decls : member_decl member_decls
             | member_decl
             ;

member_decl : type T_IDENT T_SEMICOLON
            ;

for_stmt : T_FOR T_IDENT T_ASSIGN constant T_TO constant var_decls statements T_END
         ;

while_stmt : T_WHILE T_LPAREN expression T_RPAREN var_decls statements T_END
           ;

if_then_stmt : T_IF T_LPAREN expression T_RPAREN T_THEN statements T_END
             ;

if_then_else_stmt : T_IF T_LPAREN expression T_RPAREN T_THEN statements T_ELSE statements T_END
                  ;

assign_stmt : T_IDENT T_ASSIGN expression T_SEMICOLON
            ;

expression : cond_expr
           | goto_expr
           | call_expr
           | struct_access_expr
           ;

expr_list : expression T_COMMA expr_list
          | expression
          ;

formal_list : formal T_COMMA expr_list
            | formal
            ;

formal : type T_IDENT
       ;

/* Idea for cond_expr and bin_op_expr pattern from: https://github.com/UO-cis561/reflex-bison-ast/blob/master/src/calc.yxx */
cond_expr : cond_expr T_AND cond_expr
          | cond_expr T_OR cond_expr
          | T_BANG cond_expr
          | bin_op_expr T_LT bin_op_expr
          | bin_op_expr T_GT bin_op_expr
          | bin_op_expr T_LE bin_op_expr
          | bin_op_expr T_GE bin_op_expr
          | bin_op_expr T_EQ bin_op_expr
          | bin_op_expr
          ;

bin_op_expr : bin_op_expr T_PLUS bin_op_expr
            | bin_op_expr T_MINUS bin_op_expr
            | bin_op_expr T_MUL bin_op_expr
            | bin_op_expr T_DIV bin_op_expr
            | value
            ;

/*
TODO: Figure this out

negate_expr : T_MINUS value %prec UMINUS
            | value
            ;
*/

value : T_IDENT { printf("ident is: %s\n", $1); }
      | constant
      ;

constant : T_TRUE
         | T_FALSE
         | T_NIL
         | L_NUM
         | L_STR
         ;

goto_expr : T_GOTO T_IDENT T_SEMICOLON
          ;

call_expr : T_IDENT T_LPAREN expr_list T_RPAREN
          ;

struct_access_expr : expression T_DOT T_IDENT
                   ;

%%


void yyerror(const char *s) {
    printf("%s on line %d: %s\n", s, yylineno, yytext);
}
