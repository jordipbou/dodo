#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

void fib(ctx_t *c) {
	DUP(c);
	LIT1(c);	
	GT(c);
	if (c->T != 0) {
		c->T = c->S; c->S = pop(c->scope);
		DEC(c);
		DUP(c);
		DEC(c);
		fib(c);
		SWAP(c);
		fib(c);
		ADD(c);
	} else {
		c->T = c->S; c->S = pop(c->scope);
	}
}

void main() {
	cell_t b[262000];
	ctx_t *c = malloc(sizeof(ctx_t));
	scp_t *s = malloc(sizeof(scp_t));
	s->block = init_block(b, 262000);
	c->scope = s;
	c->T = c->S = 0;

	c->T = 36;
	fib(c);

	printf("%ld  ok\n", c->T);

	//free(c);
	//cell_t b[32];
	//init_block(b, 32);
	//printf("Free list length: %ld\n", length(FREE_LIST_HEAD(b)));
	//dump_block(b);
}
