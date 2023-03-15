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
	fib(x);
	printf("%ld\n", A(S(x)));

	//repeat(x);
}
