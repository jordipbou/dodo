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

//// Aprox. time 1.570s (Using C stack as return stack)
//// Using return stack, aprox. time 1.725s
//void fib(X* x) {
//	//printf("swap %ld\n", (C)&swap);
//	//printf("dup %ld\n", (C)&duplicate);
//	//printf("> %ld\n", (C)&gt);
//	//printf("branch %ld\n", (C)&branch);
//	//printf("- %ld\n", (C)&sub);
//	//printf("rot %ld\n", (C)&rot);
//	//printf("x %ld\n", (C)&exec_x);
//	//printf("+ %ld\n", (C)&add);
//	//printf("drop %ld\n", (C)&drop);
//	C code =
//		cons(x,
//			cons(x, (C)&swap, AS(PRM,
//			cons(x, (C)&duplicate, AS(PRM,
//			cons(x, 1, AS(ATM,
//			cons(x, (C)&gt, AS(PRM,
//			cons(x, (C)&branch, AS(PRM,
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
//			cons(x, 0, AS(LST, 0)))))))))))))), 
//		AS(LST, 
//		cons(x, (C)&exec_x, AS(PRM,
//		cons(x, (C)&swap, AS(PRM,
//		cons(x, (C)&drop, AS(PRM, 0))))))));
//
//	inner(x, code);
//}
//
//// Aprox. time 0.670s
//// Using return stack, aprox. time 0.700s
//void c_fib(X* x) {
//	duplicate(x);
//	S(x) = cons(x, 1, AS(ATM, S(x)));
//	gt(x);
//	C b = A(S(x)); S(x) = recl(x, S(x));
//	if (b) {
//		S(x) = cons(x, 1, AS(ATM, S(x)));
//		sub(x);
//		duplicate(x);
//		S(x) = cons(x, 1, AS(ATM, S(x)));
//		sub(x);
//		c_fib(x);
//		swap(x);
//		c_fib(x);
//		add(x);
//	}
//}
//
//// Aprox. time 0.180s
//// Using return stack, aprox. time 0.190s
//void c_c_fib(X* x) {
//	if (A(S(x)) > 1) {
//		A(S(x)) = A(S(x)) - 1;
//		S(x) = cons(x, A(S(x)) - 1, AS(ATM, S(x)));
//		c_c_fib(x);
//		swap(x);
//		c_c_fib(x);
//		add(x);
//	}
//}
//
//C nfib(C n) {
//	if (n > 1) return nfib(n - 1) + nfib(n - 2);
//	else return n;
//}
//
//// Aprox. time 0.038s
//void nat_fib(X* x) {
//	UF1(x);
//	C n = A(S(x)); S(x) = recl(x, S(x));
//	C r = nfib(n);
//	S(x) = cons(x, r, AS(ATM, S(x)));
//}
//
////C repeat(X* x) {
////	C code =
////		cons(x, (C)&key, AS(PRM,
////		cons(x, (C)&emit, AS(PRM, 0))));
////
////	C r = inner(x, code);
////	if (r != 0) printf("ERROR %ld\n", r);
////}

void main(int argc, char *argv[]) {
	C sz = 8192;
	B bk[sz];
	//X* x = bootstrap(init(bk, sz));
	X* x = init(bk, sz);

	//S(x) = cons(x, 36, AS(ATM, 0));
	//fib(x);
	////c_fib(x);
	////c_c_fib(x);
	////nat_fib(x);
	//if (!x->err) printf("%ld\n", A(S(x)));

	//////repeat(x);
	//FILE *fptr;
	//B buf[255];
	//if (argc == 2) {
	//	fptr = fopen(argv[1], "r");
	//	while (fgets(buf, 255, fptr)) {
	//		evaluate(x, buf);
	//		switch (x->err) {
	//			case 0: break;
	//			case ERR_STACK_UNDERFLOW: printf("ERR::stack underflow\n"); x->err = 0; break;
	//			case ERR_EXPECTED_LIST: printf("ERR::expected list as top of stack\n"); x->err = 0; break;
	//			case ERR_UNDEFINED_WORD: printf("ERR::undefined word\n"); break;
	//			case ERR_EMPTY_TIB: x->err = 0; break;
	//			default: printf("ERR %ld\n", x->err); return;
	//		}
	//	}
	//}
	//do {
	//	fgets(buf, 255, stdin);
	//	evaluate(x, buf);
	//	switch (x->err) {
	//		case 0: break;
	//		case ERR_STACK_UNDERFLOW: printf("ERR::stack underflow\n"); x->err = 0; break;
	//		case ERR_EXPECTED_LIST: printf(" ERR::expected list as top of stack\n"); x->err = 0; break;
	//			case ERR_EMPTY_TIB: x->err = 0; break;
	//		default: printf("ERR %ld\n", x->err); return;
	//	}
	//	dump_stack(x);
	//} while(1);
}
