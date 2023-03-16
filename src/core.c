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

// Aprox. time 1.570s
C fib(X* x) {
	C code =
		cons(x,
			cons(x, (C)&swap, AS(PRM,
			cons(x, (C)&duplicate, AS(PRM,
			cons(x, 1, AS(ATM,
			cons(x, (C)&gt, AS(PRM,
			cons(x, BRANCH, AS(PRM,
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
			cons(x, 0, AS(LST, 0)))))))))))))), 
		AS(LST, 
		cons(x, (C)&exec_x, AS(PRM,
		cons(x, (C)&swap, AS(PRM,
		cons(x, (C)&drop, AS(PRM, 0))))))));

	C r = inner(x, code);
	if (r != 0) printf("ERROR %ld\n", r);
}

// Aprox. time 0.670s
C c_fib(X* x) {
	duplicate(x);
	S(x) = cons(x, 1, AS(ATM, S(x)));
	gt(x);
	C b = A(S(x)); S(x) = recl(x, S(x));
	if (b) {
		S(x) = cons(x, 1, AS(ATM, S(x)));
		sub(x);
		duplicate(x);
		S(x) = cons(x, 1, AS(ATM, S(x)));
		sub(x);
		c_fib(x);
		swap(x);
		c_fib(x);
		add(x);
	}
}

// Aprox. time 0.180s
C c_c_fib(X* x) {
	if (A(S(x)) > 1) {
		A(S(x)) = A(S(x)) - 1;
		S(x) = cons(x, A(S(x)) - 1, AS(ATM, S(x)));
		c_c_fib(x);
		swap(x);
		c_c_fib(x);
		add(x);
	}
}

//C repeat(X* x) {
//	C code =
//		cons(x, (C)&key, AS(PRM,
//		cons(x, (C)&emit, AS(PRM, 0))));
//
//	C r = inner(x, code);
//	if (r != 0) printf("ERROR %ld\n", r);
//}

void main(int argc, char *argv[]) {
	C sz = 32168;
	B bk[sz];
	X* x = init(bk, sz);

	S(x) = cons(x, 36, AS(ATM, 0));
	c_c_fib(x);
	printf("%ld\n", A(S(x)));

	//repeat(x);
}
