#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "token.h"

// Prototypes
node *parse(t_list *tokens);
void print_ast(node *ast);

#endif // PARSER_H
