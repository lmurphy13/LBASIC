#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "token.h"
#include "lexer.h"

#define N_KEYWORDS 13
#define MAX_KEYWORD_LEN 20

// Prototypes
static char *input_file(const char *path);
static void emit_token(t_list *lst, token_type type, const char *literal);
static void tokenize(char *prog_buff);
static bool check_singles(char c);
static bool is_keyword(char *lexeme, token_type *type);

// Globals
t_list *token_list;

static char keywords[N_KEYWORDS][MAX_KEYWORD_LEN] = {
    "func",
    "for",
    "while",
    "to",
    "end",
    "struct",
    "true",
    "false",
    "nil",
    "int",
    "bool",
    "string",
    "float"
};

// See lexer.h
t_list *lex(const char *path) {
    char *prog_buff = input_file(path);

    if (prog_buff != NULL) {
        printf("%s\n\n", prog_buff);

        token_list = t_list_new();
  
        tokenize(prog_buff);
    }

    if (prog_buff != NULL) {
        free(prog_buff);
    }

    return token_list;
}

// Takes a file path and returns a buffer containing the contents of the file at path
static char *input_file(const char *path) {
    FILE *fp = fopen(path, "r");
    char *buffer = { "\0" };

    if (fp != NULL) {
        // Get file size
        fseek(fp, 0, SEEK_END);
        const size_t file_size = ftell(fp);

        // Seek back to the beginning of the file
        fseek(fp, 0, SEEK_SET);

        // Allocate the buffer. Freed in lex()
        buffer = (char *)malloc(sizeof(char) * file_size);

        // Copy the file contents into the buffer
        fread(buffer, file_size, 1, fp);
        
        // Append \0 to end of buffer
        strncat(buffer, "\0", strlen("\0"));

        fclose(fp);
    } else {
        printf("Unable to open file for reading");
    }

    return buffer;
}

// Appends a t_list struct to the doubly-linked list of tokens
static void emit_token(t_list *lst, token_type type, const char *literal) {
    t_list *new_tok = (t_list *)malloc(sizeof(t_list));
    token *tok = (token *)malloc(sizeof(token));

	memset(tok->literal, 0, MAX_LITERAL);

    tok->type = type;
    strncpy(tok->literal, literal, strlen(literal));
    
    if (new_tok != NULL) {
        if (tok != NULL) {
            new_tok->tok = tok;
            t_list_append(lst, new_tok);
        } else {
            printf("ERROR: Unable to create tok\n");
        }
    } else {
        printf("ERROR: Unable to create new_tok\n");
    }
}

static bool check_singles(char c) {
    bool retval = true;

    switch (c) {
        case '(':   emit_token(token_list, T_LPAREN, ""); break;
        case ')':   emit_token(token_list, T_RPAREN, ""); break;
        case ':':   emit_token(token_list, T_COLON, ""); break;
        case ';':   emit_token(token_list, T_SEMICOLON, ""); break;
        // Intentional fallthrough
        case '<':
        case '>':
        case '!':
        case '"':   return true;
        default:
            retval = false;
    }

    return retval;
}

static bool is_keyword(char *lexeme, token_type *type) {
    int idx;
    for (idx = 0; idx < N_KEYWORDS; idx++) {
        if (strcmp(lexeme, keywords[idx]) == 0) {
            *type = (idx + T_FUNC); 
            return true;
        }
    }

    return false;
}

static void tokenize(char *prog_buff) {
    static int state = 0;
    char *c = prog_buff;
    char lexeme[MAX_LITERAL];
    token_type tmp;
    memset(lexeme, 0, sizeof(lexeme));

    while (*c != '\0') {
        // Skip whitespace
        if (*c == ' ' || *c == '\n' || *c == '\t' || *c == '\r') { c++; continue; }

        // Check single character tokens
        if (check_singles(*c)) {

            // Beginning of string literal
            if (*c == '"') {
                // Read until ending quote
                c++;
                while (*c != '"') {
                    sprintf(lexeme, "%s%c", lexeme, *c);
                    c++;
                }
                emit_token(token_list, L_STR, lexeme);
                
                // Reset lexeme
                memset(lexeme, 0, sizeof(lexeme));

                c++;
                continue;
            }

			// Beginning of 
			else if (


            // Reset lexeme
            memset(lexeme, 0, sizeof(lexeme));

            

            c++;
            continue;
        }

        else if (is_keyword(lexeme, &tmp)) {
            emit_token(token_list, tmp, lexeme);
            memset(lexeme, 0, sizeof(lexeme));

            c++;
            continue;
        }

        else {
            printf("unknown token: %s\n", lexeme);
        }

        sprintf(lexeme, "%s%c", lexeme, *c);
        printf("%s\n", lexeme);

        c++;
    }
}
