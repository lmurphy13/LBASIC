/**
 * LBASIC Entry Point
 * File: main.c
 * Author: Liam M. Murphy
 */

#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"
#include "token.h"

void print_usage() {
    printf("LBASIC Usage\n");
    printf("\t./lbasic <path>\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {

        // Lexical analysis
        t_list *token_list = lex(argv[1]);

        if (token_list != NULL) {
#if defined(DEBUG)
            print_list(token_list);
#endif
            // Syntactic analysis
            node *program = parse(token_list);

            if (program != NULL) {
                print_ast(program);
            } else {
                printf("ERROR: Invalid AST generated during parsing.\n");
                exit(COMPILER_ERROR_BAD_AST);
            }
        } else {
            printf("ERROR: Provide valid file path.\n");
            exit(COMPILER_ERROR_UNKNOWN_PATH);
        }
    } else {
        print_usage();
    }
}
