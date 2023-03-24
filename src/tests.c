// TODO: Idea::use strings to compare context results
// TODO: Idea::use diff between context to see changes

#include<stdlib.h>
#include "dodo.h"
#include "unity.h"

#define PUSH(x, v)		(S(x) = cons(x, (C)v, AS(ATM, S(x))))
#define PUSHL(x, l)		(S(x) = cons(x, l, AS(LST, S(x))))

void setUp() {}

void tearDown() {}

// TAGGED POINTER BASED TYPING INFORMATION

void test_BASIC_pairs() {
	C p = (C)malloc(sP);

	A(p) = 7;	
	D(p) = AS(ATM, 8);
	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(8, N(p));
	TEST_ASSERT_EQUAL_INT(ATM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(LST, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(PRM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(WRD, T(p));
	D(p) = AS(LST, 16);
	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(16, N(p));
	TEST_ASSERT_NOT_EQUAL_INT(ATM, T(p));
	TEST_ASSERT_EQUAL_INT(LST, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(PRM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(WRD, T(p));
	D(p) = AS(PRM, 24);
	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(24, N(p));
	TEST_ASSERT_NOT_EQUAL_INT(ATM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(LST, T(p));
	TEST_ASSERT_EQUAL_INT(PRM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(WRD, T(p));
	D(p) = AS(WRD, 32);
	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(32, N(p));
	TEST_ASSERT_NOT_EQUAL_INT(ATM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(LST, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(PRM, T(p));
	TEST_ASSERT_EQUAL_INT(WRD, T(p));
}

void test_BASIC_link_pairs() {
	C p = (C)malloc(sC * 2);

	A(p) = 7;
	D(p) = AS(ATM, 16);

	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(16, N(p));
	TEST_ASSERT_EQUAL_INT(ATM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(LST, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(PRM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(WRD, T(p));
	
	LK(p, 32);

	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(32, N(p));
	TEST_ASSERT_EQUAL_INT(ATM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(LST, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(PRM, T(p));
	TEST_ASSERT_NOT_EQUAL_INT(WRD, T(p));
}

// LIST LENGTH

void test_LST_length() {
	C p1 = (C)malloc(2*sC);
	C p2 = (C)malloc(2*sC);
	C p3 = (C)malloc(2*sC);

	D(p1) = AS(ATM, p2);
	D(p2) = p3;
	D(p3) = 0;

	TEST_ASSERT_EQUAL_INT(0, length(0));
	TEST_ASSERT_EQUAL_INT(1, length(p3));
	TEST_ASSERT_EQUAL_INT(2, length(p2));
	TEST_ASSERT_EQUAL_INT(3, length(p1));
}

//void test_BASIC_words() {
//	C w = (C)malloc(sC * 2);
//	C nfa = (C)malloc(sC * 2);
//	C xt = (C)malloc(sC * 2);
//
//	A(w) = nfa;
//	D(w) = AS(NON_IMM_PRM, 0);
//	A(nfa) = (C)"word-name";
//	D(nfa) = AS(ATM, xt);
//	A(xt) = 7;
//	D(xt) = 0;
//
//	TEST_ASSERT_EQUAL_INT(1, PRMITIVE(w));
//	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(w));
//	TEST_ASSERT_EQUAL_STRING("word-name", NFA(w));
//	TEST_ASSERT_EQUAL_INT(xt, XT(w));
//
//	D(w) = AS(IMM_PRM, 0);
//
//	TEST_ASSERT_EQUAL_INT(1, PRMITIVE(w));
//	TEST_ASSERT_EQUAL_INT(1, IMMEDIATE(w));
//	TEST_ASSERT_EQUAL_STRING("word-name", NFA(w));
//	TEST_ASSERT_EQUAL_INT(xt, XT(w));
//
//	D(w) = AS(NON_IMM_COLON, 0);
//
//	TEST_ASSERT_EQUAL_INT(0, PRMITIVE(w));
//	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(w));
//	TEST_ASSERT_EQUAL_STRING("word-name", NFA(w));
//	TEST_ASSERT_EQUAL_INT(xt, XT(w));
//
//	D(w) = AS(IMM_COLON, 0);
//
//	TEST_ASSERT_EQUAL_INT(0, PRMITIVE(w));
//	TEST_ASSERT_EQUAL_INT(1, IMMEDIATE(w));
//	TEST_ASSERT_EQUAL_STRING("word-name", NFA(w));
//	TEST_ASSERT_EQUAL_INT(xt, XT(w));
//}
//
// CONTEXT

#define f_nodes(x)	((x->sz - ALIGN(sizeof(X), sP)) / sP - 3)

//void test_X_minimum_block_size() {
//	C size = 2;
//	B block[size];
//	X* x = init(block, size);
//
//	TEST_ASSERT_NULL(x);
//
//	C size2 = sizeof(X) + 2*2*sC;
//	B block2[size2];
//	X* x2 = init(block2, size2);
//
//	TEST_ASSERT_NOT_NULL(x2);
//}

void test_X_block_initialization() {
	C size = 512;
	B block[size];
	X* x = init(block, size);
		
	TEST_ASSERT_NOT_EQUAL(0, x);

	TEST_ASSERT_EQUAL(size, x->sz);

	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
	TEST_ASSERT_EQUAL_INT(f_nodes(x), x->free);

	TEST_ASSERT_EQUAL_INT(((B*)x) + sizeof(X), BOTTOM(x));
	TEST_ASSERT_EQUAL_INT(BOTTOM(x), x->hr);
	TEST_ASSERT_EQUAL_INT(ALIGN(x->hr, sP), x->th);
	TEST_ASSERT_EQUAL_INT(ALIGN(((B*)x) + size - sP - 1, sP), TOP(x));
	TEST_ASSERT_EQUAL_INT(TOP(x), x->ds);
	TEST_ASSERT_EQUAL_INT(TOP(x) - sP, x->rs);
	TEST_ASSERT_EQUAL_INT(TOP(x) - sP - sP, x->fr);

	TEST_ASSERT_EQUAL_INT(0, x->lt);
	TEST_ASSERT_EQUAL_INT(0, S(x));
	TEST_ASSERT_EQUAL_INT(0, R(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(0, x->st);

	TEST_ASSERT_NULL(x->tb);
	TEST_ASSERT_EQUAL_INT(0, x->in);
}

// LST CREATION AND DESTRUCTION (AUTOMATIC MEMORY MANAGEMENT)

void test_LST_cons() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C list = cons(x, 7, cons(x, 11, cons(x, 13, 0)));

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, x->free);
	TEST_ASSERT_EQUAL_INT(3, length(list));
	TEST_ASSERT_EQUAL_INT(7, A(list));
	TEST_ASSERT_EQUAL_INT(11, A(D(list)));
	TEST_ASSERT_EQUAL_INT(13, A(D(D(list))));

	C p = cons(x, 7, 0);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 4, x->free);
	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(0, D(p));
}

void test_LST_recl() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C list = cons(x, 7, cons(x, 11, cons(x, 13, 0)));
	C tail = recl(x, list);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, x->free);
	TEST_ASSERT_EQUAL_INT(2, length(tail));
	TEST_ASSERT_EQUAL_INT(11, A(tail));
	TEST_ASSERT_EQUAL_INT(13, A(D(tail)));

	tail = recl(x, recl(x, tail));

	TEST_ASSERT_EQUAL_INT(f_nodes(x), x->free);
	TEST_ASSERT_EQUAL_INT(0, length(tail));
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

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 6, x->free);
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

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, x->free);
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

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, x->free);
	TEST_ASSERT_EQUAL(ATM, T(tail));
	TEST_ASSERT_EQUAL_INT(17, A(tail));

	tail = recl(x, tail);

	TEST_ASSERT_EQUAL_INT(f_nodes(x), x->free);
	TEST_ASSERT_EQUAL_INT(0, tail);
}

void test_LST_clone() {
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

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 6, x->free);
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

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 12, x->free);
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

void test_LST_reverse() {
	C p1 = (C)malloc(sP);
	C p2 = (C)malloc(sP);
	C p3 = (C)malloc(sP);
	
	D(p1) = p2;
	D(p2) = p3;
	D(p3) = 0;

	C r = reverse(p1, 0);

	TEST_ASSERT_EQUAL_INT(p3, r);
	TEST_ASSERT_EQUAL_INT(p2, N(r));
	TEST_ASSERT_EQUAL_INT(p1, N(N(r)));
}

//void test_LST_depth() {
//	C p1 = (C)malloc(2*sC);
//	C p2 = (C)malloc(2*sC);
//	C p3 = (C)malloc(2*sC);
//	C p4 = (C)malloc(2*sC);
//	C p5 = (C)malloc(2*sC);
//	C p6 = (C)malloc(2*sC);
//
//	TEST_ASSERT_EQUAL_INT(0, depth(0, 0));
//	TEST_ASSERT_EQUAL_INT(0, depth(0, 1));
//
//	D(p1) = AS(ATM, 0);
//	
//	TEST_ASSERT_EQUAL_INT(1, depth(p1, 0));
//	TEST_ASSERT_EQUAL_INT(1, depth(p1, 1));
//
//	D(p1) = AS(PRM, 0);
//
//	TEST_ASSERT_EQUAL_INT(1, depth(p1, 0));
//	TEST_ASSERT_EQUAL_INT(1, depth(p1, 1));
//
//	D(p1) = AS(WRD, 0);
//
//	TEST_ASSERT_EQUAL_INT(1, depth(p1, 0));
//	TEST_ASSERT_EQUAL_INT(1, depth(p1, 1));
//
//	D(p1) = AS(LST, 0);
//	A(p1) = 0;
//
//	TEST_ASSERT_EQUAL_INT(1, depth(p1, 0));
//	TEST_ASSERT_EQUAL_INT(1, depth(p1, 1));
//
//	D(p1) = AS(LST, p2);
//	A(p1) = p3;
//	D(p2) = AS(ATM, 0);
//	D(p3) = AS(ATM, p4);
//	D(p4) = AS(LST, 0);
//	A(p4) = p5;
//	D(p5) = AS(ATM, p6);
//	D(p6) = AS(PRM, 0);
//
//	TEST_ASSERT_EQUAL_INT(6, depth(p1, 1));
//	TEST_ASSERT_EQUAL_INT(5, depth(p1, 0));
//}
//
//void test_LST_disjoint() {
//	C p1 = (C)malloc(2*sC);
//	C p2 = (C)malloc(2*sC);
//	C p3 = (C)malloc(2*sC);
//	C p4 = (C)malloc(2*sC);
//	C p5 = (C)malloc(2*sC);
//	C p6 = (C)malloc(2*sC);
//	C r1 = (C)malloc(2*sC);
//	C r2 = (C)malloc(2*sC);
//	C r3 = (C)malloc(2*sC);
//	C r4 = (C)malloc(2*sC);
//	C r5 = (C)malloc(2*sC);
//	C r6 = (C)malloc(2*sC);
//
//	TEST_ASSERT_EQUAL_INT(0, disjoint(p1, p1, 0));
//	TEST_ASSERT_EQUAL_INT(0, disjoint(p1, p1, 1));
//	
//	D(p1) = AS(ATM, 0);
//	A(p1) = 7;
//
//	D(p2) = AS(ATM, 0);
//	A(p2) = 11;
//
//	D(p3) = AS(ATM, 0);
//	A(p3) = 7;
//
//	D(p4) = AS(PRM, 0);
//	A(p4) = 7;
//
//	TEST_ASSERT_EQUAL_INT(0, disjoint(p1, p2, 0));
//	TEST_ASSERT_EQUAL_INT(1, disjoint(p1, p3, 0));
//	TEST_ASSERT_EQUAL_INT(0, disjoint(p1, p4, 0));
//	TEST_ASSERT_EQUAL_INT(0, disjoint(p1, p2, 1));
//	TEST_ASSERT_EQUAL_INT(1, disjoint(p1, p3, 1));
//	TEST_ASSERT_EQUAL_INT(0, disjoint(p1, p4, 1));
//
//	D(p1) = AS(LST, p2);
//	A(p1) = p3;
//	D(p2) = AS(ATM, 0);
//	A(p2) = 7;
//	D(p3) = AS(ATM, p4);
//	A(p3) = 11;
//	D(p4) = AS(LST, 0);
//	A(p4) = p5;
//	D(p5) = AS(ATM, p6);
//	A(p5) = 13;
//	D(p6) = AS(PRM, 0);
//	A(p6) = 17;
//
//	D(r1) = AS(LST, r2);
//	A(r1) = r3;
//	D(r2) = AS(ATM, 0);
//	A(r2) = 7;
//	D(r3) = AS(ATM, r4);
//	A(r3) = 11;
//	D(r4) = AS(LST, 0);
//	A(r4) = r5;
//	D(r5) = AS(ATM, r6);
//	A(r5) = 13;
//	D(r6) = AS(PRM, 0);
//	A(r6) = 17;
//
//	TEST_ASSERT_EQUAL_INT(1, disjoint(p1, r1, 1));
//
//	D(r3) = AS(ATM, p4);
//
//	TEST_ASSERT_EQUAL_INT(0, disjoint(p1, r1, 1));
//
//	D(r3) = AS(ATM, r4);
//	A(r4) = p5;
//
//	TEST_ASSERT_EQUAL_INT(0, disjoint(p1, r1, 1));
//
//	A(r4) = r5;
//	D(r1) = AS(LST, r2);
//
//	TEST_ASSERT_EQUAL_INT(1, disjoint(p1, r1, 0));
//}
//
//void test_STACK_push() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	PUSH(x, 7);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
//
//	PUSH(x, 11);
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
//}
//
//void test_STACK_pop() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	C v = pop(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, v);
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//
//	v = pop(x);
//
//	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, v);
//}
//
// STACK PRIMITIVES

void test_STACK_duplicate_atom() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, S(x)));
	duplicate(x);

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, x->free);
	TEST_ASSERT_EQUAL_INT(ATM, T(S(x)));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(S(x))));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));

	S(x) = cons(x, 11, AS(ATM, S(x)));
	duplicate(x);

	TEST_ASSERT_EQUAL_INT(4, length(S(x)));
	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 4, x->free);
	TEST_ASSERT_EQUAL_INT(ATM, T(S(x)));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(S(x))));
	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(N(S(x)))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(N(N(S(x))))));
	TEST_ASSERT_EQUAL_INT(7, A(N(N(S(x)))));
	TEST_ASSERT_EQUAL_INT(7, A(N(N(N(S(x))))));
}

