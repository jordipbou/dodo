#include<stdlib.h>
#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_depth_height() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);
	C p4 = (C)malloc(sizeof(C) * 2);

	D(p1) = p2;
	D(p2) = 0;
	A(p2) = 0;
	A(p3) = p2;
	A(p4) = p3;

	TEST_ASSERT_EQUAL_INT(0, depth(0));
	TEST_ASSERT_EQUAL_INT(1, depth(p2));
	TEST_ASSERT_EQUAL_INT(2, depth(p1));

	TEST_ASSERT_EQUAL_INT(0, height(0));
	TEST_ASSERT_EQUAL_INT(1, height(p2));
	TEST_ASSERT_EQUAL_INT(2, height(p3));
	TEST_ASSERT_EQUAL_INT(3, height(p4));
}

void test_block_initialization() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(0, depth(D(Z(x))));

	TEST_ASSERT_EQUAL_INT(BOTTOM(x), ((B*)x) + sizeof(X));
	TEST_ASSERT_EQUAL_INT(BOTTOM(x), x->here);
	TEST_ASSERT_EQUAL_INT(x->there, ALIGN(x->here, 2*sizeof(C)));
	TEST_ASSERT_EQUAL_INT(
		ALIGN(((B*)x) + size - 2*sizeof(C) - 1, 2*sizeof(C)), 
		TOP(x));
	TEST_ASSERT_EQUAL_INT(TOP(x), Z(x));
	TEST_ASSERT_EQUAL_INT(0, D(Z((x))));
	TEST_ASSERT_EQUAL_INT(0, x->rstack);

	TEST_ASSERT_EQUAL_INT(0, x->dict);

	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 1;
	TEST_ASSERT_EQUAL_INT(free_nodes, height(A(Z(x))));
}

void test_push() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	TEST_ASSERT_EQUAL_INT(0, depth(S(x)));
	TEST_ASSERT_EQUAL_INT(0, x->err);
	push(x, T_ATOM, 13);
	TEST_ASSERT_EQUAL_INT(1, depth(S(x)));
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(13, A(S(x)));
	TEST_ASSERT(IS_ATOM(S(x)));
	push(x, T_LIST, 7);
	TEST_ASSERT_EQUAL_INT(2, depth(S(x)));
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT(!IS_ATOM(S(x)));
	TEST_ASSERT_EQUAL_INT(13, A(D_(S(x))));
	TEST_ASSERT(IS_ATOM(D_(S(x))));

	while (height(A(Z(x))) > 0) { push(x, T_ATOM, 7); }

	TEST_ASSERT_EQUAL_INT(0, x->err);
	push(x, T_ATOM, 17);
	TEST_ASSERT_EQUAL_INT(ERR_OVERFLOW, x->err);
	TEST_ASSERT_EQUAL_INT(7, A(D(Z(x))));
}

void test_lit() {
C size = 256;
	B block[size];
	X* x = init(block, size);

	lit(x, 13);
	lit(x, 7);
	TEST_ASSERT_EQUAL_INT(2, depth(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT(IS_ATOM(S(x)));
	TEST_ASSERT_EQUAL_INT(13, A(D_(S(x))));
	TEST_ASSERT(IS_ATOM(D_(S(x))));
}

void test_pop() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	TEST_ASSERT_EQUAL_INT(0, x->err);
	C v = pop(x);
	TEST_ASSERT_EQUAL_INT(0, v);
	TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW, x->err);

	x->err = 0;

	lit(x, 13);
	lit(x, 7);

	TEST_ASSERT_EQUAL_INT(2, depth(S(x)));
	v = pop(x);
	TEST_ASSERT_EQUAL_INT(1, depth(S(x)));
	TEST_ASSERT_EQUAL_INT(7, v);
	v = pop(x);
	TEST_ASSERT_EQUAL_INT(0, depth(S(x)));
	TEST_ASSERT_EQUAL_INT(13, v);
}

