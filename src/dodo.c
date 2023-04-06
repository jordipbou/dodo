#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include"dodo.h"

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
	IP(ctx) =
		cons(ctx, 36, AS(ATOM,
		cons(ctx,
			cons(ctx, (CELL)&swap, AS(PRIM,
			cons(ctx, (CELL)&duplicate, AS(PRIM,
			cons(ctx, 1, AS(ATOM,
			cons(ctx, (CELL)&gt, AS(PRIM,
			cons(ctx,
				cons(ctx, 1, AS(ATOM,
				cons(ctx, (CELL)&sub, AS(PRIM,
				cons(ctx, (CELL)&duplicate, AS(PRIM,
				cons(ctx, 1, AS(ATOM,
				cons(ctx, (CELL)&sub, AS(PRIM,
				cons(ctx, (CELL)&rot, AS(PRIM,
				cons(ctx, (CELL)&exec_x, AS(PRIM,
				cons(ctx, (CELL)&rot, AS(PRIM,
				cons(ctx, (CELL)&rot, AS(PRIM,
				cons(ctx, (CELL)&exec_x, AS(PRIM,
				cons(ctx, (CELL)&rot, AS(PRIM,
				cons(ctx, (CELL)&add, AS(PRIM, 0)))))))))))))))))))))))),
			AS(LIST,
			cons(ctx, 0, AS(LIST,
			cons(ctx, (CELL)&branch, AS(PRIM, 0)))))))))))))),
		AS (LIST,
		cons(ctx, (CELL)&exec_x, AS(PRIM,
		cons(ctx, (CELL)&swap, AS(PRIM,
		cons(ctx, (CELL)&drop, AS(PRIM, 0))))))))));

	while(step(ctx));

	//printf("<%ld> %ld\n", length(S(ctx)), A(S(ctx)));
	dump_context(ctx);
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

void main(int argc, char *argv[]) {
	CELL sz = 30000;
	BYTE bk[sz];
	CTX* ctx = bootstrap(init(bk, sz));

	//dump_context(ctx);

	//fib(ctx);
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
			TOS(ctx) = cons(ctx, strlen(buf), AS(ATOM, cons(ctx, (CELL)buf, AS(ATOM, TOS(ctx)))));
			evaluate(ctx);
			if (ctx->err != 0 && ctx->err != -10) {
					printf("ERROR: %ld\n", ctx->err);
					dump_context(ctx);
					return;
			}
			ctx->err = 0;
			//dump_context(ctx);
		}
	}

	if (argc == 1 || argc == 3) {
		do {
			fgets(buf, 255, stdin);
			TOS(ctx) = cons(ctx, strlen(buf), AS(ATOM, cons(ctx, (CELL)buf, AS(ATOM, TOS(ctx)))));
			evaluate(ctx);
			if (ctx->err != 0 && ctx->err != -10) {
					printf("ERROR: %ld\n", ctx->err);
					dump_context(ctx);
					return;
			}
			ctx->err = 0;
			//dump_context(ctx);
		} while(1);
	}

	dump_context(ctx);
}
