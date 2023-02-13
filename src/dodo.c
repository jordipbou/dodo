#include<stdio.h>
#include"dodo.h"

void main() {
	CELL size = 8192;
	BYTE block[size];
	CTX* ctx = init(block, size);

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

	//ctx->latest =
		//cons(ctx, cons(ctx, (CELL)"+", AS(ATOM, cons(ctx, (CELL)&_add, AS(PRIM, 0)))), AS(LIST, 
		//cons(ctx, cons(ctx, (CELL)"dup", AS(ATOM, cons(ctx, (CELL)&_dup, AS(PRIM, 0)))), AS(LIST,
		//cons(ctx, cons(ctx, (CELL)":", AS(ATOM, cons(ctx, (CELL)&_colon, AS(PRIM, 0)))), AS(CALL,
		//cons(ctx, cons(ctx, (CELL)";", AS(ATOM, cons(ctx, (CELL)&_semicolon, AS(PRIM, 0)))), AS(CALL, 
		//0))))))));

	reveal(ctx, primitive(body(header(ctx, "+"), cons(ctx, (CELL)&_add, AS(PRIM, 0)))));
	reveal(ctx, primitive(body(header(ctx, "dup"), cons(ctx, (CELL)&_dup, AS(PRIM, 0)))));
	reveal(ctx, immediate(primitive(body(header(ctx, ":"), cons(ctx, (CELL)&_colon, AS(PRIM, 0))))));
	reveal(ctx, immediate(primitive(body(header(ctx, ";"), cons(ctx, (CELL)&_semicolon, AS(PRIM, 0))))));
	reveal(ctx, immediate(primitive(body(header(ctx, "{"), cons(ctx, (CELL)&_lbrace, AS(PRIM, 0))))));
	reveal(ctx, immediate(primitive(body(header(ctx, "}"), cons(ctx, (CELL)&_rbrace, AS(PRIM, 0))))));
	reveal(ctx, primitive(body(header(ctx, ".b"), cons(ctx, (CELL)&_dump_body, AS(PRIM, 0)))));
	reveal(ctx, primitive(body(header(ctx, "execute"), cons(ctx, (CELL)&_execute, AS(PRIM, 0)))));
	reveal(ctx, primitive(body(header(ctx, ".s"), cons(ctx, (CELL)&_dump_stack, AS(PRIM, 0)))));

	BYTE buf[255];
	CELL result;
	do {
		fgets(buf, 255, stdin);
		result = evaluate(ctx, buf);
		if (result != 0) { printf("ERROR: %ld\n", result); return; }
	} while(1);
}
