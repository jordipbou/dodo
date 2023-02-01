#include<stdlib.h>
#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_types() {
	C p = (C)malloc(sizeof(C) * 2);

	D(p) = T(ATM, 8);
	TEST_ASSERT_EQUAL_INT(8, D_(p));
	TEST_ASSERT_EQUAL_INT(ATM, _D(p));
	TEST_ASSERT(IS(ATM, p));
	TEST_ASSERT(!IS(LST, p));
	TEST_ASSERT(!IS(BRN, p));
	TEST_ASSERT(!IS(PRM, p));
	TEST_ASSERT(VAL(p));
	TEST_ASSERT(!REF(p));
	D(p) = T(LST, 8);
	TEST_ASSERT_EQUAL_INT(8, D_(p));
	TEST_ASSERT_EQUAL_INT(LST, _D(p));
	TEST_ASSERT(!IS(ATM, p));
	TEST_ASSERT(IS(LST, p));
	TEST_ASSERT(!IS(BRN, p));
	TEST_ASSERT(!IS(PRM, p));
	TEST_ASSERT(!VAL(p));
	TEST_ASSERT(REF(p));
	D(p) = T(BRN, 8);
	TEST_ASSERT_EQUAL_INT(8, D_(p));
	TEST_ASSERT_EQUAL_INT(BRN, _D(p));
	TEST_ASSERT(!IS(ATM, p));
	TEST_ASSERT(!IS(LST, p));
	TEST_ASSERT(IS(BRN, p));
	TEST_ASSERT(!IS(PRM, p));
	TEST_ASSERT(!VAL(p));
	TEST_ASSERT(REF(p));
	D(p) = T(PRM, 8);
	TEST_ASSERT_EQUAL_INT(8, D_(p));
	TEST_ASSERT_EQUAL_INT(PRM, _D(p));
	TEST_ASSERT(!IS(ATM, p));
	TEST_ASSERT(!IS(LST, p));
	TEST_ASSERT(!IS(BRN, p));
	TEST_ASSERT(IS(PRM, p));
	TEST_ASSERT(VAL(p));
	TEST_ASSERT(!REF(p));
}

void test_length() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);

	D(p1) = T(ATM, p2);
	D(p2) = p3;
	D(p3) = 0;

	TEST_ASSERT_EQUAL_INT(0, length(0));
	TEST_ASSERT_EQUAL_INT(1, length(p3));
	TEST_ASSERT_EQUAL_INT(2, length(p2));
	TEST_ASSERT_EQUAL_INT(3, length(p1));
}

void test_depth() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);
	C p4 = (C)malloc(sizeof(C) * 2);
	C p5 = (C)malloc(sizeof(C) * 2);

	D(p1) = T(ATM, p2);
	D(p2) = T(LST, p3);
	A(p2) = p4;
	D(p3) = T(PRM, 0);
	D(p4) = p5;
	D(p5) = 0;

	TEST_ASSERT_EQUAL_INT(3, length(p1));
	TEST_ASSERT_EQUAL_INT(2, length(A(p2)));
	TEST_ASSERT_EQUAL_INT(5, depth(p1));
	TEST_ASSERT_EQUAL_INT(4, depth(p2));
	TEST_ASSERT_EQUAL_INT(2, depth(p4));
	TEST_ASSERT_EQUAL_INT(1, depth(p3));
	TEST_ASSERT_EQUAL_INT(1, depth(p5));
	TEST_ASSERT_EQUAL_INT(0, depth(0));
}

void test_min_length() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);

	D(p1) = p2;
	D(p2) = p3;
	D(p3) = 0;

	TEST_ASSERT(min_length(p1, 3));
	TEST_ASSERT(min_length(p1, 2));
	TEST_ASSERT(min_length(p1, 1));
	TEST_ASSERT(min_length(p1, 0));
	TEST_ASSERT(!min_length(p2, 3));
	TEST_ASSERT(min_length(p2, 2));
	TEST_ASSERT(min_length(p2, 1));
	TEST_ASSERT(min_length(p2, 0));
	TEST_ASSERT(!min_length(p3, 3));
	TEST_ASSERT(!min_length(p3, 2));
	TEST_ASSERT(min_length(p3, 1));
	TEST_ASSERT(min_length(p3, 0));
}

void test_last() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);

	D(p1) = p2;
	D(p2) = p3;
	D(p3) = 0;

	TEST_ASSERT_EQUAL_INT(0, last(0));
	TEST_ASSERT_EQUAL_INT(p3, last(p1));
	TEST_ASSERT_EQUAL_INT(p3, last(p2));
	TEST_ASSERT_EQUAL_INT(p3, last(p3));
}

#define free_nodes(x)			((x->size - sizeof(X)) / (2*sizeof(C)))

