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
#include "translate.h"
#include "typechecker.h"

#include "test.h"

void print_usage() {
    printf("LBASIC Compiler Usage\n");
    printf("    ./lbasic -v or --version\n");
    printf("    ./lbasic -t or --test (debug build only)\n");
    printf("    ./lbasic -h or --help\n");
    printf("    ./lbasic <path>\n");
}

void print_version() {
    printf("LBASIC Compiler v0.2 - September 2023\n");
    printf("Author: Liam M. Murphy\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {

        if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0)) {
            print_version();
            return 0;
        }

        else if ((strcmp(argv[1], "-t") == 0) || (strcmp(argv[1], "--test") == 0)) {
#if defined(DEBUG)
            run_tests();
#else
            printf("Test suite unavailable in production builds\n");
#endif
            return 0;
        }

        else if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
            print_usage();
            return 0;
        }

        // Lexical analysis
        t_list *token_list = lex(argv[1]);

        if (token_list != NULL) {
#if defined(DEBUG)
//            print_list(token_list);
#endif
            // Syntactic analysis
            node *program = parse(token_list);

            if (program != NULL) {
#if defined(DEBUG)
                print_ast(program);
#endif
                // Cleanup token_list
                t_list_free(token_list);

                // Semantic analysis
                typecheck(program);

                // Translate to IR
                vector *ir = translate(program);

                //codegen(ir, argv[1]);
            } else {
                log_error("Unreadable AST generated during parsing.");
            }
        } else {
            log_error("Provide valid file path.");
        }
    } else {
        print_usage();
    }
}
