#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include"dodo.h"
#include"dodo_dbg.h"

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}

CELL fib(CTX* ctx) {
	CELL result;
	//R(ctx) = cons(ctx, (CELL)&fib, AS(PRIM, R(ctx)));
	//duplicate(ctx);
	//PUSH(ctx, 1, &S(ctx));
	//gt(ctx);
	//CELL b = pop(ctx, &S(ctx));
	if (CAR(S(ctx)) > 1) {
		//PUSH(ctx, 1, &S(ctx));
		//sub(ctx);
		//duplicate(ctx);
		//PUSH(ctx, 1, &S(ctx));
		//sub(ctx);
		CAR(S(ctx)) = CAR(S(ctx)) - 1;
		if ((S(ctx) = cons(ctx, CAR(S(ctx)) - 1, S(ctx))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
		if ((result = fib(ctx)) != 0) { ERR(ctx, result); }
		//swap(ctx);
		CELL t = CAR(S(ctx));
		CAR(S(ctx)) = CAR(NEXT(S(ctx)));
		CAR(NEXT(S(ctx))) = t;
		if ((result = fib(ctx)) != 0) { ERR(ctx, result); }
		//add(ctx);
		CAR(NEXT(S(ctx))) = CAR(NEXT(S(ctx))) + CAR(S(ctx));
		if ((S(ctx) = reclaim(ctx, S(ctx))) == 0) { ERR(ctx, ERR_STACK_UNDERFLOW); }
	}
	//R(ctx) = reclaim(ctx, R(ctx));
	return 0;
}

void main(int argc, char *argv[]) {
	CELL size = 1638400;
	BYTE block[size];
	//CTX* ctx = init(block, size);

	//PUSH(ctx, 36, &S(ctx));
	//CELL result = fib(ctx);

	//if (result != 0) {
	//	printf("ERROR: %ld\n", result);
	//} else {
	//	printf("%ld\n", CAR(S(ctx)));
	//}

	CTX* ctx = bootstrap(init(block, size));

	ADD_PRIMITIVE(ctx, ".s", &dump_stack, 0);
	ADD_PRIMITIVE(ctx, "words", &words, 0);
	ADD_PRIMITIVE(ctx, "see", &see, 0);
	ADD_PRIMITIVE(ctx, "nfa", &nfa, 0);
	ADD_PRIMITIVE(ctx, "xt", &xt, 0);
	ADD_PRIMITIVE(ctx, "pair", &pair, 0);
	ADD_PRIMITIVE(ctx, "next", &next, 0);
	ADD_PRIMITIVE(ctx, "sp@", &sp_fetch, 0);
	ADD_PRIMITIVE(ctx, "carcdr", &carcdr, 0);

//ADD_PRIMITIVE(ctx, "x", &dbg_exec_x, 0);

	FILE *fptr;
	BYTE buf[255];
	CELL result;
	if (argc == 2) {
		fptr = fopen(argv[1], "r");
		while (fgets(buf, 255, fptr)) {
			//result = dbg_evaluate(ctx, strlwr(buf));
			//result = evaluate(ctx, strlwr(buf));
			result = evaluate(ctx, buf);
			if (result != 0) {
					printf("ERROR: %ld\n", result);
					return;
			}
		}
	} else {
		do {
			fgets(buf, 255, stdin);
			//result = dbg_evaluate(ctx, buf);
			result = evaluate(ctx, buf);
			if (result != 0) {
				printf("ERROR: %ld\n", result);
				return;
			}
		} while(1);
	}
//	CTX* ctx = bootstrap(init(block, size));
//
//	FILE *fptr;
//	BYTE buf[255];
//	CELL result;
//	if (argc == 2) {
//		fptr = fopen(argv[1], "r");
//		while (fgets(buf, 255, fptr)) {
//			result = evaluate(ctx, strlwr(buf));
//			if (result != 0) { 
//				switch (result) {
//					case -1: printf("Stack overflow\n"); break;
//					case -2: printf("Stack underflow\n"); break;
//					case -3: printf("Undefined word: %.*s\n", (int)(ctx->in - ctx->token), ctx->tib + ctx->token); break;
//					case -4: printf("Not enough memory\n"); break;
//					case -5: printf("Zero length name\n"); break;
//					case -6: printf("Atom expected\n"); break;
//					case -7: printf("Return stack underflow\n"); break;
//					case -8: break;
//					default: printf("ERROR: %ld\n", result); break;
//				}
//				printf("TIB: %s\n", ctx->tib + ctx->token);
//				return;
//			}
//		}
//	} else {
//		do {
//			fgets(buf, 255, stdin);
//			result = evaluate(ctx, buf);
//			if (result != 0) { 
//				switch (result) {
//					case -1: printf("Stack overflow\n"); break;
//					case -2: printf("Stack underflow\n"); break;
//					case -3: printf("Undefined word: %.*s\n", (int)(ctx->in - ctx->token), ctx->tib + ctx->token); break;
//					case -4: printf("Not enough memory\n"); break;
//					case -5: printf("Zero length name\n"); break;
//					case -6: printf("Atom expected\n"); break;
//					case -7: printf("Return stack underflow\n"); break;
//					case -8: break;
//					default: printf("ERROR: %ld\n", result); break;
//				}
//				printf("TIB: %s\n", ctx->tib + ctx->token);
//				return;
//			}
//		} while(1);
//	}
}