void test_block_initialization() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(0, length(x->s));
	TEST_ASSERT_EQUAL_INT(free_nodes(x), length(x->f));

	TEST_ASSERT_EQUAL_INT(((B*)x) + sizeof(X), BOTTOM(x));
	TEST_ASSERT_EQUAL_INT(BOTTOM(x), x->here);
	TEST_ASSERT_EQUAL_INT(ALIGN(x->here, 2*sizeof(C)), x->there);
	TEST_ASSERT_EQUAL_INT(ALIGN(((B*)x) + size - 2*sizeof(C) - 1, 2*sizeof(C)), TOP(x));
	TEST_ASSERT_EQUAL_INT(TOP(x), x->f);

	TEST_ASSERT_EQUAL_INT(0, x->dict);
	TEST_ASSERT_EQUAL_INT(0, x->s);
	TEST_ASSERT_EQUAL_INT(0, x->rstack);
}

void test_cons() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C l = cons(x, 7, cons(x, 11, cons(x, 13, 0)));

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 3, length(x->f));
	TEST_ASSERT_EQUAL_INT(3, length(l));
	TEST_ASSERT_EQUAL_INT(7, A(l));
	TEST_ASSERT_EQUAL_INT(11, A(D(l)));
	TEST_ASSERT_EQUAL_INT(13, A(D(D(l))));

	C p = cons(x, 7, 0);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 4, length(x->f));
	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(0, D(p));
	
	while (length(x->f) > 0) { cons(x, 1, 0); }

	C p3 = cons(x, 13, 0);
	TEST_ASSERT_EQUAL_INT(0, p3);
}

void test_reclaim() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C l = cons(x, 7, cons(x, 11, cons(x, 13, 0)));
	C m = reclaim(x, l);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 2, length(x->f));
	TEST_ASSERT_EQUAL_INT(2, length(m));
	TEST_ASSERT_EQUAL_INT(11, A(m));
	TEST_ASSERT_EQUAL_INT(13, A(D(m)));
}

void test_clone() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C l = 
		cons(x, 7, T(ATM, 
		cons(x, 11, T(PRM, 
		cons(x, 
			cons(x, 13, T(ATM,
			cons(x, 17, T(ATM, 0)))), T(LST,
		cons(x, 19, T(ATM, 0))))))));

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 6, length(x->f));
	TEST_ASSERT_EQUAL_INT(4, length(l));
	TEST_ASSERT(IS(ATM, l));
	TEST_ASSERT_EQUAL_INT(7, A(l));
	TEST_ASSERT(IS(PRM, D_(l)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(l)));
	TEST_ASSERT(IS(LST, D_(D_(l))));
	TEST_ASSERT_EQUAL_INT(2, length(D_(D_(l))));
	TEST_ASSERT(IS(ATM, A(D_(D_(l)))));
	TEST_ASSERT_EQUAL_INT(13, A(A(D_(D_(l)))));
	TEST_ASSERT(IS(ATM, D_(A(D_(D_(l))))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(A(D_(D_(l))))));
	TEST_ASSERT(IS(ATM, D_(D_(D_(l)))));
	TEST_ASSERT_EQUAL_INT(19, A(D_(D_(D_(l)))));

	C c = clone(x, l);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 12, length(x->f));
	TEST_ASSERT_EQUAL_INT(4, length(c));
	TEST_ASSERT(IS(ATM, c));
	TEST_ASSERT_EQUAL_INT(7, A(c));
	TEST_ASSERT(IS(PRM, D_(c)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(c)));
	TEST_ASSERT(IS(LST, D_(D_(c))));
	TEST_ASSERT_EQUAL_INT(2, length(D_(D_(c))));
	TEST_ASSERT(IS(ATM, A(D_(D_(c)))));
	TEST_ASSERT_EQUAL_INT(13, A(A(D_(D_(c)))));
	TEST_ASSERT(IS(ATM, D_(A(D_(D_(c))))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(A(D_(D_(c))))));
	TEST_ASSERT(IS(ATM, D_(D_(D_(c)))));
	TEST_ASSERT_EQUAL_INT(19, A(D_(D_(D_(c)))));

	TEST_ASSERT_NOT_EQUAL_INT(l, c);
	TEST_ASSERT_NOT_EQUAL_INT(D_(l), D_(c));
	TEST_ASSERT_NOT_EQUAL_INT(D_(D_(l)), D_(D_(c)));
	TEST_ASSERT_NOT_EQUAL_INT(A(D_(D_(l))), A(D_(D_(c))));
	TEST_ASSERT_NOT_EQUAL_INT(D_(A(D_(D_(l)))), D_(A(D_(D_(c)))));
	TEST_ASSERT_NOT_EQUAL_INT(D_(D_(D_(l))), D_(D_(D_(c))));


}

void test_push() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT_EQUAL_INT(11, A(x->s));
	push(x, ATM, 7);
	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT_EQUAL_INT(7, A(x->s));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(x->s)));

	//while (length(x->f) > 0) { push(x, ATM, 7); }

	//TEST_ASSERT_EQUAL_INT(free_nodes(x), length(x->s));
	//TEST_ASSERT_EQUAL_INT(0, x->err);
	//push(x, ATM, 13);
	//TEST_ASSERT_EQUAL_INT(ERR_OVERFLOW, x->err);
	//TEST_ASSERT_EQUAL_INT(7, A(x->s));
}

