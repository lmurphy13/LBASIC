#include <stdio.h>
#include "token.h"
#include "lexer.h"

void print_usage() {
	printf("LBASIC Usage\n");
	printf("\t./lbasic <path>\n");
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
		t_list *token_list = lex(argv[1]);

		if (token_list != NULL) {
			print_list(token_list);
		} else {
			printf("ERROR: Provide valid file path.\n");
		}
	} else {
		print_usage();
	}
}
