#include<stdio.h>
#include "dodo.h"

void dump_stack(CTX* ctx) {
	printf("<%ld> ", depth(ctx->dstack));
	PAIR* p = ctx->dstack;
	while (p != NIL) { printf("%ld ", p->value); p = NEXT(p); }
	printf("\n");
}

void fib(CTX* ctx) {
	dup(ctx);
	ctx->dstack = ncons(ctx, 1, ctx->dstack);
	gt(ctx);
	if (TOS(ctx)) {
		POP(ctx);
		ctx->dstack = ncons(ctx, 1, ctx->dstack);
		sub(ctx);
		dup(ctx);
		ctx->dstack = ncons(ctx, 1, ctx->dstack);
		sub(ctx);
		fib(ctx);
		swap(ctx);
		fib(ctx);
		add(ctx);
	} else {
		POP(ctx);
	}
}

void main() {
	CELL size = 8192;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->dstack = ncons(ctx, 36, ctx->dstack);

	fib(ctx);

	printf("%ld\n", TOS(ctx));
}
