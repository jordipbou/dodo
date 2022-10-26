#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

void fib(H* bl) {
	DUP(bl);
	LIT(bl, 1);	
	GT(bl);
	if (pop(bl) != 0) {
		DEC(bl);
		DUP(bl);
		DEC(bl);
		fib(bl);
		SWAP(bl);
		fib(bl);
		ADD(bl);
	}
}

void main() {
	C b[262000];
	H* bl = init_bl(b, 262000);

	// :fib d1>?_d_`s`+();
	allot(bl, 15);
	*(bl->ip + 0) = ':';
	*(bl->ip + 1) = 'd';
	*(bl->ip + 2) = '1';
	*(bl->ip + 3) = '>';
	*(bl->ip + 4) = '?';
	*(bl->ip + 5) = '_';
	*(bl->ip + 6) = 'd';
	*(bl->ip + 7) = '_';
	*(bl->ip + 8) = '`';
	*(bl->ip + 9) = 's';
	*(bl->ip + 10) = '`';
	*(bl->ip + 11) = '+';
	*(bl->ip + 12) = '(';
	*(bl->ip + 13) = ')';
	*(bl->ip + 14) = ';';

	LIT(bl, 36);
	eval(bl);
	//fib(c);

	//allot(b, 3);
	//c->PC = (char *)(b + Hsz);
	//*(c->PC + 0) = '1';
	//*(c->PC + 1) = '+';
	//*(c->PC + 2) = ';';

	//c->T = 2;
	//eval(c);

	//printf("%ld  ok\n", pop(bl));
	dump_stack(bl);

	//free(c);
	//dump_block(b);
}
