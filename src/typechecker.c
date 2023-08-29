/**
 * LBASIC Semantic Analyzer Module
 * File: typechecker.c
 * Author: Liam M. Murphy
 */

#include "typechecker.h"

#include "error.h"

#include <stdio.h>

/*
 * Pass 1: Build a symbol table
 * Pass 2: Verify types and scope
 */
void typecheck(node *ast) {
    node *root = ast;

    if (root != NULL) {
        
    } else {
        log_error("Unable to access AST for type checking");
    }
}

// Might not be needed
void print_checked_ast(node *ast) {
    return;
}
