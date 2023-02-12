#include<stdio.h>
#include"dodo.h"

void main() {
	//CELL size = 8192;
	//BYTE block[size];
	//CTX* ctx = init(block, size);

	//ctx->stack = cons(ctx, 36, AS(ATOM, ctx->stack));

	//// recursive fibonacci
	//CELL xlist = cons(ctx, (CELL)&_dup, 0);
	//CELL rest = 
	//	cons(ctx, 1, AS(ATOM,
	//	cons(ctx, (CELL)&_gt, AS(PRIM,
	//	cons(ctx, (CELL)&_zjump, AS(PRIM,
	//	cons(ctx, 0, AS(ATOM,
	//	cons(ctx, 1, AS(ATOM,
	//	cons(ctx, (CELL)&_sub, AS(PRIM,
	//	cons(ctx, (CELL)&_dup, AS(PRIM,
	//	cons(ctx, 1, AS(ATOM,
	//	cons(ctx, (CELL)&_sub, AS(PRIM,
	//	cons(ctx, xlist, AS(CALL,
	//	cons(ctx, (CELL)&_swap, AS(PRIM,
	//	cons(ctx, xlist, AS(CALL,
	//	cons(ctx, (CELL)&_add, AS(PRIM, 0))))))))))))))))))))))))));

	//CDR(xlist) = AS(PRIM, rest);

	//execute(ctx, xlist);

	//printf("%ld\n", CAR(ctx->stack));
}
