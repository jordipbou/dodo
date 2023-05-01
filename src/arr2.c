#include "arr.h"
#include<stdio.h>

int fib(CONTEXT* x) {
	dup(x);
	PUSH(x, 1);
	gt(x);
	if (POP(x)) {
		PUSH(x, 1);
		sub(x);
		dup(x);
		PUSH(x, 1);
		sub(x);
		fib(x);
		swap(x);
		fib(x);
		add(x);
	}
}

int main() {
	CONTEXT* x = init();
	BYTE* i;
	CELL n;
	/*BYTE* c = ":d1>?1-d1-cs`+();";*/
	BYTE c[19] = { ':', 'd', '1', '>', 'z', 12, '1', '-', 'd', '1', '-', 'c', -11, 's', 'c', -14, '+', ';', 0 };

/*
	PUSH(x, 36);
	fib(x);
	dump_stack(x);
*/

	PUSH(x, 36);
	/*PUSH(x, 8);*/
	x->ip = c;
	while (x->ip != 0) { 
		/*
		for (n = 0; n < S(x)->length; n++) printf("%ld ", S(x)->data[n]);
		printf("||");
		for (i = x->ip; i < (c + 18); i++) { if (*i > 32) { printf("%c", *(i)); } else { printf("%d", *i); } }
		*/
		stepI(x);
		/*
		getchar();
		*/
	}
	dump_stack(x);
}
