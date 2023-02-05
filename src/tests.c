#include<stdlib.h>
#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

// TAGGED POINTER BASED TYPING INFORMATION

void test_types() {
	C p = (C)malloc(sizeof(C) * 2);

	D(p) = T(ATM, 8);
	TEST_ASSERT_EQUAL_INT(8, D_(p));
	TEST_ASSERT_EQUAL_INT(ATM, _D(p));
	TEST_ASSERT(IS(ATM, p));
	TEST_ASSERT(!IS(LST, p));
	TEST_ASSERT(!IS(JMP, p));
	TEST_ASSERT(!IS(PRM, p));
	TEST_ASSERT(!REF(p));
	D(p) = T(LST, 8);
	TEST_ASSERT_EQUAL_INT(8, D_(p));
	TEST_ASSERT_EQUAL_INT(LST, _D(p));
	TEST_ASSERT(!IS(ATM, p));
	TEST_ASSERT(IS(LST, p));
	TEST_ASSERT(!IS(JMP, p));
	TEST_ASSERT(!IS(PRM, p));
	TEST_ASSERT(REF(p));
	D(p) = T(JMP, 8);
	TEST_ASSERT_EQUAL_INT(8, D_(p));
	TEST_ASSERT_EQUAL_INT(JMP, _D(p));
	TEST_ASSERT(!IS(ATM, p));
	TEST_ASSERT(!IS(LST, p));
	TEST_ASSERT(IS(JMP, p));
	TEST_ASSERT(!IS(PRM, p));
	TEST_ASSERT(REF(p));
	D(p) = T(PRM, 8);
	TEST_ASSERT_EQUAL_INT(8, D_(p));
	TEST_ASSERT_EQUAL_INT(PRM, _D(p));
	TEST_ASSERT(!IS(ATM, p));
	TEST_ASSERT(!IS(LST, p));
	TEST_ASSERT(!IS(JMP, p));
	TEST_ASSERT(IS(PRM, p));
	TEST_ASSERT(!REF(p));
}

//  LIST FUNCTIONS

void test_lth() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);

	D(p1) = T(ATM, p2);
	D(p2) = p3;
	D(p3) = 0;

	TEST_ASSERT_EQUAL_INT(0, lth(0));
	TEST_ASSERT_EQUAL_INT(1, lth(p3));
	TEST_ASSERT_EQUAL_INT(2, lth(p2));
	TEST_ASSERT_EQUAL_INT(3, lth(p1));
}

void test_dth() {
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

	TEST_ASSERT_EQUAL_INT(3, lth(p1));
	TEST_ASSERT_EQUAL_INT(2, lth(A(p2)));
	TEST_ASSERT_EQUAL_INT(5, dth(p1));
	TEST_ASSERT_EQUAL_INT(4, dth(p2));
	TEST_ASSERT_EQUAL_INT(2, dth(p4));
	TEST_ASSERT_EQUAL_INT(1, dth(p3));
	TEST_ASSERT_EQUAL_INT(1, dth(p5));
	TEST_ASSERT_EQUAL_INT(0, dth(0));
}

void test_mlth() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);

	D(p1) = p2;
	D(p2) = p3;
	D(p3) = 0;

	TEST_ASSERT(mlth(p1, 3));
	TEST_ASSERT(mlth(p1, 2));
	TEST_ASSERT(mlth(p1, 1));
	TEST_ASSERT(mlth(p1, 0));
	TEST_ASSERT(!mlth(p2, 3));
	TEST_ASSERT(mlth(p2, 2));
	TEST_ASSERT(mlth(p2, 1));
	TEST_ASSERT(mlth(p2, 0));
	TEST_ASSERT(!mlth(p3, 3));
	TEST_ASSERT(!mlth(p3, 2));
	TEST_ASSERT(mlth(p3, 1));
	TEST_ASSERT(mlth(p3, 0));
}

void test_lst() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);

	D(p1) = p2;
	D(p2) = p3;
	D(p3) = 0;

	TEST_ASSERT_EQUAL_INT(0, lst(0));
	TEST_ASSERT_EQUAL_INT(p3, lst(p1));
	TEST_ASSERT_EQUAL_INT(p3, lst(p2));
	TEST_ASSERT_EQUAL_INT(p3, lst(p3));
}

void test_rev() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);

	D(p1) = p2;
	D(p2) = p3;
	D(p3) = 0;
	C l = rev(p1, 0);

	TEST_ASSERT_EQUAL_INT(3, lth(l));
	TEST_ASSERT_EQUAL_INT(p3, l);
	TEST_ASSERT_EQUAL_INT(p2, D_(l));
	TEST_ASSERT_EQUAL_INT(p1, D_(D_(l)));
}

// CONTEXT

#define free_nodes(x)			((x->size - sizeof(X)) / (2*sizeof(C)))

void test_block_size() {
	C size = 2;
	B block[size];
	X* x = init(block, size);

	TEST_ASSERT_EQUAL_PTR(0, x);
}