void test_pop() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	//TEST_ASSERT_EQUAL_INT(0, x->err);
	//C v = pop(x);
	//TEST_ASSERT_EQUAL_INT(0, v);
	//TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW, x->err);

	//x->err = 0;

	push(x, ATM, 11);
	push(x, ATM, 7);

	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	C v = pop(x);
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT_EQUAL_INT(7, v);
	v = pop(x);
	TEST_ASSERT_EQUAL_INT(0, length(x->s));
	TEST_ASSERT_EQUAL_INT(11, v);
}

void test_empty() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	_empty(x);

	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
}

void test_join_atom_atom_1() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(2, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
}

void test_join_atom_atom_2() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(2, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(x->s)));
}

void test_join_atom_empty_1() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	_empty(x);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
}

void test_join_atom_empty_2() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);

	_empty(x);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(13, A(D_(x->s)));
}

void test_join_atom_list_1() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 11);
	_join(x);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
}

void test_join_atom_list_2() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 17);

	push(x, ATM, 13);
	push(x, ATM, 11);
	_join(x);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(D_(x->s)));
}

void test_join_list_atom_1() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
}

void test_join_list_atom_2() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 17);

	push(x, ATM, 13);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_join(x);

	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(D_(x->s)));
}

void test_join_list_list_1() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 17);
	push(x, ATM, 13);
	_join(x);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(4, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(D_(D_(A(x->s))))));
}

void test_join_list_list_2() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 19);

	push(x, ATM, 17);
	push(x, ATM, 13);
	_join(x);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_join(x);

	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(4, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(19, A(D_(x->s)));
}

void test_quote_1() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	_quote(x);

	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
}

void test_quote_2() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);

	push(x, ATM, 7);
	_quote(x);

	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, length(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(x->s)));
}

void test_quote_3() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_quote(x);

	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, length(A(x->s)));
	TEST_ASSERT(IS(LST, A(x->s)));
	TEST_ASSERT_EQUAL_INT(2, length(A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(A(x->s)))));
}

void test_quote_4() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_quote(x);

	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, length(A(x->s)));
	TEST_ASSERT(IS(LST, A(x->s)));
	TEST_ASSERT_EQUAL_INT(2, length(A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(A(x->s)))));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(13, A(D_(x->s)));
}

