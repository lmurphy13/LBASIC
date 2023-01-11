alpha [a-zA-Z]
digit [0-9]

%%
[ \t]   ;
[\n]   { yylineno = yylineno + 1; }
"("     return T_LPAREN;
")"     return T_RPAREN;
":="    return T_ASSIGN;
":"     return T_COLON;
";"     return T_SEMICOLON;
","     return T_COMMA;
"."     return T_DOT;
"'"     {
            int c;
            
            // Comments are single line only.
            for (;;) {
                while ( (c = input()) != '\n' && c != EOF ) ;
                if (c == '\n') {
                    yylineno = yylineno + 1;
                    break;
                }

            }
        }
"+"     { printf("found plus\n"); return T_PLUS; }
"-"     return T_MINUS;
"*"     return T_MUL;
"/"     return T_DIV;
"%"     return T_MOD;
"->"    return T_OFTYPE;
"<"     return T_LT;
">"     return T_GT;
"!"     return T_BANG;
"=="    return T_EQ;
"<="    return T_LE;
">="    return T_GE;
"!="    return T_NE;
"and"   return T_AND;
"or"    return T_OR;
"func"  return T_FUNC;
"for"   return T_FOR;
"while" return T_WHILE;
"to"    return T_TO;
"end"   return T_END;
"struct"    return T_STRUCT;
"true"  return T_TRUE;
"false" return T_FALSE;
"nil"   return T_NIL;
"int"   return T_INT;
"bool"  return T_BOOL;
"string"    return T_STRING;
"float"     return T_FLOAT;
"void"  return T_VOID;
"goto"  return T_GOTO;
"if"    return T_IF;
"then"  return T_THEN;
"else"  return T_ELSE;
"return"    return T_RETURN;
{alpha}({alpha}|{digit})*   { printf("ident: %s\n", yytext); yylval.id = yytext; return T_IDENT; }
{digit}+                    { printf("num: %d\n", atoi(yytext)); yylval.num = atoi(yytext); return L_NUM; }
\"(\\.|[^\\"])*\"           { printf("str: %s\n", yytext); yylval.str = yytext; return L_STR; }
[ \t]+  /* whitespace */
.   return yytext[0];
%%
