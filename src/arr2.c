#include "sloth.h"
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
	x->code->data[0] = (CELL)&dup;
	x->code->data[1] = (CELL)&one;
	x->code->data[2] = (CELL)&gt;
	x->code->data[3] = (CELL)&when;
	x->code->data[4] = (CELL)&one;
	x->code->data[5] = (CELL)&sub;
	x->code->data[6] = (CELL)&dup;
	x->code->data[7] = (CELL)&one;
	x->code->data[8] = (CELL)&sub;
	x->code->data[9] = (CELL)&recurse;
	x->code->data[10] = (CELL)&swap;
	x->code->data[11] = (CELL)&recurse;
	x->code->data[12] = (CELL)&add;
	x->code->data[13] = (CELL)&ret;

	while (stepI(x) != 0) {}

	dump_stack(x);
}