void test_dup_atom() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	_dup(x);

	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 2, length(x->f));
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(D_(x->s)));

	push(x, ATM, 11);
	_dup(x);

	TEST_ASSERT_EQUAL_INT(4, length(x->s));
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 4, length(x->f));
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(x->s));
	TEST_ASSERT_EQUAL_INT(11, A(D_(x->s)));
	TEST_ASSERT(IS(ATM, D_(D_(x->s))));
	TEST_ASSERT(IS(ATM, D_(D_(D_(x->s)))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(D_(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(D_(D_(x->s)))));
}

void test_dup_list() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	push(x, ATM, 11);
	_join(x);
	_quote(x);

	// ((11, 7))
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 4, length(x->f));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, length(A(x->s)));
	TEST_ASSERT(IS(LST, A(x->s)));
	TEST_ASSERT_EQUAL_INT(2, length(A(A(x->s))));
	TEST_ASSERT(IS(ATM, A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(11, A(A(A(x->s))));
	TEST_ASSERT(IS(ATM, D_(A(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(A(x->s)))));

	push(x, ATM, 13);
	push(x, ATM, 17);
	_join(x);

	// (17, 13)
	// ((11, 7))
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 7, length(x->f));
	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(2, length(A(x->s)));
	TEST_ASSERT(IS(ATM, A(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(A(x->s)));
	TEST_ASSERT(IS(ATM, D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(x->s))));

	_join(x);

	// (17, 13, (11, 7))
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 6, length(x->f));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, length(A(x->s)));
	TEST_ASSERT(IS(ATM, A(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(A(x->s)));
	TEST_ASSERT(IS(ATM, D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(x->s))));
	TEST_ASSERT(IS(LST, D_(D_(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(2, length(A(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, A(D_(D_(A(x->s))))));
	TEST_ASSERT_EQUAL_INT(11, A(A(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, D_(A(D_(D_(A(x->s)))))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(D_(D_(A(x->s)))))));

	_dup(x);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 12, length(x->f));
	TEST_ASSERT_EQUAL_INT(2, length(x->s));

	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, length(A(x->s)));
	TEST_ASSERT(IS(ATM, A(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(A(x->s)));
	TEST_ASSERT(IS(ATM, D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(x->s))));
	TEST_ASSERT(IS(LST, D_(D_(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(2, length(A(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, A(D_(D_(A(x->s))))));
	TEST_ASSERT_EQUAL_INT(11, A(A(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, D_(A(D_(D_(A(x->s)))))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(D_(D_(A(x->s)))))));

	TEST_ASSERT(IS(LST, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(3, length(A(D_(x->s))));
	TEST_ASSERT(IS(ATM, A(D_(x->s))));
	TEST_ASSERT_EQUAL_INT(17, A(A(D_(x->s))));
	TEST_ASSERT(IS(ATM, D_(A(D_(x->s)))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(D_(x->s)))));
	TEST_ASSERT(IS(LST, D_(D_(A(D_(x->s))))));
	TEST_ASSERT_EQUAL_INT(2, length(A(D_(D_(A(D_(x->s)))))));
	TEST_ASSERT(IS(ATM, A(D_(D_(A(D_(x->s)))))));
	TEST_ASSERT_EQUAL_INT(11, A(A(D_(D_(A(D_(x->s)))))));
	TEST_ASSERT(IS(ATM, D_(A(D_(D_(A(D_(x->s))))))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(D_(D_(A(D_(x->s))))))));

	TEST_ASSERT_NOT_EQUAL_INT(x->s, D_(x->s));
	TEST_ASSERT_NOT_EQUAL_INT(A(x->s), A(D_(x->s)));
	TEST_ASSERT_NOT_EQUAL_INT(D_(A(x->s)), D_(A(D_(x->s))));
	TEST_ASSERT_NOT_EQUAL_INT(D_(D_(A(x->s))), D_(D_(A(D_(x->s)))));
	TEST_ASSERT_NOT_EQUAL_INT(A(D_(D_(A(x->s)))), A(D_(D_(A(D_(x->s))))));
	TEST_ASSERT_NOT_EQUAL_INT(D_(A(D_(D_(A(x->s))))), D_(A(D_(D_(A(D_(x->s)))))));
}

void test_swap_1() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_quote(x);
	_swap(x);
	TEST_ASSERT_EQUAL_INT(2, length(x->s));
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT_EQUAL_INT(11, A(x->s));
	TEST_ASSERT(IS(LST, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(1, length(A(D_(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(A(D_(x->s))));
}

void test_swap_2() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_quote(x);
	_swap(x);
	TEST_ASSERT_EQUAL_INT(3, length(x->s));
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT_EQUAL_INT(11, A(x->s));
	TEST_ASSERT(IS(LST, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(1, length(A(D_(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(A(D_(x->s))));
	TEST_ASSERT(IS(ATM, D_(D_(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(x->s))));
}

void test_add() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	push(x, ATM, 11);
	_add(x);
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT_EQUAL_INT(18, A(x->s));
}

void test_sub() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_sub(x);
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT_EQUAL_INT(4, A(x->s));
}

void test_mul() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_mul(x);
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT_EQUAL_INT(77, A(x->s));
}

void test_div() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 77);
	push(x, ATM, 11);
	_div(x);
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(x->s));
}

void test_mod() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_mod(x);
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	TEST_ASSERT_EQUAL_INT(4, A(x->s));
}

void test_gt() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 7);
	_gt(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 13);
	_gt(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 7);
	_gt(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);
}

void test_lt() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 7);
	_lt(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 13);
	_lt(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 7);
	_lt(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);
}

void test_eq() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13),
	push(x, ATM, 7);
	_eq(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 13);
	_eq(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 7);
	_eq(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);
}

void test_neq() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 7);
	_neq(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 13);
	_neq(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 7);
	_neq(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, length(x->s));
	pop(x);
}

///void test_drop() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	push(x, ATM, 7);
//	_drop(x);
//
//	TEST_ASSERT_EQUAL_INT(0, length(x->s));
//	TEST_ASSERT_EQUAL_INT(free_nodes(x), height(x->f));
//
//	push(x, ATM, 7);
//	_quote(x);
//	_drop(x);
//
//	TEST_ASSERT_EQUAL_INT(0, length(x->s));
//	TEST_ASSERT_EQUAL_INT(free_nodes(x), height(x->f));
//}
//
////void test_clear_stack() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 3;
////
////	TEST_ASSERT_EQUAL_INT(free_nodes, height(x->f));
////
////	push(x, 13);
////	push(x, 7);
////	
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(x->f));
////
////	_sclear(x);
////
////	TEST_ASSERT_EQUAL_INT(0, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(free_nodes, height(x->f));
////}
////
////void test_push_stack() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 3;
////
////	push(x, 13);
////	push(x, 7);
////
////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(x->f));
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////
////	_spush(x);
////
////	TEST_ASSERT_EQUAL_INT(free_nodes - 3, height(x->f));
////	TEST_ASSERT_EQUAL_INT(7, A(D(P(x))));
////	TEST_ASSERT_EQUAL_INT(13, A(D(D(P(x)))));
////	TEST_ASSERT_EQUAL_INT(2, height(P(x)));
////	TEST_ASSERT_EQUAL_INT(R(x), A(P(x)));
////	TEST_ASSERT_EQUAL_INT(0, length(K(x)));
////}
////
////void test_drop_stack() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 3;
////
////	_sdrop(x);
////
////	TEST_ASSERT_EQUAL_INT(0, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(1, length(P(x)));
////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
////	TEST_ASSERT_EQUAL_INT(free_nodes, height(x->f));
////
////	push(x, 13);
////	push(x, 7);
////
////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(x->f));
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(7, T(x));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
////
////	_spush(x);
////
////	TEST_ASSERT_EQUAL_INT(free_nodes - 3, height(x->f));
////
////	push(x, 21);
////
////	TEST_ASSERT_EQUAL_INT(free_nodes - 4, height(x->f));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(21, T(x));
////
////	_sdrop(x);
////
////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(x->f));
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(7, T(x));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
////}
////
////void test_over() {
////	C size = 256;
////	B block[size];
////	X* x = init(block, size);
////
////	push(x, 13);
////	push(x, 7);
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(7, T(x));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////	_over(x);
////	TEST_ASSERT_EQUAL_INT(3, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(13, T(x));
////	TEST_ASSERT_EQUAL_INT(7, x->s);
////	TEST_ASSERT_EQUAL_INT(13, A(D(D(K(x)))));
////}
////
////void test_rot() {
////	C size = 256;
////	B block[size];
////	X* x = init(block, size);
////
////	push(x, 21);
////	push(x, 13);
////	push(x, 7);
////	TEST_ASSERT_EQUAL_INT(3, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(7, T(x));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////	TEST_ASSERT_EQUAL_INT(21, A(D(D(K(x)))));
////	_rot(x);
////	TEST_ASSERT_EQUAL_INT(3, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(21, T(x));
////	TEST_ASSERT_EQUAL_INT(7, x->s);
////	TEST_ASSERT_EQUAL_INT(13, A(D(D(K(x)))));
////}
////
////void test_drop() {
////	C size = 256;
////	B block[size];
////	X* x = init(block, size);
////
////	push(x, 21);
////	push(x, 13);
////	push(x, 7);
////	TEST_ASSERT_EQUAL_INT(3, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(7, T(x));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////	TEST_ASSERT_EQUAL_INT(21, A(D(D(K(x)))));
////	_drop(x);
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(13, T(x));
////	TEST_ASSERT_EQUAL_INT(21, x->s);
////	_drop(x);
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(21, T(x));
////	_drop(x);
////	TEST_ASSERT_EQUAL_INT(0, length(K(x)));
////	_drop(x);
////	TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW, x->err);
////}
////
////void test_rev() {
////	C size = 256;
////	B block[size];
////	X* x = init(block, size);
////
////	push(x, 21);
////	push(x, 13);
////	push(x, 7);
////	push(x, 5);
////	push(x, 3);
////	TEST_ASSERT_EQUAL_INT(5, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(3, T(x));
////	TEST_ASSERT_EQUAL_INT(5, x->s);
////	TEST_ASSERT_EQUAL_INT(7, A(D(D(K(x)))));
////	TEST_ASSERT_EQUAL_INT(13, A(D(D(D(K(x))))));
////	TEST_ASSERT_EQUAL_INT(21, A(D(D(D(D(K(x)))))));
////	_rev(x);
////	TEST_ASSERT_EQUAL_INT(5, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(21, T(x));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////	TEST_ASSERT_EQUAL_INT(7, A(D(D(K(x)))));
////	TEST_ASSERT_EQUAL_INT(5, A(D(D(D(K(x))))));
////	TEST_ASSERT_EQUAL_INT(3, A(D(D(D(D(K(x)))))));
////}
////
////void test_binops() {
////	C size = 256;
////	B block[size];
////	X* x = init(block, size);
////
////	push(x, 7);
////	push(x, 13);
////	_add(x);
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(20, T(x));
////	pop(x);
////
////	push(x, 13);
////	push(x, 7);
////	_sub(x);
////	TEST_ASSERT_EQUAL_INT(6, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 13);
////	push(x, 7);
////	_mul(x);
////	TEST_ASSERT_EQUAL_INT(91, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 30);
////	push(x, 5);
////	_div(x);
////	TEST_ASSERT_EQUAL_INT(6, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 9);
////	push(x, 2);
////	_mod(x);
////	TEST_ASSERT_EQUAL_INT(T(x), 1);
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 13);
////	push(x, 7);
////	_gt(x);
////	TEST_ASSERT_EQUAL_INT(1, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 7);
////	push(x, 13);
////	_gt(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 7);
////	push(x, 7);
////	_gt(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 13);
////	push(x, 7);
////	_lt(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 7);
////	push(x, 13);
////	_lt(x);
////	TEST_ASSERT_EQUAL_INT(1, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 7);
////	push(x, 7);
////	_lt(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 13),
////	push(x, 7);
////	_eq(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 7);
////	push(x, 13);
////	_eq(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 7);
////	push(x, 7);
////	_eq(x);
////	TEST_ASSERT_EQUAL_INT(1, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 13);
////	push(x, 7);
////	_neq(x);
////	TEST_ASSERT_EQUAL_INT(1, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 7);
////	push(x, 13);
////	_neq(x);
////	TEST_ASSERT_EQUAL_INT(1, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 7);
////	push(x, 7);
////	_neq(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 0);
////	push(x, 0);
////	_and(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 0);
////	push(x, 1);
////	_and(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 1);
////	push(x, 0);
////	_and(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 1);
////	push(x, 1);
////	_and(x);
////	TEST_ASSERT_EQUAL_INT(1, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 0);
////	push(x, 0);
////	_or(x);
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 0);
////	push(x, 1);
////	_or(x);
////	TEST_ASSERT_EQUAL_INT(1, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 1);
////	push(x, 0);
////	_or(x);
////	TEST_ASSERT_EQUAL_INT(1, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////
////	push(x, 1);
////	push(x, 1);
////	_or(x);
////	TEST_ASSERT_EQUAL_INT(1, T(x));
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	pop(x);
////}
////
////void test_interpreter_atom() {
////	C size = 256;
////	B block[size];
////	X* x = init(block, size);
////
////	C code = ATM(x, 13, ATM(x, 7, 0));
////	inner(x, code);
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(7, T(x));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////}
////
////void test_interpreter_primitive() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C code = ATM(x, 13, ATM(x, 7, PRM(x, &_add, PRM(x, &_dup, 0))));
////	inner(x, code);
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(20, T(x));
////	TEST_ASSERT_EQUAL_INT(20, x->s);
////}
////
////void test_interpreter_branch() {
////	C size = 256;
////	B block[size];
////	X* x = init(block, size);
////
////	C code = BRN(x, ATM(x, 7, 0), ATM(x, 13, 0), 0);
////
////	push(x, 1);
////	inner(x, code);
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(7, T(x));
////	pop(x);
////
////	push(x, 0);
////	inner(x, code);
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(13, T(x));
////}
////
////void test_interpreter_continued_branch() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C code = BRN(x, ATM(x, 7, 0), ATM(x, 13, 0), ATM(x, 21, 0));
////
////	push(x, 1);
////	inner(x, code);
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(21, T(x));
////	TEST_ASSERT_EQUAL_INT(7, x->s);
////	pop(x);
////	pop(x);
////
////	push(x, 0);
////	inner(x, code);
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(21, T(x));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////}
////
////void test_interpreter_recursion() {
////	C size = 1024;
////	B block[size];
////	X* x = init(block, size);
////
////	C code = 
////		PRM(x, &_dup,
////		ATM(x, 0,
////		PRM(x, &_gt,
////		BRN(x,
////			// True branch
////			ATM(x, 1,
////			PRM(x, &_sub,
////			PRM(x, &_swap,
////			ATM(x, 2,
////			PRM(x, &_add,
////			PRM(x, &_swap,
////			RECURSION(x, 0))))))),
////			// False branch
////			0,
////		0))));
////		
////	push(x, 0);
////	push(x, 5);
////	inner(x, code);
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(0, T(x));
////	TEST_ASSERT_EQUAL_INT(10, x->s);
////}
////
////void test_interpreter_word() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	C square = PRM(x, &_dup, PRM(x, &_mul, 0));
////
////	// Tail call
////	C code = ATM(x, 5, WORD(x, square, 0));
////
////	inner(x, code);
////	TEST_ASSERT_EQUAL_INT(1, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(25, T(x));
////	pop(x);
////
////	// Non tail call
////	code = ATM(x, 3, WORD(x, square, ATM(x, 13, 0)));
////
////	inner(x, code);
////	TEST_ASSERT_EQUAL_INT(2, length(K(x)));
////	TEST_ASSERT_EQUAL_INT(13, T(x));
////	TEST_ASSERT_EQUAL_INT(9, x->s);
////}
////
//////////void test_allot() {
//////////	C size = 256;
//////////	B block[size];
//////////	X* ctx = init(block, size);
//////////
//////////	C free_nodes = (size - sizeof(X)) / sizeof(PAIR);
//////////
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->here - BOTTOM(ctx));
//////////	TEST_ASSERT_EQUAL_INT(free_nodes, length(ctx->free));
//////////	allot(ctx, 3);
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_INT(3, ctx->here - BOTTOM(ctx));
//////////	TEST_ASSERT_EQUAL_INT(BOTTOM(ctx) + 3 + RESERVED(ctx), ctx->there);
//////////	allot(ctx, 13);
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_INT(16, ctx->here - BOTTOM(ctx));
//////////	allot(ctx, 1);
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_INT(17, ctx->here - BOTTOM(ctx));
//////////	TEST_ASSERT_LESS_THAN(free_nodes, length(ctx->free));
//////////
//////////	C fnodes = length(ctx->free);
//////////	B* here = ctx->here;
//////////	C reserved = RESERVED(ctx);
//////////
//////////	// Ensure reserved memory is 0 to allow next tests to pass
//////////	allot(ctx, RESERVED(ctx));
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_PTR(here + reserved, ctx->here);
//////////	TEST_ASSERT_EQUAL_INT(0, RESERVED(ctx));
//////////	TEST_ASSERT_EQUAL_INT(fnodes, length(ctx->free));
//////////
//////////	allot(ctx, sizeof(C) * 8);
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*8, ctx->here);
//////////	TEST_ASSERT_EQUAL_INT(fnodes - 4, length(ctx->free));
//////////
//////////	allot(ctx, -(sizeof(C) * 4));
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*4, ctx->here);
//////////	TEST_ASSERT_EQUAL_INT(fnodes - 2, length(ctx->free));
//////////	allot(ctx, -(sizeof(C) - 1));
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*3 + 1, ctx->here);
//////////	TEST_ASSERT_EQUAL_INT(fnodes - 2, length(ctx->free));
//////////	allot(ctx, -(sizeof(C) + 1));
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*2, ctx->here);
//////////	TEST_ASSERT_EQUAL_INT(fnodes - 1, length(ctx->free));
//////////
//////////	allot(ctx, 2048);
//////////	TEST_ASSERT_EQUAL_INT(ERR_NOT_ENOUGH_MEMORY, ctx->err);
//////////}
//////////
//////////void test_align() {
//////////	C size = 256;
//////////	B block[size];
//////////	X* ctx = init(block, size);
//////////
//////////	// This ensures here will be aligned with a pair 
//////////	allot(ctx, RESERVED(ctx));
//////////
//////////	allot(ctx, 1);
//////////	TEST_ASSERT_NOT_EQUAL_INT(ctx->here, ALIGN(ctx->here, sizeof(C)));
//////////
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////
//////////	align(ctx);
//////////	TEST_ASSERT_EQUAL_INT(ctx->here, ALIGN(ctx->here, sizeof(C)));
//////////
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////}
//////////
//////////void test_stack_to_list() {
//////////}
//////////
////////////void test_append() {
////////////}
////////////
////////////void test_copy() {
////////////	C size = 256;
////////////	B block[size];
////////////	X* ctx = init(block, size);
////////////
////////////	C free_nodes = (size - sizeof(X)) / sizeof(PAIR);
////////////
////////////	// TODO
////////////}
//////////
//////////void test_header_body_reveal() {
//////////	C size = 2048;
//////////	B block[size];
//////////	X* ctx = init(block, size);
//////////
//////////	B* here = ctx->here;
//////////	PAIR* w = header(ctx, "test", 4);
//////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////////	TEST_ASSERT_EQUAL_PTR(ctx->here, ALIGN(here, sizeof(C)) + sizeof(C) + 4 + 1);
//////////	TEST_ASSERT(IS(ATM, w));
//////////	TEST_ASSERT_EQUAL_INT(4, COUNT(NFA(w)));
//////////	TEST_ASSERT_EQUAL_STRING("test", NFA(w));
//////////	TEST_ASSERT_EQUAL_INT(((B*)REF(REF(w))) + 4, DFA(w));
//////////	TEST_ASSERT_EQUAL_INT(0, CFA(w));
//////////
//////////	TEST_ASSERT_NULL(ctx->dict);
//////////
//////////	PAIR* cfa = cons(ctx, ATM, 7, cons(ctx, ATM, 13, 0));
//////////	body(ctx, w, cfa);
//////////
//////////	TEST_ASSERT_EQUAL_INT(cfa, CFA(w));
//////////	TEST_ASSERT_EQUAL_INT(7, CFA(w)->value);
//////////	TEST_ASSERT_EQUAL_INT(13, NEXT(CFA(w))->value);
//////////
//////////	reveal(ctx, w);
//////////
//////////	TEST_ASSERT_EQUAL_PTR(w, ctx->dict);
//////////	TEST_ASSERT_NULL(NEXT(ctx->dict));
//////////
//////////	TEST_ASSERT_FALSE(IS_IMMEDIATE(w));
//////////
//////////	_immediate(ctx);
//////////
//////////	TEST_ASSERT_TRUE(IS_IMMEDIATE(w));
//////////}
//////////
////////////void test_find() {
////////////	C size = 512;
////////////	B block[size];
////////////	X* ctx = init(block, size);
////////////
////////////	PAIR* dup = reveal(ctx, header(ctx, "dup", 3));
////////////	PAIR* swap = reveal(ctx, header(ctx, "swap", 4));
////////////	PAIR* test = reveal(ctx, header(ctx, "test", 4));
////////////
////////////	TEST_ASSERT_EQUAL_PTR(dup, find(ctx, "dup", 3));
////////////	TEST_ASSERT_EQUAL_PTR(swap, find(ctx, "swap", 4));
////////////	TEST_ASSERT_EQUAL_PTR(test, find(ctx, "test", 4));
////////////	TEST_ASSERT_EQUAL_PTR(0, find(ctx, "nop", 3));
////////////
////////////	TEST_ASSERT_EQUAL_PTR(test, ctx->dict);
////////////	TEST_ASSERT_EQUAL_PTR(swap, NEXT(ctx->dict));
////////////	TEST_ASSERT_EQUAL_PTR(dup, NEXT(NEXT(ctx->dict)));
////////////	TEST_ASSERT_EQUAL_PTR(0, NEXT(NEXT(NEXT(ctx->dict))));
////////////}
////////////
////////////void test_dodo_initialization() {
////////////	C size = 4096;
////////////	B block[size];
////////////	X* ctx = dodo(init(block, size));
////////////
////////////	TEST_ASSERT_EQUAL_PTR(&_add, CFA(find(ctx, "+", 1))->value);
////////////}
////////////
////////////void test_fib() {
////////////	C size = 8192;
////////////	B block[size];
////////////	X* ctx = init(block, size);
////////////
////////////	// : fib dup 1 > if 1- dup 1- recurse swap recurse + then ;
////////////
////////////	ctx->ip =
////////////		cons(ctx, (C)&_dup, T_PRM,
////////////		cons(ctx, 1, ATM,
////////////		cons(ctx, (C)&_gt, T_PRM,
////////////		cons(ctx,
////////////			0,
////////////			T_BRN,
////////////			cons(ctx, 1, ATM,
////////////			cons(ctx, (C)&_sub, T_PRM,
////////////			cons(ctx, (C)&_dup, T_PRM,
////////////			cons(ctx, 1, ATM,
////////////			cons(ctx, (C)&_sub, T_PRM,
////////////			cons(ctx, (C)&_rec, T_PRM,
////////////			cons(ctx, (C)&_swap, T_PRM,
////////////			cons(ctx, (C)&_rec, T_PRM,	
////////////			cons(ctx, (C)&_add, T_PRM, 0)))))))))))));
////////////
////////////		ctx->dstack = cons(ctx, 6, ATM, 0);
////////////		ctx->rstack = cons(ctx, (C)ctx->ip, T_WORD, 0);
////////////
////////////		inner(ctx);
////////////
////////////		//printf("%ld\n", ctx->dstack->value);
////////////}
//////////
int main() {
	UNITY_BEGIN();

	RUN_TEST(test_types);

	RUN_TEST(test_length);
	RUN_TEST(test_depth);
	RUN_TEST(test_min_length);
	RUN_TEST(test_last);

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_cons);
	RUN_TEST(test_reclaim);
	RUN_TEST(test_clone);

	RUN_TEST(test_push);
	RUN_TEST(test_pop);

	RUN_TEST(test_empty);

	RUN_TEST(test_join_atom_atom_1);
	RUN_TEST(test_join_atom_atom_2);
	RUN_TEST(test_join_atom_empty_1);
	RUN_TEST(test_join_atom_empty_2);
	RUN_TEST(test_join_atom_list_1);
	RUN_TEST(test_join_atom_list_2);
	RUN_TEST(test_join_list_atom_1);
	RUN_TEST(test_join_list_atom_2);
	RUN_TEST(test_join_list_list_1);
	RUN_TEST(test_join_list_list_2);

	RUN_TEST(test_quote_1);
	RUN_TEST(test_quote_2);
	RUN_TEST(test_quote_3);
	RUN_TEST(test_quote_4);

	RUN_TEST(test_dup_atom);
	RUN_TEST(test_dup_list);

	RUN_TEST(test_swap_1);
	RUN_TEST(test_swap_2);

	RUN_TEST(test_add);
	RUN_TEST(test_sub);
	RUN_TEST(test_mul);
	RUN_TEST(test_div);
	RUN_TEST(test_mod);

	RUN_TEST(test_gt);
	RUN_TEST(test_lt);
	RUN_TEST(test_eq);
	RUN_TEST(test_neq);

		//RUN_TEST(test_drop);

	//RUN_TEST(test_clear_stack);
	//RUN_TEST(test_push_stack);
	//RUN_TEST(test_drop_stack);

	//RUN_TEST(test_over);
	//RUN_TEST(test_rot);
	//RUN_TEST(test_drop);
	//RUN_TEST(test_rev);
	//RUN_TEST(test_binops);

	//RUN_TEST(test_interpreter_atom);
	//RUN_TEST(test_interpreter_primitive);
	//RUN_TEST(test_interpreter_branch);
	//RUN_TEST(test_interpreter_continued_branch);
	//RUN_TEST(test_interpreter_recursion);
	//RUN_TEST(test_interpreter_word);

//	//RUN_TEST(test_allot);
//	//RUN_TEST(test_align);
//
//	//RUN_TEST(test_stack_to_list);
//
//	////RUN_TEST(test_fib);
//
//	////RUN_TEST(test_append);
//	////RUN_TEST(test_copy);
//
//
//	//RUN_TEST(test_header_body_reveal);
//	////RUN_TEST(test_find);
//
//	////RUN_TEST(test_dodo_initialization);
//
	return UNITY_END();
}
