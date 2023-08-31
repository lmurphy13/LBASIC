/**
 * LBASIC Semantic Analyzer Public Definitions
 * File: typechecker.h
 * Author: Liam M. Murphy
 */

#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "ast.h"
#include "token.h"

// Prototypes
void typecheck(node *ast);
void print_checked_ast(node *ast);
void print_symbol_tables(void);

#endif // TYPECHECKER_H