void test_block_initialization() {
	C size = 512;
	B block[size];
	X* x = init(block, size);
		
	TEST_ASSERT_NOT_EQUAL(0, x);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(0, lth(x->s));
	TEST_ASSERT_EQUAL_INT(free_nodes(x), lth(x->f));

	TEST_ASSERT_EQUAL_INT(((B*)x) + sizeof(X), BOTTOM(x));
	TEST_ASSERT_EQUAL_INT(BOTTOM(x), x->here);
	TEST_ASSERT_EQUAL_INT(ALIGN(x->here, 2*sizeof(C)), x->there);
	TEST_ASSERT_EQUAL_INT(ALIGN(((B*)x) + size - 2*sizeof(C) - 1, 2*sizeof(C)), TOP(x));
	TEST_ASSERT_EQUAL_INT(TOP(x), x->f);

	TEST_ASSERT_EQUAL_INT(0, x->dict);
	TEST_ASSERT_EQUAL_INT(0, x->s);
	TEST_ASSERT_EQUAL_INT(0, x->r);
	TEST_ASSERT_EQUAL_INT(0, x->cf);
	TEST_ASSERT_EQUAL_INT(0, x->cp);
	TEST_ASSERT_EQUAL_INT(0, x->st);
}

// LIST CREATION AND DESTRUCTION (AUTOMATIC MEMORY MANAGEMENT)

void test_cns() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C l = cns(x, 7, cns(x, 11, cns(x, 13, 0)));

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 3, lth(x->f));
	TEST_ASSERT_EQUAL_INT(3, lth(l));
	TEST_ASSERT_EQUAL_INT(7, A(l));
	TEST_ASSERT_EQUAL_INT(11, A(D(l)));
	TEST_ASSERT_EQUAL_INT(13, A(D(D(l))));

	C p = cns(x, 7, 0);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 4, lth(x->f));
	TEST_ASSERT_EQUAL_INT(7, A(p));
	TEST_ASSERT_EQUAL_INT(0, D(p));
	
	while (lth(x->f) > 0) { cns(x, 1, 0); }

	C p3 = cns(x, 13, 0);
	TEST_ASSERT_EQUAL_INT(0, p3);
}

void test_rcl() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C l = cns(x, 7, cns(x, 11, cns(x, 13, 0)));
	C m = rcl(x, l);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 2, lth(x->f));
	TEST_ASSERT_EQUAL_INT(2, lth(m));
	TEST_ASSERT_EQUAL_INT(11, A(m));
	TEST_ASSERT_EQUAL_INT(13, A(D(m)));
}

void test_cln() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C l = 
		cns(x, 7, T(ATM, 
		cns(x, 11, T(PRM, 
		cns(x, 
			cns(x, 13, T(ATM,
			cns(x, 17, T(ATM, 0)))), T(LST,
		cns(x, 19, T(ATM, 0))))))));

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 6, lth(x->f));
	TEST_ASSERT_EQUAL_INT(4, lth(l));
	TEST_ASSERT(IS(ATM, l));
	TEST_ASSERT_EQUAL_INT(7, A(l));
	TEST_ASSERT(IS(PRM, D_(l)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(l)));
	TEST_ASSERT(IS(LST, D_(D_(l))));
	TEST_ASSERT_EQUAL_INT(2, lth(D_(D_(l))));
	TEST_ASSERT(IS(ATM, A(D_(D_(l)))));
	TEST_ASSERT_EQUAL_INT(13, A(A(D_(D_(l)))));
	TEST_ASSERT(IS(ATM, D_(A(D_(D_(l))))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(A(D_(D_(l))))));
	TEST_ASSERT(IS(ATM, D_(D_(D_(l)))));
	TEST_ASSERT_EQUAL_INT(19, A(D_(D_(D_(l)))));

	C c = cln(x, l);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 12, lth(x->f));
	TEST_ASSERT_EQUAL_INT(4, lth(c));
	TEST_ASSERT(IS(ATM, c));
	TEST_ASSERT_EQUAL_INT(7, A(c));
	TEST_ASSERT(IS(PRM, D_(c)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(c)));
	TEST_ASSERT(IS(LST, D_(D_(c))));
	TEST_ASSERT_EQUAL_INT(2, lth(D_(D_(c))));
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

void test_rmv() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C l = 
		cns(x, 
			cns(x, 
				cns(x, 7, T(ATM, 
				cns(x, 11, T(ATM, 0)))), T(LST, 
			cns(x, 13, T(ATM, 0)))), T(LST, 
		0));

	rmv(x, l);
	TEST_ASSERT_EQUAL_INT(0, lth(x->s));
	TEST_ASSERT_EQUAL_INT(free_nodes(x), lth(x->f));
}

// BASIC STACK OPERATIONS

void test_push() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT_EQUAL_INT(11, A(x->s));
	push(x, ATM, 7);
	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT_EQUAL_INT(7, A(x->s));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(x->s)));
}

void test_pop() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	C v = pop(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(7, v);
	v = pop(x);
	TEST_ASSERT_EQUAL_INT(0, lth(x->s));
	TEST_ASSERT_EQUAL_INT(11, v);
}

