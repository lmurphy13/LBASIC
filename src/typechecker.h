/**
 * LBASIC Semantic Analyzer Public Definitions
 * File: typechecker.h
 * Author: Liam M. Murphy
 */

#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "ast.h"
#include "symtab.h"
#include "token.h"

// Prototypes
void typecheck(node *ast);
symtab_t *get_symbol_table(void);

#endif // TYPECHECKER_H
