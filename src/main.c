#include <stdio.h>
#include "token.h"
#include "lexer.h"

#define FPATH "/home/liam/Documents/lbasic/test/test1.lb"

int main(int argc, char *argv[]) {
   t_list *token_list = lex(FPATH);

   print_list(token_list);

}