void test_STACK_duplicate_list() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = 
		cons(x, 
			cons(x, 17, AS(ATM,
			cons(x, 13, AS(ATM,
			cons(x, 
				cons(x, 11, AS(ATM,
				cons(x, 7, AS(ATM, 0)))),
				AS(LST, 0)))))),
			AS(LST, S(x)));

	// (17, 13, (11, 7))
	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 6, x->free);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
	TEST_ASSERT_EQUAL_INT(ATM, T(A(S(x))));
	TEST_ASSERT_EQUAL_INT(17, A(A(S(x))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(13, A(N(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(LST, T(N(N(A(S(x))))));
	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(S(x)))))));
	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(N(A(S(x)))))));
	TEST_ASSERT_EQUAL_INT(11, A(A(N(N(A(S(x)))))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(N(A(S(x))))))));
	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(N(A(S(x))))))));

	duplicate(x);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 12, x->free);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));

	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
	TEST_ASSERT_EQUAL_INT(ATM, T(A(S(x))));
	TEST_ASSERT_EQUAL_INT(17, A(A(S(x))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(13, A(N(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(LST, T(N(N(A(S(x))))));
	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(S(x)))))));
	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(N(A(S(x)))))));
	TEST_ASSERT_EQUAL_INT(11, A(A(N(N(A(S(x)))))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(N(A(S(x))))))));
	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(N(A(S(x))))))));

	TEST_ASSERT_EQUAL_INT(LST, T(N(S(x))));
	TEST_ASSERT_EQUAL_INT(3, length(A(N(S(x)))));
	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(S(x)))));
	TEST_ASSERT_EQUAL_INT(17, A(A(N(S(x)))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(S(x))))));
	TEST_ASSERT_EQUAL_INT(13, A(N(A(N(S(x))))));
	TEST_ASSERT_EQUAL_INT(LST, T(N(N(A(N(S(x)))))));
	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(N(S(x))))))));
	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(N(A(N(S(x))))))));
	TEST_ASSERT_EQUAL_INT(11, A(A(N(N(A(N(S(x))))))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(N(A(N(S(x)))))))));
	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(N(A(N(S(x)))))))));

	TEST_ASSERT_NOT_EQUAL_INT(S(x), N(S(x)));
	TEST_ASSERT_NOT_EQUAL_INT(A(S(x)), A(N(S(x))));
	TEST_ASSERT_NOT_EQUAL_INT(N(A(S(x))), N(A(N(S(x)))));
	TEST_ASSERT_NOT_EQUAL_INT(N(N(A(S(x)))), N(N(A(N(S(x))))));
	TEST_ASSERT_NOT_EQUAL_INT(A(N(N(A(S(x))))), A(N(N(A(N(S(x)))))));
	TEST_ASSERT_NOT_EQUAL_INT(N(A(N(N(A(S(x)))))), N(A(N(N(A(N(S(x))))))));
}

void test_STACK_swap_1() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
	swap(x);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
}

void test_STACK_swap_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 11, AS(ATM, cons(x, cons(x, 7, AS(ATM, cons(x, 5, AS(ATM, 0)))), AS(LST, 0))));
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(ATM, T(S(x)));
	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
	TEST_ASSERT_EQUAL_INT(LST, T(N(S(x))));
	TEST_ASSERT_EQUAL_INT(2, length(A(N(S(x)))));
	TEST_ASSERT_EQUAL_INT(7, A(A(N(S(x)))));
	TEST_ASSERT_EQUAL_INT(5, A(N(A(N(S(x))))));
	swap(x);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
	TEST_ASSERT_EQUAL_INT(5, A(N(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(S(x))));
	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
}

void test_STACK_drop() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, S(x)));

	drop(x);

	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
	TEST_ASSERT_EQUAL_INT(f_nodes(x), x->free);

	S(x) = 
		cons(x,
			cons(x, 7, AS(ATM,
			cons(x,
				cons(x, 11, AS(ATM,
				cons(x, 13, AS(ATM, 0)))),
			AS(LST, 0)))),
		AS(LST, S(x)));

	drop(x);

	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
	TEST_ASSERT_EQUAL_INT(f_nodes(x), x->free);
}

void test_STACK_over() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));

	over(x);

	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(11, A(N(N(S(x)))));

	S(x) = 
		cons(x, 7, AS(ATM, 
		cons(x, 
			cons(x, 11, AS(ATM,
			cons(x, 13, AS(ATM, 0)))),
		AS(LST, 0))));

	over(x);

	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
	TEST_ASSERT_EQUAL_INT(11, A(A(S(x))));
	TEST_ASSERT_EQUAL_INT(13, A(N(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
	TEST_ASSERT_NOT_EQUAL_INT(N(N(S(x))), S(x));
	TEST_ASSERT_NOT_EQUAL_INT(A(N(N(S(x)))), A(S(x)));
	TEST_ASSERT_NOT_EQUAL_INT(N(A(N(N(S(x))))), N(A(S(x))));
}

void test_STACK_rot() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, cons(x, 13, AS(ATM, 0))))));

	rot(x);

	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(11, A(N(N(S(x)))));
}

// COMPARISON PRMITIVES

void test_COMPARISON_gt() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
	gt(x);
	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));

	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
	gt(x);
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));

	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
	gt(x);
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
}

void test_COMPARISON_lt() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
	lt(x);
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));

	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
	lt(x);
	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));

	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
	lt(x);
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
}

void test_COMPARISON_eq() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
	eq(x);
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));

	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
	eq(x);
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));

	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
	eq(x);
	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
}

void test_COMPARISON_neq() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
	neq(x);
	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));

	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
	neq(x);
	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));

	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
	neq(x);
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
}

// ARITHMETIC PRMITIVES

void test_ARITHMETIC_add() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
	add(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(18, A(S(x)));
}

void test_ARITHMETIC_sub() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
	sub(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(4, A(S(x)));
}

void test_ARITHMETIC_mul() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
	mul(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(77, A(S(x)));
}

void test_ARITHMETIC_division() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 11, AS(ATM, cons(x, 77, AS(ATM, 0))));
	division(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
}

void test_ARITHMETIC_mod() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
	mod(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(4, A(S(x)));
}

// BIT PRMITIVES

void test_BIT_and() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
	and(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(3, A(S(x)));

	S(x) = cons(x, 0, AS(ATM, cons(x, 0, AS(ATM, 0))));
	and(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));

	S(x) = cons(x, -1, AS(ATM, cons(x, 0, AS(ATM, 0))));
	and(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));

	S(x) = cons(x, 0, AS(ATM, cons(x, -1, AS(ATM, 0))));
	and(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));

	S(x) = cons(x, -1, AS(ATM, cons(x, -1, AS(ATM, 0))));	
	and(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
}

void test_BIT_or() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
	or(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(15, A(S(x)));

	S(x) = cons(x, 0, AS(ATM, cons(x, 0, AS(ATM, 0))));
	or(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));

	S(x) = cons(x, -1, AS(ATM, cons(x, 0, AS(ATM, 0))));
	or(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));

	S(x) = cons(x, 0, AS(ATM, cons(x, -1, AS(ATM, 0))));
	or(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));

	S(x) = cons(x, -1, AS(ATM, cons(x, -1, AS(ATM, 0))));
	or(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
}

void test_BIT_invert() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, 7, AS(ATM, 0));
	invert(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(-8, A(S(x)));

	S(x) = cons(x, 0, AS(ATM, 0));
	invert(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));

	S(x) = cons(x, 1, AS(ATM, 0));
	invert(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(-2, A(S(x)));

	S(x) = cons(x, -1, AS(ATM, 0));
	invert(x);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
}

// LIST PRIMITIVES

void test_LIST_empty() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	empty(x);
	
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(0, length(A(S(x))));
}

void test_LIST_list_to_stack() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	list_to_stack(x);

	TEST_ASSERT_EQUAL_INT(E_UF, x->err);
	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(x->ds));

	x->err = 0;

	S(x) = cons(x, 0, AS(ATM, 0));

	list_to_stack(x);

	TEST_ASSERT_EQUAL_INT(E_EL, x->err);
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(x->ds));
}

void test_LIST_list_to_stack_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	empty(x);

	list_to_stack(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
	TEST_ASSERT_EQUAL_INT(2, length(x->ds));
}

void test_LIST_list_to_stack_3() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0)))), AS(LST, 0));

	list_to_stack(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(ATM, T(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(S(x))));
	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(2, length(x->ds));
}

void test_LIST_stack_to_list() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	stack_to_list(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(1, length(x->ds));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(0, length(A(S(x))));
}

void test_LIST_stack_to_list_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	empty(x);
	list_to_stack(x);
	stack_to_list(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(1, length(x->ds));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(0, length(A(S(x))));
}

void test_LIST_stack_to_list_3() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	empty(x);
	list_to_stack(x);
	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
	stack_to_list(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(1, length(x->ds));
	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
	TEST_ASSERT_EQUAL_INT(11, A(A(S(x))));
	TEST_ASSERT_EQUAL_INT(7, A(N(A(S(x)))));
}

// INNER INTERPRETER

void test_INNER_NEXT_1() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	R(x) = 0;

	NEXT(x);

	TEST_ASSERT_EQUAL_INT(0, R(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);
}

void test_INNER_NEXT_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C i1 = cons(x, 11, AS(ATM, 0));
	C i2 = cons(x, 7, AS(ATM, i1));

	R(x) = i2;

	NEXT(x);

	TEST_ASSERT_EQUAL_INT(i1, R(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);

	NEXT(x);

	TEST_ASSERT_EQUAL_INT(0, R(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);
}

void test_INNER_NEXT_3() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C i1 = cons(x, 11, AS(ATM, 0));
	C i2 = cons(x, 7, AS(ATM, i1));

	R(x) = i2;

	x->rs = cons(x, 0, AS(LST, x->rs));

	NEXT(x);

	// First item on previous stack represents word called and is removed after
	// deleting top stack.

	TEST_ASSERT_EQUAL_INT(i1, R(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(1, length(x->rs));

	NEXT(x);

	TEST_ASSERT_EQUAL_INT(0, R(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);
}

void test_INNER_STEP_nothing() {
	//TEST_IGNORE();
	C size = 512;
	B block[size];
	X* x = init(block, size);

	STEP(x);

	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, length(R(x)));
}

void test_INNER_STEP_atom() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	R(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));

	STEP(x); 

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(R(x)));
	TEST_ASSERT_EQUAL_INT(13, A(S(x)));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, length(R(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(13, A(N(S(x))));
}

void test_INNER_STEP_list() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	R(x) =
		cons(x, 
			cons(x, 7, AS(ATM, 
			cons(x, 11, AS(ATM, 
			cons(x, 13, AS(ATM, 0)))))), 
		AS(LST, 0));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(x->ds));
	TEST_ASSERT_EQUAL_INT(0, length(R(x)));
	TEST_ASSERT_EQUAL_INT(1, length(x->rs));

	TEST_ASSERT_EQUAL(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(S(x))))));
}

void test_INNER_STEP_list_2() {
	//TEST_IGNORE();
	C size = 512;
	B block[size];
	X* x = init(block, size);

	R(x) =
		cons(x, 
			cons(x, 7, AS(ATM, 
			cons(x, 11, AS(ATM, 
			cons(x, 13, AS(ATM, 0)))))), 
		AS(LST, 
		cons(x, 17, AS(ATM, 0))));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(17, A(S(x)));
	TEST_ASSERT_EQUAL(LST, T(N(S(x))));
}

void test_INNER_STEP_primitive() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	R(x) =
		cons(x, 13, AS(ATM, 
		cons(x, 7, AS(ATM, 
		cons(x, (C)&add, AS(PRM, 
		cons(x, (C)&duplicate, AS(PRM, 0))))))));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(13, A(S(x)));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(13, A(N(S(x))));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(20, A(S(x)));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(20, A(S(x)));
	TEST_ASSERT_EQUAL_INT(20, A(N(S(x))));
}

void test_INNER_CALL_word() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C word = 
		cons(x, 
			cons(x, (C)"test_word", AS(ATM,
			cons(x, (C)&duplicate, AS(PRM, 
			cons(x, (C)&add, AS(PRM, 0)))))),
		AS(LST, 0));

	CALL(x, XT(word));	

	TEST_ASSERT_EQUAL_INT(2, length(x->rs));
	TEST_ASSERT_EQUAL_INT(2, length(R(x)));
	TEST_ASSERT_EQUAL_INT(PRM, T(R(x)));
	TEST_ASSERT_EQUAL_INT((C)&duplicate, A(R(x)));
	TEST_ASSERT_EQUAL_INT(PRM, T(N(R(x))));
	TEST_ASSERT_EQUAL_INT((C)&add, A(N(R(x))));
}

void test_INNER_STEP_word() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C word = 
		cons(x, 
			cons(x, (C)"test_word", AS(ATM,
			cons(x, (C)&duplicate, AS(PRM, 
			cons(x, (C)&add, AS(PRM, 0)))))),
		AS(LST, 0));

	R(x) = cons(x, word, AS(WRD, 0));

	S(x) = cons(x, 5, AS(ATM, 0));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(5, A(S(x)));
	TEST_ASSERT_EQUAL_INT(2, length(x->rs));
	TEST_ASSERT_EQUAL_INT(2, length(R(x)));
	TEST_ASSERT_EQUAL_INT(1, length(A(N(x->rs))));
	TEST_ASSERT_EQUAL_INT(WRD, T(A(N(x->rs))));
	TEST_ASSERT_EQUAL_INT(word, A(A(N(x->rs))));
	TEST_ASSERT_EQUAL_INT(PRM, T(R(x)));
	TEST_ASSERT_EQUAL_INT((C)&duplicate, A(R(x)));
	TEST_ASSERT_EQUAL_INT(PRM, T(N(R(x))));
	TEST_ASSERT_EQUAL_INT((C)&add, A(N(R(x))));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(5, A(S(x)));
	TEST_ASSERT_EQUAL_INT(5, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(2, length(x->rs));
	TEST_ASSERT_EQUAL_INT(1, length(R(x)));
	TEST_ASSERT_EQUAL_INT((C)&add, A(R(x)));
	TEST_ASSERT_EQUAL_INT(word, A(A(N(x->rs))));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(10, A(S(x)));
	TEST_ASSERT_EQUAL_INT(0, length(R(x)));
	TEST_ASSERT_EQUAL_INT(1, length(x->rs));
}