// COMPILATION PILE OPERATIONS

void test_cppush() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	TEST_ASSERT_EQUAL_INT(0, x->cp);
	TEST_ASSERT_EQUAL_INT(0, lth(x->cp));

	cppush(x);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 1, lth(x->f));
	TEST_ASSERT_EQUAL_INT(1, lth(x->cp));
	TEST_ASSERT(IS(LST, x->cp));
	TEST_ASSERT_EQUAL_INT(0, lth(A(x->cp)));

	cppush(x);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 2, lth(x->f));
	TEST_ASSERT_EQUAL_INT(2, lth(x->cp));
	TEST_ASSERT(IS(LST, x->cp));
	TEST_ASSERT_EQUAL_INT(0, lth(A(x->cp)));
	TEST_ASSERT(IS(LST, D_(x->cp)));
	TEST_ASSERT_EQUAL_INT(0, lth(A(D_(x->cp))));
}

void test_cppop_1() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	cppush(x);
	cppop(x);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 1, lth(x->f));
	TEST_ASSERT_EQUAL_INT(0, lth(x->cp));
	TEST_ASSERT_EQUAL_INT(0, x->cp);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(0, lth(A(x->s)));
}

void test_cspush() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	cppush(x);
	cspush(x, cns(x, 7, T(ATM, 0)));

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 2, lth(x->f));
	TEST_ASSERT_EQUAL_INT(1, lth(x->cp));
	TEST_ASSERT_EQUAL_INT(1, lth(A(x->cp)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->cp)));

	cspush(x, cns(x, 11, T(ATM, 0)));

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 3, lth(x->f));
	TEST_ASSERT_EQUAL_INT(1, lth(x->cp));
	TEST_ASSERT_EQUAL_INT(2, lth(A(x->cp)));
	TEST_ASSERT(IS(ATM, A(x->cp)));
	TEST_ASSERT_EQUAL_INT(11, A(A(x->cp)));
	TEST_ASSERT(IS(ATM, D_(A(x->cp))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(x->cp))));

	cppush(x);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 4, lth(x->f));
	TEST_ASSERT_EQUAL_INT(2, lth(x->cp));
	TEST_ASSERT_EQUAL_INT(0, lth(A(x->cp)));
	TEST_ASSERT_EQUAL_INT(2, lth(A(D_(x->cp))));
	TEST_ASSERT(IS(ATM, A(D_(x->cp))));
	TEST_ASSERT_EQUAL_INT(11, A(A(D_(x->cp))));
	TEST_ASSERT(IS(ATM, D_(A(D_(x->cp)))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(D_(x->cp)))));

	cspush(x, cns(x, 13, T(ATM, 0)));

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 5, lth(x->f));
	TEST_ASSERT_EQUAL_INT(2, lth(x->cp));
	TEST_ASSERT_EQUAL_INT(1, lth(A(x->cp)));
	TEST_ASSERT(IS(ATM, A(x->cp)));
	TEST_ASSERT_EQUAL_INT(13, A(A(x->cp)));
	TEST_ASSERT_EQUAL_INT(2, lth(A(D_(x->cp))));
	TEST_ASSERT(IS(ATM, A(D_(x->cp))));
	TEST_ASSERT_EQUAL_INT(11, A(A(D_(x->cp))));
	TEST_ASSERT(IS(ATM, D_(A(D_(x->cp)))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(D_(x->cp)))));
}

void test_cppop_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	cppush(x);
	cspush(x, cns(x, 7, T(ATM, 0)));
	cspush(x, cns(x, 11, T(ATM, 0)));
	cppush(x);
	cspush(x, cns(x, 13, T(ATM, 0)));
	cppop(x);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 5, lth(x->f));
	TEST_ASSERT_EQUAL_INT(1, lth(x->cp));
	TEST_ASSERT_EQUAL_INT(3, lth(A(x->cp)));
	TEST_ASSERT(IS(LST, A(x->cp)));
	TEST_ASSERT_EQUAL_INT(1, lth(A(A(x->cp))));
	TEST_ASSERT_EQUAL_INT(13, A(A(A(x->cp))));
	TEST_ASSERT(IS(ATM, D_(A(x->cp))));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->cp))));
	TEST_ASSERT(IS(ATM, D_(D_(A(x->cp)))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(D_(A(x->cp)))));

	cppop(x);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 5, lth(x->f));
	TEST_ASSERT_EQUAL_INT(0, lth(x->cp));
	TEST_ASSERT_EQUAL_INT(0, x->cp);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, lth(A(x->s)));
	TEST_ASSERT(IS(LST, A(x->s)));
	TEST_ASSERT_EQUAL_INT(1, lth(A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(A(A(x->s))));
	TEST_ASSERT(IS(ATM, D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT(IS(ATM, D_(D_(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(D_(A(x->s)))));
}

void test_empty() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	_empty(x);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
}

void test_join_atom_atom_1() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(2, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
}

void test_join_atom_atom_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(2, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(x->s)));
}

void test_join_atom_empty_1() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	_empty(x);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
}

