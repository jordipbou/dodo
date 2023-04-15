#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
//#include"dodo.h"
#include "simp.h"

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}

// CURRENT: 16s
// TEST: 6.33s (Before using execution pile of stacks)
void fib(CTX* ctx) {
	ctx->ip =
		cons(&ctx->fstack, 30, AS(ATOM,
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, (CELL)&swap, AS(PRIM,
			cons(&ctx->fstack, (CELL)&duplicate, AS(PRIM,
			cons(&ctx->fstack, 1, AS(ATOM,
			cons(&ctx->fstack, (CELL)&gt, AS(PRIM,
			cons(&ctx->fstack, (CELL)
				cons(&ctx->fstack, 1, AS(ATOM,
				cons(&ctx->fstack, (CELL)&sub, AS(PRIM,
				cons(&ctx->fstack, (CELL)&duplicate, AS(PRIM,
				cons(&ctx->fstack, 1, AS(ATOM,
				cons(&ctx->fstack, (CELL)&sub, AS(PRIM,
				cons(&ctx->fstack, (CELL)&rot, AS(PRIM,
				cons(&ctx->fstack, (CELL)&exec_x, AS(PRIM,
				cons(&ctx->fstack, (CELL)&rot, AS(PRIM,
				cons(&ctx->fstack, (CELL)&rot, AS(PRIM,
				cons(&ctx->fstack, (CELL)&exec_x, AS(PRIM,
				cons(&ctx->fstack, (CELL)&rot, AS(PRIM,
				cons(&ctx->fstack, (CELL)&add, AS(PRIM, 0)))))))))))))))))))))))),
			AS(LIST,
			cons(&ctx->fstack, 0, AS(LIST,
			cons(&ctx->fstack, (CELL)&branch, AS(PRIM, 0)))))))))))))),
		AS (LIST,
		cons(&ctx->fstack, (CELL)&exec_x, AS(PRIM,
		cons(&ctx->fstack, (CELL)&swap, AS(PRIM,
		cons(&ctx->fstack, (CELL)&drop, AS(PRIM, 0))))))))));

	while(step(ctx));

	//printf("<%ld> %ld\n", length(S(ctx)), A(S(ctx)));
	//dump_context(ctx);
	BYTE buf[255];
	buf[0] = 0;
	print(buf, S(ctx), 1, ctx);
	printf("%s\n", buf);
}

void fibW(CTX* ctx) {
	ctx->ip =
		cons(&ctx->fstack, 30, AS(ATOM,
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, (CELL)&swap, AS(PRIM,
			cons(&ctx->fstack, (CELL)&duplicate, AS(PRIM,
			cons(&ctx->fstack, 1, AS(ATOM,
			cons(&ctx->fstack, (CELL)&gt, AS(PRIM,
			cons(&ctx->fstack, (CELL)&ifelse, AS(PRIM,
			cons(&ctx->fstack, (CELL)
				cons(&ctx->fstack, 1, AS(ATOM,
				cons(&ctx->fstack, (CELL)&sub, AS(PRIM,
				cons(&ctx->fstack, (CELL)&duplicate, AS(PRIM,
				cons(&ctx->fstack, 1, AS(ATOM,
				cons(&ctx->fstack, (CELL)&sub, AS(PRIM,
				cons(&ctx->fstack, (CELL)&rot, AS(PRIM,
				cons(&ctx->fstack, (CELL)&exec_x, AS(PRIM,
				cons(&ctx->fstack, (CELL)&rot, AS(PRIM,
				cons(&ctx->fstack, (CELL)&rot, AS(PRIM,
				cons(&ctx->fstack, (CELL)&exec_x, AS(PRIM,
				cons(&ctx->fstack, (CELL)&rot, AS(PRIM,
				cons(&ctx->fstack, (CELL)&add, AS(PRIM, 0)))))))))))))))))))))))),
			AS(LIST, 
			cons(&ctx->fstack, 0, AS(LIST, 0)))))))))))))),
		AS (LIST,
		cons(&ctx->fstack, (CELL)&exec_x, AS(PRIM,
		cons(&ctx->fstack, (CELL)&swap, AS(PRIM,
		cons(&ctx->fstack, (CELL)&drop, AS(PRIM, 0))))))))));

	while(step(ctx));

	printf("<%ld> %ld\n", length(S(ctx), 0), S(ctx)->value);
}

//// TEST: 0.22s
//void fibR(CTX* ctx) {
//	if (A(S(ctx)) > 1) {
//		A(S(ctx)) -= 1;
//		duplicate(ctx);
//		A(S(ctx)) -= 1;
//		fibR(ctx);
//		swap(ctx);
//		fibR(ctx);
//		add(ctx);
//	}
//}
//
//// TEST: 0.04s
//void fibN(CTX* ctx) {
//	S(ctx) = cons(ctx, 36, AS(ATOM, 0));
//	fibR(ctx);
//	printf("<%ld> %ld\n", length(S(ctx)), A(S(ctx)));
//}
//
//int fibC(int n) {
//	if (n > 1) return fibC(n - 1) + fibC(n - 2);
//	else return n;
//}

void dump_data_stack(CTX* ctx, NODE* ds) {
	if (NEXT(ds)) {
		dump_data_stack(ctx, NEXT(ds));
	}
	BYTE buf[255];
	buf[0] = 0;
	print(buf, ds, 0, ctx);
	printf("%s\n", buf);
}

void main(int argc, char *argv[]) {
	CELL sz = 30000;
	BYTE bk[sz];
	//CTX* ctx = init(bk, sz);
	CTX* ctx = bootstrap(init(bk, sz));

	//dump_context(ctx);

	//fib(ctx);
	//fibW(ctx);
	//fibN(ctx);
	//S(ctx) = cons(ctx, 36, AS(ATOM, S(ctx)));
	//fibR(ctx);
	//printf("%d\n", fibC(36));
	//printf("%ld\n", A(S(ctx)));

	FILE *fptr;
	BYTE buf[255];
	if (argc == 2 || argc == 3) {
		fptr = fopen(argv[1], "r");
		while (fgets(buf, 255, fptr)) {
			eval(ctx, buf);
			if (ctx->err != 0 && ctx->err != ERR_END_OF_INPUT_SOURCE) {
					printf("ERROR: %ld\n", ctx->err);
					//dump_context(ctx);
					return;
			}
			ctx->err = 0;
			//dump_context(ctx);
		}
	}

	if (argc == 1 || argc == 3) {
		do {
			printf("IN: ");
			fgets(buf, 255, stdin);
			//TOS(ctx) = cons(ctx, strlen(buf), AS(ATOM, cons(ctx, (CELL)buf, AS(ATOM, TOS(ctx)))));
			eval(ctx, buf);
			if (ctx->err != 0 && ctx->err != ERR_END_OF_INPUT_SOURCE) {
					printf("ERROR: %ld\n", ctx->err);
					//dump_context(ctx);
					return;
			}
			ctx->err = 0;
			if (length(S(ctx), 0) > 0) {
				printf("\n--- Data Stack:\n");
				dump_data_stack(ctx, S(ctx));
			}
			//dump_context(ctx);
		} while(1);
	}

	dump_data_stack(ctx, S(ctx));
}
