#include<stdio.h>
#include"dodo.h"

void main() {
	CELL size = 20000;
	BYTE block[size];
	CTX* ctx = init(block, size);

	reveal(ctx, immediate(primitive(body(header(ctx, ":"), cons(ctx, (CELL)&D_colon, AS(PRIM, 0))))));
	reveal(ctx, primitive(body(header(ctx, "parse"), cons(ctx, (CELL)&D_parse, AS(PRIM, 0)))));
	reveal(ctx, immediate(primitive(body(header(ctx, ";"), cons(ctx, (CELL)&D_semicolon, AS(PRIM, 0))))));
	reveal(ctx, primitive(body(header(ctx, "immediate"), cons(ctx, (CELL)&D_immediate, AS(PRIM, 0)))));

	reveal(ctx, primitive(body(header(ctx, "sp@"), cons(ctx, (CELL)&D_sp_fetch, AS(PRIM, 0)))));
	reveal(ctx, primitive(body(header(ctx, "@"), cons(ctx, (CELL)&D_fetch, AS(PRIM, 0)))));
	reveal(ctx, primitive(body(header(ctx, "+"), cons(ctx, (CELL)&D_add, AS(PRIM, 0)))));
	reveal(ctx, primitive(body(header(ctx, "nand"), cons(ctx, (CELL)&D_nand, AS(PRIM, 0)))));

	reveal(ctx, primitive(body(header(ctx, ".s"), cons(ctx, (CELL)&_dump_stack, AS(PRIM, 0)))));

	//reveal(ctx, primitive(body(header(ctx, "+"), cons(ctx, (CELL)&D_add, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(header(ctx, "dup"), cons(ctx, (CELL)&_dup, AS(PRIM, 0)))));
	//reveal(ctx, immediate(primitive(body(header(ctx, "{"), cons(ctx, (CELL)&_lbrace, AS(PRIM, 0))))));
	//reveal(ctx, immediate(primitive(body(header(ctx, "}"), cons(ctx, (CELL)&_rbrace, AS(PRIM, 0))))));
	//reveal(ctx, primitive(body(header(ctx, ".b"), cons(ctx, (CELL)&_dump_body, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(header(ctx, "execute"), cons(ctx, (CELL)&_execute, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(header(ctx, ".s"), cons(ctx, (CELL)&_dump_stack, AS(PRIM, 0)))));

	FILE *fptr;
	BYTE buf[255];
	CELL result;
	//fptr = fopen("test.dodo", "r");
	fptr = fopen("01-helloworld.forth", "r");
	while (fgets(buf, 255, fptr)) {
		printf("BUF: %s", buf);
		result = evaluate(ctx, buf);
		if (result != 0) { 
			printf("ERROR: %ld\n", result); 
			_dump_stack(ctx);
			_dump_compiled(ctx);
			_dump_defs(ctx);
			printf("TOKEN: %.*s\n", ctx->in - ctx->token, ctx->tib + ctx->token);
			CELL p = ctx->latest;
			printf("WORDS: ");
			while (p) {
				printf("%s ", NFA(p));
				p = NEXT(p);
			}
			printf("\n");
			return; 
		}
	}
	//CELL result;
	//do {
	//	fgets(buf, 255, stdin);
	//	result = evaluate(ctx, buf);
	//	if (result != 0) { printf("ERROR: %ld\n", result); return; }
	//} while(1);
}
