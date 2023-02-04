#include<stdio.h>
#include<string.h>
#include<inttypes.h>
#include<errno.h>
#include"dodo.h"

void main() {
	C size = 16384;
	B block[size];
	X* x = bootstrap(init(block, size));

	B ibuf[255];
	B* tok;

	while (!x->err) {
		printf("> ");
		scanf("%s", ibuf);
		tok = strtok(ibuf, " ");
		while (tok != NULL) {
			C w = find(x, tok);
			if (w) inner(x, LAMBDA(x, A(w), 0));
			else {
				intmax_t num = strtoimax(tok, NULL, 10);
				if (num == INTMAX_MAX && errno == ERANGE) {
					x->err = -1000;
				} else {
					inner(x, ATOM(x, num, 0));
				}
			}
			tok = strtok(NULL, " ");
		}
		printf(" ok <%ld>\n", lth(x->s));
		//printf("<%ld> %ld %ld...\n", lth(x->s), x->s ? A(x->s) : 0, D_(x->s) ? A(D_(x->s)) : 0);
	}
}