void test_join_atom_empty_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);

	_empty(x);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(13, A(D_(x->s)));
}

void test_join_atom_list_1() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 11);
	_join(x);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
}

void test_join_atom_list_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 17);

	push(x, ATM, 13);
	push(x, ATM, 11);
	_join(x);
	push(x, ATM, 7);
	_join(x);

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(D_(x->s)));
}

void test_join_list_atom_1() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
}

void test_join_list_atom_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 17);

	push(x, ATM, 13);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_join(x);

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(D_(x->s)));
}

void test_join_list_list_1() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 17);
	push(x, ATM, 13);
	_join(x);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_join(x);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(4, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(D_(D_(A(x->s))))));
}

void test_join_list_list_2() {
	C size = 512;
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

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(4, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(19, A(D_(x->s)));
}

void test_quote_1() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	_quote(x);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
}

void test_quote_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);

	push(x, ATM, 7);
	_quote(x);

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(x->s)));
}

void test_quote_3() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_quote(x);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(A(x->s)));
	TEST_ASSERT(IS(LST, A(x->s)));
	TEST_ASSERT_EQUAL_INT(2, lth(A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(A(x->s)))));
}

void test_quote_4() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_join(x);
	_quote(x);

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(A(x->s)));
	TEST_ASSERT(IS(LST, A(x->s)));
	TEST_ASSERT_EQUAL_INT(2, lth(A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(A(x->s)))));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(13, A(D_(x->s)));
}