void test_cons_atom_atom() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 1;

	TEST_ASSERT_EQUAL_INT(free_nodes, height(F(x)));
	lit(x, 17);
	lit(x, 13);
	lit(x, 7);
	TEST_ASSERT_EQUAL_INT(free_nodes - 3, height(F(x)));
	TEST_ASSERT_EQUAL_INT(3, depth(S(x)));
	_cons(x);
	TEST_ASSERT_EQUAL_INT(free_nodes - 4, height(F(x)));
	TEST_ASSERT_EQUAL_INT(2, depth(S(x)));
	TEST_ASSERT(IS_LIST(S(x)));
	TEST_ASSERT_EQUAL_INT(2, depth(A(S(x))));
	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(S(x)))));
}

void test_cons_list_atom() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 1;

	lit(x, 23);

	lit(x, 7);

	lit(x, 17);
	lit(x, 13);
	_cons(x);
	_cons(x);

	TEST_ASSERT_EQUAL_INT(free_nodes - 6, height(F(x)));
	TEST_ASSERT_EQUAL_INT(23, A(D_(S(x))));
	TEST_ASSERT_EQUAL_INT(2, depth(A(S(x))));
	TEST_ASSERT_EQUAL_INT(2, depth(A(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(13, A(A(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(A(A(S(x))))));
	TEST_ASSERT_EQUAL_INT(7, A(D_(A(S(x)))));
}

void test_cons_atom_list() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 1;

	lit(x, 23);

	lit(x, 17);
	lit(x, 13);
	_cons(x);
	lit(x, 7);
	TEST_ASSERT_EQUAL_INT(free_nodes - 5, height(F(x)));
	_cons(x);
	TEST_ASSERT_EQUAL_INT(free_nodes - 5, height(F(x)));
	TEST_ASSERT_EQUAL_INT(2, depth(S(x)));
	TEST_ASSERT_EQUAL_INT(23, A(D_(S(x))));
	TEST_ASSERT_EQUAL_INT(3, depth(A(S(x))));
	TEST_ASSERT(IS_ATOM(A(S(x))));
	TEST_ASSERT_EQUAL_INT(7, A(A(S(x))));
	TEST_ASSERT(IS_ATOM(D_(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(S(x)))));
	TEST_ASSERT(IS_ATOM(D_(D_(A(S(x))))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(D_(A(S(x))))));
}

void test_cons_list_list() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 1;

	lit(x, 23);

	lit(x, 19);
	lit(x, 17);
	_cons(x);

	lit(x, 13);
	lit(x, 7);
	_cons(x);

	_cons(x);

	TEST_ASSERT_EQUAL_INT(free_nodes - 7, height(F(x)));
	TEST_ASSERT_EQUAL_INT(2, depth(S(x)));
	TEST_ASSERT_EQUAL_INT(23, A(D_(S(x))));
	TEST_ASSERT_EQUAL_INT(3, depth(A(S(x))));
	TEST_ASSERT_EQUAL_INT(2, depth(A(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(7, A(A(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(13, A(D_(A(A(S(x))))));
	TEST_ASSERT_EQUAL_INT(17, A(D_(A(S(x)))));
	TEST_ASSERT_EQUAL_INT(19, A(D_(D_(A(S(x))))));
}

void test_carcdr() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 1;

	lit(x, 21);
	lit(x, 13);
	_cons(x);
	lit(x, 7);
	_cons(x);

	_carcdr(x);
	TEST_ASSERT_EQUAL_INT(0, x->err);
	TEST_ASSERT_EQUAL_INT(free_nodes - 4, height(F(x)));
	TEST_ASSERT_EQUAL_INT(2, depth(S(x)));
	TEST_ASSERT(IS_ATOM(S(x)));
	TEST_ASSERT_EQUAL_INT(7, A(S(x)));
	TEST_ASSERT(IS_LIST(D_(S(x))));
	TEST_ASSERT_EQUAL_INT(2, depth(A(D_(S(x)))));
	TEST_ASSERT_EQUAL_INT(13, A(A(D_(S(x)))));
	TEST_ASSERT_EQUAL_INT(21, A(D_(A(D_(S(x))))));
}

void test_drop() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 1;

	lit(x, 13); lit(x, 7); _cons(x);
	TEST_ASSERT_EQUAL_INT(free_nodes - 3, height(F(x)));

	_drop(x);

	TEST_ASSERT_EQUAL_INT(0, depth(S(x)));
	TEST_ASSERT_EQUAL_INT(free_nodes, height(F(x)));

}

//
//	TEST_ASSERT_EQUAL_INT(free_nodes, height(A(Z(x))));
//	TEST_ASSERT_EQUAL_INT(0, depth(D(Z(x))));
//
//	C p = _cons(x, 13, T(T_ATOM, 0));
//	TEST_ASSERT_EQUAL_INT(free_nodes - 1, height(A(Z(x))));
//	TEST_ASSERT_EQUAL_INT(0, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(p));
//	TEST_ASSERT_EQUAL_INT(0, D_(p));
//	
//	while (height(A(Z(x))) > 0) { _cons(x, 1, T(T_ATOM, 0)); }
//
//	C p2 = _cons(x, 21, T(T_ATOM, 0));
//	TEST_ASSERT_EQUAL_INT(ERR_OVERFLOW, x->err);
//}
//
//void test_dup() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	push(x, 7);
//	_dup(x);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(A(D(Z(x))), A(D_(D(Z(x)))));
//}
//
//void test_swap() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	push(x, 13);
//	push(x, 7);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D_(D(Z(x)))));
//	_swap(x);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D_(D(Z(x)))));
//}
//
//void test_over() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	push(x, 13);
//	push(x, 7);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D_(D(Z(x)))));
//	_over(x);
//	TEST_ASSERT_EQUAL_INT(3, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D_(D(Z(x)))));
//	TEST_ASSERT_EQUAL_INT(13, A(D(D(D(Z(x))))));
//}
//
//void test_rot() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	push(x, 21);
//	push(x, 13);
//	push(x, 7);
//	TEST_ASSERT_EQUAL_INT(3, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D_(D(Z(x)))));
//	TEST_ASSERT_EQUAL_INT(21, A(D(D(D(Z(x))))));
//	_rot(x);
//	TEST_ASSERT_EQUAL_INT(3, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(21, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D_(D(Z(x)))));
//	TEST_ASSERT_EQUAL_INT(13, A(D(D(D(Z(x))))));
//}
//
//void test_drop() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	push(x, 21);
//	push(x, 13);
//	push(x, 7);
//	TEST_ASSERT_EQUAL_INT(3, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D_(D(Z(x)))));
//	TEST_ASSERT_EQUAL_INT(21, A(D(D(D(Z(x))))));
//	_drop(x);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(21, A(D_(D(Z(x)))));
//	_drop(x);
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(21, A(D(Z(x))));
//	_drop(x);
//	TEST_ASSERT_EQUAL_INT(0, depth(D(Z(x))));
//	_drop(x);
//	TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW, x->err);
//}
//
//void test_binops() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	push(x, 7);
//	push(x, 13);
//	_add(x);
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(20, A(D(Z(x))));
//	pop(x);
//
//	push(x, 13);
//	push(x, 7);
//	_sub(x);
//	TEST_ASSERT_EQUAL_INT(6, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 13);
//	push(x, 7);
//	_mul(x);
//	TEST_ASSERT_EQUAL_INT(91, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 30);
//	push(x, 5);
//	_div(x);
//	TEST_ASSERT_EQUAL_INT(6, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 9);
//	push(x, 2);
//	_mod(x);
//	TEST_ASSERT_EQUAL_INT(A(D(Z(x))), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 13);
//	push(x, 7);
//	_gt(x);
//	TEST_ASSERT_EQUAL_INT(1, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 7);
//	push(x, 13);
//	_gt(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 7);
//	push(x, 7);
//	_gt(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 13);
//	push(x, 7);
//	_lt(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 7);
//	push(x, 13);
//	_lt(x);
//	TEST_ASSERT_EQUAL_INT(1, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 7);
//	push(x, 7);
//	_lt(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 13),
//	push(x, 7);
//	_eq(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 7);
//	push(x, 13);
//	_eq(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 7);
//	push(x, 7);
//	_eq(x);
//	TEST_ASSERT_EQUAL_INT(1, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 13);
//	push(x, 7);
//	_neq(x);
//	TEST_ASSERT_EQUAL_INT(1, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 7);
//	push(x, 13);
//	_neq(x);
//	TEST_ASSERT_EQUAL_INT(1, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 7);
//	push(x, 7);
//	_neq(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 0);
//	push(x, 0);
//	_and(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 0);
//	push(x, 1);
//	_and(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 1);
//	push(x, 0);
//	_and(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 1);
//	push(x, 1);
//	_and(x);
//	TEST_ASSERT_EQUAL_INT(1, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 0);
//	push(x, 0);
//	_or(x);
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 0);
//	push(x, 1);
//	_or(x);
//	TEST_ASSERT_EQUAL_INT(1, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 1);
//	push(x, 0);
//	_or(x);
//	TEST_ASSERT_EQUAL_INT(1, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//
//	push(x, 1);
//	push(x, 1);
//	_or(x);
//	TEST_ASSERT_EQUAL_INT(1, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	pop(x);
//}
//
//void test_interpreter_atom() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	C code = ATOM(x, 13, ATOM(x, 7, 0));
//	inner(x, code);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D_(D(Z(x)))));
//}
//
//void test_interpreter_primitive() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	C code = ATOM(x, 13, ATOM(x, 7, PRIMITIVE(x, &_add, PRIMITIVE(x, &_dup, 0))));
//	inner(x, code);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(20, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(20, A(D_(D(Z(x)))));
//}
//
//void test_interpreter_branch() {
//	C size = 256;
//	B block[size];
//	X* x = init(block, size);
//
//	C code = BRANCH(x, ATOM(x, 7, 0), ATOM(x, 13, 0), 0);
//
//	push(x, 1);
//	inner(x, code);
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D(Z(x))));
//	pop(x);
//
//	push(x, 0);
//	inner(x, code);
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D(Z(x))));
//}
//
//void test_interpreter_continued_branch() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	C code = BRANCH(x, ATOM(x, 7, 0), ATOM(x, 13, 0), ATOM(x, 21, 0));
//
//	push(x, 1);
//	inner(x, code);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(21, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(7, A(D_(D(Z(x)))));
//	pop(x);
//	pop(x);
//
//	push(x, 0);
//	inner(x, code);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(21, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D_(D(Z(x)))));
//}
//
//void test_interpreter_recursion() {
//	C size = 1024;
//	B block[size];
//	X* x = init(block, size);
//
//	C code = 
//		PRIMITIVE(x, &_dup,
//		ATOM(x, 0,
//		PRIMITIVE(x, &_gt,
//		BRANCH(x,
//			// True branch
//			ATOM(x, 1,
//			PRIMITIVE(x, &_sub,
//			PRIMITIVE(x, &_swap,
//			ATOM(x, 2,
//			PRIMITIVE(x, &_add,
//			PRIMITIVE(x, &_swap,
//			RECURSION(x, 0))))))),
//			// False branch
//			0,
//		0))));
//		
//	push(x, 0);
//	push(x, 5);
//	inner(x, code);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(0, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(10, A(D_(D(Z(x)))));
//}
//
//void test_interpreter_word() {
//	C size = 512;
//	B block[size];
//	X* x = init(block, size);
//
//	C square = PRIMITIVE(x, &_dup, PRIMITIVE(x, &_mul, 0));
//
//	// Tail call
//	C code = ATOM(x, 5, WORD(x, square, 0));
//
//	inner(x, code);
//	TEST_ASSERT_EQUAL_INT(1, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(25, A(D(Z(x))));
//	pop(x);
//
//	// Non tail call
//	code = ATOM(x, 3, WORD(x, square, ATOM(x, 13, 0)));
//
//	inner(x, code);
//	TEST_ASSERT_EQUAL_INT(2, depth(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(13, A(D(Z(x))));
//	TEST_ASSERT_EQUAL_INT(9, A(D_(D(Z(x)))));
//}

