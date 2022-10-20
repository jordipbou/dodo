#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

void fib(ctx_t *c) {
	DUP(c);
	LIT(c, 1);	
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
	c->scope->dstack = c->scope->rstack = NULL;
	c->T = c->S = 0;

	// :fib d1>?_d_`s`+();
	allot(b, 15);
	c->PC = (char *)(b + HEADER_SIZE);
	*(c->PC + 0) = ':';
	*(c->PC + 1) = 'd';
	*(c->PC + 2) = '1';
	*(c->PC + 3) = '>';
	*(c->PC + 4) = '?';
	*(c->PC + 5) = '_';
	*(c->PC + 6) = 'd';
	*(c->PC + 7) = '_';
	*(c->PC + 8) = '`';
	*(c->PC + 9) = 's';
	*(c->PC + 10) = '`';
	*(c->PC + 11) = '+';
	*(c->PC + 12) = '(';
	*(c->PC + 13) = ')';
	*(c->PC + 14) = ';';

	LIT(c, 36);
	eval(c);
	//fib(c);

	//allot(b, 3);
	//c->PC = (char *)(b + HEADER_SIZE);
	//*(c->PC + 0) = '1';
	//*(c->PC + 1) = '+';
	//*(c->PC + 2) = ';';

	//c->T = 2;
	//eval(c);

	printf("%ld  ok\n", c->T);

	//free(c);
	//cell_t b[32];
	//init_block(b, 32);
	//printf("Free list length: %ld\n", length(FREE_LIST_HEAD(b)));
	//dump_block(b);
}
