#include<stdio.h>
#include "dodo.h"

void main() {
	CELL size = 8192;
	BYTE block[size];
	CTX* ctx = init(block, size);

	PUSH(ctx, AS(ATOM, 36));

	CELL xlist = 
		PRIM(ctx, &_dup, 
		cons(ctx, AS(ATOM, 1),
		PRIM(ctx, &_gt,
		COND(ctx,
			// True branch
			cons(ctx, AS(ATOM, 1),
			PRIM(ctx, &_sub,
			PRIM(ctx, &_dup,
			cons(ctx, AS(ATOM, 1),
			PRIM(ctx, &_sub,
			REC(ctx, 
			PRIM(ctx, &_swap,
			REC(ctx, 
			PRIM(ctx, &_add, NIL))))))))),
			// False
			NIL))));
	
	inner(ctx, xlist);

	printf("%ld\n", VALUE(TOS(ctx)));
}
