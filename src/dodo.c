#include<stdio.h>
#include "dodo.h"

void main() {
	CELL size = 8192;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stacks.stack = alloc(ctx, T_ATOM, 36, ctx->stacks.stack);

	PAIR* xlist = 
		alloc(ctx, T_PRIMITIVE, (CELL)&_dup,
		alloc(ctx, T_ATOM, 1,
		alloc(ctx, T_PRIMITIVE, (CELL)&_gt,
		alloc(ctx, T_LIST, 
			// False
			AS(ST_BRANCH, NIL),
			// True
			alloc(ctx, T_ATOM, 1,
			alloc(ctx, T_PRIMITIVE, (CELL)&_sub,
			alloc(ctx, T_PRIMITIVE, (CELL)&_dup,
			alloc(ctx, T_ATOM, 1,
			alloc(ctx, T_PRIMITIVE, (CELL)&_sub,
			alloc(ctx, T_LIST, AS(ST_RECURSION, NIL),
			alloc(ctx, T_PRIMITIVE, (CELL)&_swap,
			alloc(ctx, T_LIST, AS(ST_RECURSION, NIL),
			alloc(ctx, T_PRIMITIVE, (CELL)&_add, NIL)))))))))))));
	
	//ctx->rstack = alloc(ctx, T_LIST, (CELL)AS(ST_WORD, ctx->ip), ctx->rstack);

	inner(ctx, xlist);

	printf("%ld\n", TOS(ctx));
}
