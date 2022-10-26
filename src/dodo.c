#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

void fib(C* bl) {
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
	printf("Sizeof byte: %ld\n", sizeof(B));
	printf("Sizeof cell: %ld\n", sizeof(C));
	printf("Sizeof pair: %ld\n", sizeof(P));
	printf("Sizeof header: %ld\n", sizeof(H));

	C bl[262000];
	init_bl(bl, 262000);

	// :fib d1>?_d_`s`+();
	allot(bl, 15);
	PC(bl) = (C)(bl + Hsz);
	*((C*)(PC(bl) + 0)) = ':';
	*((C*)(PC(bl) + 1)) = 'd';
	*((C*)(PC(bl) + 2)) = '1';
	*((C*)(PC(bl) + 3)) = '>';
	*((C*)(PC(bl) + 4)) = '?';
	*((C*)(PC(bl) + 5)) = '_';
	*((C*)(PC(bl) + 6)) = 'd';
	*((C*)(PC(bl) + 7)) = '_';
	*((C*)(PC(bl) + 8)) = '`';
	*((C*)(PC(bl) + 9)) = 's';
	*((C*)(PC(bl) + 10)) = '`';
	*((C*)(PC(bl) + 11)) = '+';
	*((C*)(PC(bl) + 12)) = '(';
	*((C*)(PC(bl) + 13)) = ')';
	*((C*)(PC(bl) + 14)) = ';';

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

	printf("%ld  ok\n", pop(bl));

	//free(c);
	//dump_block(b);
}
