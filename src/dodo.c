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

void main(int argc, char *argv[]) {
	C sz = 16384;
	B bk[sz];
	X* x = init(bk, sz);

	R(x) =
		cons(x, 36, AS(ATM,
		cons(x,
			cons(x, (C)&swap, AS(PRM,
			cons(x, (C)&duplicate, AS(PRM,
			cons(x, 1, AS(ATM,
			cons(x, (C)&gt, AS(PRM,
			cons(x,
				cons(x, 1, AS(ATM,
				cons(x, (C)&sub, AS(PRM,
				cons(x, (C)&duplicate, AS(PRM,
				cons(x, 1, AS(ATM,
				cons(x, (C)&sub, AS(PRM,
				cons(x, (C)&rot, AS(PRM,
				cons(x, (C)&exec_x, AS(PRM,
				cons(x, (C)&rot, AS(PRM,
				cons(x, (C)&rot, AS(PRM,
				cons(x, (C)&exec_x, AS(PRM,
				cons(x, (C)&rot, AS(PRM,
				cons(x, (C)&add, AS(PRM, 0)))))))))))))))))))))))),
			AS(LST,
			cons(x, 0, AS(LST,
			cons(x, (C)&branch, AS(PRM, 0)))))))))))))),
		AS (LST,
		cons(x, (C)&exec_x, AS(PRM,
		cons(x, (C)&swap, AS(PRM,
		cons(x, (C)&drop, AS(PRM, 0))))))))));

	inner(x);

	printf("<%ld> %ld\n", length(S(x)), A(S(x)));
	//CELL size = 1638400;
	//BYTE block[size];
	//CTX* ctx = bootstrap(init(block, size));

	//ADD_PRIMITIVE(ctx, ".s", &dump_stack, 0);
	//ADD_PRIMITIVE(ctx, "words", &words, 0);
	//ADD_PRIMITIVE(ctx, "see", &see, 0);
	//ADD_PRIMITIVE(ctx, "nfa", &nfa, 0);
	//ADD_PRIMITIVE(ctx, "xt", &xt, 0);
	//ADD_PRIMITIVE(ctx, "pair", &pair, 0);
	//ADD_PRIMITIVE(ctx, "next", &next, 0);
	//ADD_PRIMITIVE(ctx, "sp@", &sp_fetch, 0);
	//ADD_PRIMITIVE(ctx, "carcdr", &carcdr, 0);

	//ADD_PRIMITIVE(ctx, "x", &dbg_exec_x, 0);

	//FILE *fptr;
	//BYTE buf[255];
	//CELL result;
	//if (argc == 2) {
	//	fptr = fopen(argv[1], "r");
	//	while (fgets(buf, 255, fptr)) {
	//		//result = dbg_evaluate(ctx, strlwr(buf));
	//		//result = evaluate(ctx, strlwr(buf));
	//		result = evaluate(ctx, buf);
	//		if (result != 0) {
	//				printf("ERROR: %ld\n", result);
	//				return;
	//		}
	//	}
	//} else {
	//	do {
	//		fgets(buf, 255, stdin);
	//		//result = dbg_evaluate(ctx, buf);
	//		result = evaluate(ctx, buf);
	//		if (result != 0) {
	//			printf("ERROR: %ld\n", result);
	//			return;
	//		}
	//	} while(1);
	//}
}
