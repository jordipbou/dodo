#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include"core.h"

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}

//// CURRENT: 16s
//// TEST: 6.33s (Before using execution pile of stacks)
//void fib(X* x) {
//	R(x) =
//		cons(x, 36, AS(ATM,
//		cons(x,
//			cons(x, (C)&swap, AS(PRM,
//			cons(x, (C)&duplicate, AS(PRM,
//			cons(x, 1, AS(ATM,
//			cons(x, (C)&gt, AS(PRM,
//			cons(x,
//				cons(x, 1, AS(ATM,
//				cons(x, (C)&sub, AS(PRM,
//				cons(x, (C)&duplicate, AS(PRM,
//				cons(x, 1, AS(ATM,
//				cons(x, (C)&sub, AS(PRM,
//				cons(x, (C)&rot, AS(PRM,
//				cons(x, (C)&exec_x, AS(PRM,
//				cons(x, (C)&rot, AS(PRM,
//				cons(x, (C)&rot, AS(PRM,
//				cons(x, (C)&exec_x, AS(PRM,
//				cons(x, (C)&rot, AS(PRM,
//				cons(x, (C)&add, AS(PRM, 0)))))))))))))))))))))))),
//			AS(LST,
//			cons(x, 0, AS(LST,
//			cons(x, (C)&branch, AS(PRM, 0)))))))))))))),
//		AS (LST,
//		cons(x, (C)&exec_x, AS(PRM,
//		cons(x, (C)&swap, AS(PRM,
//		cons(x, (C)&drop, AS(PRM, 0))))))))));
//
//	//inner(x);
//	while(STEP(x));
//
//	printf("<%ld> %ld\n", length(S(x)), A(S(x)));
//}
//
//// TEST: 0.22s
//void fibR(X* x) {
//	if (A(S(x)) > 1) {
//		A(S(x)) -= 1;
//		duplicate(x);
//		A(S(x)) -= 1;
//		fibR(x);
//		swap(x);
//		fibR(x);
//		add(x);
//	}
//}
//
//// TEST: 0.04s
//void fibN(X* x) {
//	S(x) = cons(x, 36, AS(ATM, 0));
//	fibR(x);
//	printf("<%ld> %ld\n", length(S(x)), A(S(x)));
//}
//
//int fibC(int n) {
//	if (n > 1) return fibC(n - 1) + fibC(n - 2);
//	else return n;
//}

void main(int argc, char *argv[]) {
	CELL sz = 16384;
	BYTE bk[sz];
	CTX* ctx = bootstrap(init(bk, sz));

	//dump_context(x);

	//fib(x);
	//fibN(x);
	//S(x) = cons(x, 36, AS(ATM, S(x)));
	//fibR(x);
	//printf("%d\n", fibC(36));
	//printf("%ld\n", A(S(x)));

	FILE *fptr;
	BYTE buf[255];
	if (argc == 2) {
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
	} else {
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
