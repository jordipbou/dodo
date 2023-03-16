#include<stdlib.h>
#include "core.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

// TAGGED POINTER BASED TYPING INFORMATION

void test_basic_types() {
	C pair = (C)malloc(sizeof(C) * 2);

	D(pair) = AS(ATM, 8);
	TEST_ASSERT_EQUAL_INT(8, N(pair));
	TEST_ASSERT_EQUAL_INT(ATM, T(pair));
	TEST_ASSERT_EQUAL(ATM, T(pair));
	TEST_ASSERT_NOT_EQUAL(LST, T(pair));
	D(pair) = AS(LST, 8);
	TEST_ASSERT_EQUAL_INT(8, N(pair));
	TEST_ASSERT_EQUAL_INT(LST, T(pair));
	TEST_ASSERT_NOT_EQUAL(ATM, T(pair));
	TEST_ASSERT_EQUAL(LST, T(pair));
}

// CONTEXT

#define f_nodes(x)			(((x->size - sizeof(X)) / (2*sizeof(C))) - 2)
#define FREE(x)					(length(F(x)) - 1)

void test_X_block_initialization() {
	C size = 512;
	B block[size];
	X* x = init(block, size);
		
	TEST_ASSERT_NOT_EQUAL(0, x);

	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));

	TEST_ASSERT_EQUAL_INT(((B*)x) + sizeof(X), BOTTOM(x));
	TEST_ASSERT_EQUAL_INT(BOTTOM(x), x->here);
	TEST_ASSERT_EQUAL_INT(ALIGN(x->here, 2*sizeof(C)), x->there);
	TEST_ASSERT_EQUAL_INT(ALIGN(((B*)x) + size - 2*sizeof(C) - 1, 2*sizeof(C)), TOP(x));
	TEST_ASSERT_EQUAL_INT(TOP(x) - 2*sizeof(C), x->free);

	TEST_ASSERT_EQUAL_INT(0, x->latest);
	TEST_ASSERT_EQUAL_INT(0, S(x));
	TEST_ASSERT_EQUAL_INT(0, R(x));
	TEST_ASSERT_EQUAL_INT(0, x->ip);
	TEST_ASSERT_EQUAL_INT(0, x->err);
}

// LST CREATION AND DESTRUCTION (AUTOMATIC MEMORY MANAGEMENT)

void test_LST_cons() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C l = cons(x, 7, cons(x, 11, cons(x, 13, 0)));

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
	TEST_ASSERT_EQUAL_INT(3, length(l));
	TEST_ASSERT_EQUAL_INT(7, A(l));
	TEST_ASSERT_EQUAL_INT(11, A(D(l)));
	TEST_ASSERT_EQUAL_INT(13, A(D(D(l))));

	C p = cons(x, 7, 0);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 4, FREE(x));
	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(0, D(p));

	while (F(x) != x->there) { cons(x, 1, 0); }

	C p3 = cons(x, 13, 0);
	TEST_ASSERT_EQUAL_INT(0, p3);
	TEST_ASSERT_EQUAL_INT(ERR_STACK_OVERFLOW, x->err);
}

void test_LST_clon() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C l = 
		cons(x, 7, AS(ATM, 
		cons(x, 11, AS(PRM, 
		cons(x, 
			cons(x, 13, AS(ATM,
			cons(x, 17, AS(ATM, 0)))), AS(LST,
		cons(x, 19, AS(ATM, 0))))))));

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 6, FREE(x));
	TEST_ASSERT_EQUAL_INT(4, length(l));
	TEST_ASSERT_EQUAL(ATM, T(l));
	TEST_ASSERT_EQUAL_INT(7, A(l));
	TEST_ASSERT_EQUAL(PRM, T(N(l)));
	TEST_ASSERT_EQUAL_INT(11, A(N(l)));
	TEST_ASSERT_EQUAL(LST, T(N(N(l))));
	TEST_ASSERT_EQUAL_INT(2, length(N(N(l))));
	TEST_ASSERT_EQUAL(ATM, T(A(N(N(l)))));
	TEST_ASSERT_EQUAL_INT(13, A(A(N(N(l)))));
	TEST_ASSERT_EQUAL(ATM, T(N(A(N(N(l))))));
	TEST_ASSERT_EQUAL_INT(17, A(N(A(N(N(l))))));
	TEST_ASSERT_EQUAL(ATM, T(N(N(N(l)))));
	TEST_ASSERT_EQUAL_INT(19, A(N(N(N(l)))));

	C c = clone(x, l);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 12, FREE(x));
	TEST_ASSERT_EQUAL_INT(4, length(c));
	TEST_ASSERT_EQUAL(ATM, T(c));
	TEST_ASSERT_EQUAL_INT(7, A(c));
	TEST_ASSERT_EQUAL(PRM, T(N(c)));
	TEST_ASSERT_EQUAL_INT(11, A(N(c)));
	TEST_ASSERT_EQUAL(LST, T(N(N(c))));
	TEST_ASSERT_EQUAL_INT(2, length(N(N(c))));
	TEST_ASSERT_EQUAL(ATM, T(A(N(N(c)))));
	TEST_ASSERT_EQUAL_INT(13, A(A(N(N(c)))));
	TEST_ASSERT_EQUAL(ATM, T(N(A(N(N(c))))));
	TEST_ASSERT_EQUAL_INT(17, A(N(A(N(N(c))))));
	TEST_ASSERT_EQUAL(ATM, T(N(N(N(c)))));
	TEST_ASSERT_EQUAL_INT(19, A(N(N(N(c)))));

	TEST_ASSERT_NOT_EQUAL_INT(l, c);
	TEST_ASSERT_NOT_EQUAL_INT(N(l), N(c));
	TEST_ASSERT_NOT_EQUAL_INT(N(N(l)), N(N(c)));
	TEST_ASSERT_NOT_EQUAL_INT(A(N(N(l))), A(N(N(c))));
	TEST_ASSERT_NOT_EQUAL_INT(N(A(N(N(l)))), N(A(N(N(c)))));
	TEST_ASSERT_NOT_EQUAL_INT(N(N(N(l))), N(N(N(c))));
}

void test_LST_recl() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C list = cons(x, 7, cons(x, 11, cons(x, 13, 0)));
	C tail = recl(x, list);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
	TEST_ASSERT_EQUAL_INT(2, length(tail));
	TEST_ASSERT_EQUAL_INT(11, A(tail));
	TEST_ASSERT_EQUAL_INT(13, A(D(tail)));
}

void test_LST_recl_list() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C list = 
		cons(x, 5, AS(ATM,
		cons(x, 
			cons(x, 7,  AS(ATM,
			cons(x, 11, AS(ATM,
			cons(x, 13, AS(ATM, 0)))))), AS(LST, 
		cons(x, 17, AS(ATM, 0))))));

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 6, FREE(x));
	TEST_ASSERT_EQUAL(ATM, T(list));
	TEST_ASSERT_EQUAL_INT(5, A(list));
	TEST_ASSERT_EQUAL(LST, T(N(list)));
	TEST_ASSERT_EQUAL_INT(3, length(A(N(list))));
	TEST_ASSERT_EQUAL(ATM, T(A(N(list))));
	TEST_ASSERT_EQUAL_INT(7, A(A(N(list))));
	TEST_ASSERT_EQUAL(ATM, T(N(A(N(list)))));
	TEST_ASSERT_EQUAL_INT(11, A(N(A(N(list)))));
	TEST_ASSERT_EQUAL(ATM, T(N(N(A(N(list))))));
	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(N(list))))));
	TEST_ASSERT_EQUAL(ATM, T(N(N(list))));
	TEST_ASSERT_EQUAL_INT(17, A(N(N(list))));

	C tail = recl(x, list);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, FREE(x));
	TEST_ASSERT_EQUAL(LST, T(tail));
	TEST_ASSERT_EQUAL_INT(3, length(A(tail)));
	TEST_ASSERT_EQUAL(ATM, T(A(tail)));
	TEST_ASSERT_EQUAL_INT(7, A(A(tail)));
	TEST_ASSERT_EQUAL(ATM, T(N(A(tail))));
	TEST_ASSERT_EQUAL_INT(11, A(N(A(tail))));
	TEST_ASSERT_EQUAL(ATM, T(N(N(A(tail)))));
	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(tail)))));
	TEST_ASSERT_EQUAL(ATM, T(N(tail)));
	TEST_ASSERT_EQUAL_INT(17, A(N(tail)));

	tail = recl(x, tail);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
	TEST_ASSERT_EQUAL(ATM, T(tail));
	TEST_ASSERT_EQUAL_INT(17, A(tail));

	tail = recl(x, tail);

	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));
	TEST_ASSERT_EQUAL_INT(0, tail);
}

