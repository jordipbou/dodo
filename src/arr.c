#include "arr.h"

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
	BYTE* c = ":d1>?1-d1-`s`+();";

/*
	PUSH(x, 36);
	fib(x);
	dump_stack(x);
*/

	PUSH(x, 36);
	x->ip = c;
	while (x->ip != 0) { stepI(x); }
	dump_stack(x);
}
