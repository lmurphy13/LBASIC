/**
 * LBASIC Entry Point
 * File: main.c
 * Author: Liam M. Murphy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

extern FILE *yyin;
extern FILE *yyout;
extern int yyparse(void);

void print_usage() {
    printf("LBASIC Compiler Usage\n");
    printf("    ./lbasic -v or --version\n");
    printf("    ./lbasic <path>\n");
}

void print_version() {
    printf("LBASIC Compiler v0.1 - November 2022\n");
    printf("Author: Liam M. Murphy\n");
}

/*
int main(int argc, char *argv[]) {
    yyin = fopen(argv[1], "r");
    
    if (!yyparse()) {
        printf("parse success\n");
    } else {
        printf("parse failure\n");
    }

    fclose(yyin);

}
*/

int main(int argc, char *argv[]) {
    if (argc > 1) {

        if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0)) {
            print_version();
            return 0;
        }

        yyin = fopen(argv[2], "r");

        if (yyin != NULL) {
            if (!yyparse()) {
                printf("parse success\n");
            } else {
                printf("parse failed\n");
            }

        } else {
            log_error("Provide valid file path.\n");
            exit(COMPILER_ERROR_UNKNOWN_PATH);
        }
    } else {
        print_usage();
    }
}