//void test_LST_rvs() {
//	C p1 = (C)malloc(2*sizeof(C));
//	C p2 = (C)malloc(2*sizeof(C));
//	C p3 = (C)malloc(2*sizeof(C));
//	
//	D(p1) = p2;
//	D(p2) = p3;
//	D(p3) = 0;
//
//	C r = rvs(p1, 0);
//
//	TEST_ASSERT_EQUAL_INT(p3, r);
//	TEST_ASSERT_EQUAL_INT(p2, N(r));
//	TEST_ASSERT_EQUAL_INT(p1, N(N(r)));
//}

void test_LST_length() {
	C p1 = (C)malloc(2*sizeof(C));
	C p2 = (C)malloc(2*sizeof(C));
	C p3 = (C)malloc(2*sizeof(C));

	D(p1) = AS(ATM, p2);
	D(p2) = p3;
	D(p3) = 0;

	TEST_ASSERT_EQUAL_INT(0, length(0));
	TEST_ASSERT_EQUAL_INT(1, length(p3));
	TEST_ASSERT_EQUAL_INT(2, length(p2));
	TEST_ASSERT_EQUAL_INT(3, length(p1));
}

// INNER INTERPRETER

void test_INNER_execute_atom() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C xlist = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
	inner(x, xlist);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(13, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(0, x->err);
}

