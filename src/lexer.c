#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "lexer.h"
#include "token.h"

#define N_KEYWORDS 14
#define MAX_KEYWORD_LEN 20

// Prototypes
static char *input_file(const char *path);
static void emit_token(t_list *lst, token_type type, const char *literal);
static void tokenize(char *prog_buff);
static bool check_singles(char c);
static bool is_keyword(char *lexeme, token_type *type);

// Globals
t_list *token_list;
static int char_num = -1;
static int line_num = 1;

static char keywords[N_KEYWORDS][MAX_KEYWORD_LEN] = {"func",   "for",    "while", "to",  "end",
                                                     "struct", "true",   "false", "nil", "int",
                                                     "bool",   "string", "float", "goto"};

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
    FILE *fp     = fopen(path, "r");
    char *buffer = {"\0"};

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
    token *tok      = (token *)malloc(sizeof(token));

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
    case '(':
        emit_token(token_list, T_LPAREN, "(");
        break;
    case ')':
        emit_token(token_list, T_RPAREN, ")");
        break;
    case ';':
        emit_token(token_list, T_SEMICOLON, ";");
        break;
    // Intentional fallthrough
    case '<':
    case '>':
    case '!':
    case ':':
    case '\'':
    case '"':
        return true;
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

static bool is_digit(char c) { return (c >= '0' && c <= '9'); }

// Get the next char from the buffer
static char get_char(char *prog_buff) {
    char_num++;
    return *(prog_buff + char_num);
}

// Decrement buffer index
static void unget_char() { char_num--; }

static void tokenize(char *prog_buff) {
    static int state = 0;
    char c;
    char lexeme[MAX_LITERAL];
    token_type tmp;

    memset(lexeme, 0, sizeof(lexeme));

    c = get_char(prog_buff);
    while (c != '\0') {
        // Skip whitespace
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (c == '\n') {
                line_num++;
            }
            c = get_char(prog_buff);
            continue;
        }

        // Check single character tokens
        if (check_singles(c)) {

            // Beginning of string literal
            if (c == '"') {
                // Read until ending quote
                c = get_char(prog_buff);
                while (c != '"') {
                    sprintf(lexeme, "%s%c", lexeme, c);
                    c = get_char(prog_buff);
                }
                emit_token(token_list, L_STR, lexeme);

                // Reset lexeme
                memset(lexeme, 0, sizeof(lexeme));
            }

            // Beginning of a comment
            else if (c == '\'') {
                // Read until a new line is seen
                while (c != '\n') {
                    c = get_char(prog_buff);

                    if (c == '\n') {
                        line_num++;
                    }
                }
                c = get_char(prog_buff);
                continue;
            }

            // Beginning of assignment token
            else if (c == ':') {
                c = get_char(prog_buff);
                if (c == '=') {
                    emit_token(token_list, T_ASSIGN, ":=");

                    // Reset lexeme
                    memset(lexeme, 0, sizeof(lexeme));
                    c = get_char(prog_buff);
                    continue;
                }
            }

            // Reset lexeme
            // memset(lexeme, 0, sizeof(lexeme));
            c = get_char(prog_buff);
            continue;
        }

        // Identifier or keyword?
        else if (isalpha(c)) {
            char tmp_delim = 0;

            // Read until newline, space, colon, semicolon, or lparen
            while ((c != '\n') && (c != ' ')) {
                // Append to lexeme
                sprintf(lexeme, "%s%c", lexeme, c);
                c = get_char(prog_buff);

                if (c == '\n') {
                    line_num++;
                }

                if (c == '(' || c == ':' || c == ';') {
                    tmp_delim = c;
                    break;
                }
            }

            if (is_keyword(lexeme, &tmp)) {
                emit_token(token_list, tmp, lexeme);
                // Reset lexeme
                memset(lexeme, 0, sizeof(lexeme));
            } else {
                // Identifier
                emit_token(token_list, T_IDENT, lexeme);
                memset(lexeme, 0, sizeof(lexeme));
            }

            if (tmp_delim) {
                if (check_singles(tmp_delim)) {
                    if (tmp_delim == ':') {
                        c = get_char(prog_buff);
                        if (c == '=') {
                            unget_char();
                            continue;
                        } else {
                            unget_char();
                            emit_token(token_list, T_COLON, ":");
                        }
                    }
                }
            }
        }

        // Number?
        else if (is_digit(c)) {
            // Read until not a digit
            while (is_digit(c)) {
                // Append to lexeme
                sprintf(lexeme, "%s%c", lexeme, c);
                c = get_char(prog_buff);
            }
            unget_char();
            emit_token(token_list, L_NUM, lexeme);
            memset(lexeme, 0, sizeof(lexeme));
        }

        else {
            printf("ERROR: Unknown character on line %d: \"%c\" (index: %d)\n", line_num, c,
                   char_num);
            exit(LEXER_ERROR_UNKNOWN_CHARACTER);
        }

        c = get_char(prog_buff);
    }
}
