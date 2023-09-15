/**
 * LBASIC Semantic Analyzer Public Definitions
 * File: typechecker.h
 * Author: Liam M. Murphy
 */

#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "ast.h"
#include "token.h"

typedef struct type_s {
    data_type datatype;
    char struct_type[MAX_LITERAL];
} type_t;

// Prototypes
void typecheck(node *ast);
void print_symbol_tables(void);

#endif // TYPECHECKER_H
