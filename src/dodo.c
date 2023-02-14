#include<stdio.h>
#include"dodo.h"

void main() {
	//CELL size = 20000;
	//BYTE block[size];
	//CTX* ctx = init(block, size);

	//reveal(ctx, immediate(primitive(body(ctx, header(ctx, ":"), cons(ctx, (CELL)&D_colon, AS(PRIM, 0))))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "PARSE"), cons(ctx, (CELL)&D_parse, AS(PRIM, 0)))));
	//reveal(ctx, immediate(primitive(body(ctx, header(ctx, ";"), cons(ctx, (CELL)&D_semicolon, AS(PRIM, 0))))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "IMMEDIATE"), cons(ctx, (CELL)&D_immediate, AS(PRIM, 0)))));

	//reveal(ctx, primitive(body(ctx, header(ctx, "SP@"), cons(ctx, (CELL)&D_sp_fetch, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "@"), cons(ctx, (CELL)&D_fetch, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "!"), cons(ctx, (CELL)&D_store, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "+"), cons(ctx, (CELL)&D_add, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "NAND"), cons(ctx, (CELL)&D_nand, AS(PRIM, 0)))));

	//reveal(ctx, primitive(body(ctx, header(ctx, "HERE"), cons(ctx, (CELL)&D_here, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "LATEST"), cons(ctx, (CELL)&D_latest, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "STATE"), cons(ctx, (CELL)&D_state, AS(PRIM, 0)))));

	//reveal(ctx, primitive(body(ctx, header(ctx, "BASE"), cons(ctx, (CELL)&D_base, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "CELL"), cons(ctx, (CELL)&D_cell, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "OVER"), cons(ctx, (CELL)&D_over, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "PARSE-NAME"), cons(ctx, (CELL)&D_parse_name, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "SLITERAL"), cons(ctx, (CELL)&D_s_literal, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "HEADER"), cons(ctx, (CELL)&D_header, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, "REVEAL"), cons(ctx, (CELL)&D_reveal, AS(PRIM, 0)))));
	//reveal(ctx, primitive(body(ctx, header(ctx, ","), cons(ctx, (CELL)&D_comma, AS(PRIM, 0)))));

	//reveal(ctx, primitive(body(ctx, header(ctx, ".S"), cons(ctx, (CELL)&_dump_stack, AS(PRIM, 0)))));

	////reveal(ctx, primitive(body(ctx, header(ctx, "+"), cons(ctx, (CELL)&D_add, AS(PRIM, 0)))));
	////reveal(ctx, primitive(body(ctx, header(ctx, "dup"), cons(ctx, (CELL)&_dup, AS(PRIM, 0)))));
	////reveal(ctx, immediate(primitive(body(ctx, header(ctx, "{"), cons(ctx, (CELL)&_lbrace, AS(PRIM, 0))))));
	////reveal(ctx, immediate(primitive(body(ctx, header(ctx, "}"), cons(ctx, (CELL)&_rbrace, AS(PRIM, 0))))));
	////reveal(ctx, primitive(body(ctx, header(ctx, ".b"), cons(ctx, (CELL)&_dump_body, AS(PRIM, 0)))));
	////reveal(ctx, primitive(body(ctx, header(ctx, "execute"), cons(ctx, (CELL)&_execute, AS(PRIM, 0)))));
	////reveal(ctx, primitive(body(ctx, header(ctx, ".s"), cons(ctx, (CELL)&_dump_stack, AS(PRIM, 0)))));

	//FILE *fptr;
	//BYTE buf[255];
	//CELL result;
	////fptr = fopen("test.dodo", "r");
	////fptr = fopen("ttester.fs", "r");
	//fptr = fopen("dodo.fs", "r");
	//while (fgets(buf, 255, fptr)) {
	//	printf("BUF: %s", buf);
	//	result = evaluate(ctx, buf);
	//	if (result != 0) { 
	//		printf("ERROR: %ld\n", result); 
	//		_dump_stack(ctx);
	//		_dump_compiled(ctx);
	//		_dump_defs(ctx);
	//		printf("TOKEN: %.*s\n", ctx->in - ctx->token, ctx->tib + ctx->token);
	//		CELL p = ctx->latest;
	//		printf("WORDS: ");
	//		while (p) {
	//			printf("%s ", NFA(p));
	//			p = NEXT(p);
	//		}
	//		printf("\n");
	//		return; 
	//	}
	//}
	//do {
	//	fgets(buf, 255, stdin);
	//	result = evaluate(ctx, buf);
	//	if (result != 0) { 
	//		printf("ERROR: %ld\n", result); 
	//		_dump_stack(ctx);
	//		_dump_compiled(ctx);
	//		_dump_defs(ctx);
	//		printf("TOKEN: %.*s\n", ctx->in - ctx->token, ctx->tib + ctx->token);
	//		CELL p = ctx->latest;
	//		printf("WORDS: ");
	//		while (p) {
	//			printf("%s ", NFA(p));
	//			p = NEXT(p);
	//		}
	//		printf("\n");
	//		return; 
	//	}
	//} while(1);
}
