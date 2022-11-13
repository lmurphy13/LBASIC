/**
 * LBASIC Entry Point
 * File: main.c
 * Author: Liam M. Murphy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"
#include "token.h"

void print_usage() {
    printf("LBASIC Compiler Usage\n");
    printf("    ./lbasic -v or --version\n");
    printf("    ./lbasic <path>\n");
}

void print_version() {
    printf("LBASIC Compiler v0.1 - November 2022\n");
    printf("Author: Liam M. Murphy\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {

        if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0)) {
            print_version();
            return 0;
        }

        // Lexical analysis
        t_list *token_list = lex(argv[1]);

        if (token_list != NULL) {
#if defined(DEBUG)
            print_list(token_list);
#endif
            // Syntactic analysis
            node *program = parse(token_list);

            if (program != NULL) {
                // Cleanup token_list
                //                t_list_free(token_list);

#if defined(DEBUG)
                print_ast(program);
#endif
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
