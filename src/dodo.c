#include<stdio.h>
#include"dodo.h"

//CELL find_prim(CTX* ctx, CELL addr) {
//	CELL word = ctx->latest;
//	while (word && CAR(XT(word)) != addr) {
//		word = NEXT(word);
//	}
//	return word;
//}
//
//void dump_list(CTX* ctx, CELL pair, CELL order) {
//	if (pair) {
//		if (order) dump_list(ctx, NEXT(pair), order);
//		switch (TYPE(pair)) {
//			case ATOM: printf("%ld ", CAR(pair)); break;
//			case LIST: printf("{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
//			case PRIM: printf("%s ", (BYTE*)NFA(find_prim(ctx, CAR(pair)))); break;
//			case CALL: printf("X::{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
//		}
//		if (!order) dump_list(ctx, NEXT(pair), order);
//	}
//}
//
//CELL _dump_stack(CTX* ctx) {
//	printf("<%ld> ", length(ctx->stack));
//	dump_list(ctx, ctx->stack, 1);
//	printf("\n");
//
//	return 0;
//}

void main() {
	CELL size = 20000;
	BYTE block[size];
	CTX* ctx = bootstrap(init(block, size));

	//add_primitive(ctx, ".s", &_dump_stack);
	//add_primitive(ctx, "+", &D_add);
	//add_primitive(ctx, "{", &D_lbrace);
	//add_primitive(ctx, "}", &D_rbrace);
	//add_primitive(ctx, ":", &D_colon);
	//add_primitive(ctx, ";", &D_semicolon);

	FILE *fptr;
	BYTE buf[255];
	CELL result;
	fptr = fopen("test.dodo", "r");
	////fptr = fopen("ttester.fs", "r");
	////fptr = fopen("dodo.fs", "r");
	while (fgets(buf, 255, fptr)) {
		result = evaluate(ctx, buf);
		if (result != 0) { 
			printf("ERROR: %ld\n", result); 
		}
	}
	do {
		fgets(buf, 255, stdin);
		result = evaluate(ctx, buf);
		if (result != 0) { 
			printf("ERROR: %ld\n", result); 
		}
	} while(1);
}
