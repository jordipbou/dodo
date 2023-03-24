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
void fib(X* x) {
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

	//inner(x);
	while(STEP(x));

	printf("<%ld> %ld\n", length(S(x)), A(S(x)));
}

// TEST: 0.22s
void fibR(X* x) {
	if (A(S(x)) > 1) {
		A(S(x)) -= 1;
		duplicate(x);
		A(S(x)) -= 1;
		fibR(x);
		swap(x);
		fibR(x);
		add(x);
	}
}

// TEST: 0.04s
void fibN(X* x) {
	S(x) = cons(x, 36, AS(ATM, 0));
	fibR(x);
	printf("<%ld> %ld\n", length(S(x)), A(S(x)));
}

int fibC(int n) {
	if (n > 1) return fibC(n - 1) + fibC(n - 2);
	else return n;
}

void main(int argc, char *argv[]) {
	C sz = 16384;
	B bk[sz];
	X* x = bootstrap(init(bk, sz));

	fib(x);
	//fibN(x);
	//printf("%d\n", fibC(36));

	//FILE *fptr;
	//B buf[255];
	//C result;
	//if (argc == 2) {
	//	fptr = fopen(argv[1], "r");
	//	while (fgets(buf, 255, fptr)) {
	//		S(x) = cons(x, strlen(buf), AS(ATM, cons(x, (C)buf, AS(ATM, S(x)))));
	//		evaluate(x);
	//		if (x->err != 0 && x->err != E_EIB) {
	//				printf("ERROR: %ld\n", result);
	//				return;
	//		}
	//		x->err = 0;
	//	}
	//} else {
	//	do {
	//		fgets(buf, 255, stdin);
	//		S(x) = cons(x, strlen(buf), AS(ATM, cons(x, (C)buf, AS(ATM, S(x)))));
	//		evaluate(x);
	//		if (x->err != 0 && x->err != E_EIB) {
	//				printf("ERROR: %ld\n", result);
	//				return;
	//		}
	//		x->err = 0;
	//	} while(1);
	//}
}
