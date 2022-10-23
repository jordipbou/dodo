#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

void fib(X *c) {
	DUP(c);
	LIT(c, 1);	
	GT(c);
	if (c->T != 0) {
		c->T = c->S; c->S = pop(c->sc);
		DEC(c);
		DUP(c);
		DEC(c);
		fib(c);
		SWAP(c);
		fib(c);
		ADD(c);
	} else {
		c->T = c->S; c->S = pop(c->sc);
	}
}

void main() {
	C b[262000];
	X *c = malloc(sizeof(X));
	S *s = malloc(sizeof(S));
	s->bl = init_bl(b, 262000);
	c->sc = s;
	c->sc->ds = c->sc->rs = NULL;
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
	//dump_block(b);
}
