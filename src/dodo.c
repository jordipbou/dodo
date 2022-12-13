#include<stdio.h>
#include "dodo.h"

void fib(CTX* ctx) {
	_dup(ctx);
	push(ctx, 1);
	_gt(ctx);
	if (pop(ctx)) {
		_dec(ctx);
		_dup(ctx);
		_dec(ctx);
		fib(ctx);
		_swap(ctx);
		fib(ctx);
		_add(ctx);
	}
}

void main() {
	BYTE block[65536];
	CTX* ctx = init(block, 65536);
	push(ctx, 36);
	fib(ctx);
	printf("%ld\n", pop(ctx));
}
