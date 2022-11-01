#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int token_count = 0;

t_list *t_list_new(void) {
    t_list *new = (t_list *)malloc(sizeof(t_list));
    new->next   = NULL;
    new->prev   = NULL;

    token *tok = (token *)malloc(sizeof(token));
    memset(tok->literal, 0, MAX_LITERAL);
    tok->type = T_HEAD;
    strncpy(tok->literal, "HEAD", strlen("HEAD"));

    new->tok = tok;

    if (new != NULL) {
        return new;
    } else {
        return NULL;
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

            lst_ptr = t_list_next(lst_ptr);
        }

        printf("\nNum tokens: %u\n", token_count);
    }
}