void test_INNER_STEP_words() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C dup_ = 
		cons(x,
			cons(x, (C)"dup", AS(ATM,
			cons(x, (C)&duplicate, AS(PRM, 0)))),
		AS(LST, 0));

	C add_ =
		cons(x,
			cons(x, (C)"+", AS(ATM,
			cons(x, (C)&add, AS(PRM, 0)))),
		AS(LST, 0));

	C double_ =
		cons(x,
			cons(x, (C)"double", AS(ATM,
			cons(x, dup_, AS(WRD,
			cons(x, add_, AS(WRD, 0)))))),
		AS(LST, 0));

	R(x) = cons(x, double_, AS(WRD, 0));

	S(x) = cons(x, 7, AS(ATM, 0));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(2, length(x->rs));
	TEST_ASSERT_EQUAL_INT(1, length(N(x->rs)));
	TEST_ASSERT_EQUAL_INT(double_, A(A(N(x->rs))));
	TEST_ASSERT_EQUAL_INT(2, length(R(x)));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(3, length(x->rs));
	TEST_ASSERT_EQUAL_INT(1, length(R(x)));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(2, length(x->rs));
	TEST_ASSERT_EQUAL_INT(1, length(R(x)));
	
	STEP(x);

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(3, length(x->rs));

	STEP(x);

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(14, A(S(x)));
	TEST_ASSERT_EQUAL_INT(0, length(R(x)));
	TEST_ASSERT_EQUAL_INT(1, length(x->rs));
}

void test_INNER_STEP_words_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C dup_ = 
		cons(x,
			cons(x, (C)"dup", AS(ATM,
			cons(x, (C)&duplicate, AS(PRM, 0)))),
		AS(LST, 0));

	C add_ =
		cons(x,
			cons(x, (C)"+", AS(ATM,
			cons(x, (C)&add, AS(PRM, 0)))),
		AS(LST, 0));

	C double_ =
		cons(x,
			cons(x, (C)"double", AS(ATM,
			cons(x, dup_, AS(WRD,
			cons(x, add_, AS(WRD, 0)))))),
		AS(LST, 0));

	R(x) = cons(x, double_, AS(WRD, 0));

	S(x) = cons(x, 7, AS(ATM, 0));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(14, A(S(x)));
	TEST_ASSERT_EQUAL_INT(0, length(R(x)));
	TEST_ASSERT_EQUAL_INT(1, length(x->rs));
}


// EXECUTION PRMITIVEs

void test_EXEC_exec_i() {
	//TEST_IGNORE();
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, (C)&duplicate, AS(PRM, cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))))));

	exec_i(x);

	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(13, A(N(N(S(x)))));
}

void test_EXEC_exec_i_2() {
	//TEST_IGNORE();
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, cons(x, (C)&duplicate, AS(PRM, 0)), AS(LST, cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))))));

	exec_i(x);

	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(13, A(N(N(S(x)))));
}

void test_EXEC_exec_i_3() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, (C)&duplicate, AS(PRM, cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))))));
	R(x) = cons(x, (C)&exec_i, AS(PRM, 0));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(13, A(N(N(S(x)))));
}

void test_EXEC_exec_i_4() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, cons(x, (C)&duplicate, AS(PRM, 0)), AS(LST, cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))))));
	R(x) = cons(x, (C)&exec_i, AS(PRM, 0));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(13, A(N(N(S(x)))));
}

void test_EXEC_branch() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	R(x) = 
		cons(x, 1, AS(ATM,
		cons(x, cons(x, 7, AS(ATM, 0)), AS(LST,
		cons(x, cons(x, 11, AS(ATM, 0)), AS(LST,
		cons(x, (C)&branch, AS(PRM, 0))))))));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(x->rs));
	TEST_ASSERT_EQUAL_INT(0, length(R(x)));
}

void test_EXEC_branch_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	R(x) = 
		cons(x, 0, AS(ATM,
		cons(x, cons(x, 7, AS(ATM, 0)), AS(LST,
		cons(x, cons(x, 11, AS(ATM, 0)), AS(LST,
		cons(x, (C)&branch, AS(PRM, 0))))))));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(x->rs));
	TEST_ASSERT_EQUAL_INT(0, length(R(x)));
}

void test_EXEC_branch_3() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	R(x) =
		cons(x, 1, AS(ATM,
		cons(x, cons(x, 7, AS(ATM, 0)), AS(LST, 
		cons(x, cons(x, 11, AS(ATM, 0)), AS(LST,
		cons(x, (C)&branch, AS(PRM,
		cons(x, 13, AS(ATM, 0))))))))));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));

	S(x) = 0;

	R(x) =
		cons(x, 0, AS(ATM,
		cons(x, cons(x, 7, AS(ATM, 0)), AS(LST, 
		cons(x, cons(x, 11, AS(ATM, 0)), AS(LST, 
		cons(x, (C)&branch, AS(PRM, 0))))))));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
	TEST_ASSERT_EQUAL_INT(11, A(S(x)));

}

//void test_EXEC_zjump() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	C dest = cons(x, 7, AS(ATM, 0));
//	C src = 
//		cons(x, (C)&zjump, AS(PRM, 
//		cons(x, dest, AS(ATM, 
//		cons(x, 13, AS(ATM, 0))))));
//
//	S(x) = cons(x, 1, AS(ATM, 0));
//
//	inner(x, src);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
//
//	S(x) = cons(x, 0, AS(ATM, 0));
//
//	inner(x, src);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//}
//
//void test_EXEC_jump() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	C dest = cons(x, 7, AS(ATM, 0));
//	C src = cons(x, (C)&jump, AS(PRM, cons(x, dest, AS(ATM, 0))));
//
//	inner(x, src);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//}

void test_EXEC_exec_x() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, (C)&duplicate, AS(PRM, cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))))));

	exec_x(x);

	TEST_ASSERT_EQUAL_INT(4, length(S(x)));
	TEST_ASSERT_EQUAL_INT(PRM, T(S(x)));
	TEST_ASSERT_EQUAL_INT((C)&duplicate, A(S(x)));
	TEST_ASSERT_EQUAL_INT(PRM, T(N(S(x))));
	TEST_ASSERT_EQUAL_INT((C)&duplicate, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(N(S(x)))));
	TEST_ASSERT_EQUAL_INT(7, A(N(N(S(x)))));
	TEST_ASSERT_EQUAL_INT(ATM, T(N(N(N(S(x))))));
	TEST_ASSERT_EQUAL_INT(11, A(N(N(N(S(x))))));
}

void test_EXEC_exec_x_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, (C)&duplicate, AS(PRM, cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))))));
	R(x) = cons(x, (C)&exec_x, AS(PRM, 0));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(4, length(S(x)));
	TEST_ASSERT_EQUAL_INT(PRM, T(S(x)));
	TEST_ASSERT_EQUAL_INT((C)&duplicate, A(S(x)));
	TEST_ASSERT_EQUAL_INT(PRM, T(N(S(x))));
	TEST_ASSERT_EQUAL_INT((C)&duplicate, A(N(S(x))));
}

void test_EXEC_exec_x_3() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	S(x) = cons(x, cons(x, (C)&duplicate, AS(PRM, 0)), AS(LST, cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))))));
	R(x) = cons(x, (C)&exec_x, AS(PRM, 0));

	while(STEP(x));

	TEST_ASSERT_EQUAL_INT(4, length(S(x)));
	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
	TEST_ASSERT_EQUAL_INT(1, length(A(S(x))));
	TEST_ASSERT_EQUAL_INT((C)&duplicate, A(A(S(x))));
	TEST_ASSERT_EQUAL_INT(LST, T(N(S(x))));
	TEST_ASSERT_EQUAL_INT(1, length(A(N(S(x)))));
	TEST_ASSERT_EQUAL_INT((C)&duplicate, A(A(N(S(x)))));
}

// PARSING

void test_PARSING_parse() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	x->tb = "  parsing \""; x->il = strlen(x->tb);
	S(x) = cons(x, '"', AS(ATM, 0));

	parse(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(10, A(S(x)));
	TEST_ASSERT_EQUAL_MEMORY("  parsing ", (B*)(A(N(S(x)))), A(S(x)));
}

void test_PARSING_parse_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	x->tb = ""; x->il = 0;
	S(x) = cons(x, '"', AS(ATM, 0));

	parse(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)x->tb, A(N(S(x))));
}

void test_PARSING_parse_3() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	x->tb = "\""; x->il = strlen(x->tb);
	S(x) = cons(x, '"', AS(ATM, 0));

	parse(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)x->tb, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(1, x->in);
}

void test_PARSING_parse_4() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	x->tb = 0;
	S(x) = cons(x, '"', AS(ATM, 0));

	parse(x);

	TEST_ASSERT_EQUAL_INT(E_EIB, x->err);
}

void test_PARSING_parse_name() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	x->tb = 0;
	
	parse_name(x);

	TEST_ASSERT_EQUAL_INT(E_EIB, x->err);
}

void test_PARSING_parse_name_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	x->tb = "test"; x->il = strlen(x->tb);
	
	parse_name(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(4, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)x->tb, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(4, x->in);
}

void test_PARSING_parse_name_3() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	x->tb = ""; x->il = 0;
	
	parse_name(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)x->tb, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(0, x->in);
}

void test_PARSING_parse_name_4() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	x->tb = ": double dup + ;"; x->il = strlen(x->tb);
	
	parse_name(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)x->tb, A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(1, x->in);
	TEST_ASSERT_EQUAL_MEMORY(":", A(N(S(x))), A(S(x)));

	parse_name(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(4, length(S(x)));
	TEST_ASSERT_EQUAL_INT(6, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)(x->tb + 2), A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(8, x->in);
	TEST_ASSERT_EQUAL_MEMORY("double", A(N(S(x))), A(S(x)));

	parse_name(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(6, length(S(x)));
	TEST_ASSERT_EQUAL_INT(3, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)(x->tb + 9), A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(12, x->in);
	TEST_ASSERT_EQUAL_MEMORY("dup", A(N(S(x))), A(S(x)));

	parse_name(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(8, length(S(x)));
	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)(x->tb + 13), A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(14, x->in);
	TEST_ASSERT_EQUAL_MEMORY("+", A(N(S(x))), A(S(x)));

	parse_name(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(10, length(S(x)));
	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)(x->tb + 15), A(N(S(x))));
	TEST_ASSERT_EQUAL_INT(16, x->in);
	TEST_ASSERT_EQUAL_MEMORY(";", A(N(S(x))), A(S(x)));

	parse_name(x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(12, length(S(x)));
	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
	TEST_ASSERT_EQUAL_INT((C)(x->tb + 16), A(N(S(x))));
}

// CONTIGUOUS MEMORY PRIMITIVES

void test_MEM_grow() {
	C size = 4096;
	B block[size];
	X* x = init(block, size);

	// Ensure that RESERVED is 0 before the test
	x->hr = (B*)x->th;

	grow(x);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, x->free);
	TEST_ASSERT_EQUAL_INT(sP, RESERVED(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);

	grow(x);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, x->free);
	TEST_ASSERT_EQUAL_INT(2*sP, RESERVED(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);

	while(x->free) { grow(x); }

	grow(x);

	TEST_ASSERT_EQUAL_INT(E_NEM, x->err);
}

void test_MEM_shrink() {
	C size = 4096;
	B block[size];
	X* x = init(block, size);

	// Ensure that RESERVED is 0 before the test
	x->hr = (B*)x->th;

	grow(x);
	grow(x);
	shrink(x);

	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, x->free);
	TEST_ASSERT_EQUAL_INT(sP, RESERVED(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);

	shrink(x);

	TEST_ASSERT_EQUAL_INT(f_nodes(x), x->free);
	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));
	TEST_ASSERT_EQUAL_INT(0, x->err);

	shrink(x);

	TEST_ASSERT_EQUAL_INT(E_NER, x->err);
}

void test_MEM_allot() {
	C size = 1024;
	B block[size];
	X* x = init(block, size);

	B* here = x->hr;
	S(x) = cons(x, 0, AS(ATM, S(x)));
	allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(here, x->hr);

	S(x) = cons(x, 13, AS(ATM, S(x)));
	allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(here + 13, x->hr);

	C fnodes = x->free;
	here = x->hr;
	C reserved = RESERVED(x);

	// Ensure reserved memory is 0 to allow next tests to pass
	S(x) = cons(x, RESERVED(x), AS(ATM, S(x)));
	allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here + reserved, x->hr);
	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));
	TEST_ASSERT_EQUAL_INT(fnodes, x->free);

	here = x->hr;

	S(x) = cons(x, 4*sP, AS(ATM, S(x)));
	allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here + 4*sP, x->hr);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, x->free);
	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));

	here = x->hr;

	S(x) = cons(x, 2*sP - 3, AS(ATM, S(x)));
	allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here + 2*sP - 3, x->hr);
	TEST_ASSERT_EQUAL_INT(fnodes - 6, x->free);
	TEST_ASSERT_EQUAL_INT(3, RESERVED(x));

	S(x) = cons(x, -(2*sP - 3), AS(ATM, S(x)));
	allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here, x->hr);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, x->free);
	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));

	S(x) = cons(x, -1, AS(ATM, S(x)));
	allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here - 1, x->hr);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, x->free);
	TEST_ASSERT_EQUAL_INT(1, RESERVED(x));

	here = x->hr;
	reserved = RESERVED(x);
	fnodes = x->free;

	S(x) = cons(x, 2048, AS(ATM, S(x)));
	allot(x);
	TEST_ASSERT_EQUAL_INT(E_NEM, x->err);
	TEST_ASSERT_EQUAL_PTR(here, x->hr);
	TEST_ASSERT_EQUAL_INT(reserved, RESERVED(x));
	TEST_ASSERT_EQUAL_INT(fnodes, x->free);

	x->err = 0;
	S(x) = cons(x, -2048, AS(ATM, S(x)));
	allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(BOTTOM(x), x->hr);
	TEST_ASSERT_EQUAL_INT((C)ALIGN(x->hr, sP), x->th);
	TEST_ASSERT_EQUAL_INT(f_nodes(x), x->free);
}

// DICTIONARY

