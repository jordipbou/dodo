#include<stdio.h>
#include "dodo.h"

void main() {
	CELL size = 8192;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stacks.stack = cons(ctx, T_ATOM, 36, ctx->stacks.stack);

	PAIR* xlist = 
		cons(ctx, T_PRIMITIVE, (CELL)&_dup,
		cons(ctx, T_ATOM, 1,
		cons(ctx, T_PRIMITIVE, (CELL)&_gt,
		cons(ctx, T_LIST, 
			// False
			AS(ST_BRANCH, NIL),
			// True
			cons(ctx, T_ATOM, 1,
			cons(ctx, T_PRIMITIVE, (CELL)&_sub,
			cons(ctx, T_PRIMITIVE, (CELL)&_dup,
			cons(ctx, T_ATOM, 1,
			cons(ctx, T_PRIMITIVE, (CELL)&_sub,
			cons(ctx, T_LIST, AS(ST_RECURSION, NIL),
			cons(ctx, T_PRIMITIVE, (CELL)&_swap,
			cons(ctx, T_LIST, AS(ST_RECURSION, NIL),
			cons(ctx, T_PRIMITIVE, (CELL)&_add, NIL)))))))))))));
	
	//ctx->rstack = cons(ctx, T_LIST, (CELL)AS(ST_WORD, ctx->ip), ctx->rstack);

	inner(ctx, xlist);

	printf("%ld\n", TOS(ctx));
}