void test_dup_atom() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	_dup(x);

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 2, lth(x->f));
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT(IS(ATM, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(D_(x->s)));

	push(x, ATM, 11);
	_dup(x);

	TEST_ASSERT_EQUAL_INT(4, lth(x->s));
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 4, lth(x->f));
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
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 4, lth(x->f));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(A(x->s)));
	TEST_ASSERT(IS(LST, A(x->s)));
	TEST_ASSERT_EQUAL_INT(2, lth(A(A(x->s))));
	TEST_ASSERT(IS(ATM, A(A(x->s))));
	TEST_ASSERT_EQUAL_INT(11, A(A(A(x->s))));
	TEST_ASSERT(IS(ATM, D_(A(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(A(x->s)))));

	push(x, ATM, 13);
	push(x, ATM, 17);
	_join(x);

	// (17, 13)
	// ((11, 7))
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 7, lth(x->f));
	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(2, lth(A(x->s)));
	TEST_ASSERT(IS(ATM, A(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(A(x->s)));
	TEST_ASSERT(IS(ATM, D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(x->s))));

	_join(x);

	// (17, 13, (11, 7))
	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 6, lth(x->f));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, lth(A(x->s)));
	TEST_ASSERT(IS(ATM, A(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(A(x->s)));
	TEST_ASSERT(IS(ATM, D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(x->s))));
	TEST_ASSERT(IS(LST, D_(D_(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(2, lth(A(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, A(D_(D_(A(x->s))))));
	TEST_ASSERT_EQUAL_INT(11, A(A(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, D_(A(D_(D_(A(x->s)))))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(D_(D_(A(x->s)))))));

	_dup(x);

	TEST_ASSERT_EQUAL_INT(free_nodes(x) - 12, lth(x->f));
	TEST_ASSERT_EQUAL_INT(2, lth(x->s));

	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, lth(A(x->s)));
	TEST_ASSERT(IS(ATM, A(x->s)));
	TEST_ASSERT_EQUAL_INT(17, A(A(x->s)));
	TEST_ASSERT(IS(ATM, D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(x->s))));
	TEST_ASSERT(IS(LST, D_(D_(A(x->s)))));
	TEST_ASSERT_EQUAL_INT(2, lth(A(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, A(D_(D_(A(x->s))))));
	TEST_ASSERT_EQUAL_INT(11, A(A(D_(D_(A(x->s))))));
	TEST_ASSERT(IS(ATM, D_(A(D_(D_(A(x->s)))))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(D_(D_(A(x->s)))))));

	TEST_ASSERT(IS(LST, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(3, lth(A(D_(x->s))));
	TEST_ASSERT(IS(ATM, A(D_(x->s))));
	TEST_ASSERT_EQUAL_INT(17, A(A(D_(x->s))));
	TEST_ASSERT(IS(ATM, D_(A(D_(x->s)))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(D_(x->s)))));
	TEST_ASSERT(IS(LST, D_(D_(A(D_(x->s))))));
	TEST_ASSERT_EQUAL_INT(2, lth(A(D_(D_(A(D_(x->s)))))));
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
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_quote(x);
	_swap(x);
	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT_EQUAL_INT(11, A(x->s));
	TEST_ASSERT(IS(LST, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(1, lth(A(D_(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(A(D_(x->s))));
}

void test_swap_2() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_quote(x);
	_swap(x);
	TEST_ASSERT_EQUAL_INT(3, lth(x->s));
	TEST_ASSERT(IS(ATM, x->s));
	TEST_ASSERT_EQUAL_INT(11, A(x->s));
	TEST_ASSERT(IS(LST, D_(x->s)));
	TEST_ASSERT_EQUAL_INT(1, lth(A(D_(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(A(D_(x->s))));
	TEST_ASSERT(IS(ATM, D_(D_(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(x->s))));
}

void test_drop() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	_drop(x);

	TEST_ASSERT_EQUAL_INT(0, lth(x->s));
	TEST_ASSERT_EQUAL_INT(free_nodes(x), lth(x->f));

	push(x, ATM, 13);
	push(x, ATM, 11);
	_join(x);
	_quote(x);
	push(x, ATM, 7);
	_join(x);
	_drop(x);

	TEST_ASSERT_EQUAL_INT(0, lth(x->s));
	TEST_ASSERT_EQUAL_INT(free_nodes(x), lth(x->f));
}

void test_over() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_over(x);
	TEST_ASSERT_EQUAL_INT(3, lth(x->s));
	TEST_ASSERT_EQUAL_INT(11, A(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(D_(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(D_(x->s))));
	pop(x); pop(x); pop(x);

	push(x, ATM, 13);
	push(x, ATM, 11);
	_join(x);
	push(x, ATM, 7);
	_over(x);
	TEST_ASSERT_EQUAL_INT(3, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(2, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(x->s)));
	TEST_ASSERT_NOT_EQUAL_INT(D_(D_(x->s)), x->s);
	TEST_ASSERT_NOT_EQUAL_INT(A(D_(D_(x->s))), A(x->s));
	TEST_ASSERT_NOT_EQUAL_INT(D_(A(D_(D_(x->s)))), D_(A(x->s)));
}

void test_rot() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 11);
	push(x, ATM, 7);
	_rot(x);
	TEST_ASSERT_EQUAL_INT(3, lth(x->s));
	TEST_ASSERT_EQUAL_INT(13, A(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(D_(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(D_(x->s))));
}

void test_add() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	push(x, ATM, 11);
	_add(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(18, A(x->s));
}

void test_sub() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_sub(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(4, A(x->s));
}

void test_mul() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_mul(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(77, A(x->s));
}

void test_div() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 77);
	push(x, ATM, 11);
	_div(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(x->s));
}

void test_mod() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_mod(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(4, A(x->s));
}

void test_gt() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 7);
	_gt(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 13);
	_gt(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 7);
	_gt(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);
}

void test_lt() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 7);
	_lt(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 13);
	_lt(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 7);
	_lt(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);
}

void test_eq() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13),
	push(x, ATM, 7);
	_eq(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 13);
	_eq(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 7);
	_eq(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);
}

void test_neq() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 13);
	push(x, ATM, 7);
	_neq(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 13);
	_neq(x);
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);

	push(x, ATM, 7);
	push(x, ATM, 7);
	_neq(x);
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	pop(x);
}

void test_and() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	push(x, ATM, 11);
	_and(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(3, A(x->s));
	pop(x);

	push(x, ATM, 0);
	push(x, ATM, 0);
	_and(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	pop(x);

	push(x, ATM, 0);
	push(x, ATM, -1);
	_and(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	pop(x);

	push(x, ATM, -1);
	push(x, ATM, 0);
	_and(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	pop(x);

	push(x, ATM, -1);
	push(x, ATM, -1);
	_and(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(-1, A(x->s));
	pop(x);
}

void test_or() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 11);
	push(x, ATM, 7);
	_or(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(15, A(x->s));
	pop(x);

	push(x, ATM, 0);
	push(x, ATM, 0);
	_or(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	pop(x);

	push(x, ATM, 0);
	push(x, ATM, -1);
	_or(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(-1, A(x->s));
	pop(x);

	push(x, ATM, -1);
	push(x, ATM, 0);
	_or(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(-1, A(x->s));
	pop(x);

	push(x, ATM, -1);
	push(x, ATM, -1);
	_or(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(-1, A(x->s));
	pop(x);
}

void test_invert() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	_invert(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(-8, A(x->s));
	pop(x);

	push(x, ATM, 0);
	_invert(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(-1, A(x->s));
	pop(x);

	push(x, ATM, 1);
	_invert(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(-2, A(x->s));
	pop(x);

	push(x, ATM, -1);
	_invert(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	pop(x);
}

void test_not() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	push(x, ATM, 7);
	_not(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	pop(x);

	push(x, ATM, 0);
	_not(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(1, A(x->s));
	pop(x);

	push(x, ATM, 1);
	_not(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	pop(x);

	push(x, ATM, -1);
	_not(x);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	pop(x);
}

void test_interpreter_atom() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C code = ATOM(x, 13, ATOM(x, 7, 0));
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(x->s));
	TEST_ASSERT_EQUAL_INT(13, A(D_(x->s)));
}

void test_interpreter_primitive() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C code = ATOM(x, 13, ATOM(x, 7, PRIMITIVE(x, &_add, PRIMITIVE(x, &_dup, 0))));
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT_EQUAL_INT(20, A(x->s));
	TEST_ASSERT_EQUAL_INT(20, A(D_(x->s)));
}

void test_interpreter_branch() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C code = BRANCH(x, ATOM(x, 7, 0), ATOM(x, 13, 0), 0);

	push(x, ATM, 1);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(x->s));
	pop(x);

	push(x, ATM, 0);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(13, A(x->s));
}

void test_interpreter_continued_branch() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C code = BRANCH(x, ATOM(x, 7, 0), ATOM(x, 13, 0), ATOM(x, 21, 0));

	push(x, ATM, 1);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT_EQUAL_INT(21, A(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(D_(x->s)));
	pop(x);
	pop(x);

	push(x, ATM, 0);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT_EQUAL_INT(21, A(x->s));
	TEST_ASSERT_EQUAL_INT(13, A(D_(x->s)));
}

void test_interpreter_recursion() {
	C size = 1024;
	B block[size];
	X* x = init(block, size);

	C code = 
		PRIMITIVE(x, &_dup,
		ATOM(x, 0,
		PRIMITIVE(x, &_gt,
		BRANCH(x,
			// True branch
			ATOM(x, 1,
			PRIMITIVE(x, &_sub,
			PRIMITIVE(x, &_swap,
			ATOM(x, 2,
			PRIMITIVE(x, &_add,
			PRIMITIVE(x, &_swap,
			RECURSION(x, 0))))))),
			// False branch
			0,
		0))));
	
	push(x, ATM, 0);
	push(x, ATM, 5);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT_EQUAL_INT(0, A(x->s));
	TEST_ASSERT_EQUAL_INT(10, A(D_(x->s)));
}

void test_interpreter_list() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C c3 = cns(x, 13, T(ATM, 0));
	C c2 = cns(x, 11, T(ATM, 0));
	C c1 = cns(x, 7, T(ATM, 0));
	R(c1, c2);
	R(c2, c3);
	C l = cns(x, c1, T(LST, 0));

	inner(x, l);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT(IS(LST, x->s));
	TEST_ASSERT_EQUAL_INT(3, lth(A(x->s)));
	TEST_ASSERT_EQUAL_INT(7, A(A(x->s)));
	TEST_ASSERT_EQUAL_INT(11, A(D_(A(x->s))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(D_(A(x->s)))));
	TEST_ASSERT_NOT_EQUAL_INT(l, x->s);
	TEST_ASSERT_NOT_EQUAL_INT(c1, A(x->s));
	TEST_ASSERT_NOT_EQUAL_INT(c2, D_(A(x->s)));
	TEST_ASSERT_NOT_EQUAL_INT(c3, D_(D_(A(x->s))));
}

void test_interpreter_lambda() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C code = LAMBDA(x, PRIMITIVE(x, &_dup, PRIMITIVE(x, &_mul, 0)), 0);

	push(x, ATM, 5);
	inner(x, code);

	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
	TEST_ASSERT_EQUAL_INT(25, A(x->s));
}

void test_allot() {
	C size = 1024;
	B block[size];
	X* x = init(block, size);

	B* here = x->here;
	push(x, ATM, 0);
	_allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(here, x->here);

	push(x, ATM, 13);
	_allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(here + 13, x->here);

	C fnodes = lth(x->f);
	here = x->here;
	C reserved = RESERVED(x);

	// Ensure reserved memory is 0 to allow next tests to pass
	push(x, ATM, RESERVED(x));
	_allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here + reserved, x->here);
	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));
	TEST_ASSERT_EQUAL_INT(fnodes, lth(x->f));

	here = x->here;

	push(x, ATM, 8*sizeof(C));
	_allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here + 8*sizeof(C), x->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, lth(x->f));
	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));

	here = x->here;

	push(x, ATM, 2*sizeof(C) - 3);
	_allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here + 2*sizeof(C) - 3, x->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 5, lth(x->f));
	TEST_ASSERT_EQUAL_INT(3, RESERVED(x));

	push(x, ATM, -(2*sizeof(C) - 3));
	_allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here, x->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, lth(x->f));
	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));

	push(x, ATM, -1);
	_allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(here - 1, x->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, lth(x->f));
	TEST_ASSERT_EQUAL_INT(1, RESERVED(x));

	here = x->here;
	reserved = RESERVED(x);
	fnodes = lth(x->f);

	push(x, ATM, 2048);
	_allot(x);
	TEST_ASSERT_EQUAL_INT(ERR_NOT_ENOUGH_MEMORY, x->err);
	TEST_ASSERT_EQUAL_PTR(here, x->here);
	TEST_ASSERT_EQUAL_INT(reserved, RESERVED(x));
	TEST_ASSERT_EQUAL_INT(fnodes, lth(x->f));

	x->err = 0;

	push(x, ATM, -2048);
	_allot(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_PTR(BOTTOM(x), x->here);
	TEST_ASSERT_EQUAL_INT((C)ALIGN(x->here, 2*sizeof(C)), x->there);
	TEST_ASSERT_EQUAL_INT(free_nodes(x), lth(x->f));
}