void test_DICT_find_name() {
	C size = 4096;
	B block[size];
	X* x = init(block, size);

	x->lt =
		cons(x, cons(x, (C)"test2", AS(ATM, 0)), AS(DEF,
		cons(x, cons(x, (C)"join", AS(ATM, 0)), AS(NDCS_DEF,
		cons(x, cons(x, (C)"dup", AS(ATM, cons(x, (C)&duplicate, AS(PRM, 0)))), AS(PRIM,
		cons(x, cons(x, (C)"test", AS(ATM, cons(x, (C)&add, AS(PRM, 0)))), AS(NDCS_PRIM, 0))))))));

	x->tb = "";
	
	parse_name(x);
	find_name(x);
	C i = A(S(x));
	C w = A(N(S(x)));

	TEST_ASSERT_EQUAL_INT(0, i);
	TEST_ASSERT_EQUAL_INT(E_ZLN, x->err);

	x->err = 0;
	x->tb = "dup"; x->il = strlen(x->tb);
	x->in = 0;

	parse_name(x);
	find_name(x);
	w = A(S(x));

	TEST_ASSERT_NOT_EQUAL_INT(0, w);
	TEST_ASSERT_EQUAL_INT(N(N(x->lt)), w);

	x->tb = "   join  "; x->il = strlen(x->tb);
	x->in = 0;

	parse_name(x);
	find_name(x);
	w = A(S(x));

	TEST_ASSERT_NOT_EQUAL_INT(0, w);
	TEST_ASSERT_EQUAL_INT(N(x->lt), w);

	x->tb = "test"; x->il = strlen(x->tb);
	x->in = 0;

	parse_name(x);
	find_name(x);
	w = A(S(x));

	TEST_ASSERT_NOT_EQUAL_INT(0, w);
	TEST_ASSERT_EQUAL_STRING("test", (B*)NFA(w));
	TEST_ASSERT_EQUAL_INT(N(N(N(x->lt))), w);
}

