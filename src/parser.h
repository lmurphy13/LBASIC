/**
 * LBASIC Syntax Analyzer Public Definitions
 * File: parser.h
 * Author: Liam M. Murphy
 */

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "token.h"

// Prototypes
node *parse(t_list *tokens);

#endif // PARSER_H
