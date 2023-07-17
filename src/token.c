/**
 * LBASIC Token Module
 * File: token.c
 * Author: Liam M. Murphy
 */

#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int token_count = 0;

t_list *t_list_new(void) {
    t_list *new = (t_list *)malloc(sizeof(t_list) + 1);
    memset(new, 0, sizeof(t_list));
    new->next = NULL;
    new->prev = NULL;

    token *tok = (token *)malloc(sizeof(token));
    memset(tok->literal, 0, MAX_LITERAL);
    strncpy(tok->literal, "HEAD", strlen("HEAD"));

    tok->type = T_HEAD;
    tok->line = 0;
    tok->col  = 0;

    new->tok = tok;

    return new;
}

void t_list_free(t_list *lst) {
    // Seek to end
    while (lst->next != NULL) {
        lst = lst->next;
    }

    // Walk back up the list
    while (lst->prev != NULL) {
        lst = lst->prev;

        free(lst->next->tok);
        free(lst->next);
    }

    if (lst != NULL) {
        free(lst);
    }
}

void t_list_append(t_list *lst, t_list *new_tok) {
    if (lst != NULL) {
        if (new_tok != NULL) {
            while (lst->next != NULL) {
                lst = t_list_next(lst);
            }

            if (lst->next == NULL) {
                new_tok->prev = lst;
                new_tok->next = NULL;

                lst->next = new_tok;

                token_count++;
            }
        } else {
            printf("ERROR: Cannot access new_tok\n");
        }
    } else {
        printf("ERROR: Cannot access lst\n");
    }
}

t_list *t_list_next(t_list *lst) {
    if (lst == NULL) {
        return NULL;
    } else {
        return lst->next;
    }
}

t_list *t_list_prev(t_list *lst) {
    if (lst == NULL) {
        printf("null element\n");
        return NULL;
    } else {
        return lst->prev;
    }
}

void print_list(t_list *lst) {
    if (lst != NULL) {
        t_list *lst_ptr = lst;

        while (lst_ptr != NULL) {
            printf("Type: %d\n", lst_ptr->tok->type);
            printf("Literal: %s\n", lst_ptr->tok->literal);
            printf("Line: %d\n", lst_ptr->tok->line);

            lst_ptr = t_list_next(lst_ptr);
        }

        printf("\nNum tokens: %u\n\n", token_count);
    }
}