//void test_PARSING_parse_token() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//	B* str = "   test  ";
//
//	x->tb = str;
//	x->token = 0;
//	x->in = 0;
//
//	parse_token(x);
//
//	TEST_ASSERT_EQUAL_INT(4, x->in - x->token);
//	TEST_ASSERT_EQUAL_INT((C)str + 3, x->tb + x->token);
//
//	parse_token(x);
//
//	TEST_ASSERT_EQUAL_INT(0, x->in - x->token);
//	TEST_ASSERT_EQUAL_INT((C)str + 9, x->tb + x->token);
//
//	B* str2 = "";
//	x->tb = str2;
//	x->token = 0;
//	x->in = 0;
//
//	parse_token(x);
//
//	TEST_ASSERT_EQUAL_INT(0, x->in - x->token);
//	TEST_ASSERT_EQUAL_INT((C)str2, x->tb + x->token);
//
//	B* str3 = ": name    word1 ;";
//	x->tb = str3;
//	x->token = 0;
//	x->in = 0;
//
//	parse_token(x);
//
//	TEST_ASSERT_EQUAL_INT(1, x->in - x->token);
//	TEST_ASSERT(!strncmp(":", x->tb + x->token, x->in - x->token));
//	TEST_ASSERT_EQUAL_INT((C)str3, x->tb + x->token);
//
//	parse_token(x);
//
//	TEST_ASSERT_EQUAL_INT(4, x->in - x->token);
//	TEST_ASSERT(!strncmp("name", x->tb + x->token, x->in - x->token));
//	TEST_ASSERT_EQUAL_INT((C)str3 + 2, x->tb + x->token);
//
//	parse_token(x);
//
//	TEST_ASSERT_EQUAL_INT(5, x->in - x->token);
//	TEST_ASSERT(!strncmp("word1", x->tb + x->token, x->in - x->token));
//	TEST_ASSERT_EQUAL_INT((C)str3 + 10, x->tb + x->token);
//
//	parse_token(x);
//
//	TEST_ASSERT_EQUAL_INT(1, x->in - x->token);
//	TEST_ASSERT(!strncmp(";", x->tb + x->token, x->in - x->token));
//	TEST_ASSERT_EQUAL_INT((C)str3 + 16, x->tb + x->token);
//
//	parse_token(x);
//
//	TEST_ASSERT_EQUAL_INT(0, x->in - x->token);
//	TEST_ASSERT_EQUAL_INT((C)str3 + 17, x->tb + x->token);
//
//	parse_token(x);
//
//	TEST_ASSERT_EQUAL_INT(0, x->in - x->token);
//	TEST_ASSERT_EQUAL_INT((C)str3 + 17, x->tb + x->token);
//}
//
//void test_PARSING_find_name() {
//	C size = 4096;
//	B block[size];
//	X* x = init(block, size);
//
//	x->latest =	
//		cons(x, cons(x, (C)"test2", AS(ATM, 0)), AS(LST,
//		cons(x, cons(x, (C)"dup", AS(ATM, 0)), AS(WRD,
//		cons(x, cons(x, (C)"join", AS(ATM, 0)), AS(LST,
//		cons(x, cons(x, (C)"test", AS(ATM, 0)), AS(LST, 0))))))));
//
//	x->tb = "dup";
//	x->token = 0;
//	x->in = 3;
//
//	C word = find_name(x);
//
//	TEST_ASSERT_NOT_EQUAL_INT(0, word);
//	TEST_ASSERT_EQUAL_INT(1, IMMEDIATE(word));
//	TEST_ASSERT_EQUAL_INT(N(x->latest), word);
//
//	x->tb = "   join  ";
//	x->token = 0;
//	x->in = 0;
//
//	parse_token(x);
//	word = find_name(x);
//
//	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(word));
//	TEST_ASSERT_EQUAL_INT(N(N(x->latest)), word);
//
//	x->tb = "test";
//	x->token = 0;
//	x->in = 0;
//
//	parse_token(x);
//	word = find_name(x);
//
//	TEST_ASSERT_EQUAL_STRING("test", (B*)NFA(word));
//	TEST_ASSERT_EQUAL_INT(N(N(N(x->latest))), word);
//}
//
//void test_COMPILATION_compile_word() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	C word1 =
//		cons(x,
//			cons(x, (C)"word1", AS(ATM,
//			cons(x, 0, AS(PRM, 0)))),
//		AS(ATM, 0));
//		
//	C word1_imm =
//		cons(x,
//			cons(x, (C)"word1_imm", AS(ATM,
//			cons(x, 0, AS(PRM, 0)))),
//		AS(PRM, 0));
//
//	C word2 =
//		cons(x,
//			cons(x, (C)"word2", AS(ATM,
//			cons(x, 7, AS(ATM, 0)))),
//		AS(LST, 0));
//
//	C word2_imm =
//		cons(x,
//			cons(x, (C)"word2_imm", AS(ATM,
//			cons(x, 7, AS(ATM, 0)))),
//		AS(WRD, 0));
//
//	C compilation = compile_word(x, word1);
//
//	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(C(x)));
//	TEST_ASSERT_EQUAL_INT(PRM, T(C(x)));
//	TEST_ASSERT_EQUAL_INT(0, A(C(x)));
//
//	compilation = compile_word(x, word1_imm);
//
//	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(2, length(C(x)));
//	TEST_ASSERT_EQUAL_INT(PRM, T(C(x)));
//	TEST_ASSERT_EQUAL_INT(0, A(C(x)));
//
//	compilation = compile_word(x, word2);
//
//	TEST_ASSERT_EQUAL_INT(3, length(C(x)));
//	TEST_ASSERT_EQUAL_INT(WRD, T(C(x)));
//	TEST_ASSERT_EQUAL_INT(word2, A(C(x)));
//
//	compilation = compile_word(x, word2_imm);
//
//	TEST_ASSERT_EQUAL_INT(4, length(C(x)));
//	TEST_ASSERT_EQUAL_INT(WRD, T(C(x)));
//	TEST_ASSERT_EQUAL_INT(word2_imm, A(C(x)));
//}
//
//void test_OUTER_evaluate_numbers() {
//	C size = 4096;
//	B block[size];
//	X* x = init(block, size);
//
//	evaluate(x, "   7 11    13  ");
//
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(N(S(x)))));
//}
//
//void test_OUTER_evaluate_words() {
//	C size = 4096;
//	B block[size];
//	X* x = init(block, size);
//
//	x->latest = 
//		cons(x,
//			cons(x, (C)"test", AS(ATM,
//			cons(x, 7, AS(ATM,
//			cons(x, 11, AS(ATM,
//			cons(x, 13, AS(ATM, 0)))))))),
//		AS(LST, 0));
//
//	evaluate(x, " test   ");
//
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(N(S(x)))));
//}
//
//void test_PRMITIVES_zjump() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C dest = cons(x, 7, AS(ATM, 0));
//
//	C code = 
//		cons(x, 1, AS(ATM,
//		cons(x, 0, AS(PRM,
//		cons(x, dest, AS(ATM, 0))))));
//
//	execute(x, code);
//
//	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//
//	code =
//		cons(x, 0, AS(ATM,
//		cons(x, 0, AS(PRM,
//		cons(x, dest, AS(ATM, 0))))));
//
//	execute(x, code);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//}
//
//void test_PRMITIVES_jump() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C dest = cons(x, 7, AS(ATM, 0));
//
//	C code = 
//		cons(x, 1, AS(PRM,
//		cons(x, dest, AS(ATM, 0))));
//
//	execute(x, code);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//}
//
//void test_PRMITIVES_postpone() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C imm_word =
//		cons(x,
//			cons(x, (C)"imm-word", AS(ATM,
//			cons(x, 7, AS(PRM, 0)))),
//		AS(IMM_PRM, 0));
//
//	x->latest =
//		cons(x,
//			cons(x, (C)"postpone", AS(ATM,
//			cons(x, (C)&postpone, AS(PRM, 0)))),
//		AS(IMM_PRM, imm_word));
//
//	evaluate(x, "postpone imm-word");
//
//	TEST_ASSERT_EQUAL_INT(1, length(C(x)));
//	TEST_ASSERT_EQUAL_INT(PRM, T(C(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(C(x)));
//}
//
//void test_PRMITIVES_parse() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	x->latest = 
//		cons(x,
//			cons(x, (C)"parse", AS(ATM,
//			cons(x, (C)&parse, AS(PRM, 0)))),
//		AS(NON_IMM_PRM, 0));
//
//	evaluate(x, "39 parse just a string that ends on a single quote'");
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	C l = pop(x);
//	C a = pop(x);
//	TEST_ASSERT_EQUAL_MEMORY(" just a string that ends on a single quote", a, l);
//}
//
//void test_PRMITIVES_parse_name() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	x->latest = 
//		cons(x,
//			cons(x, (C)"parse-name", AS(ATM,
//			cons(x, (C)&parse_name, AS(PRM, 0)))),
//		AS(NON_IMM_PRM, 0));
//
//	evaluate(x, "parse-name");
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	C l = pop(x);
//	C a = pop(x);
//	TEST_ASSERT_EQUAL_INT(0, l);	
//
//	evaluate(x, "parse-name    my-name  ");
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	l = pop(x);
//	a = pop(x);
//	TEST_ASSERT_EQUAL_MEMORY("my-name", a, l);
//}
//
//void test_PRMITIVES_spush() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	PUSH(x, 13);
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	spush(x);
//
//	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(2, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(3, length(A(N(x->pile))));
//	TEST_ASSERT_EQUAL_INT(7, A(A(N(x->pile))));
//	TEST_ASSERT_EQUAL_INT(11, A(N(A(N(x->pile)))));
//	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(N(x->pile))))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 4, FREE(x));
//}
//
//void test_PRMITIVES_clear() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	PUSH(x, 13);
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	clear(x);
//
//	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));
//}
//
//void test_PRMITIVES_sdrop() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	PUSH(x, 13);
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	spush(x);
//
//	PUSH(x, 19);
//	PUSH(x, 17);
//
//	sdrop(x);
//
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(N(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
//}
//
//void test_PRMITIVES_stack_to_list() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	spush(x);
//
//	PUSH(x, 17);
//	PUSH(x, 13);
//
//	stack_to_list(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
//	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(17, A(N(A(S(x)))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(11, A(N(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, FREE(x));
//
//	pop(x);
//
//	stack_to_list(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
//	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(11, A(N(A(S(x)))));
//}
//
//void test_PRMITIVES_list_to_stack() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	spush(x);
//
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	stack_to_list(x);
//
//	list_to_stack(x);
//
//	TEST_ASSERT_EQUAL_INT(2, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(0, A(N(x->pile)));
//}
//
//void test_PRMITIVES_list_to_stack_2() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	list_to_stack(x);
//
//	TEST_ASSERT_EQUAL_INT(2, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(0, A(N(x->pile)));
//}
//
//void test_PRMITIVES_brackets() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	lbracket(x);	
//
//	TEST_ASSERT_EQUAL_INT(0, x->state);
//
//	rbracket(x);
//
//	TEST_ASSERT_EQUAL_INT(1, x->state);
//
//	lbracket(x);
//
//	TEST_ASSERT_EQUAL_INT(0, x->state);
//}
//
//void test_PRMITIVES_reverse_stack() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	reverse_stack(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//}
//
//void test_PRMITIVES_reverse_stack_2() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	spush(x);
//
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	reverse_stack(x);
//
//	TEST_ASSERT_EQUAL_INT(2, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(0, length(A(N(x->pile))));	
//}
//
//void test_PRMITIVES_braces() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	lbrace(x);
//
//	compile_number(x, 1);
//	compile_number(x, 2);
//	compile_number(x, 3);
//
//	rbrace(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(0, x->cpile);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
//	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(1, A(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(2, A(N(A(S(x)))));
//	TEST_ASSERT_EQUAL_INT(3, A(N(N(A(S(x))))));
//}
//
//void test_PRMITIVES_duplicate_atom() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 7, AS(ATM, S(x)));
//
//	duplicate(x);
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
//	TEST_ASSERT_EQUAL_INT(ATM, T(S(x)));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//
//	S(x) = cons(x, 11, AS(ATM, S(x)));
//
//	duplicate(x);
//
//	TEST_ASSERT_EQUAL_INT(4, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 4, FREE(x));
//	TEST_ASSERT_EQUAL_INT(ATM, T(S(x)));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(N(N(S(x))))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(N(N(S(x))))));
//}
//
//void test_PRMITIVES_duplicate_list() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = 
//		cons(x, 
//			cons(x, 17, AS(ATM,
//			cons(x, 13, AS(ATM,
//			cons(x, 
//				cons(x, 11, AS(ATM,
//				cons(x, 7, AS(ATM, 0)))),
//				AS(LST, 0)))))),
//			AS(LST, S(x)));
//
//	// (17, 13, (11, 7))
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 6, FREE(x));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
//	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(17, A(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(S(x)))));
//	TEST_ASSERT_EQUAL_INT(13, A(N(A(S(x)))));
//	TEST_ASSERT_EQUAL_INT(LST, T(N(N(A(S(x))))));
//	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(S(x)))))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(N(A(S(x)))))));
//	TEST_ASSERT_EQUAL_INT(11, A(A(N(N(A(S(x)))))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(N(A(S(x))))))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(N(A(S(x))))))));
//
//	duplicate(x);
//
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 12, FREE(x));
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//
//	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
//	TEST_ASSERT_EQUAL_INT(3, length(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(17, A(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(S(x)))));
//	TEST_ASSERT_EQUAL_INT(13, A(N(A(S(x)))));
//	TEST_ASSERT_EQUAL_INT(LST, T(N(N(A(S(x))))));
//	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(S(x)))))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(N(A(S(x)))))));
//	TEST_ASSERT_EQUAL_INT(11, A(A(N(N(A(S(x)))))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(N(A(S(x))))))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(N(A(S(x))))))));
//
//	TEST_ASSERT_EQUAL_INT(LST, T(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(3, length(A(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(17, A(A(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(S(x))))));
//	TEST_ASSERT_EQUAL_INT(13, A(N(A(N(S(x))))));
//	TEST_ASSERT_EQUAL_INT(LST, T(N(N(A(N(S(x)))))));
//	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(N(S(x))))))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(A(N(N(A(N(S(x))))))));
//	TEST_ASSERT_EQUAL_INT(11, A(A(N(N(A(N(S(x))))))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(A(N(N(A(N(S(x)))))))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(N(A(N(S(x)))))))));
//
//	TEST_ASSERT_NOT_EQUAL_INT(S(x), N(S(x)));
//	TEST_ASSERT_NOT_EQUAL_INT(A(S(x)), A(N(S(x))));
//	TEST_ASSERT_NOT_EQUAL_INT(N(A(S(x))), N(A(N(S(x)))));
//	TEST_ASSERT_NOT_EQUAL_INT(N(N(A(S(x)))), N(N(A(N(S(x))))));
//	TEST_ASSERT_NOT_EQUAL_INT(A(N(N(A(S(x))))), A(N(N(A(N(S(x)))))));
//	TEST_ASSERT_NOT_EQUAL_INT(N(A(N(N(A(S(x)))))), N(A(N(N(A(N(S(x))))))));
//}
//
//void test_PRMITIVES_swap() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	PUSH(x, 13);
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	swap(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(N(N(S(x)))));
//}
//
//void test_PRMITIVES_drop() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	PUSH(x, 11);
//	PUSH(x, 7);
//
//	drop(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//
//	drop(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//
//	drop(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(x->pile));
//	TEST_ASSERT_EQUAL_INT(0, length(S(x)));
//}
//
//void test_PRMITIVES_rot() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 3, AS(ATM, cons(x, 2, AS(ATM, cons(x, 1, AS(ATM, 0))))));
//
//	rot(x);
//
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(3, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(2, A(N(N(S(x)))));
//}
//
//void test_PRMITIVES_over() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
//
//	over(x);
//
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(11, A(N(N(S(x)))));
//
//	S(x) = 
//		cons(x, 7, AS(ATM, 
//		cons(x, 
//			cons(x, 11, AS(ATM,
//			cons(x, 13, AS(ATM, 0)))),
//		AS(LST, 0))));
//
//	over(x);
//
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
//	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(11, A(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(N(A(S(x)))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//	TEST_ASSERT_NOT_EQUAL_INT(N(N(S(x))), S(x));
//	TEST_ASSERT_NOT_EQUAL_INT(A(N(N(S(x)))), A(S(x)));
//	TEST_ASSERT_NOT_EQUAL_INT(N(A(N(N(S(x))))), N(A(S(x))));
//}
//
//void test_PRMITIVES_exec_atom() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 5, AS(ATM, 0));
//
//	exec(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(5, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
//}
//
//void test_PRMITIVES_exec_list() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C list =
//		cons(x, 7, AS(ATM,
//		cons(x, 11, AS(ATM,
//		cons(x, (C)&add, AS(PRM, 0))))));
//
//	S(x) = cons(x, list, AS(LST, 0));
//
//	exec(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(18, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
//}
//
//void test_PRMITIVES_exec_primitive() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) =
//		cons(x, (C)&swap, AS(PRM,
//		cons(x, 7, AS(ATM,
//		cons(x, 11, AS(ATM, 0))))));
//
//	exec(x);
//
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//}
//
//void test_PRMITIVES_exec_primitive2() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C code =
//		cons(x, (C)exec, AS(PRM,
//		cons(x, 13, AS(ATM, 0))));
//
//	S(x) =
//		cons(x, (C)&swap, AS(PRM,
//		cons(x, 7, AS(ATM,
//		cons(x, 11, AS(ATM, 0))))));
//
//	execute(x, code);
//
//	reclaim(x, reclaim(x, code));
//
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(N(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
//}
//
//void test_PRMITIVES_exec_word() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C __double__ =
//		cons(x,
//			cons(x, (C)"double", AS(ATM,
//			cons(x, (C)&duplicate, AS(PRM,
//			cons(x, (C)&add, AS(PRM, 0)))))),
//		AS(LST, 0));
//
//	S(x) = 
//		cons(x, (C)__double__, AS(WRD,
//		cons(x, 5, AS(ATM, 0))));
//
//	exec(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(10, A(S(x)));
//}
//
//void test_PRMITIVES_exec_word2() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C __double__ =
//		cons(x,
//			cons(x, (C)"double", AS(ATM,
//			cons(x, (C)&duplicate, AS(PRM,
//			cons(x, (C)&add, AS(PRM, 0)))))),
//		AS(LST, 0));
//
//	C code =
//		cons(x, (C)exec, AS(PRM,
//		cons(x, 7, AS(ATM, 0))));
//
//	S(x) = 
//		cons(x, (C)__double__, AS(WRD,
//		cons(x, 5, AS(ATM, 0))));
//
//	execute(x, code);
//
//	reclaim(x, __double__);
//	reclaim(x, reclaim(x, code));
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(10, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
//}
//
//void test_PRMITIVES_exec_x_atom() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 5, AS(ATM, 0));
//
//	exec_x(x);
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(5, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(5, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
//}
//
//void test_PRMITIVES_exec_x_list() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C list =
//		cons(x, 7, AS(ATM,
//		cons(x, 11, AS(ATM,
//		cons(x, (C)&add, AS(PRM, 0))))));
//
//	S(x) = cons(x, list, AS(LST, 0));
//
//	exec_x(x);
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(18, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(list, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, FREE(x));
//}
//
//void test_PRMITIVES_exec_x_primitive() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) =
//		cons(x, (C)&swap, AS(PRM,
//		cons(x, 7, AS(ATM,
//		cons(x, 11, AS(ATM, 0))))));
//
//	exec_x(x);
//
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//	TEST_ASSERT_EQUAL_INT((C)&swap, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(11, A(N(N(S(x)))));
//}
//
//void test_PRMITIVES_exec_x_primitive2() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C code =
//		cons(x, (C)exec_x, AS(PRM,
//		cons(x, 13, AS(ATM, 0))));
//
//	S(x) =
//		cons(x, (C)&swap, AS(PRM,
//		cons(x, 7, AS(ATM,
//		cons(x, 11, AS(ATM, 0))))));
//
//	execute(x, code);
//
//	reclaim(x, reclaim(x, code));
//
//	TEST_ASSERT_EQUAL_INT(4, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT((C)&swap, A(N(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(11, A(N(N(N(S(x))))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 4, FREE(x));
//}
//
//void test_PRMITIVES_exec_x_word() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C __double__ =
//		cons(x,
//			cons(x, (C)"double", AS(ATM,
//			cons(x, (C)&duplicate, AS(PRM,
//			cons(x, (C)&add, AS(PRM, 0)))))),
//		AS(LST, 0));
//
//	S(x) = 
//		cons(x, (C)__double__, AS(WRD,
//		cons(x, 5, AS(ATM, 0))));
//
//	exec_x(x);
//
//	TEST_ASSERT_EQUAL_INT(2, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(__double__ + __double__, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(5, A(N(S(x))));
//}
//
//void test_PRMITIVES_exec_x_word2() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C __double__ =
//		cons(x,
//			cons(x, (C)"double", AS(ATM,
//			cons(x, (C)&duplicate, AS(PRM,
//			cons(x, (C)&add, AS(PRM, 0)))))),
//		AS(LST, 0));
//
//	C code =
//		cons(x, (C)exec_x, AS(PRM,
//		cons(x, 7, AS(ATM, 0))));
//
//	S(x) = 
//		cons(x, (C)__double__, AS(WRD,
//		cons(x, 5, AS(ATM, 0))));
//
//	execute(x, code);
//
//	reclaim(x, __double__);
//	reclaim(x, reclaim(x, code));
//
//	TEST_ASSERT_EQUAL_INT(3, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(__double__ + __double__, A(N(S(x))));
//	TEST_ASSERT_EQUAL_INT(5, A(N(N(S(x)))));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
//}
//
//void test_PRMITIVES_branch() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = 
//		cons(x, cons(x, 11, AS(ATM, 0)), AS(LST,
//		cons(x, cons(x, 7, AS(ATM, 0)), AS(LST,
//		cons(x, 1, AS(ATM, 0))))));
//
//	branch(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//
//	S(x) = 
//		cons(x, cons(x, 11, AS(ATM, 0)), AS(LST,
//		cons(x, cons(x, 7, AS(ATM, 0)), AS(LST,
//		cons(x, 0, AS(ATM, 0))))));
//
//	branch(x);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//}
//
//void test_PRMITIVES_branch2() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C code = 
//		cons(x,	cons(x, 7, AS(ATM, 0)), AS(LST,
//		cons(x, cons(x, 11, AS(ATM, 0)), AS(LST,
//		cons(x, (C)&branch, AS(PRM, 0))))));
//
//	S(x) = cons(x, 1, AS(ATM, 0));
//
//	execute(x, code);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//
//	S(x) = cons(x, 0, AS(ATM, 0));
//
//	execute(x, code);
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(11, A(S(x)));
//}
//
//void test_PRMITIVES_ARITHMETIC_add() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	add(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(18, A(S(x)));
//}
//
//void test_PRMITIVES_ARITHMETIC_sub() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
//	sub(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(4, A(S(x)));
//}
//
//void test_PRMITIVES_ARITHMETIC_mul() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	mul(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(77, A(S(x)));
//}
//
//void test_PRMITIVES_ARITHMETIC_division() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 11, AS(ATM, cons(x, 77, AS(ATM, 0))));
//	division(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
//}
//
//void test_PRMITIVES_ARITHMETIC_mod() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
//	mod(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(4, A(S(x)));
//}
//
//void test_PRMITIVES_COMPARISON_gt() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
//	gt(x);
//	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//
//	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	gt(x);
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	gt(x);
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//}
//
//void test_PRMITIVES_COMPARISON_lt() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
//	lt(x);
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//
//	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	lt(x);
//	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	lt(x);
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//}
//
//void test_PRMITIVES_COMPARISON_eq() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 13, AS(ATM, 0))));
//	eq(x);
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//
//	S(x) = cons(x, 13, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	eq(x);
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	eq(x);
//	TEST_ASSERT_EQUAL_INT(1, A(S(x)));
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//}
//
//void test_PRMITIVES_BIT_and() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	and(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(3, A(S(x)));
//
//	S(x) = cons(x, 0, AS(ATM, cons(x, 0, AS(ATM, 0))));
//	and(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//
//	S(x) = cons(x, -1, AS(ATM, cons(x, 0, AS(ATM, 0))));
//	and(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//
//	S(x) = cons(x, 0, AS(ATM, cons(x, -1, AS(ATM, 0))));
//	and(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//
//	S(x) = cons(x, -1, AS(ATM, cons(x, -1, AS(ATM, 0))));	
//	and(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
//}
//
//void test_PRMITIVES_BIT_or() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, 0))));
//	or(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(15, A(S(x)));
//
//	S(x) = cons(x, 0, AS(ATM, cons(x, 0, AS(ATM, 0))));
//	or(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//
//	S(x) = cons(x, -1, AS(ATM, cons(x, 0, AS(ATM, 0))));
//	or(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
//
//	S(x) = cons(x, 0, AS(ATM, cons(x, -1, AS(ATM, 0))));
//	or(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
//
//	S(x) = cons(x, -1, AS(ATM, cons(x, -1, AS(ATM, 0))));
//	or(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
//}
//
//void test_PRMITIVES_BIT_invert() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	S(x) = cons(x, 7, AS(ATM, 0));
//	invert(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(-8, A(S(x)));
//
//	S(x) = cons(x, 0, AS(ATM, 0));
//	invert(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(-1, A(S(x)));
//
//	S(x) = cons(x, 1, AS(ATM, 0));
//	invert(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(-2, A(S(x)));
//
//	S(x) = cons(x, -1, AS(ATM, 0));
//	invert(x);
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(0, A(S(x)));
//}

//// STACK PRMITIVES
//
//void test_STACK_swap_1() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	x->stack = cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0))));
//	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
//	TEST_ASSERT_EQUAL_INT(11, A(x->stack));
//	TEST_ASSERT_EQUAL_INT(7, A(N(x->stack)));
//	swap(x);
//	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
//	TEST_ASSERT_EQUAL_INT(7, A(x->stack));
//	TEST_ASSERT_EQUAL_INT(11, A(N(x->stack)));
//}
//
//void test_STACK_swap_2() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	x->stack = cons(x, 11, AS(ATM, cons(x, cons(x, 7, AS(ATM, cons(x, 5, AS(ATM, 0)))), AS(LST, 0))));
//	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
//	TEST_ASSERT_EQUAL_INT(ATM, T(x->stack));
//	TEST_ASSERT_EQUAL_INT(11, A(x->stack));
//	TEST_ASSERT_EQUAL_INT(LST, T(N(x->stack)));
//	TEST_ASSERT_EQUAL_INT(2, length(A(N(x->stack))));
//	TEST_ASSERT_EQUAL_INT(7, A(A(N(x->stack))));
//	TEST_ASSERT_EQUAL_INT(5, A(N(A(N(x->stack)))));
//	swap(x);
//	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
//	TEST_ASSERT_EQUAL_INT(LST, T(x->stack));
//	TEST_ASSERT_EQUAL_INT(2, length(A(x->stack)));
//	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
//	TEST_ASSERT_EQUAL_INT(5, A(N(A(x->stack))));
//	TEST_ASSERT_EQUAL_INT(ATM, T(N(x->stack)));
//	TEST_ASSERT_EQUAL_INT(11, A(N(x->stack)));
//}
//
//void test_STACK_drop() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	x->stack = cons(x, 7, AS(ATM, x->stack));
//
//	drop(x);
//
//	TEST_ASSERT_EQUAL_INT(0, length(x->stack));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));
//
//	x->stack = 
//		cons(x,
//			cons(x, 7, AS(ATM,
//			cons(x,
//				cons(x, 11, AS(ATM,
//				cons(x, 13, AS(ATM, 0)))),
//			AS(LST, 0)))),
//		AS(LST, x->stack));
//
//	drop(x);
//
//	TEST_ASSERT_EQUAL_INT(0, length(x->stack));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));
//}
//
//void test_STACK_rot() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	x->stack = cons(x, 7, AS(ATM, cons(x, 11, AS(ATM, cons(x, 13, AS(ATM, 0))))));
//
//	rot(x);
//
//	TEST_ASSERT_EQUAL_INT(3, length(x->stack));
//	TEST_ASSERT_EQUAL_INT(13, A(x->stack));
//	TEST_ASSERT_EQUAL_INT(7, A(N(x->stack)));
//	TEST_ASSERT_EQUAL_INT(11, A(N(N(x->stack))));
//}
//
//// ARITHMETIC PRMITIVES
//
//// COMPARISON PRMITIVES
//
//// BIT PRMITIVES
//
//// PARSING
//
//// OUTER INTERPRETER
//
//// CONTIGUOUS MEMORY
//
//void test_MEM_compile_str() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	C result;
//
//	x->tb = " test string\"  ";
//	x->in = 0;
//	result = compile_str(x);
//
//	TEST_ASSERT_EQUAL_INT(0, result);
//
//	B* addr = (B*)A(x->stack);
//
//	TEST_ASSERT_EQUAL_STRING("test string", addr);
//	TEST_ASSERT_EQUAL_INT(11, *((C*)(addr - sC)));
//}
//
//////  LST FUNCTIONS
//
//void test_append() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	C dest = 0;
//	C result = 0;
//
//	result = append(x);
//
//	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, result);
//
//	x->stack = cons(x, (C)&dest, AS(ATM, x->stack));
//	x->stack = cons(x, 7, AS(ATM, x->stack));
//
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
//
//	result = append(x);
//
//	TEST_ASSERT_EQUAL_INT(0, result);
//	TEST_ASSERT_EQUAL_INT(1, length(dest));
//	TEST_ASSERT_EQUAL_INT(7, A(dest));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
//
//	x->stack = reclaim(x, x->stack);
//	dest = reclaim(x, dest);
//
//	TEST_ASSERT_EQUAL_INT(0, length(x->stack));
//	TEST_ASSERT_EQUAL_INT(f_nodes(x), FREE(x));
//
//	x->stack = cons(x, cons(x, 7, AS(ATM, 0)), AS(LST, x->stack));
//	x->stack = cons(x, 11, AS(ATM, x->stack));
//
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
//
//	result = append(x);
//
//	TEST_ASSERT_EQUAL_INT(0, result);
//	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
//	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
//	TEST_ASSERT_EQUAL_INT(LST, T(x->stack));
//	TEST_ASSERT_EQUAL_INT(2, length(A(x->stack)));
//}
////void test_depth() {
////	C p1 = (C)malloc(sC * 2);
////	C p2 = (C)malloc(sC * 2);
////	C p3 = (C)malloc(sC * 2);
////	C p4 = (C)malloc(sC * 2);
////	C p5 = (C)malloc(sC * 2);
////
////	D(p1) = AS(ATM, p2);
////	D(p2) = AS(LST, p3);
////	A(p2) = p4;
////	D(p3) = AS(PRM, 0);
////	D(p4) = p5;
////	D(p5) = 0;
////
////	TEST_ASSERT_EQUAL_INT(3, length(p1));
////	TEST_ASSERT_EQUAL_INT(2, length(A(p2)));
////	TEST_ASSERT_EQUAL_INT(5, depth(p1));
////	TEST_ASSERT_EQUAL_INT(4, depth(p2));
////	TEST_ASSERT_EQUAL_INT(2, depth(p4));
////	TEST_ASSERT_EQUAL_INT(1, depth(p3));
////	TEST_ASSERT_EQUAL_INT(1, depth(p5));
////	TEST_ASSERT_EQUAL_INT(0, depth(0));
////}
////
//// CONTIGUOUS MEMORY
//
////void test_align() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	// This ensures here will be aligned with a p 
////	C result = allot(x, RESERVED(x));
////
////	// And this ensures that it will not be aligned
////	result = allot(x, 1);
////	TEST_ASSERT_NOT_EQUAL_INT(ALIGN(x->here, sC), x->here);
////	TEST_ASSERT_EQUAL_INT(0, result);
////
////	result = align(x);
////	TEST_ASSERT_EQUAL_INT(ALIGN(x->here, sC), x->here);
////	TEST_ASSERT_EQUAL_INT(0, result);
////}
////
////// WRD DEFINITIONS
////
////void test_header() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	B* here = x->here;
////
////	C h = header(x, "test");
////
////	TEST_ASSERT_EQUAL_STRING("test", NFA(h));
////	TEST_ASSERT_EQUAL_PTR(x->here, DFA(h));
////	TEST_ASSERT_EQUAL_INT(0, XT(h));
////	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(h));
////}
////
////void test_body() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C h = header(x, "test");
////
////	C w = body(h, cons(x, 11, AS(ATM, cons(x, 7, AS(ATM, 0)))));
////
////	execute(x, XT(w));
////
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(7, A(x->stack));
////	TEST_ASSERT_EQUAL_INT(11, A(N(x->stack)));
////}
////
////void test_reveal() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C h = header(x, "test1");
////	C h2 = header(x, "test2");
////
////	C d = reveal(x, h);
////
////	TEST_ASSERT_EQUAL_INT(h, d);
////	TEST_ASSERT_EQUAL_INT(h, x->latest);
////	TEST_ASSERT_EQUAL_INT(1, length(x->latest));
////
////	d = reveal(x, h2);
////
////	TEST_ASSERT_EQUAL_INT(h2, d);
////	TEST_ASSERT_EQUAL_INT(h2, x->latest);
////	TEST_ASSERT_EQUAL_INT(2, length(x->latest));
////	TEST_ASSERT_EQUAL_INT(h, N(x->latest));
////}
////
////void test_immediate() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C h = reveal(x, header(x, "test1"));
////
////	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(x->latest));
////
////	immediate(h);
////
////	TEST_ASSERT_EQUAL_INT(1, IMMEDIATE(x->latest));
////}
//
////void test_mlength() {
////	C p1 = (C)malloc(sC * 2);
////	C p2 = (C)malloc(sC * 2);
////	C p3 = (C)malloc(sC * 2);
////
////	D(p1) = p2;
////	D(p2) = p3;
////	D(p3) = 0;
////
////	TEST_ASSERT(mlength(p1, 3));
////	TEST_ASSERT(mlength(p1, 2));
////	TEST_ASSERT(mlength(p1, 1));
////	TEST_ASSERT(mlength(p1, 0));
////	TEST_ASSERT(!mlength(p2, 3));
////	TEST_ASSERT(mlength(p2, 2));
////	TEST_ASSERT(mlength(p2, 1));
////	TEST_ASSERT(mlength(p2, 0));
////	TEST_ASSERT(!mlength(p3, 3));
////	TEST_ASSERT(!mlength(p3, 2));
////	TEST_ASSERT(mlength(p3, 1));
////	TEST_ASSERT(mlength(p3, 0));
////}
////
////void test_last() {
////	C p1 = (C)malloc(sC * 2);
////	C p2 = (C)malloc(sC * 2);
////	C p3 = (C)malloc(sC * 2);
////
////	D(p1) = p2;
////	D(p2) = p3;
////	D(p3) = 0;
////
////	TEST_ASSERT_EQUAL_INT(0, last(0));
////	TEST_ASSERT_EQUAL_INT(p3, last(p1));
////	TEST_ASSERT_EQUAL_INT(p3, last(p2));
////	TEST_ASSERT_EQUAL_INT(p3, last(p3));
////}
////
////// BASIC STACK OPERATIONS
////
////void test_push() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 11);
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(0,X* x->err);
////	TEST_ASSERT(IS(ATM,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(11, A(x->stack));
////	push(x, ATM, 7);
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(0,X* x->err);
////	TEST_ASSERT(IS(ATM,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(7, A(x->stack));
////	TEST_ASSERT(IS(ATM, N(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(x->stack)));
////}
////
////void test_pop() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 11);
////	push(x, ATM, 7);
////
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	C v = pop(x);
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(7, v);
////	v = pop(x);
////	TEST_ASSERT_EQUAL_INT(0, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(11, v);
////}
////
////// COMPILATION PILE OPERATIONS
////
////void test_cppush() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	TEST_ASSERT_EQUAL_INT(0,X* x->cpile);
////	TEST_ASSERT_EQUAL_INT(0, length(x->cpile));
////
////	cppush(x);
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
////	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
////	TEST_ASSERT(IS(LST,X* x->cpile));
////	TEST_ASSERT_EQUAL_INT(0, length(A(x->cpile)));
////
////	cppush(x);
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
////	TEST_ASSERT_EQUAL_INT(2, length(x->cpile));
////	TEST_ASSERT(IS(LST,X* x->cpile));
////	TEST_ASSERT_EQUAL_INT(0, length(A(x->cpile)));
////	TEST_ASSERT(IS(LST, N(x->cpile)));
////	TEST_ASSERT_EQUAL_INT(0, length(A(N(x->cpile))));
////}
////
////void test_cppop_1() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	cppush(x);
////	cppop(x);
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 1, FREE(x));
////	TEST_ASSERT_EQUAL_INT(0, length(x->cpile));
////	TEST_ASSERT_EQUAL_INT(0,X* x->cpile);
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(0, length(A(x->stack)));
////}
////
////void test_cspush() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	cppush(x);
////	cspush(x, cons(x, 7, T(ATM, 0)));
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 2, FREE(x));
////	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
////	TEST_ASSERT_EQUAL_INT(1, length(A(x->cpile)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->cpile)));
////
////	cspush(x, cons(x, 11, T(ATM, 0)));
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 3, FREE(x));
////	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
////	TEST_ASSERT_EQUAL_INT(2, length(A(x->cpile)));
////	TEST_ASSERT(IS(ATM, A(x->cpile)));
////	TEST_ASSERT_EQUAL_INT(11, A(A(x->cpile)));
////	TEST_ASSERT(IS(ATM, N(A(x->cpile))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(A(x->cpile))));
////
////	cppush(x);
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 4, FREE(x));
////	TEST_ASSERT_EQUAL_INT(2, length(x->cpile));
////	TEST_ASSERT_EQUAL_INT(0, length(A(x->cpile)));
////	TEST_ASSERT_EQUAL_INT(2, length(A(N(x->cpile))));
////	TEST_ASSERT(IS(ATM, A(N(x->cpile))));
////	TEST_ASSERT_EQUAL_INT(11, A(A(N(x->cpile))));
////	TEST_ASSERT(IS(ATM, N(A(N(x->cpile)))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(x->cpile)))));
////
////	cspush(x, cons(x, 13, T(ATM, 0)));
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, FREE(x));
////	TEST_ASSERT_EQUAL_INT(2, length(x->cpile));
////	TEST_ASSERT_EQUAL_INT(1, length(A(x->cpile)));
////	TEST_ASSERT(IS(ATM, A(x->cpile)));
////	TEST_ASSERT_EQUAL_INT(13, A(A(x->cpile)));
////	TEST_ASSERT_EQUAL_INT(2, length(A(N(x->cpile))));
////	TEST_ASSERT(IS(ATM, A(N(x->cpile))));
////	TEST_ASSERT_EQUAL_INT(11, A(A(N(x->cpile))));
////	TEST_ASSERT(IS(ATM, N(A(N(x->cpile)))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(A(N(x->cpile)))));
////}
////
////void test_cppop_2() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	cppush(x);
////	cspush(x, cons(x, 7, T(ATM, 0)));
////	cspush(x, cons(x, 11, T(ATM, 0)));
////	cppush(x);
////	cspush(x, cons(x, 13, T(ATM, 0)));
////	cppop(x);
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, FREE(x));
////	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
////	TEST_ASSERT_EQUAL_INT(3, length(A(x->cpile)));
////	TEST_ASSERT(IS(LST, A(x->cpile)));
////	TEST_ASSERT_EQUAL_INT(1, length(A(A(x->cpile))));
////	TEST_ASSERT_EQUAL_INT(13, A(A(A(x->cpile))));
////	TEST_ASSERT(IS(ATM, N(A(x->cpile))));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->cpile))));
////	TEST_ASSERT(IS(ATM, N(N(A(x->cpile)))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(N(A(x->cpile)))));
////
////	cppop(x);
////
////	TEST_ASSERT_EQUAL_INT(f_nodes(x) - 5, FREE(x));
////	TEST_ASSERT_EQUAL_INT(0, length(x->cpile));
////	TEST_ASSERT_EQUAL_INT(0,X* x->cpile);
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(3, length(A(x->stack)));
////	TEST_ASSERT(IS(LST, A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(1, length(A(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(13, A(A(A(x->stack))));
////	TEST_ASSERT(IS(ATM, N(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->stack))));
////	TEST_ASSERT(IS(ATM, N(N(A(x->stack)))));
////	TEST_ASSERT_EQUAL_INT(7, A(N(N(A(x->stack)))));
////}
////
////void test_braces() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	_lbrace(x);
////	cspush(x, cons(x, 1, T(ATM, 0)));
////	cspush(x, cons(x, 2, T(ATM, 0)));
////	_lbrace(x);
////	cspush(x, cons(x, 3, T(ATM, 0)));
////	cspush(x, cons(x, 4, T(ATM, 0)));
////	_rbrace(x);
////	cspush(x, cons(x, 5, T(ATM, 0)));
////	_lbrace(x);
////	cspush(x, cons(x, 6, T(ATM, 0)));
////	_rbrace(x);
////	_rbrace(x);
////
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(0,X* x->cpile);
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(5, length(A(x->stack)));
////	TEST_ASSERT(IS(LST, A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(1, length(A(A(x->stack))));
////	TEST_ASSERT(IS(ATM, A(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(6, A(A(A(x->stack))));
////	TEST_ASSERT(IS(ATM, N(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(5, A(N(A(x->stack))));
////	TEST_ASSERT(IS(LST, N(N(A(x->stack)))));
////	TEST_ASSERT_EQUAL_INT(2, length(A(N(N(A(x->stack))))));
////	TEST_ASSERT(IS(ATM, A(N(N(A(x->stack))))));
////	TEST_ASSERT_EQUAL_INT(4, A(A(N(N(A(x->stack))))));
////	TEST_ASSERT(IS(ATM, N(A(N(N(A(x->stack)))))));
////	TEST_ASSERT_EQUAL_INT(3, A(N(A(N(N(A(x->stack)))))));
////	TEST_ASSERT(IS(ATM, N(N(N(A(x->stack))))));
////	TEST_ASSERT_EQUAL_INT(2, A(N(N(N(A(x->stack))))));
////	TEST_ASSERT(IS(ATM, N(N(N(N(A(x->stack)))))));
////	TEST_ASSERT_EQUAL_INT(1, A(N(N(N(N(A(x->stack)))))));
////}
////
////void test_empty() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	_empty(x);
////
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(0, A(x->stack));
////}
////
////void test_join_atom_atom_1() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 11);
////	push(x, ATM, 7);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(2, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->stack))));
////}
////
////void test_join_atom_atom_2() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 13);
////
////	push(x, ATM, 11);
////	push(x, ATM, 7);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(2, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(x->stack)));
////}
////
////void test_join_atom_empty_1() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	_empty(x);
////	push(x, ATM, 7);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(1, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////}
////
////void test_join_atom_empty_2() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 13);
////
////	_empty(x);
////	push(x, ATM, 7);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(1, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(13, A(N(x->stack)));
////}
////
////void test_join_atom_list_1() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 13);
////	push(x, ATM, 11);
////	_join(x);
////	push(x, ATM, 7);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(3, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(x->stack)))));
////}
////
////void test_join_atom_list_2() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 17);
////
////	push(x, ATM, 13);
////	push(x, ATM, 11);
////	_join(x);
////	push(x, ATM, 7);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(3, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(x->stack)))));
////	TEST_ASSERT(IS(ATM, N(x->stack)));
////	TEST_ASSERT_EQUAL_INT(17, A(N(x->stack)));
////}
////
////void test_join_list_atom_1() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 13);
////	push(x, ATM, 11);
////	push(x, ATM, 7);
////	_join(x);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(3, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(x->stack)))));
////}
////
////void test_join_list_atom_2() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 17);
////
////	push(x, ATM, 13);
////	push(x, ATM, 11);
////	push(x, ATM, 7);
////	_join(x);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(3, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(x->stack)))));
////	TEST_ASSERT(IS(ATM, N(x->stack)));
////	TEST_ASSERT_EQUAL_INT(17, A(N(x->stack)));
////}
////
////void test_join_list_list_1() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 17);
////	push(x, ATM, 13);
////	_join(x);
////	push(x, ATM, 11);
////	push(x, ATM, 7);
////	_join(x);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(4, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(x->stack)))));
////	TEST_ASSERT_EQUAL_INT(17, A(N(N(N(A(x->stack))))));
////}
////
////void test_join_list_list_2() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 19);
////
////	push(x, ATM, 17);
////	push(x, ATM, 13);
////	_join(x);
////	push(x, ATM, 11);
////	push(x, ATM, 7);
////	_join(x);
////	_join(x);
////
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(4, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(13, A(N(N(A(x->stack)))));
////	TEST_ASSERT_EQUAL_INT(17, A(N(N(N(A(x->stack))))));
////	TEST_ASSERT(IS(ATM, N(x->stack)));
////	TEST_ASSERT_EQUAL_INT(19, A(N(x->stack)));
////}
////
////void test_quote_1() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 7);
////	_quote(x);
////
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(1, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////}
////
////void test_quote_2() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 11);
////
////	push(x, ATM, 7);
////	_quote(x);
////
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(1, length(A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(7, A(A(x->stack)));
////	TEST_ASSERT(IS(ATM, N(x->stack)));
////	TEST_ASSERT_EQUAL_INT(11, A(N(x->stack)));
////}
////
////void test_quote_3() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 11);
////	push(x, ATM, 7);
////	_join(x);
////	_quote(x);
////
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(1, length(A(x->stack)));
////	TEST_ASSERT(IS(LST, A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(2, length(A(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(7, A(A(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(A(x->stack)))));
////}
////
////void test_quote_4() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 13);
////
////	push(x, ATM, 11);
////	push(x, ATM, 7);
////	_join(x);
////	_quote(x);
////
////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////	TEST_ASSERT(IS(LST,X* x->stack));
////	TEST_ASSERT_EQUAL_INT(1, length(A(x->stack)));
////	TEST_ASSERT(IS(LST, A(x->stack)));
////	TEST_ASSERT_EQUAL_INT(2, length(A(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(7, A(A(A(x->stack))));
////	TEST_ASSERT_EQUAL_INT(11, A(N(A(A(x->stack)))));
////	TEST_ASSERT(IS(ATM, N(x->stack)));
////	TEST_ASSERT_EQUAL_INT(13, A(N(x->stack)));
////}
////
////void test_neq() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 13);
////	push(x, ATM, 7);
////	_neq(x);
////	TEST_ASSERT_EQUAL_INT(1, A(x->stack));
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	pop(x);
////
////	push(x, ATM, 7);
////	push(x, ATM, 13);
////	_neq(x);
////	TEST_ASSERT_EQUAL_INT(1, A(x->stack));
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	pop(x);
////
////	push(x, ATM, 7);
////	push(x, ATM, 7);
////	_neq(x);
////	TEST_ASSERT_EQUAL_INT(0, A(x->stack));
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	pop(x);
////}
////
////void test_not() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	push(x, ATM, 7);
////	_not(x);
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(0, A(x->stack));
////	pop(x);
////
////	push(x, ATM, 0);
////	_not(x);
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(1, A(x->stack));
////	pop(x);
////
////	push(x, ATM, 1);
////	_not(x);
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(0, A(x->stack));
////	pop(x);
////
////	push(x, ATM, -1);
////	_not(x);
////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////	TEST_ASSERT_EQUAL_INT(0, A(x->stack));
////	pop(x);
////}
////
////#define ATM(x, n, d)							cons(x, n, T(ATM, d))
////#define LST(x, l, d)							cons(x, l, T(LST, d))
////#define PRM(x, p, d)				cons(x, (C)p, T(PRM, d))
////#define RECURSION(x, d)						PRM(x, 0, d)
////#define JUMP(x, j, d)							cons(x, ATM(x, j, 0), T(JMP, d))
////#define LAMBDA(x, w, d)						cons(x, cons(x, cons(x, w, T(LST, 0)), T(LST, 0)), T(JMP, d))
////#define CALL(x,X* xt, d)						cons(x, cons(x,X* xt, T(LST, 0)), T(JMP, d))
////C BRANCH(X*X* x, C t, C f, C d) {
////	if (t) R(last(t), d); else t = d;
////	if (f) R(last(f), d); else f = d;
////	return cons(x, cons(x, t, T(LST, cons(x, f, T(LST, 0)))), T(JMP, d));
////}
////
////void test_allot_str() {
////	C size = 512;
////	B block[size];
////	X*X* x = init(block, size);
////
////	B* here =X* x->here;
////
////	B* s = allot_str(x, 11);
////	strcpy(s, "test string");
////
////	TEST_ASSERT_EQUAL_PTR(here + sC, s);
////	TEST_ASSERT_EQUAL_STRING("test string", s);
////	TEST_ASSERT_EQUAL_INT(11, count(s));
////	TEST_ASSERT_EQUAL_PTR(here + sC + 12,X* x->here);
////}
////
//////void test_to_number() {
//////	C size = 512;
//////	B block[size];
//////	X*X* x = init(block, size);
//////
//////	x->ibuf = "256";
//////	parse_token(x);
//////	_to_number(x);
//////	C n = pop(x);
//////
//////	TEST_ASSERT_EQUAL_INT(256, n);
//////}
////
////////void test_clear_stack() {
////////	C size = 512;
////////	B block[size];
////////	X*X* x = init(block, size);
////////
////////	C f_nodes = (size - sizeof(X)) / (2*sC) - 3;
////////
////////	TEST_ASSERT_EQUAL_INT(f_nodes, height(x->free));
////////
////////	push(x, 13);
////////	push(x, 7);
////////	
////////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(f_nodes - 2, height(x->free));
////////
////////	_sclear(x);
////////
////////	TEST_ASSERT_EQUAL_INT(0, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(f_nodes, height(x->free));
////////}
////////
////////void test_push_stack() {
////////	C size = 512;
////////	B block[size];
////////	X*X* x = init(block, size);
////////
////////	C f_nodes = (size - sizeof(X)) / (2*sC) - 3;
////////
////////	push(x, 13);
////////	push(x, 7);
////////
////////	TEST_ASSERT_EQUAL_INT(f_nodes - 2, height(x->free));
////////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////////
////////	_spush(x);
////////
////////	TEST_ASSERT_EQUAL_INT(f_nodes - 3, height(x->free));
////////	TEST_ASSERT_EQUAL_INT(7, A(D(P(x))));
////////	TEST_ASSERT_EQUAL_INT(13, A(D(D(P(x)))));
////////	TEST_ASSERT_EQUAL_INT(2, height(P(x)));
////////	TEST_ASSERT_EQUAL_INT(R(x), A(P(x)));
////////	TEST_ASSERT_EQUAL_INT(0, length(x->stack));
////////}
////////
////////void test_drop_stack() {
////////	C size = 512;
////////	B block[size];
////////	X*X* x = init(block, size);
////////
////////	C f_nodes = (size - sizeof(X)) / (2*sC) - 3;
////////
////////	_sdrop(x);
////////
////////	TEST_ASSERT_EQUAL_INT(0, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(1, length(P(x)));
////////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
////////	TEST_ASSERT_EQUAL_INT(f_nodes, height(x->free));
////////
////////	push(x, 13);
////////	push(x, 7);
////////
////////	TEST_ASSERT_EQUAL_INT(f_nodes - 2, height(x->free));
////////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(7, A(x->stack));
////////	TEST_ASSERT_EQUAL_INT(13,X* x->stack);
////////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
////////
////////	_spush(x);
////////
////////	TEST_ASSERT_EQUAL_INT(f_nodes - 3, height(x->free));
////////
////////	push(x, 21);
////////
////////	TEST_ASSERT_EQUAL_INT(f_nodes - 4, height(x->free));
////////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(21, A(x->stack));
////////
////////	_sdrop(x);
////////
////////	TEST_ASSERT_EQUAL_INT(f_nodes - 2, height(x->free));
////////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(7, A(x->stack));
////////	TEST_ASSERT_EQUAL_INT(13,X* x->stack);
////////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
////////}
////////
////////void test_drop() {
////////	C size = 512;
////////	B block[size];
////////	X*X* x = init(block, size);
////////
////////	push(x, 21);
////////	push(x, 13);
////////	push(x, 7);
////////	TEST_ASSERT_EQUAL_INT(3, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(7, A(x->stack));
////////	TEST_ASSERT_EQUAL_INT(13,X* x->stack);
////////	TEST_ASSERT_EQUAL_INT(21, A(D(D(K(x)))));
////////	_drop(x);
////////	TEST_ASSERT_EQUAL_INT(2, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(13, A(x->stack));
////////	TEST_ASSERT_EQUAL_INT(21,X* x->stack);
////////	_drop(x);
////////	TEST_ASSERT_EQUAL_INT(1, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(21, A(x->stack));
////////	_drop(x);
////////	TEST_ASSERT_EQUAL_INT(0, length(x->stack));
////////	_drop(x);
////////	TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW,X* x->err);
////////}
////////
////////void test_rev() {
////////	C size = 512;
////////	B block[size];
////////	X*X* x = init(block, size);
////////
////////	push(x, 21);
////////	push(x, 13);
////////	push(x, 7);
////////	push(x, 5);
////////	push(x, 3);
////////	TEST_ASSERT_EQUAL_INT(5, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(3, A(x->stack));
////////	TEST_ASSERT_EQUAL_INT(5,X* x->stack);
////////	TEST_ASSERT_EQUAL_INT(7, A(D(D(K(x)))));
////////	TEST_ASSERT_EQUAL_INT(13, A(D(D(D(K(x))))));
////////	TEST_ASSERT_EQUAL_INT(21, A(D(D(D(D(K(x)))))));
////////	_rev(x);
////////	TEST_ASSERT_EQUAL_INT(5, length(x->stack));
////////	TEST_ASSERT_EQUAL_INT(21, A(x->stack));
////////	TEST_ASSERT_EQUAL_INT(13,X* x->stack);
////////	TEST_ASSERT_EQUAL_INT(7, A(D(D(K(x)))));
////////	TEST_ASSERT_EQUAL_INT(5, A(D(D(D(K(x))))));
////////	TEST_ASSERT_EQUAL_INT(3, A(D(D(D(D(K(x)))))));
////////}
////////
//////////////void test_stack_to_list() {
//////////////}
//////////////
////////////////void test_append() {
////////////////}
////////////////
////////////////void test_copy() {
////////////////	C size = 512;
////////////////	B block[size];
////////////////	X* x = init(block, size);
////////////////
////////////////	C f_nodes = (size - sizeof(X)) / sizeof(PAIR);
////////////////
////////////////	// TODO
////////////////}
//////////////
//////////////void test_header_body_reveal() {
//////////////	C size = 2048;
//////////////	B block[size];
//////////////	X* x = init(block, size);
//////////////
//////////////	B* here = x->here;
//////////////	PAIR* w = header(x, "test", 4);
//////////////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////////////	TEST_ASSERT_EQUAL_PTR(x->here, ALIGN(here, sC) + sC + 4 + 1);
//////////////	TEST_ASSERT(IS(ATM, w));
//////////////	TEST_ASSERT_EQUAL_INT(4, COUNT(NFA(w)));
//////////////	TEST_ASSERT_EQUAL_STRING("test", NFA(w));
//////////////	TEST_ASSERT_EQUAL_INT(((B*)REF(REF(w))) + 4, DFA(w));
//////////////	TEST_ASSERT_EQUAL_INT(0, CFA(w));
//////////////
//////////////	TEST_ASSERT_NULL(x->dict);
//////////////
//////////////	PAIR* cfa = cons(x, ATM, 7, cons(x, ATM, 13, 0));
//////////////	body(x, w, cfa);
//////////////
//////////////	TEST_ASSERT_EQUAL_INT(cfa, CFA(w));
//////////////	TEST_ASSERT_EQUAL_INT(7, CFA(w)->value);
//////////////	TEST_ASSERT_EQUAL_INT(13, N(CFA(w))->value);
//////////////
//////////////	reveal(x, w);
//////////////
//////////////	TEST_ASSERT_EQUAL_PTR(w, x->dict);
//////////////	TEST_ASSERT_NULL(N(x->dict));
//////////////
//////////////	TEST_ASSERT_FALSE(IS_IMMEDIATE(w));
//////////////
//////////////	_immediate(x);
//////////////
//////////////	TEST_ASSERT_TRUE(IS_IMMEDIATE(w));
//////////////}
//////////////
////////////////void test_find() {
////////////////	C size = 512;
////////////////	B block[size];
////////////////	X* x = init(block, size);
////////////////
////////////////	PAIR* dup = reveal(x, header(x, "dup", 3));
////////////////	PAIR* swap = reveal(x, header(x, "swap", 4));
////////////////	PAIR* test = reveal(x, header(x, "test", 4));
////////////////
////////////////	TEST_ASSERT_EQUAL_PTR(dup, find(x, "dup", 3));
////////////////	TEST_ASSERT_EQUAL_PTR(swap, find(x, "swap", 4));
////////////////	TEST_ASSERT_EQUAL_PTR(test, find(x, "test", 4));
////////////////	TEST_ASSERT_EQUAL_PTR(0, find(x, "nop", 3));
////////////////
////////////////	TEST_ASSERT_EQUAL_PTR(test, x->dict);
////////////////	TEST_ASSERT_EQUAL_PTR(swap, N(x->dict));
////////////////	TEST_ASSERT_EQUAL_PTR(dup, N(N(x->dict)));
////////////////	TEST_ASSERT_EQUAL_PTR(0, N(N(N(x->dict))));
////////////////}
////////////////
////////////////void test_dodo_initialization() {
////////////////	C size = 4096;
////////////////	B block[size];
////////////////	X* x = dodo(init(block, size));
////////////////
////////////////	TEST_ASSERT_EQUAL_PTR(&_add, CFA(find(x, "+", 1))->value);
////////////////}
////////////////
////////////////void test_fib() {
////////////////	C size = 8192;
////////////////	B block[size];
////////////////	X* x = init(block, size);
////////////////
////////////////	// : fib dup 1 > if 1- dup 1- recurse swap recurse + then ;
////////////////
////////////////	x->ip =
////////////////		cons(x, (C)&_dup, T_PRM,
////////////////		cons(x, 1, ATM,
////////////////		cons(x, (C)&_gt, T_PRM,
////////////////		cons(x,
////////////////			0,
////////////////			T_JMP,
////////////////			cons(x, 1, ATM,
////////////////			cons(x, (C)&_sub, T_PRM,
////////////////			cons(x, (C)&_dup, T_PRM,
////////////////			cons(x, 1, ATM,
////////////////			cons(x, (C)&_sub, T_PRM,
////////////////			cons(x, (C)&_rec, T_PRM,
////////////////			cons(x, (C)&_swap, T_PRM,
////////////////			cons(x, (C)&_rec, T_PRM,	
////////////////			cons(x, (C)&_add, T_PRM, 0)))))))))))));
////////////////
////////////////		x->dictstack = cons(x, 6, ATM, 0);
////////////////		x->rstack = cons(x, (C)x->ip, T_WRD, 0);
////////////////
////////////////		while(STEP(x));
////////////////
////////////////		//printf("%ld\n", x->dictstack->value);
////////////////}
//////////////

//void test_EVALUATE_braces() {
//	C size = 2048;
//	B block[size];
//	X* x = bootstrap(init(block, size));
//
//	evaluate(x, "{ 3 4 }");
//
//	TEST_ASSERT_EQUAL_INT(1, length(S(x)));
//	TEST_ASSERT_EQUAL_INT(LST, T(S(x)));
//	TEST_ASSERT_EQUAL_INT(2, length(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(3, A(A(S(x))));
//	TEST_ASSERT_EQUAL_INT(4, A(N(A(S(x)))));
//}

int main() {
	UNITY_BEGIN();

	// TAGGED POINTER BASED TYPING INFORMATION
	RUN_TEST(test_BASIC_pairs);
	RUN_TEST(test_BASIC_link_pairs);
	//RUN_TEST(test_BASIC_words);

	// LIST LENGTH
	RUN_TEST(test_LST_length);

	// CONTEXT
	//RUN_TEST(test_X_minimum_block_size);
	RUN_TEST(test_X_block_initialization);

	// LST CREATION AND DESTRUCTION (AUTOMATIC MEMORY MANAGEMENT)
	RUN_TEST(test_LST_cons);
	RUN_TEST(test_LST_recl);
	RUN_TEST(test_LST_recl_list);
	RUN_TEST(test_LST_clone);
	RUN_TEST(test_LST_reverse);
	//RUN_TEST(test_LST_depth);
	//RUN_TEST(test_LST_disjoint);

	//// BASIC STACK OPERATIONS
	//RUN_TEST(test_STACK_push);
	//RUN_TEST(test_STACK_pop);

	// STACK PRMITIVES
	RUN_TEST(test_STACK_duplicate_atom);
	RUN_TEST(test_STACK_duplicate_list);
	RUN_TEST(test_STACK_swap_1);
	RUN_TEST(test_STACK_swap_2);
	RUN_TEST(test_STACK_drop);
	RUN_TEST(test_STACK_over);
	RUN_TEST(test_STACK_rot);

	// COMPARISON PRMITIVES
	RUN_TEST(test_COMPARISON_gt);
	RUN_TEST(test_COMPARISON_lt);
	RUN_TEST(test_COMPARISON_eq);
	RUN_TEST(test_COMPARISON_neq);

	// ARITHMETIC PRMITIVES
	RUN_TEST(test_ARITHMETIC_add);
	RUN_TEST(test_ARITHMETIC_sub);
	RUN_TEST(test_ARITHMETIC_mul);
	RUN_TEST(test_ARITHMETIC_division);
	RUN_TEST(test_ARITHMETIC_mod);

	// BIT PRIMITIVES
	RUN_TEST(test_BIT_and);
	RUN_TEST(test_BIT_or);
	RUN_TEST(test_BIT_invert);

	// LIST PRIMITIVES
	RUN_TEST(test_LIST_empty);
	RUN_TEST(test_LIST_list_to_stack);
	RUN_TEST(test_LIST_list_to_stack_2);
	RUN_TEST(test_LIST_list_to_stack_3);
	RUN_TEST(test_LIST_stack_to_list);
	RUN_TEST(test_LIST_stack_to_list_2);
	RUN_TEST(test_LIST_stack_to_list_3);

	// INNER INTERPRETER
	RUN_TEST(test_INNER_NEXT_1);
	RUN_TEST(test_INNER_NEXT_2);
	RUN_TEST(test_INNER_NEXT_3);
	RUN_TEST(test_INNER_STEP_nothing);
	RUN_TEST(test_INNER_STEP_atom);
	RUN_TEST(test_INNER_STEP_list);
	RUN_TEST(test_INNER_STEP_list_2);
	RUN_TEST(test_INNER_STEP_primitive);
	RUN_TEST(test_INNER_CALL_word);
	RUN_TEST(test_INNER_STEP_word);
	RUN_TEST(test_INNER_STEP_words);
	RUN_TEST(test_INNER_STEP_words_2);

	// EXECUTION PRMITIVES
	RUN_TEST(test_EXEC_exec_i);
	RUN_TEST(test_EXEC_exec_i_2);
	RUN_TEST(test_EXEC_exec_i_3);
	RUN_TEST(test_EXEC_exec_i_4);
	RUN_TEST(test_EXEC_exec_x);
	RUN_TEST(test_EXEC_exec_x_2);
	RUN_TEST(test_EXEC_branch);
	RUN_TEST(test_EXEC_branch_2);
	RUN_TEST(test_EXEC_branch_3);
	//RUN_TEST(test_EXEC_zjump);
	//RUN_TEST(test_EXEC_jump);

	// PARSING
	RUN_TEST(test_PARSING_parse);
	RUN_TEST(test_PARSING_parse_2);
	RUN_TEST(test_PARSING_parse_3);
	RUN_TEST(test_PARSING_parse_4);
	RUN_TEST(test_PARSING_parse_name);
	RUN_TEST(test_PARSING_parse_name_2);
	RUN_TEST(test_PARSING_parse_name_3);
	RUN_TEST(test_PARSING_parse_name_4);

	// MEMORY
	RUN_TEST(test_MEM_grow);
	RUN_TEST(test_MEM_shrink);
	RUN_TEST(test_MEM_allot);

	// DICTIONARY
	RUN_TEST(test_DICT_find_name);
	//// COMPILATION
	//RUN_TEST(test_COMPILATION_compile_word);

	//// OUTER INTERPRETER
	//RUN_TEST(test_OUTER_evaluate_numbers);
	//RUN_TEST(test_OUTER_evaluate_words);

	//// PRMITIVES
	//RUN_TEST(test_PRMITIVES_zjump);
	//RUN_TEST(test_PRMITIVES_jump);
	//RUN_TEST(test_PRMITIVES_postpone);
	//RUN_TEST(test_PRMITIVES_parse);
	//RUN_TEST(test_PRMITIVES_parse_name);
	//RUN_TEST(test_PRMITIVES_spush);
	//RUN_TEST(test_PRMITIVES_clear);
	//RUN_TEST(test_PRMITIVES_sdrop);
	//RUN_TEST(test_PRMITIVES_stack_to_list);
	//RUN_TEST(test_PRMITIVES_list_to_stack);
	//RUN_TEST(test_PRMITIVES_list_to_stack_2);
	//RUN_TEST(test_PRMITIVES_brackets);
	//RUN_TEST(test_PRMITIVES_reverse_stack);
	//RUN_TEST(test_PRMITIVES_reverse_stack_2);
	//RUN_TEST(test_PRMITIVES_braces);
	//RUN_TEST(test_PRMITIVES_duplicate_atom);
	//RUN_TEST(test_PRMITIVES_duplicate_list);
	//RUN_TEST(test_PRMITIVES_swap);
	//RUN_TEST(test_PRMITIVES_drop);
	//RUN_TEST(test_PRMITIVES_rot);
	//RUN_TEST(test_PRMITIVES_over);
	//RUN_TEST(test_PRMITIVES_exec_atom);
	//RUN_TEST(test_PRMITIVES_exec_list);
	//RUN_TEST(test_PRMITIVES_exec_primitive);
	//RUN_TEST(test_PRMITIVES_exec_primitive2);
	//RUN_TEST(test_PRMITIVES_exec_word);
	//RUN_TEST(test_PRMITIVES_exec_word2);
	//RUN_TEST(test_PRMITIVES_exec_x_atom);
	//RUN_TEST(test_PRMITIVES_exec_x_list);
	//RUN_TEST(test_PRMITIVES_exec_x_primitive);
	//RUN_TEST(test_PRMITIVES_exec_x_primitive2);
	//RUN_TEST(test_PRMITIVES_exec_x_word);
	//RUN_TEST(test_PRMITIVES_exec_x_word2);
	//RUN_TEST(test_PRMITIVES_branch);
	//RUN_TEST(test_PRMITIVES_branch2);
	//RUN_TEST(test_PRMITIVES_ARITHMETIC_add);
	//RUN_TEST(test_PRMITIVES_ARITHMETIC_sub);
	//RUN_TEST(test_PRMITIVES_ARITHMETIC_mul);
	//RUN_TEST(test_PRMITIVES_ARITHMETIC_division);
	//RUN_TEST(test_PRMITIVES_ARITHMETIC_mod);
	//RUN_TEST(test_PRMITIVES_COMPARISON_gt);
	//RUN_TEST(test_PRMITIVES_COMPARISON_lt);
	//RUN_TEST(test_PRMITIVES_COMPARISON_eq);
	//RUN_TEST(test_PRMITIVES_BIT_and);
	//RUN_TEST(test_PRMITIVES_BIT_or);
	//RUN_TEST(test_PRMITIVES_BIT_invert);

	//RUN_TEST(test_EVALUATE_braces);


	//// STACK PRMITIVES
	//RUN_TEST(test_STACK_swap_1);
	//RUN_TEST(test_STACK_swap_2);
	//RUN_TEST(test_STACK_drop);
	//RUN_TEST(test_STACK_over);
	//RUN_TEST(test_STACK_rot);

	//// ARITHMETIC PRMITIVES
	//// COMPARISON PRMITIVES
	//// BIT PRMITIVES
	//// CONTIGUOUS MEMORY
	//RUN_TEST(test_MEM_allot);
	//RUN_TEST(test_MEM_compile_str);
	////RUN_TEST(test_align);

	////  LST FUNCTIONS
	//RUN_TEST(test_append);
	//RUN_TEST(test_depth);

	//// WRD DEFINITIONS
	//RUN_TEST(test_header);
	//RUN_TEST(test_body);
	//RUN_TEST(test_reveal);
	//RUN_TEST(test_immediate);

//	RUN_TEST(test_mlength);
//	RUN_TEST(test_last);
//
//	// COMPILATION PILE OPERATIONS
//	RUN_TEST(test_cppush);
//	RUN_TEST(test_cppop_1);
//	RUN_TEST(test_cspush);
//	RUN_TEST(test_cppop_2);
//
//	RUN_TEST(test_empty);
//
//	RUN_TEST(test_join_atom_atom_1);
//	RUN_TEST(test_join_atom_atom_2);
//	RUN_TEST(test_join_atom_empty_1);
//	RUN_TEST(test_join_atom_empty_2);
//	RUN_TEST(test_join_atom_list_1);
//	RUN_TEST(test_join_atom_list_2);
//	RUN_TEST(test_join_list_atom_1);
//	RUN_TEST(test_join_list_atom_2);
//	RUN_TEST(test_join_list_list_1);
//	RUN_TEST(test_join_list_list_2);
//
//	RUN_TEST(test_find);
//	//RUN_TEST(test_to_number);
//
////	//RUN_TEST(test_stack_to_list);
////
////	////RUN_TEST(test_fib);
////
////	////RUN_TEST(test_append);
////	////RUN_TEST(test_copy);
////
////
////	//RUN_TEST(test_header_body_reveal);
////	////RUN_TEST(test_find);
////
////	////RUN_TEST(test_dodo_initialization);
////
	return UNITY_END();
}
