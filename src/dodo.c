#include<stdio.h>
#include "dodo.h"

void main() {
	CELL size = 8192;
	BYTE block[size];
	CTX* ctx = init(block, size);

	push(ctx, 40);

	CELL xlist = 
		PRIM(ctx, &_dup, 
		cons(ctx, 1, 
		PRIM(ctx, &_gt,
		COND(ctx, 
			// True branch
			cons(ctx, 1,
			PRIM(ctx, &_sub,
			PRIM(ctx, &_dup,
			cons(ctx, 1,
			PRIM(ctx, &_sub,
			PRIM(ctx, RECURSION, 
			PRIM(ctx, &_swap,
			PRIM(ctx, RECURSION,
			PRIM(ctx, &_add, NIL))))))))),
			// False
			NIL))));
	
	inner(ctx, xlist);

	printf("%ld\n", TOS(ctx));
}
