/**
 * LBASIC Lexical Analyzer
 * File: lexer.c
 * Author: Liam M. Murphy
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "lexer.h"
#include "token.h"
#include "utils.h"

#define N_KEYWORDS 21
#define MAX_KEYWORD_LEN 20

// Prototypes
static char *input_file(const char *path);
static void emit_token(t_list *lst, token_type type, const char *literal);
static void tokenize(char *prog_buff);
static bool check_singles(char c);
static bool is_keyword(char *lexeme, token_type *type);

// Globals
t_list *token_list;
vector *line_map;
static int char_num = -1;
static int line_num = 1;
static int col_num  = 1;

// Elements must remain in this order
static char keywords[N_KEYWORDS][MAX_KEYWORD_LEN] = {
    "and", "or",   "func",   "for",   "while", "to",   "end", "struct", "true", "false", "nil",
    "int", "bool", "string", "float", "void",  "goto", "if",  "then",   "else", "return"};

void split_into_lines(const char *path) {
    line_map = mk_vector();

    if (line_map == NULL) {
        log_error("Unable to allocate a line_map vector");
    }

    FILE *fp = fopen(path, "r");
    if (fp != NULL) {

        char line_buff[MAX_LINE] = {'\0'};
        while (fgets(line_buff, MAX_LINE, fp) != NULL) {
            line_t *newline = (line_t *)malloc(sizeof(line_t));
            memset(newline, 0, sizeof(newline));

            snprintf(newline->text, sizeof(newline->text), "%s", line_buff);
            vector_add(line_map, newline);
        }

        fclose(fp);
    }

    printf("Lines:\n");

    vecnode *ln = line_map->head;
    int i       = 1;
    while (ln != NULL) {
        line_t *line = (line_t *)ln->data;
        printf("%3d. %s", i, line->text);
        i++;

        ln = ln->next;
    }
}

// See lexer.h
t_list *lex(const char *path) {
    char *prog_buff = input_file(path);

    if (prog_buff != NULL) {
        split_into_lines(path);
        printf("=================================\n");

        token_list = t_list_new();

        if (token_list == NULL) {
            log_error("Unable to allocate memory for token_list");
        }

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
    char *buffer = NULL;

    if (fp != NULL) {
        // Get file size
        fseek(fp, 0, SEEK_END);
        const size_t file_size = ftell(fp);
        printf("file size: %d\n", file_size);

        // Seek back to the beginning of the file
        rewind(fp);

        // Allocate the buffer. Freed in lex()
        buffer = (char *)malloc(file_size + 1);

        if (buffer != NULL) {
            // Copy the file contents into the buffer
            fread(buffer, 1, file_size, fp);

            // Append \0 to end of buffer
            buffer[file_size] = '\0';
        }

        fclose(fp);
    } else {
        log_error("Unable to open file for reading");
        exit(LEXER_ERROR_BAD_FILE_POINTER);
    }

    return buffer;
}

// Appends a t_list struct to the doubly-linked list of tokens
static void emit_token(t_list *lst, token_type type, const char *literal) {
    t_list *new_tok = (t_list *)malloc(sizeof(t_list));
    token *tok      = (token *)malloc(sizeof(token));

    memset(tok->literal, 0, MAX_LITERAL);

    tok->type = type;
    tok->line = line_num;
    tok->col  = col_num;
    strncpy(tok->literal, literal, strlen(literal));

    if (tok->type != T_EOF) {
        // Get token's line from line_map
        vecnode *vn = get_nth_node(line_map, line_num);
        if (vn != NULL) {
            line_t *line = (line_t *)vn->data;

            if (line != NULL) {
                snprintf(tok->line_str, sizeof(tok->line_str), "%s", line->text);
            }
        }
    }

    if (new_tok != NULL) {
        if (tok != NULL) {
            new_tok->tok = tok;
            t_list_append(lst, new_tok);
        } else {
            log_error("Unable to create tok");
        }
    } else {
        log_error("Unable to create new_tok");
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
    case '+':
        emit_token(token_list, T_PLUS, "+");
        break;
    case '*':
        emit_token(token_list, T_MUL, "*");
        break;
    case '/':
        emit_token(token_list, T_DIV, "/");
        break;
    case '%':
        emit_token(token_list, T_MOD, "%");
        break;
    case ',':
        emit_token(token_list, T_COMMA, ",");
        break;
    case '.':
        emit_token(token_list, T_DOT, ".");
        break;
    // Intentional fallthrough
    case '<':
    case '>':
    case '=':
    case '!':
    case ':':
    case '-':
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
            *type = (idx + T_AND);
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
    char c           = 0;
    char lexeme[MAX_LITERAL];
    token_type tmp;

    memset(lexeme, 0, sizeof(lexeme));

    c = get_char(prog_buff);
    while (c != '\0') {
        // Skip whitespace
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            col_num++;
            if (c == '\n') {
                line_num++;
                col_num = 1;
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
                    col_num++;
                    if (c == '\n') {
                        line_num++;
                        col_num = 1;
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

            else {
                switch (c) {
                case '=':
                    c = get_char(prog_buff);
                    if (c == '=') {
                        col_num++;
                        emit_token(token_list, T_EQ, "==");
                    } else {
                        unget_char();
                    }
                    break;
                case '<':
                    c = get_char(prog_buff);
                    if (c == '=') {
                        col_num++;
                        emit_token(token_list, T_LE, "<=");
                    } else {
                        unget_char();
                        emit_token(token_list, T_LT, "<");
                    }
                    break;
                case '>':
                    c = get_char(prog_buff);
                    if (c == '=') {
                        col_num++;
                        emit_token(token_list, T_GE, ">=");
                    } else {
                        unget_char();
                        emit_token(token_list, T_GT, ">");
                    }
                    break;
                case '!':
                    c = get_char(prog_buff);
                    if (c == '=') {
                        col_num++;
                        emit_token(token_list, T_NE, "!=");
                    } else {
                        unget_char();
                        emit_token(token_list, T_BANG, "!");
                    }
                    break;
                case '-':
                    c = get_char(prog_buff);
                    // Are we a number?
                    if (is_digit(c)) {
                        // Append '-' to lexeme
                        col_num++;
                        sprintf(lexeme, "%s%c", lexeme, '-');
                        goto lex_num;
                        // Yes, using a goto is bad, but it's the easiest way to
                        // directly parse a negative number with atoi or atof if we
                        // encode the - within the token
                    } else {
                        if (c == '>') {
                            col_num++;
                            emit_token(token_list, T_OFTYPE, "->");
                        } else {
                            unget_char();
                            emit_token(token_list, T_MINUS, "-");
                        }
                    }
                    break;
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
            bool inc_line  = false;
            // Read until newline, space, colon, semicolon, period, or lparen
            while ((c != '\n') && (c != ' ')) {
                col_num++;
                // Append to lexeme
                sprintf(lexeme, "%s%c", lexeme, c);
                c = get_char(prog_buff);

                if (c == '\n') {
                    inc_line = true;
                }

                if (c == '(' || c == ')' || c == ':' || c == ';' || c == ',' || c == '.') {
                    tmp_delim = c;
                    break;
                }
            }
            col_num++;

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
                        col_num++;
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

            /* In the case where we've found a token at the end of a line, we need to "delay" the
             * line number increment until after we've
             * emitted the token. Otherwise, our token will be assigned a line number that is below
             * where the token's real position is. */
            if (inc_line) {
                line_num++;
                col_num  = 1;
                inc_line = false;
            }
        }

        // Number?
        else if (is_digit(c)) {
            // Read until not a digit
        lex_num:
            while (is_digit(c)) {
                // Append to lexeme
                sprintf(lexeme, "%s%c", lexeme, c);
                c = get_char(prog_buff);
                col_num++;
            }

            // Are we a float?
            if (c == '.') {
                // Append dot to lexeme
                sprintf(lexeme, "%s%c", lexeme, c);
                // Advance lexeme
                c = get_char(prog_buff);
                col_num++;

                if (!is_digit(c)) {
                    printf("ERROR: Unknown character on line %d, col %d: \"%c\" (index: %d)\n",
                           line_num, col_num, c, char_num);
                    exit(LEXER_ERROR_UNKNOWN_CHARACTER);
                }

                // Read until we hit another non-digit character
                while (is_digit(c)) {
                    sprintf(lexeme, "%s%c", lexeme, c);
                    c = get_char(prog_buff);
                    col_num++;
                }

                unget_char();
                emit_token(token_list, L_FLOAT, lexeme);
                memset(lexeme, 0, sizeof(lexeme));
            } else {
                unget_char();
                emit_token(token_list, L_INTEGER, lexeme);
                memset(lexeme, 0, sizeof(lexeme));
            }
        }

        else {
            printf("ERROR: Unknown character on line %d, col %d: \"%c\" (index: %d)\n", line_num,
                   col_num, c, char_num);
            exit(LEXER_ERROR_UNKNOWN_CHARACTER);
        }

        c = get_char(prog_buff);
    }

    // Once we hit '\0', append the EOF token
    emit_token(token_list, T_EOF, "EOF");
}