//////void test_allot() {
//////	C size = 256;
//////	B block[size];
//////	X* x = init(block, size);
//////
//////	C free_nodes = (size - sizeof(X)) / sizeof(PAIR);
//////
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_INT(0, x->here - BOTTOM(x));
//////	TEST_ASSERT_EQUAL_INT(free_nodes, depth(x->free));
//////	allot(x, 3);
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_INT(3, x->here - BOTTOM(x));
//////	TEST_ASSERT_EQUAL_INT(BOTTOM(x) + 3 + RESERVED(x), x->there);
//////	allot(x, 13);
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_INT(16, x->here - BOTTOM(x));
//////	allot(x, 1);
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_INT(17, x->here - BOTTOM(x));
//////	TEST_ASSERT_LESS_THAN(free_nodes, depth(x->free));
//////
//////	C fnodes = depth(x->free);
//////	B* here = x->here;
//////	C reserved = RESERVED(x);
//////
//////	// Ensure reserved memory is 0 to allow next tests to pass
//////	allot(x, RESERVED(x));
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved, x->here);
//////	TEST_ASSERT_EQUAL_INT(0, RESERVED(x));
//////	TEST_ASSERT_EQUAL_INT(fnodes, depth(x->free));
//////
//////	allot(x, sizeof(C) * 8);
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*8, x->here);
//////	TEST_ASSERT_EQUAL_INT(fnodes - 4, depth(x->free));
//////
//////	allot(x, -(sizeof(C) * 4));
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*4, x->here);
//////	TEST_ASSERT_EQUAL_INT(fnodes - 2, depth(x->free));
//////	allot(x, -(sizeof(C) - 1));
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*3 + 1, x->here);
//////	TEST_ASSERT_EQUAL_INT(fnodes - 2, depth(x->free));
//////	allot(x, -(sizeof(C) + 1));
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*2, x->here);
//////	TEST_ASSERT_EQUAL_INT(fnodes - 1, depth(x->free));
//////
//////	allot(x, 2048);
//////	TEST_ASSERT_EQUAL_INT(ERR_NOT_ENOUGH_MEMORY, x->err);
//////}
//////
//////void test_align() {
//////	C size = 256;
//////	B block[size];
//////	X* x = init(block, size);
//////
//////	// This ensures here will be aligned with a pair 
//////	allot(x, RESERVED(x));
//////
//////	allot(x, 1);
//////	TEST_ASSERT_NOT_EQUAL_INT(x->here, ALIGN(x->here, sizeof(C)));
//////
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////
//////	align(x);
//////	TEST_ASSERT_EQUAL_INT(x->here, ALIGN(x->here, sizeof(C)));
//////
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////}
//////
//////void test_stack_to_list() {
//////}
//////
////////void test_append() {
////////}
////////
////////void test_copy() {
////////	C size = 256;
////////	B block[size];
////////	X* x = init(block, size);
////////
////////	C free_nodes = (size - sizeof(X)) / sizeof(PAIR);
////////
////////	// TODO
////////}
//////
//////void test_header_body_reveal() {
//////	C size = 2048;
//////	B block[size];
//////	X* x = init(block, size);
//////
//////	B* here = x->here;
//////	PAIR* w = header(x, "test", 4);
//////	TEST_ASSERT_EQUAL_INT(0, x->err);
//////	TEST_ASSERT_EQUAL_PTR(x->here, ALIGN(here, sizeof(C)) + sizeof(C) + 4 + 1);
//////	TEST_ASSERT(IS(ATOM, w));
//////	TEST_ASSERT_EQUAL_INT(4, COUNT(NFA(w)));
//////	TEST_ASSERT_EQUAL_STRING("test", NFA(w));
//////	TEST_ASSERT_EQUAL_INT(((B*)REF(REF(w))) + 4, DFA(w));
//////	TEST_ASSERT_EQUAL_INT(0, CFA(w));
//////
//////	TEST_ASSERT_NULL(x->dict);
//////
//////	PAIR* cfa = _cons(x, ATOM, 7, _cons(x, ATOM, 13, 0));
//////	body(x, w, cfa);
//////
//////	TEST_ASSERT_EQUAL_INT(cfa, CFA(w));
//////	TEST_ASSERT_EQUAL_INT(7, CFA(w)->value);
//////	TEST_ASSERT_EQUAL_INT(13, NEXT(CFA(w))->value);
//////
//////	reveal(x, w);
//////
//////	TEST_ASSERT_EQUAL_PTR(w, x->dict);
//////	TEST_ASSERT_NULL(NEXT(x->dict));
//////
//////	TEST_ASSERT_FALSE(IS_IMMEDIATE(w));
//////
//////	_immediate(x);
//////
//////	TEST_ASSERT_TRUE(IS_IMMEDIATE(w));
//////}
//////
////////void test_find() {
////////	C size = 512;
////////	B block[size];
////////	X* x = init(block, size);
////////
////////	PAIR* dup = reveal(x, header(x, "dup", 3));
////////	PAIR* swap = reveal(x, header(x, "swap", 4));
////////	PAIR* test = reveal(x, header(x, "test", 4));
////////
////////	TEST_ASSERT_EQUAL_PTR(dup, find(x, "dup", 3));
////////	TEST_ASSERT_EQUAL_PTR(swap, find(x, "swap", 4));
////////	TEST_ASSERT_EQUAL_PTR(test, find(x, "test", 4));
////////	TEST_ASSERT_EQUAL_PTR(0, find(x, "nop", 3));
////////
////////	TEST_ASSERT_EQUAL_PTR(test, x->dict);
////////	TEST_ASSERT_EQUAL_PTR(swap, NEXT(x->dict));
////////	TEST_ASSERT_EQUAL_PTR(dup, NEXT(NEXT(x->dict)));
////////	TEST_ASSERT_EQUAL_PTR(0, NEXT(NEXT(NEXT(x->dict))));
////////}
////////
////////void test_dodo_initialization() {
////////	C size = 4096;
////////	B block[size];
////////	X* x = dodo(init(block, size));
////////
////////	TEST_ASSERT_EQUAL_PTR(&_add, CFA(find(x, "+", 1))->value);
////////}
////////
////////void test_fib() {
////////	C size = 8192;
////////	B block[size];
////////	X* x = init(block, size);
////////
////////	// : fib dup 1 > if 1- dup 1- recurse swap recurse + then ;
////////
////////	x->ip =
////////		_cons(x, (C)&_dup, T_PRIMITIVE,
////////		_cons(x, 1, ATOM,
////////		_cons(x, (C)&_gt, T_PRIMITIVE,
////////		_cons(x,
////////			0,
////////			T_BRANCH,
////////			_cons(x, 1, ATOM,
////////			_cons(x, (C)&_sub, T_PRIMITIVE,
////////			_cons(x, (C)&_dup, T_PRIMITIVE,
////////			_cons(x, 1, ATOM,
////////			_cons(x, (C)&_sub, T_PRIMITIVE,
////////			_cons(x, (C)&_rec, T_PRIMITIVE,
////////			_cons(x, (C)&_swap, T_PRIMITIVE,
////////			_cons(x, (C)&_rec, T_PRIMITIVE,	
////////			_cons(x, (C)&_add, T_PRIMITIVE, 0)))))))))))));
////////
////////		x->dstack = _cons(x, 6, ATOM, 0);
////////		x->rstack = _cons(x, (C)x->ip, T_WORD, 0);
////////
////////		inner(x);
////////
////////		//printf("%ld\n", x->dstack->value);
////////}
//////
int main() {
	UNITY_BEGIN();

	RUN_TEST(test_depth_height);

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_push);
	RUN_TEST(test_lit);
	RUN_TEST(test_pop);
	RUN_TEST(test_cons_atom_atom);
	RUN_TEST(test_cons_list_atom);
	RUN_TEST(test_cons_atom_list);
	RUN_TEST(test_cons_list_list);
	RUN_TEST(test_carcdr);
	RUN_TEST(test_drop);

	//RUN_TEST(test_dup);
	//RUN_TEST(test_swap);
	//RUN_TEST(test_over);
	//RUN_TEST(test_rot);
	//RUN_TEST(test_drop);
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