void test_align() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	// This ensures here will be aligned with a pair 
	push(x, ATM, RESERVED(x));
	_allot(x);

	push(x, ATM, 1);
	_allot(x);
	TEST_ASSERT_NOT_EQUAL_INT(ALIGN(x->here, sizeof(C)), x->here);
	TEST_ASSERT_EQUAL_INT(0, x->err);

	_align(x);
	TEST_ASSERT_EQUAL_INT(ALIGN(x->here, sizeof(C)), x->here);
	TEST_ASSERT_EQUAL_INT(0, x->err);
}

void test_Cstr() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	B* here = x->here;

	B* str = Cstr(x, "test string", 11);

	TEST_ASSERT_EQUAL_PTR(here + sizeof(C), str);
	TEST_ASSERT_EQUAL_STRING("test string", str);
	TEST_ASSERT_EQUAL_INT(11, count(str));
	TEST_ASSERT_EQUAL_PTR(here + sizeof(C) + 12, x->here);
}

void test_header() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	B* here = x->here;

	C h = header(x, "test");

	TEST_ASSERT_EQUAL_PTR(here + 1 + sizeof(C) + 4 + 1, x->here);
	TEST_ASSERT_EQUAL_PTR(here + 1 + sizeof(C), NFA(h));
	TEST_ASSERT_EQUAL_PTR(x->here, DFA(h));
	TEST_ASSERT_EQUAL_INT(0, BODY(h));
	TEST_ASSERT_EQUAL_INT(h, D_(XT(h)));
}