void test_INNER_execute_list() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C xlist = 
		cons(x, 
			cons(x, 7, AS(ATM, 
			cons(x, 11, AS(ATM, 
			cons(x, 13, AS(ATM, 0)))))), 
		AS(LST, 0));

	inner(x, xlist);

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(S(x))))));
	TEST_ASSERT_NOT_EQUAL_INT(xlist, S(x));
	TEST_ASSERT_NOT_EQUAL_INT(A(xlist), A(S(x)));
	TEST_ASSERT_NOT_EQUAL_INT(N(A(xlist)), N(A(S(x))));
	TEST_ASSERT_NOT_EQUAL_INT(N(N(A(xlist))), N(N(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(0, x->err);
}

void test_add_t(X* x) {	A(N(S(x))) = A(N(S(x))) + A(S(x)); S(x) = recl(x, S(x)); }
void test_dup_t(X* x) { S(x) = cons(x, A(S(x)), AS(ATM, S(x))); }

void test_INNER_execute_primitive() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C xlist = 
		cons(x, 13, AS(ATM, 
		cons(x, 7, AS(ATM, 
		cons(x, (C)&test_add_t, AS(PRM, 
		cons(x, (C)&test_dup_t, AS(PRM, 0))))))));

	inner(x, xlist);

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(20, A(S(x)));
	TEST_ASSERT_EQUAL_INT(20, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(0, x->err);
}

//void test_INNER_execute_call() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	C word = 
//		cons(x, 
//			cons(x, (C)"", AS(ATM,
//			cons(x, (C)&test_dup_t, AS(PRM, 
//			cons(x, (C)&test_add_t, AS(PRM, 0)))))),
//		AS(WORD, 0));
//	C call = cons(x, word, AS(WORD, 0));
//
//	S(x) = cons(x, 5, AS(ATM, S(x)));
//
//	execute(x, call);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(10, A(S(x)));
//
//	call = cons(x, word, AS(WORD, cons(x, 13, AS(ATM, 0))));
//
//	execute(x, call);
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(20, A(N(S(x))));
//}
//
//// PILE PRMITIVES
//
//void test_PILE_spush() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	TEST_ASSERT_EQUAL_INT(1, length(x->p));
//	TEST_ASSERT_EQUAL_INT(A(x->p), S(x));
//	TEST_ASSERT_EQUAL_INT(A(x->p), O(x));
//
//	spush(x);
//
//	TEST_ASSERT_EQUAL_INT(2, length(x->p));
//	TEST_ASSERT_EQUAL_INT(A(x->p), S(x));
//	TEST_ASSERT_EQUAL_INT(A(x->p), O(x));
//}
//
////// IP PRMITIVEs
////
////void test_IP_branch() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C xt = 
////		cons(x, 1, AS(ATM,
////		cons(x, (C)&branch, AS(PRM,
////		cons(x, cons(x, 7, AS(ATM, 0)), AS(LST, 
////		cons(x, cons(x, 11, AS(ATM, 0)), AS(LST,
////		cons(x, 13, AS(ATM, 0))))))))));
////
////	execute(x, xt);
////
////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
////
////	S(x) = 0;
////
////	xt = 
////		cons(x, 0, AS(ATM,
////		cons(x, (C)&branch, AS(PRM,
////		cons(x, cons(x, 7, AS(ATM, 0)), AS(LST, 
////		cons(x, cons(x, 11, AS(ATM, 0)), AS(LST, 0))))))));
////
////	execute(x, xt);
////
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
////
////}
////
////void test_IP_jump() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C dest = cons(x, 7, AS(ATM, 0));
////	C src = cons(x, (C)&jump, AS(PRM, cons(x, dest, AS(ATM, 0))));
////
////	execute(x, src);
////
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
////}
////
////void test_IP_zjump() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C dest = cons(x, 7, AS(ATM, 0));
////	C src = 
////		cons(x, (C)&zjump, AS(PRM, 
////		cons(x, dest, AS(ATM, 
////		cons(x, 13, AS(ATM, 0))))));
////
////	S(x) = cons(x, 1, AS(ATM, 0));
////
////	execute(x, src);
////
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
////
////	S(x) = cons(x, 0, AS(ATM, 0));
////
////	execute(x, src);
////
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
////}
////
////// STACK PRMITIVES
////
////void test_STACK_duplicate_atom() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, S(x)));
////	duplicate(x);
////
////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
////	TEST_ASSERT_EQUAL_INT(ATM, T(S(x)));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
////
////	S(x) = cons(x, 11, AS(ATM, S(x)));
////	duplicate(x);
////
////	TEST_ASSERT_EQUAL_INT(4, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 4, FREE(x));
////	TEST_ASSERT_EQUAL_INT(ATM, T(S(x)));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(N(S(x)))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(N(N(S(x))))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(N(S(x)))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(N(N(S(x))))));
////}
////
////void test_STACK_duplicate_list() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = 
////		cons(x, 
////			cons(x, 17, AS(ATM,
////			cons(x, 13, AS(ATM,
////			cons(x, 
////				cons(x, 11, AS(ATM,
////				cons(x, 7, AS(ATM, 0)))),
////				AS(LST, 0)))))),
////			AS(LST, S(x)));
////
////	// (17, 13, (11, 7))
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 6, FREE(x));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
////	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(17, A(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(S(x)))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(A(S(x)))));
////	TEST_ASSERT_EQUAL_INT(LST, T(N(N(A(S(x))))));
////	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(S(x)))))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(N(A(S(x)))))));
////	TEST_ASSERT_EQUAL_INT(11, A(A(N(N(A(S(x)))))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(N(A(S(x))))))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(N(A(S(x))))))));
////
////	duplicate(x);
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 12, FREE(x));
////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
////
////	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
////	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(17, A(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(S(x)))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(A(S(x)))));
////	TEST_ASSERT_EQUAL_INT(LST, T(N(N(A(S(x))))));
////	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(S(x)))))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(N(A(S(x)))))));
////	TEST_ASSERT_EQUAL_INT(11, A(A(N(N(A(S(x)))))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(N(A(S(x))))))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(N(A(S(x))))))));
////
////	TEST_ASSERT_EQUAL_INT(LST, T(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(3, length(A(N(S(x)))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(S(x)))));
////	TEST_ASSERT_EQUAL_INT(17, A(A(N(S(x)))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(S(x))))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(A(N(S(x))))));
////	TEST_ASSERT_EQUAL_INT(LST, T(N(N(A(N(S(x)))))));
////	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(N(S(x))))))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(N(A(N(S(x))))))));
////	TEST_ASSERT_EQUAL_INT(11, A(A(N(N(A(N(S(x))))))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(N(A(N(S(x)))))))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(N(A(N(S(x)))))))));
////
////	TEST_ASSERT_NOT_EQUAL_INT(S(x), N(S(x)));
////	TEST_ASSERT_NOT_EQUAL_INT(A(S(x)), A(N(S(x))));
////	TEST_ASSERT_NOT_EQUAL_INT(N(A(S(x))), N(A(N(S(x)))));
////	TEST_ASSERT_NOT_EQUAL_INT(N(N(A(S(x)))), N(N(A(N(S(x))))));
////	TEST_ASSERT_NOT_EQUAL_INT(A(N(N(A(S(x))))), A(N(N(A(N(S(x)))))));
////	TEST_ASSERT_NOT_EQUAL_INT(N(A(N(N(A(S(x)))))), N(A(N(N(A(N(S(x))))))));
////}
////
////void test_STACK_swap_1() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
////	swap(x);
////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
////}
////
////void test_STACK_swap_2() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 11, AS(ATM, cons(x, cons(x, 7, AS(ATM, cons(x, 5, AS(ATM, 0)))), AS(LST, 0))));
////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(ATM, T(S(x)));
////	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(LST, T(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(2, length(A(N(S(x)))));
////	TEST_ASSERT_EQUAL_INT(7, A(A(N(S(x)))));
////	TEST_ASSERT_EQUAL_INT(5, A(N(A(N(S(x))))));
////	swap(x);
////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
////	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(5, A(N(A(S(x)))));
////	TEST_ASSERT_EQUAL_INT(ATM, T(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
////}
////
////void test_STACK_drop() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, S(x)));
////
////	drop(x);
////
////	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));
////
////	S(x) = 
////		cons(x,
////			cons(x, 7, AS(ATM,
////			cons(x,
////				cons(x, 11, AS(ATM,
////				cons(x, 13, AS(ATM, 0)))),
////			AS(LST, 0)))),
////		AS(LST, S(x)));
////
////	drop(x);
////
////	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));
////}
////
////void test_STACK_over() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
////
////	over(x);
////
////	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(11, A(N(N(S(x)))));
////
////	S(x) = 
////		cons(x, 7, AS(ATM, 
////		cons(x, 
////			cons(x, 11, AS(ATM,
////			cons(x, 13, AS(ATM, 0)))),
////		AS(LST, 0))));
////
////	over(x);
////
////	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
////	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(11, A(A(S(x))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(A(S(x)))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
////	TEST_ASSERT_NOT_EQUAL_INT(N(N(S(x))), S(x));
////	TEST_ASSERT_NOT_EQUAL_INT(A(N(N(S(x)))), A(S(x)));
////	TEST_ASSERT_NOT_EQUAL_INT(N(A(N(N(S(x))))), N(A(S(x))));
////}
////
////void test_STACK_rot() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, cons(x, 13, AS(ATM, 0))))));
////
////	rot(x);
////
////	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(11, A(N(N(S(x)))));
////}
////
////// ARITHMETIC PRMITIVES
////
////void test_ARITHMETIC_add() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	add(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(18, A(S(x)));
////}
////
////void test_ARITHMETIC_sub() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
////	sub(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(4, A(S(x)));
////}
////
////void test_ARITHMETIC_mul() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	mul(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(77, A(S(x)));
////}
////
////void test_ARITHMETIC_division() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 11, AS(ATM, cons(x, 77, AS(ATM, 0))));
////	division(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
////}
////
////void test_ARITHMETIC_mod() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
////	mod(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(4, A(S(x)));
////}
////
////// COMPARISON PRMITIVES
////
////void test_COMPARISON_gt() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
////	gt(x);
////	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////
////	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	gt(x);
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	gt(x);
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////}
////
////void test_COMPARISON_lt() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
////	lt(x);
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////
////	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	lt(x);
////	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	lt(x);
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////}
////
////void test_COMPARISON_eq() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
////	eq(x);
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////
////	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	eq(x);
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	eq(x);
////	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////}
////
////// BIT PRMITIVES
////
////void test_BIT_and() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
////	and(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(3, A(S(x)));
////
////	S(x) = cons(x, 0, AS(ATM, cons(x, 0, AS(ATM, 0))));
////	and(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////
////	S(x) = cons(x, -1, AS(ATM, cons(x, 0, AS(ATM, 0))));
////	and(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////
////	S(x) = cons(x, 0, AS(ATM, cons(x, -1, AS(ATM, 0))));
////	and(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////
////	S(x) = cons(x, -1, AS(ATM, cons(x, -1, AS(ATM, 0))));	
////	and(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
////}
////
////void test_BIT_or() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
////	or(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(15, A(S(x)));
////
////	S(x) = cons(x, 0, AS(ATM, cons(x, 0, AS(ATM, 0))));
////	or(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////
////	S(x) = cons(x, -1, AS(ATM, cons(x, 0, AS(ATM, 0))));
////	or(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
////
////	S(x) = cons(x, 0, AS(ATM, cons(x, -1, AS(ATM, 0))));
////	or(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
////
////	S(x) = cons(x, -1, AS(ATM, cons(x, -1, AS(ATM, 0))));
////	or(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
////}
////
////void test_BIT_invert() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	S(x) = cons(x, 7, AS(ATM, 0));
////	invert(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(-8, A(S(x)));
////
////	S(x) = cons(x, 0, AS(ATM, 0));
////	invert(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
////
////	S(x) = cons(x, 1, AS(ATM, 0));
////	invert(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(-2, A(S(x)));
////
////	S(x) = cons(x, -1, AS(ATM, 0));
////	invert(x);
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
////}
////
////// PARSING
////
////void test_PARSING_parse_token() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////	B* str = "   test  ";
////
////	x->ib = str;
////	x->tk = 0;
////	x->in = 0;
////
////	parse_token(x);
////
////	TEST_ASSERT_EQUAL_INT(4, x->in - x->tk);
////	TEST_ASSERT_EQUAL_INT((C)str + 3, x->ib + x->tk);
////
////	parse_token(x);
////
////	TEST_ASSERT_EQUAL_INT(0, x->in - x->tk);
////	TEST_ASSERT_EQUAL_INT((C)str + 9, x->ib + x->tk);
////
////	B* str2 = "";
////	x->ib = str2;
////	x->tk = 0;
////	x->in = 0;
////
////	parse_token(x);
////
////	TEST_ASSERT_EQUAL_INT(0, x->in - x->tk);
////	TEST_ASSERT_EQUAL_INT((C)str2, x->ib + x->tk);
////
////	B* str3 = ": name    word1 ;";
////	x->ib = str3;
////	x->tk = 0;
////	x->in = 0;
////
////	parse_token(x);
////
////	TEST_ASSERT_EQUAL_INT(1, x->in - x->tk);
////	TEST_ASSERT(!strncmp(":", x->ib + x->tk, x->in - x->tk));
////	TEST_ASSERT_EQUAL_INT((C)str3, x->ib + x->tk);
////
////	parse_token(x);
////
////	TEST_ASSERT_EQUAL_INT(4, x->in - x->tk);
////	TEST_ASSERT(!strncmp("name", x->ib + x->tk, x->in - x->tk));
////	TEST_ASSERT_EQUAL_INT((C)str3 + 2, x->ib + x->tk);
////
////	parse_token(x);
////
////	TEST_ASSERT_EQUAL_INT(5, x->in - x->tk);
////	TEST_ASSERT(!strncmp("word1", x->ib + x->tk, x->in - x->tk));
////	TEST_ASSERT_EQUAL_INT((C)str3 + 10, x->ib + x->tk);
////
////	parse_token(x);
////
////	TEST_ASSERT_EQUAL_INT(1, x->in - x->tk);
////	TEST_ASSERT(!strncmp(";", x->ib + x->tk, x->in - x->tk));
////	TEST_ASSERT_EQUAL_INT((C)str3 + 16, x->ib + x->tk);
////
////	parse_token(x);
////
////	TEST_ASSERT_EQUAL_INT(0, x->in - x->tk);
////	TEST_ASSERT_EQUAL_INT((C)str3 + 17, x->ib + x->tk);
////
////	parse_token(x);
////
////	TEST_ASSERT_EQUAL_INT(0, x->in - x->tk);
////	TEST_ASSERT_EQUAL_INT((C)str3 + 17, x->ib + x->tk);
////}
////
////void test_PARSING_fnd_tk() {
////	C size = 4096;
////	B block[size];
////	X* x = init(block, size);
////
////	x->latest =	
////		cons(x, cons(x, (C)"test2", AS(ATM, 0)), AS(LST,
////		cons(x, cons(x, (C)"dup", AS(ATM, 0)), AS(CALL,
////		cons(x, cons(x, (C)"join", AS(ATM, 0)), AS(LST,
////		cons(x, cons(x, (C)"test", AS(ATM, 0)), AS(LST, 0))))))));
////
////	x->ib = "dup";
////	x->tk = 0;
////	x->in = 3;
////
////	C word = fnd_tk(x);
////
////	TEST_ASSERT_NOT_EQUAL_INT(0, word);
////	TEST_ASSERT_EQUAL_INT(1, IMMEDIATE(word));
////	TEST_ASSERT_EQUAL_INT(N(x->latest), word);
////
////	x->ib = "   join  ";
////	x->tk = 0;
////	x->in = 0;
////
////	parse_token(x);
////	word = fnd_tk(x);
////
////	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(word));
////	TEST_ASSERT_EQUAL_INT(N(N(x->latest)), word);
////
////	x->ib = "test";
////	x->tk = 0;
////	x->in = 0;
////
////	parse_token(x);
////	word = fnd_tk(x);
////
////	TEST_ASSERT_EQUAL_STRING("test", (B*)NFA(word));
////	TEST_ASSERT_EQUAL_INT(N(N(N(x->latest))), word);
////}
////
////// OUTER INTERPRETER
////
////void test_OUTER_evaluate_numbers() {
////	C size = 4096;
////	B block[size];
////	X* x = init(block, size);
////
////	evaluate(x, "   7 11    13  ");
////
////	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(N(S(x)))));
////}
////
////void test_OUTER_evaluate_words() {
////	C size = 4096;
////	B block[size];
////	X* x = init(block, size);
////
////	x->latest = 
////		cons(x,
////			cons(x, (C)"test", AS(ATM,
////			cons(x, 7, AS(ATM,
////			cons(x, 11, AS(ATM,
////			cons(x, 13, AS(ATM, 0)))))))),
////		AS(LST, 0));
////
////	evaluate(x, " test   ");
////
////	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(N(S(x)))));
////}
////
////// CONTIGUOUS MEMORY
////
////void test_MEM_allot() {
////	C size = 1024;
////	B block[size];
////	X* x = init(block, size);
////
////	B* here = x->here;
////	S(x) = cons(x, 0, AS(ATM, S(x)));
////	C result = allot(x);
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_INT(here, x->here);
////
////	S(x) = cons(x, 13, AS(ATM, S(x)));
////	result = allot(x);
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_INT(here + 13, x->here);
////
////	C fnodes = FREE(x);
////	here = x->here;
////	C reserved = RESERVED(x);
////
////	// Ensure reserved memory is 0 to allow next tests to pass
////	S(x) = cons(x, RESERVED(x), AS(ATM, S(x)));
////	result = allot(x);
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_PTR(here + reserved, x->here);
////	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));
////	TEST_ASSERT_EQUAL_INT(fnodes, FREE(x));
////
////	here = x->here;
////
////	S(x) = cons(x, 8*sizeof(C), AS(ATM, S(x)));
////	result = allot(x);
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_PTR(here + 8*sizeof(C), x->here);
////	TEST_ASSERT_EQUAL_INT(fnodes - 4, FREE(x));
////	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));
////
////	here = x->here;
////
////	S(x) = cons(x, 2*sizeof(C) - 3, AS(ATM, S(x)));
////	result = allot(x);
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_PTR(here + 2*sizeof(C) - 3, x->here);
////	TEST_ASSERT_EQUAL_INT(fnodes - 5, FREE(x));
////	TEST_ASSERT_EQUAL_INT(3, RESERVED(x));
////
////	S(x) = cons(x, -(2*sizeof(C) - 3), AS(ATM, S(x)));
////	result = allot(x);
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_PTR(here, x->here);
////	TEST_ASSERT_EQUAL_INT(fnodes - 4, FREE(x));
////	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));
////
////	S(x) = cons(x, -1, AS(ATM, S(x)));
////	result = allot(x);
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_PTR(here - 1, x->here);
////	TEST_ASSERT_EQUAL_INT(fnodes - 4, FREE(x));
////	TEST_ASSERT_EQUAL_INT(1, RESERVED(x));
////
////	here = x->here;
////	reserved = RESERVED(x);
////	fnodes = FREE(x);
////
////	S(x) = cons(x, 2048, AS(ATM, S(x)));
////	result = allot(x);
////	TEST_ASSERT_EQUAL_INT(ERR_NOT_ENOUGH_MEMORY, result);
////	TEST_ASSERT_EQUAL_PTR(here, x->here);
////	TEST_ASSERT_EQUAL_INT(reserved, RESERVED(x));
////	TEST_ASSERT_EQUAL_INT(fnodes, FREE(x));
////
////	S(x) = cons(x, -2048, AS(ATM, S(x)));
////	result = allot(x);
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_PTR(BOTTOM(x), x->here);
////	TEST_ASSERT_EQUAL_INT((C)ALIGN(x->here, 2*sizeof(C)), x->t);
////	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));
////}
////
////void test_MEM_compile_str() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C result;
////
////	x->ib = " test string\"  ";
////	x->in = 0;
////	result = compile_str(x);
////
////	TEST_ASSERT_EQUAL_INT(0, result);
////
////	B* addr = (B*)A(S(x));
////
////	TEST_ASSERT_EQUAL_STRING("test string", addr);
////	TEST_ASSERT_EQUAL_INT(11, *((C*)(addr - sizeof(C))));
////}
////
////////  LST FUNCTIONS
////
////void test_append() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C dest = 0;
////	C result = 0;
////
////	result = append(x);
////
////	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, result);
////
////	S(x) = cons(x, (C)&dest, AS(ATM, S(x)));
////	S(x) = cons(x, 7, AS(ATM, S(x)));
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
////
////	result = append(x);
////
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_INT(1, length(dest));
////	TEST_ASSERT_EQUAL_INT(7, A(dest));
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
////
////	S(x) = recl(x, S(x));
////	dest = recl(x, dest);
////
////	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));
////
////	S(x) = cons(x, cons(x, 7, AS(ATM, 0)), AS(LST, S(x)));
////	S(x) = cons(x, 11, AS(ATM, S(x)));
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
////
////	result = append(x);
////
////	TEST_ASSERT_EQUAL_INT(0, result);
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
////	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
////	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
////}
//////void test_depth() {
//////	C p1 = (C)malloc(sizeof(C) * 2);
//////	C p2 = (C)malloc(sizeof(C) * 2);
//////	C p3 = (C)malloc(sizeof(C) * 2);
//////	C p4 = (C)malloc(sizeof(C) * 2);
//////	C p5 = (C)malloc(sizeof(C) * 2);
//////
//////	D(p1) = AS(ATM, p2);
//////	D(p2) = AS(LST, p3);
//////	A(p2) = p4;
//////	D(p3) = AS(PRM, 0);
//////	D(p4) = p5;
//////	D(p5) = 0;
//////
//////	TEST_ASSERT_EQUAL_INT(3, length(p1));
//////	TEST_ASSERT_EQUAL_INT(2, length(A(p2)));
//////	TEST_ASSERT_EQUAL_INT(5, depth(p1));
//////	TEST_ASSERT_EQUAL_INT(4, depth(p2));
//////	TEST_ASSERT_EQUAL_INT(2, depth(p4));
//////	TEST_ASSERT_EQUAL_INT(1, depth(p3));
//////	TEST_ASSERT_EQUAL_INT(1, depth(p5));
//////	TEST_ASSERT_EQUAL_INT(0, depth(0));
//////}
//////
////// CONTIGUOUS MEMORY
////
//////void test_align() {
//////	C size = 512;
//////	B block[size];
//////	X* x = init(block, size);
//////
//////	// This ensures here will be aligned with a pair 
//////	C result = allot(x, RESERVED(x));
//////
//////	// And this ensures that it will not be aligned
//////	result = allot(x, 1);
//////	TEST_ASSERT_NOT_EQUAL_INT(ALIGN(x->here, sizeof(C)), x->here);
//////	TEST_ASSERT_EQUAL_INT(0, result);
//////
//////	result = align(x);
//////	TEST_ASSERT_EQUAL_INT(ALIGN(x->here, sizeof(C)), x->here);
//////	TEST_ASSERT_EQUAL_INT(0, result);
//////}
//////
//////// WORD DEFINITIONS
//////
//////void test_header() {
//////	C size = 512;
//////	B block[size];
//////	X* x = init(block, size);
//////
//////	B* here = x->here;
//////
//////	C h = header(x, "test");
//////
//////	TEST_ASSERT_EQUAL_STRING("test", NFA(h));
//////	TEST_ASSERT_EQUAL_PTR(x->here, DFA(h));
//////	TEST_ASSERT_EQUAL_INT(0, XT(h));
//////	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(h));
//////}
//////
//////void test_body() {
//////	C size = 512;
//////	B block[size];
//////	X* x = init(block, size);
//////
//////	C h = header(x, "test");
//////
//////	C w = body(h, cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0)))));
//////
//////	execute(x, XT(w));
//////
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
//////}
//////
//////void test_reveal() {
//////	C size = 512;
//////	B block[size];
//////	X* x = init(block, size);
//////
//////	C h = header(x, "test1");
//////	C h2 = header(x, "test2");
//////
//////	C d = reveal(x, h);
//////
//////	TEST_ASSERT_EQUAL_INT(h, d);
//////	TEST_ASSERT_EQUAL_INT(h, x->latest);
//////	TEST_ASSERT_EQUAL_INT(1, length(x->latest));
//////
//////	d = reveal(x, h2);
//////
//////	TEST_ASSERT_EQUAL_INT(h2, d);
//////	TEST_ASSERT_EQUAL_INT(h2, x->latest);
//////	TEST_ASSERT_EQUAL_INT(2, length(x->latest));
//////	TEST_ASSERT_EQUAL_INT(h, N(x->latest));
//////}
//////
//////void test_immediate() {
//////	C size = 512;
//////	B block[size];
//////	X* x = init(block, size);
//////
//////	C h = reveal(x, header(x, "test1"));
//////
//////	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(x->latest));
//////
//////	immediate(h);
//////
//////	TEST_ASSERT_EQUAL_INT(1, IMMEDIATE(x->latest));
//////}
////
//////void test_mlength() {
//////	C p1 = (C)malloc(sizeof(C) * 2);
//////	C p2 = (C)malloc(sizeof(C) * 2);
//////	C p3 = (C)malloc(sizeof(C) * 2);
//////
//////	D(p1) = p2;
//////	D(p2) = p3;
//////	D(p3) = 0;
//////
//////	TEST_ASSERT(mlength(p1, 3));
//////	TEST_ASSERT(mlength(p1, 2));
//////	TEST_ASSERT(mlength(p1, 1));
//////	TEST_ASSERT(mlength(p1, 0));
//////	TEST_ASSERT(!mlength(p2, 3));
//////	TEST_ASSERT(mlength(p2, 2));
//////	TEST_ASSERT(mlength(p2, 1));
//////	TEST_ASSERT(mlength(p2, 0));
//////	TEST_ASSERT(!mlength(p3, 3));
//////	TEST_ASSERT(!mlength(p3, 2));
//////	TEST_ASSERT(mlength(p3, 1));
//////	TEST_ASSERT(mlength(p3, 0));
//////}
//////
//////void test_last() {
//////	C p1 = (C)malloc(sizeof(C) * 2);
//////	C p2 = (C)malloc(sizeof(C) * 2);
//////	C p3 = (C)malloc(sizeof(C) * 2);
//////
//////	D(p1) = p2;
//////	D(p2) = p3;
//////	D(p3) = 0;
//////
//////	TEST_ASSERT_EQUAL_INT(0, last(0));
//////	TEST_ASSERT_EQUAL_INT(p3, last(p1));
//////	TEST_ASSERT_EQUAL_INT(p3, last(p2));
//////	TEST_ASSERT_EQUAL_INT(p3, last(p3));
//////}
//////
//////// BASIC STACK OPERATIONS
//////
//////void test_push() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 11);
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(0,X* x->err);
//////	TEST_ASSERT(IS(ATM,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//////	push(x, ATM, 7);
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(0,X* x->err);
//////	TEST_ASSERT(IS(ATM,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//////	TEST_ASSERT(IS(ATM, N(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
//////}
//////
//////void test_pop() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 11);
//////	push(x, ATM, 7);
//////
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	C v = pop(x);
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(7, v);
//////	v = pop(x);
//////	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(11, v);
//////}
//////
//////// COMPILATION PILE OPERATIONS
//////
//////void test_cppush() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	TEST_ASSERT_EQUAL_INT(0,X* x->cpile);
//////	TEST_ASSERT_EQUAL_INT(0, length(x->cpile));
//////
//////	cppush(x);
//////
//////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
//////	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
//////	TEST_ASSERT(IS(LST,X* x->cpile));
//////	TEST_ASSERT_EQUAL_INT(0, length(A(x->cpile)));
//////
//////	cppush(x);
//////
//////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
//////	TEST_ASSERT_EQUAL_INT(2, length(x->cpile));
//////	TEST_ASSERT(IS(LST,X* x->cpile));
//////	TEST_ASSERT_EQUAL_INT(0, length(A(x->cpile)));
//////	TEST_ASSERT(IS(LST, N(x->cpile)));
//////	TEST_ASSERT_EQUAL_INT(0, length(A(N(x->cpile))));
//////}
//////
//////void test_cppop_1() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	cppush(x);
//////	cppop(x);
//////
//////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
//////	TEST_ASSERT_EQUAL_INT(0, length(x->cpile));
//////	TEST_ASSERT_EQUAL_INT(0,X* x->cpile);
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(0, length(A(S(x))));
//////}
//////
//////void test_cspush() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	cppush(x);
//////	cspush(x, cons(x, 7, T(ATM, 0)));
//////
//////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
//////	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(x->cpile)));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(x->cpile)));
//////
//////	cspush(x, cons(x, 11, T(ATM, 0)));
//////
//////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
//////	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
//////	TEST_ASSERT_EQUAL_INT(2, length(A(x->cpile)));
//////	TEST_ASSERT(IS(ATM, A(x->cpile)));
//////	TEST_ASSERT_EQUAL_INT(11, A(A(x->cpile)));
//////	TEST_ASSERT(IS(ATM, N(A(x->cpile))));
//////	TEST_ASSERT_EQUAL_INT(7, A(N(A(x->cpile))));
//////
//////	cppush(x);
//////
//////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 4, FREE(x));
//////	TEST_ASSERT_EQUAL_INT(2, length(x->cpile));
//////	TEST_ASSERT_EQUAL_INT(0, length(A(x->cpile)));
//////	TEST_ASSERT_EQUAL_INT(2, length(A(N(x->cpile))));
//////	TEST_ASSERT(IS(ATM, A(N(x->cpile))));
//////	TEST_ASSERT_EQUAL_INT(11, A(A(N(x->cpile))));
//////	TEST_ASSERT(IS(ATM, N(A(N(x->cpile)))));
//////	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(x->cpile)))));
//////
//////	cspush(x, cons(x, 13, T(ATM, 0)));
//////
//////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, FREE(x));
//////	TEST_ASSERT_EQUAL_INT(2, length(x->cpile));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(x->cpile)));
//////	TEST_ASSERT(IS(ATM, A(x->cpile)));
//////	TEST_ASSERT_EQUAL_INT(13, A(A(x->cpile)));
//////	TEST_ASSERT_EQUAL_INT(2, length(A(N(x->cpile))));
//////	TEST_ASSERT(IS(ATM, A(N(x->cpile))));
//////	TEST_ASSERT_EQUAL_INT(11, A(A(N(x->cpile))));
//////	TEST_ASSERT(IS(ATM, N(A(N(x->cpile)))));
//////	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(x->cpile)))));
//////}
//////
//////void test_cppop_2() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	cppush(x);
//////	cspush(x, cons(x, 7, T(ATM, 0)));
//////	cspush(x, cons(x, 11, T(ATM, 0)));
//////	cppush(x);
//////	cspush(x, cons(x, 13, T(ATM, 0)));
//////	cppop(x);
//////
//////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, FREE(x));
//////	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
//////	TEST_ASSERT_EQUAL_INT(3, length(A(x->cpile)));
//////	TEST_ASSERT(IS(LST, A(x->cpile)));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(A(x->cpile))));
//////	TEST_ASSERT_EQUAL_INT(13, A(A(A(x->cpile))));
//////	TEST_ASSERT(IS(ATM, N(A(x->cpile))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->cpile))));
//////	TEST_ASSERT(IS(ATM, N(N(A(x->cpile)))));
//////	TEST_ASSERT_EQUAL_INT(7, A(N(N(A(x->cpile)))));
//////
//////	cppop(x);
//////
//////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, FREE(x));
//////	TEST_ASSERT_EQUAL_INT(0, length(x->cpile));
//////	TEST_ASSERT_EQUAL_INT(0,X* x->cpile);
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
//////	TEST_ASSERT(IS(LST, A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(13, A(A(A(S(x)))));
//////	TEST_ASSERT(IS(ATM, N(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//////	TEST_ASSERT(IS(ATM, N(N(A(S(x))))));
//////	TEST_ASSERT_EQUAL_INT(7, A(N(N(A(S(x))))));
//////}
//////
//////void test_braces() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	_lbrace(x);
//////	cspush(x, cons(x, 1, T(ATM, 0)));
//////	cspush(x, cons(x, 2, T(ATM, 0)));
//////	_lbrace(x);
//////	cspush(x, cons(x, 3, T(ATM, 0)));
//////	cspush(x, cons(x, 4, T(ATM, 0)));
//////	_rbrace(x);
//////	cspush(x, cons(x, 5, T(ATM, 0)));
//////	_lbrace(x);
//////	cspush(x, cons(x, 6, T(ATM, 0)));
//////	_rbrace(x);
//////	_rbrace(x);
//////
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(0,X* x->cpile);
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(5, length(A(S(x))));
//////	TEST_ASSERT(IS(LST, A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(A(S(x)))));
//////	TEST_ASSERT(IS(ATM, A(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(6, A(A(A(S(x)))));
//////	TEST_ASSERT(IS(ATM, N(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(5, A(N(A(S(x)))));
//////	TEST_ASSERT(IS(LST, N(N(A(S(x))))));
//////	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(S(x)))))));
//////	TEST_ASSERT(IS(ATM, A(N(N(A(S(x)))))));
//////	TEST_ASSERT_EQUAL_INT(4, A(A(N(N(A(S(x)))))));
//////	TEST_ASSERT(IS(ATM, N(A(N(N(A(S(x))))))));
//////	TEST_ASSERT_EQUAL_INT(3, A(N(A(N(N(A(S(x))))))));
//////	TEST_ASSERT(IS(ATM, N(N(N(A(S(x)))))));
//////	TEST_ASSERT_EQUAL_INT(2, A(N(N(N(A(S(x)))))));
//////	TEST_ASSERT(IS(ATM, N(N(N(N(A(S(x))))))));
//////	TEST_ASSERT_EQUAL_INT(1, A(N(N(N(N(A(S(x))))))));
//////}
//////
//////void test_empty() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	_empty(x);
//////
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//////}
//////
//////void test_join_atom_atom_1() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 11);
//////	push(x, ATM, 7);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//////}
//////
//////void test_join_atom_atom_2() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 13);
//////
//////	push(x, ATM, 11);
//////	push(x, ATM, 7);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(13, A(N(S(x))));
//////}
//////
//////void test_join_atom_empty_1() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	_empty(x);
//////	push(x, ATM, 7);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////}
//////
//////void test_join_atom_empty_2() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 13);
//////
//////	_empty(x);
//////	push(x, ATM, 7);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(13, A(N(S(x))));
//////}
//////
//////void test_join_atom_list_1() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 13);
//////	push(x, ATM, 11);
//////	_join(x);
//////	push(x, ATM, 7);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(S(x))))));
//////}
//////
//////void test_join_atom_list_2() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 17);
//////
//////	push(x, ATM, 13);
//////	push(x, ATM, 11);
//////	_join(x);
//////	push(x, ATM, 7);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(S(x))))));
//////	TEST_ASSERT(IS(ATM, N(S(x))));
//////	TEST_ASSERT_EQUAL_INT(17, A(N(S(x))));
//////}
//////
//////void test_join_list_atom_1() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 13);
//////	push(x, ATM, 11);
//////	push(x, ATM, 7);
//////	_join(x);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(S(x))))));
//////}
//////
//////void test_join_list_atom_2() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 17);
//////
//////	push(x, ATM, 13);
//////	push(x, ATM, 11);
//////	push(x, ATM, 7);
//////	_join(x);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(S(x))))));
//////	TEST_ASSERT(IS(ATM, N(S(x))));
//////	TEST_ASSERT_EQUAL_INT(17, A(N(S(x))));
//////}
//////
//////void test_join_list_list_1() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 17);
//////	push(x, ATM, 13);
//////	_join(x);
//////	push(x, ATM, 11);
//////	push(x, ATM, 7);
//////	_join(x);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(4, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(S(x))))));
//////	TEST_ASSERT_EQUAL_INT(17, A(N(N(N(A(S(x)))))));
//////}
//////
//////void test_join_list_list_2() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 19);
//////
//////	push(x, ATM, 17);
//////	push(x, ATM, 13);
//////	_join(x);
//////	push(x, ATM, 11);
//////	push(x, ATM, 7);
//////	_join(x);
//////	_join(x);
//////
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(4, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(S(x))))));
//////	TEST_ASSERT_EQUAL_INT(17, A(N(N(N(A(S(x)))))));
//////	TEST_ASSERT(IS(ATM, N(S(x))));
//////	TEST_ASSERT_EQUAL_INT(19, A(N(S(x))));
//////}
//////
//////void test_quote_1() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 7);
//////	_quote(x);
//////
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////}
//////
//////void test_quote_2() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 11);
//////
//////	push(x, ATM, 7);
//////	_quote(x);
//////
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//////	TEST_ASSERT(IS(ATM, N(S(x))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
//////}
//////
//////void test_quote_3() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 11);
//////	push(x, ATM, 7);
//////	_join(x);
//////	_quote(x);
//////
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(S(x))));
//////	TEST_ASSERT(IS(LST, A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(2, length(A(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(A(S(x))))));
//////}
//////
//////void test_quote_4() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 13);
//////
//////	push(x, ATM, 11);
//////	push(x, ATM, 7);
//////	_join(x);
//////	_quote(x);
//////
//////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////	TEST_ASSERT(IS(LST,X* S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, length(A(S(x))));
//////	TEST_ASSERT(IS(LST, A(S(x))));
//////	TEST_ASSERT_EQUAL_INT(2, length(A(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(7, A(A(A(S(x)))));
//////	TEST_ASSERT_EQUAL_INT(11, A(N(A(A(S(x))))));
//////	TEST_ASSERT(IS(ATM, N(S(x))));
//////	TEST_ASSERT_EQUAL_INT(13, A(N(S(x))));
//////}
//////
//////void test_neq() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 13);
//////	push(x, ATM, 7);
//////	_neq(x);
//////	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	pop(x);
//////
//////	push(x, ATM, 7);
//////	push(x, ATM, 13);
//////	_neq(x);
//////	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	pop(x);
//////
//////	push(x, ATM, 7);
//////	push(x, ATM, 7);
//////	_neq(x);
//////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	pop(x);
//////}
//////
//////void test_not() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	push(x, ATM, 7);
//////	_not(x);
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//////	pop(x);
//////
//////	push(x, ATM, 0);
//////	_not(x);
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
//////	pop(x);
//////
//////	push(x, ATM, 1);
//////	_not(x);
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//////	pop(x);
//////
//////	push(x, ATM, -1);
//////	_not(x);
//////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//////	pop(x);
//////}
//////
//////#define ATM(x, n, d)							cons(x, n, T(ATM, d))
//////#define LST(x, l, d)							cons(x, l, T(LST, d))
//////#define PRM(x, p, d)				cons(x, (C)p, T(PRM, d))
//////#define RECURSION(x, d)						PRM(x, 0, d)
//////#define JUMP(x, j, d)							cons(x, ATM(x, j, 0), T(JMP, d))
//////#define LAMBDA(x, w, d)						cons(x, cons(x, cons(x, w, T(LST, 0)), T(LST, 0)), T(JMP, d))
//////#define CALL(x,X* xt, d)						cons(x, cons(x,X* xt, T(LST, 0)), T(JMP, d))
//////C BRANCH(X*X* x, C t, C f, C d) {
//////	if (t) R(last(t), d); else t = d;
//////	if (f) R(last(f), d); else f = d;
//////	return cons(x, cons(x, t, T(LST, cons(x, f, T(LST, 0)))), T(JMP, d));
//////}
//////
//////void test_allot_str() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	B* here =X* x->here;
//////
//////	B* s = allot_str(x, 11);
//////	strcpy(s, "test string");
//////
//////	TEST_ASSERT_EQUAL_PTR(here + sizeof(C), s);
//////	TEST_ASSERT_EQUAL_STRING("test string", s);
//////	TEST_ASSERT_EQUAL_INT(11, count(s));
//////	TEST_ASSERT_EQUAL_PTR(here + sizeof(C) + 12,X* x->here);
//////}
//////
////////void test_to_number() {
////////	C size = 512;
////////	B block[size];
////////	X*X* x = init(block, size);
////////
////////	x->ibuf = "256";
////////	parse_token(x);
////////	_to_number(x);
////////	C n = pop(x);
////////
////////	TEST_ASSERT_EQUAL_INT(256, n);
////////}
//////
//////////void test_clear_s() {
//////////	C size = 512;
//////////	B block[size];
//////////	X*X* x = init(block, size);
//////////
//////////	C f_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 3;
//////////
//////////	TEST_ASSERT_EQUAL_INT(f_nodes, height(x->f));
//////////
//////////	push(x, 13);
//////////	push(x, 7);
//////////	
//////////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(f_nodes - 2, height(x->f));
//////////
//////////	_sclear(x);
//////////
//////////	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(f_nodes, height(x->f));
//////////}
//////////
//////////void test_push_s() {
//////////	C size = 512;
//////////	B block[size];
//////////	X*X* x = init(block, size);
//////////
//////////	C f_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 3;
//////////
//////////	push(x, 13);
//////////	push(x, 7);
//////////
//////////	TEST_ASSERT_EQUAL_INT(f_nodes - 2, height(x->f));
//////////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////////
//////////	_spush(x);
//////////
//////////	TEST_ASSERT_EQUAL_INT(f_nodes - 3, height(x->f));
//////////	TEST_ASSERT_EQUAL_INT(7, A(D(P(x))));
//////////	TEST_ASSERT_EQUAL_INT(13, A(D(D(P(x)))));
//////////	TEST_ASSERT_EQUAL_INT(2, height(P(x)));
//////////	TEST_ASSERT_EQUAL_INT(R(x), A(P(x)));
//////////	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//////////}
//////////
//////////void test_drop_s() {
//////////	C size = 512;
//////////	B block[size];
//////////	X*X* x = init(block, size);
//////////
//////////	C f_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 3;
//////////
//////////	_sdrop(x);
//////////
//////////	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(1, length(P(x)));
//////////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
//////////	TEST_ASSERT_EQUAL_INT(f_nodes, height(x->f));
//////////
//////////	push(x, 13);
//////////	push(x, 7);
//////////
//////////	TEST_ASSERT_EQUAL_INT(f_nodes - 2, height(x->f));
//////////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(13,X* S(x));
//////////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
//////////
//////////	_spush(x);
//////////
//////////	TEST_ASSERT_EQUAL_INT(f_nodes - 3, height(x->f));
//////////
//////////	push(x, 21);
//////////
//////////	TEST_ASSERT_EQUAL_INT(f_nodes - 4, height(x->f));
//////////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(21, A(S(x)));
//////////
//////////	_sdrop(x);
//////////
//////////	TEST_ASSERT_EQUAL_INT(f_nodes - 2, height(x->f));
//////////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(13,X* S(x));
//////////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
//////////}
//////////
//////////void test_drop() {
//////////	C size = 512;
//////////	B block[size];
//////////	X*X* x = init(block, size);
//////////
//////////	push(x, 21);
//////////	push(x, 13);
//////////	push(x, 7);
//////////	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(13,X* S(x));
//////////	TEST_ASSERT_EQUAL_INT(21, A(D(D(K(x)))));
//////////	_drop(x);
//////////	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(21,X* S(x));
//////////	_drop(x);
//////////	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(21, A(S(x)));
//////////	_drop(x);
//////////	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//////////	_drop(x);
//////////	TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW,X* x->err);
//////////}
//////////
//////////void test_rev() {
//////////	C size = 512;
//////////	B block[size];
//////////	X*X* x = init(block, size);
//////////
//////////	push(x, 21);
//////////	push(x, 13);
//////////	push(x, 7);
//////////	push(x, 5);
//////////	push(x, 3);
//////////	TEST_ASSERT_EQUAL_INT(5, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(3, A(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(5,X* S(x));
//////////	TEST_ASSERT_EQUAL_INT(7, A(D(D(K(x)))));
//////////	TEST_ASSERT_EQUAL_INT(13, A(D(D(D(K(x))))));
//////////	TEST_ASSERT_EQUAL_INT(21, A(D(D(D(D(K(x)))))));
//////////	_rev(x);
//////////	TEST_ASSERT_EQUAL_INT(5, length(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(21, A(S(x)));
//////////	TEST_ASSERT_EQUAL_INT(13,X* S(x));
//////////	TEST_ASSERT_EQUAL_INT(7, A(D(D(K(x)))));
//////////	TEST_ASSERT_EQUAL_INT(5, A(D(D(D(K(x))))));
//////////	TEST_ASSERT_EQUAL_INT(3, A(D(D(D(D(K(x)))))));
//////////}
//////////
////////////////void test_s_to_list() {
////////////////}
////////////////
//////////////////void test_append() {
//////////////////}
//////////////////
//////////////////void test_copy() {
//////////////////	C size = 512;
//////////////////	B block[size];
//////////////////	X* x = init(block, size);
//////////////////
//////////////////	C f_nodes = (size - sizeof(X)) / sizeof(PAIR);
//////////////////
//////////////////	// TODO
//////////////////}
////////////////
////////////////void test_header_body_reveal() {
////////////////	C size = 2048;
////////////////	B block[size];
////////////////	X* x = init(block, size);
////////////////
////////////////	B* here = x->here;
////////////////	PAIR* w = header(x, "test", 4);
////////////////	TEST_ASSERT_EQUAL_INT(0, x->err);
////////////////	TEST_ASSERT_EQUAL_PTR(x->here, ALIGN(here, sizeof(C)) + sizeof(C) + 4 + 1);
////////////////	TEST_ASSERT(IS(ATM, w));
////////////////	TEST_ASSERT_EQUAL_INT(4, COUNT(NFA(w)));
////////////////	TEST_ASSERT_EQUAL_STRING("test", NFA(w));
////////////////	TEST_ASSERT_EQUAL_INT(((B*)REF(REF(w))) + 4, DFA(w));
////////////////	TEST_ASSERT_EQUAL_INT(0, CFA(w));
////////////////
////////////////	TEST_ASSERT_NULL(x->latest);
////////////////
////////////////	PAIR* cfa = cons(x, ATM, 7, cons(x, ATM, 13, 0));
////////////////	body(x, w, cfa);
////////////////
////////////////	TEST_ASSERT_EQUAL_INT(cfa, CFA(w));
////////////////	TEST_ASSERT_EQUAL_INT(7, CFA(w)->value);
////////////////	TEST_ASSERT_EQUAL_INT(13, N(CFA(w))->value);
////////////////
////////////////	reveal(x, w);
////////////////
////////////////	TEST_ASSERT_EQUAL_PTR(w, x->latest);
////////////////	TEST_ASSERT_NULL(N(x->latest));
////////////////
////////////////	TEST_ASSERT_FALSE(IS_IMMEDIATE(w));
////////////////
////////////////	_immediate(x);
////////////////
////////////////	TEST_ASSERT_TRUE(IS_IMMEDIATE(w));
////////////////}
////////////////
//////////////////void test_find() {
//////////////////	C size = 512;
//////////////////	B block[size];
//////////////////	X* x = init(block, size);
//////////////////
//////////////////	PAIR* dup = reveal(x, header(x, "dup", 3));
//////////////////	PAIR* swap = reveal(x, header(x, "swap", 4));
//////////////////	PAIR* test = reveal(x, header(x, "test", 4));
//////////////////
//////////////////	TEST_ASSERT_EQUAL_PTR(dup, find(x, "dup", 3));
//////////////////	TEST_ASSERT_EQUAL_PTR(swap, find(x, "swap", 4));
//////////////////	TEST_ASSERT_EQUAL_PTR(test, find(x, "test", 4));
//////////////////	TEST_ASSERT_EQUAL_PTR(0, find(x, "nop", 3));
//////////////////
//////////////////	TEST_ASSERT_EQUAL_PTR(test, x->latest);
//////////////////	TEST_ASSERT_EQUAL_PTR(swap, N(x->latest));
//////////////////	TEST_ASSERT_EQUAL_PTR(dup, N(N(x->latest)));
//////////////////	TEST_ASSERT_EQUAL_PTR(0, N(N(N(x->latest))));
//////////////////}
//////////////////
//////////////////void test_dodo_initialization() {
//////////////////	C size = 4096;
//////////////////	B block[size];
//////////////////	X* x = dodo(init(block, size));
//////////////////
//////////////////	TEST_ASSERT_EQUAL_PTR(&_add, CFA(find(x, "+", 1))->value);
//////////////////}
//////////////////
//////////////////void test_fib() {
//////////////////	C size = 8192;
//////////////////	B block[size];
//////////////////	X* x = init(block, size);
//////////////////
//////////////////	// : fib dup 1 > if 1- dup 1- recurse swap recurse + then ;
//////////////////
//////////////////	x->ip =
//////////////////		cons(x, (C)&_dup, T_PRM,
//////////////////		cons(x, 1, ATM,
//////////////////		cons(x, (C)&_gt, T_PRM,
//////////////////		cons(x,
//////////////////			0,
//////////////////			T_JMP,
//////////////////			cons(x, 1, ATM,
//////////////////			cons(x, (C)&_sub, T_PRM,
//////////////////			cons(x, (C)&_dup, T_PRM,
//////////////////			cons(x, 1, ATM,
//////////////////			cons(x, (C)&_sub, T_PRM,
//////////////////			cons(x, (C)&_rec, T_PRM,
//////////////////			cons(x, (C)&_swap, T_PRM,
//////////////////			cons(x, (C)&_rec, T_PRM,	
//////////////////			cons(x, (C)&_add, T_PRM, 0)))))))))))));
//////////////////
//////////////////		x->latests = cons(x, 6, ATM, 0);
//////////////////		x->r = cons(x, (C)x->ip, T_WORD, 0);
//////////////////
//////////////////		inner(x);
//////////////////
//////////////////		//printf("%ld\n", x->latests->value);
//////////////////}
////////////////
int main() {
	UNITY_BEGIN();

	// TAGGED POINTER BASED TYPING INFORMATION
	RUN_TEST(test_basic_types);

	// CONTEXT
	RUN_TEST(test_X_block_initialization);

	// LST CREATION AND DESTRUCTION (AUTOMATIC MEMORY MANAGEMENT)
	RUN_TEST(test_LST_cons);
	RUN_TEST(test_LST_clon);
	RUN_TEST(test_LST_recl);
	RUN_TEST(test_LST_recl_list);
//	RUN_TEST(test_LST_rvs);
	RUN_TEST(test_LST_length);

	// INNER INTERPRETER
	RUN_TEST(test_INNER_execute_atom);
	RUN_TEST(test_INNER_execute_list);
	RUN_TEST(test_INNER_execute_primitive);
//	RUN_TEST(test_INNER_execute_call);
//
//	// PILE PRMITIVES
//	RUN_TEST(test_PILE_spush);
//
//	//// IP PRMITIVES
//	//RUN_TEST(test_IP_branch);
//	//RUN_TEST(test_IP_jump);
//	//RUN_TEST(test_IP_zjump);
//
//	//// STACK PRMITIVES
//	//RUN_TEST(test_STACK_duplicate_atom);
//	//RUN_TEST(test_STACK_duplicate_list);
//	//RUN_TEST(test_STACK_swap_1);
//	//RUN_TEST(test_STACK_swap_2);
//	//RUN_TEST(test_STACK_drop);
//	//RUN_TEST(test_STACK_over);
//	//RUN_TEST(test_STACK_rot);
//
//	//// ARITHMETIC PRMITIVES
//	//RUN_TEST(test_ARITHMETIC_add);
//	//RUN_TEST(test_ARITHMETIC_sub);
//	//RUN_TEST(test_ARITHMETIC_mul);
//	//RUN_TEST(test_ARITHMETIC_division);
//	//RUN_TEST(test_ARITHMETIC_mod);
//
//	//// COMPARISON PRMITIVES
//	//RUN_TEST(test_COMPARISON_gt);
//	//RUN_TEST(test_COMPARISON_lt);
//	//RUN_TEST(test_COMPARISON_eq);
//
//	//// BIT PRMITIVES
//	//RUN_TEST(test_BIT_and);
//	//RUN_TEST(test_BIT_or);
//	//RUN_TEST(test_BIT_invert);
//
//	//// PARSING
//	//RUN_TEST(test_PARSING_parse_token);
//	//RUN_TEST(test_PARSING_fnd_tk);
//
//	//// OUTER INTERPRETER
//	//RUN_TEST(test_OUTER_evaluate_numbers);
//	//RUN_TEST(test_OUTER_evaluate_words);
//
//	//// CONTIGUOUS MEMORY
//	//RUN_TEST(test_MEM_allot);
//	//RUN_TEST(test_MEM_compile_str);
//	////RUN_TEST(test_align);
//
//	////  LST FUNCTIONS
//	//RUN_TEST(test_append);
//	//RUN_TEST(test_depth);
//
//	//// WORD DEFINITIONS
//	//RUN_TEST(test_header);
//	//RUN_TEST(test_body);
//	//RUN_TEST(test_reveal);
//	//RUN_TEST(test_immediate);
//
////	RUN_TEST(test_mlength);
////	RUN_TEST(test_last);
////
////	// COMPILATION PILE OPERATIONS
////	RUN_TEST(test_cppush);
////	RUN_TEST(test_cppop_1);
////	RUN_TEST(test_cspush);
////	RUN_TEST(test_cppop_2);
////
////	RUN_TEST(test_braces);
////
////	RUN_TEST(test_empty);
////
////	RUN_TEST(test_join_atom_atom_1);
////	RUN_TEST(test_join_atom_atom_2);
////	RUN_TEST(test_join_atom_empty_1);
////	RUN_TEST(test_join_atom_empty_2);
////	RUN_TEST(test_join_atom_list_1);
////	RUN_TEST(test_join_atom_list_2);
////	RUN_TEST(test_join_list_atom_1);
////	RUN_TEST(test_join_list_atom_2);
////	RUN_TEST(test_join_list_list_1);
////	RUN_TEST(test_join_list_list_2);
////
////	RUN_TEST(test_quote_1);
////	RUN_TEST(test_quote_2);
////	RUN_TEST(test_quote_3);
////	RUN_TEST(test_quote_4);
////
////	RUN_TEST(test_gt);
////	RUN_TEST(test_lt);
////	RUN_TEST(test_eq);
////	RUN_TEST(test_neq);
////
////	RUN_TEST(test_not);
////
////	//RUN_TEST(test_clear_s);
////	//RUN_TEST(test_push_s);
////	//RUN_TEST(test_drop_s);
////
////
////	RUN_TEST(test_allot_str);
////
////	RUN_TEST(test_find);
////	//RUN_TEST(test_to_number);
////
//////	//RUN_TEST(test_s_to_list);
//////
//////	////RUN_TEST(test_fib);
//////
//////	////RUN_TEST(test_append);
//////	////RUN_TEST(test_copy);
//////
//////
//////	//RUN_TEST(test_header_body_reveal);
//////	////RUN_TEST(test_find);
//////
//////	////RUN_TEST(test_dodo_initialization);
//////
	return UNITY_END();
}