void test_body() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C h = header(x, "test");

	C w = body(x, h, ATOM(x, 11, ATOM(x, 7, 0)));

	inner(x, LAMBDA(x, BODY(w), 0));

	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
	TEST_ASSERT_EQUAL_INT(7, A(x->s));
	TEST_ASSERT_EQUAL_INT(11, A(D_(x->s)));
}

void test_reveal() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C h = header(x, "test1");
	C h2 = header(x, "test2");

	C d = reveal(x, h);

	TEST_ASSERT_EQUAL_INT(h, d);
	TEST_ASSERT_EQUAL_INT(h, x->dict);
	TEST_ASSERT_EQUAL_INT(1, lth(x->dict));

	d = reveal(x, h2);

	TEST_ASSERT_EQUAL_INT(h2, d);
	TEST_ASSERT_EQUAL_INT(h2, x->dict);
	TEST_ASSERT_EQUAL_INT(2, lth(x->dict));
	TEST_ASSERT_EQUAL_INT(h, D_(x->dict));
}

void test_find() {
	C size = 2048;
	B block[size];
	X* x = bootstrap(init(block, size));

	C xt = find(x, "+");

	TEST_ASSERT_EQUAL_PTR(&_add, (FUNC)A(A(xt)));

	xt = find(x, "allot");

	TEST_ASSERT_EQUAL_PTR(&_allot, (FUNC)A(A(xt)));

	xt = find(x, "test");

	TEST_ASSERT_EQUAL_INT(0, xt);
}

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
////	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(x->f));
////
////	_sclear(x);
////
////	TEST_ASSERT_EQUAL_INT(0, lth(x->s));
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
////	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
////
////	_spush(x);
////
////	TEST_ASSERT_EQUAL_INT(free_nodes - 3, height(x->f));
////	TEST_ASSERT_EQUAL_INT(7, A(D(P(x))));
////	TEST_ASSERT_EQUAL_INT(13, A(D(D(P(x)))));
////	TEST_ASSERT_EQUAL_INT(2, height(P(x)));
////	TEST_ASSERT_EQUAL_INT(R(x), A(P(x)));
////	TEST_ASSERT_EQUAL_INT(0, lth(x->s));
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
////	TEST_ASSERT_EQUAL_INT(0, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(1, lth(P(x)));
////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
////	TEST_ASSERT_EQUAL_INT(free_nodes, height(x->f));
////
////	push(x, 13);
////	push(x, 7);
////
////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(x->f));
////	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(7, A(x->s));
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
////	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(21, A(x->s));
////
////	_sdrop(x);
////
////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(x->f));
////	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(7, A(x->s));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////	TEST_ASSERT_EQUAL_INT(R(x), P(x));
////}
////
////void test_drop() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	push(x, 21);
////	push(x, 13);
////	push(x, 7);
////	TEST_ASSERT_EQUAL_INT(3, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(7, A(x->s));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////	TEST_ASSERT_EQUAL_INT(21, A(D(D(K(x)))));
////	_drop(x);
////	TEST_ASSERT_EQUAL_INT(2, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(13, A(x->s));
////	TEST_ASSERT_EQUAL_INT(21, x->s);
////	_drop(x);
////	TEST_ASSERT_EQUAL_INT(1, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(21, A(x->s));
////	_drop(x);
////	TEST_ASSERT_EQUAL_INT(0, lth(x->s));
////	_drop(x);
////	TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW, x->err);
////}
////
////void test_rev() {
////	C size = 512;
////	B block[size];
////	X* x = init(block, size);
////
////	push(x, 21);
////	push(x, 13);
////	push(x, 7);
////	push(x, 5);
////	push(x, 3);
////	TEST_ASSERT_EQUAL_INT(5, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(3, A(x->s));
////	TEST_ASSERT_EQUAL_INT(5, x->s);
////	TEST_ASSERT_EQUAL_INT(7, A(D(D(K(x)))));
////	TEST_ASSERT_EQUAL_INT(13, A(D(D(D(K(x))))));
////	TEST_ASSERT_EQUAL_INT(21, A(D(D(D(D(K(x)))))));
////	_rev(x);
////	TEST_ASSERT_EQUAL_INT(5, lth(x->s));
////	TEST_ASSERT_EQUAL_INT(21, A(x->s));
////	TEST_ASSERT_EQUAL_INT(13, x->s);
////	TEST_ASSERT_EQUAL_INT(7, A(D(D(K(x)))));
////	TEST_ASSERT_EQUAL_INT(5, A(D(D(D(K(x))))));
////	TEST_ASSERT_EQUAL_INT(3, A(D(D(D(D(K(x)))))));
////}
////
//////////void test_stack_to_list() {
//////////}
//////////
////////////void test_append() {
////////////}
////////////
////////////void test_copy() {
////////////	C size = 512;
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
//////////	PAIR* cfa = cns(ctx, ATM, 7, cns(ctx, ATM, 13, 0));
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
////////////		cns(ctx, (C)&_dup, T_PRM,
////////////		cns(ctx, 1, ATM,
////////////		cns(ctx, (C)&_gt, T_PRM,
////////////		cns(ctx,
////////////			0,
////////////			T_JMP,
////////////			cns(ctx, 1, ATM,
////////////			cns(ctx, (C)&_sub, T_PRM,
////////////			cns(ctx, (C)&_dup, T_PRM,
////////////			cns(ctx, 1, ATM,
////////////			cns(ctx, (C)&_sub, T_PRM,
////////////			cns(ctx, (C)&_rec, T_PRM,
////////////			cns(ctx, (C)&_swap, T_PRM,
////////////			cns(ctx, (C)&_rec, T_PRM,	
////////////			cns(ctx, (C)&_add, T_PRM, 0)))))))))))));
////////////
////////////		ctx->dstack = cns(ctx, 6, ATM, 0);
////////////		ctx->r = cns(ctx, (C)ctx->ip, T_WORD, 0);
////////////
////////////		inner(ctx);
////////////
////////////		//printf("%ld\n", ctx->dstack->value);
////////////}
//////////
int main() {
	UNITY_BEGIN();

	// TAGGED POINTER BASED TYPING INFORMATION
	RUN_TEST(test_types);

	//  LIST FUNCTIONS
	RUN_TEST(test_lth);
	RUN_TEST(test_dth);
	RUN_TEST(test_mlth);
	RUN_TEST(test_lst);
	RUN_TEST(test_rev);

	// CONTEXT
	RUN_TEST(test_block_size);
	RUN_TEST(test_block_initialization);

	// LIST CREATION AND DESTRUCTION (AUTOMATIC MEMORY MANAGEMENT)
	RUN_TEST(test_cns);
	RUN_TEST(test_rcl);
	RUN_TEST(test_cln);
	RUN_TEST(test_rmv);

	// BASIC STACK OPERATIONS
	RUN_TEST(test_push);
	RUN_TEST(test_pop);

	// COMPILATION PILE OPERATIONS
	RUN_TEST(test_cppush);
	RUN_TEST(test_cppop_1);
	RUN_TEST(test_cspush);
	RUN_TEST(test_cppop_2);

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
	RUN_TEST(test_drop);
	RUN_TEST(test_over);
	RUN_TEST(test_rot);

	RUN_TEST(test_add);
	RUN_TEST(test_sub);
	RUN_TEST(test_mul);
	RUN_TEST(test_div);
	RUN_TEST(test_mod);

	RUN_TEST(test_gt);
	RUN_TEST(test_lt);
	RUN_TEST(test_eq);
	RUN_TEST(test_neq);

	RUN_TEST(test_and);
	RUN_TEST(test_or);
	RUN_TEST(test_invert);
	RUN_TEST(test_not);

	//RUN_TEST(test_clear_stack);
	//RUN_TEST(test_push_stack);
	//RUN_TEST(test_drop_stack);

	RUN_TEST(test_interpreter_atom);
	RUN_TEST(test_interpreter_primitive);
	RUN_TEST(test_interpreter_branch);
	RUN_TEST(test_interpreter_continued_branch);
	RUN_TEST(test_interpreter_recursion);
	RUN_TEST(test_interpreter_list);
	RUN_TEST(test_interpreter_lambda);

	RUN_TEST(test_allot);
	RUN_TEST(test_align);

	RUN_TEST(test_Cstr);

	RUN_TEST(test_header);
	RUN_TEST(test_body);
	RUN_TEST(test_reveal);

	RUN_TEST(test_find);

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
