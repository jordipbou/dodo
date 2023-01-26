#include<stdlib.h>
#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_depth() {
	C p1 = (C)malloc(sizeof(C) * 2);
	C p2 = (C)malloc(sizeof(C) * 2);
	C p3 = (C)malloc(sizeof(C) * 2);
	C p4 = (C)malloc(sizeof(C) * 2);

	D(p1) = p2;
	D(p2) = 0;


	TEST_ASSERT_EQUAL_INT(0, depth(0));
	TEST_ASSERT_EQUAL_INT(1, depth(p2));
	TEST_ASSERT_EQUAL_INT(2, depth(p1));
}

void test_block_initialization() {
	C size = 256;
	B block[size];
	X* ctx = init(block, size);

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(0, depth(K(ctx)));

	TEST_ASSERT_EQUAL_INT(ctx->bottom, ((B*)ctx) + sizeof(X));
	TEST_ASSERT_EQUAL_INT(ctx->bottom, ctx->here);
	TEST_ASSERT_EQUAL_INT(ctx->there, ALIGN(ctx->here, 2*sizeof(C)));
	TEST_ASSERT_EQUAL_INT(
		ALIGN(((B*)ctx) + size - 2*sizeof(C) - 1, 2*sizeof(C)), 
		ctx->top);
	TEST_ASSERT_EQUAL_INT(ctx->top, ctx->nodes);
	TEST_ASSERT_EQUAL_INT(0, K(ctx));
	TEST_ASSERT_EQUAL_INT(0, ctx->rstack);

	TEST_ASSERT_EQUAL_INT(0, ctx->dict);

	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 1;
	TEST_ASSERT_EQUAL_INT(free_nodes, height(F(ctx)));
}

void test_push() {
	C size = 256;
	B block[size];
	X* ctx = init(block, size);

	TEST_ASSERT_EQUAL_INT(0, depth(K(ctx)));
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	push(ctx, 13);
	TEST_ASSERT_EQUAL_INT(1, depth(K(ctx)));
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(13, T(ctx));
	push(ctx, 7);
	TEST_ASSERT_EQUAL_INT(2, depth(K(ctx)));
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(7, T(ctx));
	TEST_ASSERT_EQUAL_INT(13, S(ctx));

	while (height(F(ctx)) > 0) { push(ctx, 7); }

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	push(ctx, 17);
	TEST_ASSERT_EQUAL_INT(ERR_OVERFLOW, ctx->err);
	TEST_ASSERT_EQUAL_INT(7, T(ctx));
}

void test_pop() {
	C size = 256;
	B block[size];
	X* ctx = init(block, size);

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	C v = pop(ctx);
	TEST_ASSERT_EQUAL_INT(0, v);
	TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW, ctx->err);

	ctx->err = 0;

	push(ctx, 13);
	push(ctx, 7);

	TEST_ASSERT_EQUAL_INT(2, depth(K(ctx)));
	v = pop(ctx);
	TEST_ASSERT_EQUAL_INT(1, depth(K(ctx)));
	TEST_ASSERT_EQUAL_INT(7, v);
	v = pop(ctx);
	TEST_ASSERT_EQUAL_INT(0, depth(K(ctx)));
	TEST_ASSERT_EQUAL_INT(13, v);
}

void test_cons() {
	C size = 256;
	B block[size];
	X* ctx = init(block, size);

	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 1;

	TEST_ASSERT_EQUAL_INT(free_nodes, height(F(ctx)));

	C p = cons(ctx, 13, 0);
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, height(F(ctx)));
	TEST_ASSERT_EQUAL_INT(13, A(p));
	TEST_ASSERT_EQUAL_INT(0, D(p));
	
	while (height(F(ctx)) > 0) { cons(ctx, 1, 0); }

	C p3 = cons(ctx, 21, 0);
	TEST_ASSERT_EQUAL_INT(ERR_OVERFLOW, ctx->err);
}

void test_dup() {
	C size = 256;
	B block[size];
	X* ctx = init(block, size);

	push(ctx, 7);
	_dup(ctx);
	TEST_ASSERT_EQUAL_INT(2, depth(K(ctx)));
	TEST_ASSERT_EQUAL_INT(T(ctx), S(ctx));
}

void test_swap() {
	C size = 256;
	B block[size];
	X* ctx = init(block, size);

	push(ctx, 13);
	push(ctx, 7);
	TEST_ASSERT_EQUAL_INT(2, depth(K(ctx)));
	TEST_ASSERT_EQUAL_INT(7, T(ctx));
	TEST_ASSERT_EQUAL_INT(13, S(ctx));
	_swap(ctx);
	TEST_ASSERT_EQUAL_INT(2, depth(K(ctx)));
	TEST_ASSERT_EQUAL_INT(13, T(ctx));
	TEST_ASSERT_EQUAL_INT(7, S(ctx));
}

void test_over() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, 13);
	push(x, 7);
	TEST_ASSERT_EQUAL_INT(2, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(7, T(x));
	TEST_ASSERT_EQUAL_INT(13, S(x));
	_over(x);
	TEST_ASSERT_EQUAL_INT(3, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(13, T(x));
	TEST_ASSERT_EQUAL_INT(7, S(x));
	TEST_ASSERT_EQUAL_INT(13, A(D(D(K(x)))));
}

void test_rot() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, 21);
	push(x, 13);
	push(x, 7);
	TEST_ASSERT_EQUAL_INT(3, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(7, T(x));
	TEST_ASSERT_EQUAL_INT(13, S(x));
	TEST_ASSERT_EQUAL_INT(21, A(D(D(K(x)))));
	_rot(x);
	TEST_ASSERT_EQUAL_INT(3, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(21, T(x));
	TEST_ASSERT_EQUAL_INT(7, S(x));
	TEST_ASSERT_EQUAL_INT(13, A(D(D(K(x)))));
}

void test_drop() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, 21);
	push(x, 13);
	push(x, 7);
	TEST_ASSERT_EQUAL_INT(3, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(7, T(x));
	TEST_ASSERT_EQUAL_INT(13, S(x));
	TEST_ASSERT_EQUAL_INT(21, A(D(D(K(x)))));
	_drop(x);
	TEST_ASSERT_EQUAL_INT(2, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(13, T(x));
	TEST_ASSERT_EQUAL_INT(21, S(x));
	_drop(x);
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(21, T(x));
	_drop(x);
	TEST_ASSERT_EQUAL_INT(0, depth(K(x)));
	_drop(x);
	TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW, x->err);
}

void test_binops() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	push(x, 7);
	push(x, 13);
	_add(x);
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(20, T(x));
	pop(x);

	push(x, 13);
	push(x, 7);
	_sub(x);
	TEST_ASSERT_EQUAL_INT(6, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 13);
	push(x, 7);
	_mul(x);
	TEST_ASSERT_EQUAL_INT(91, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 30);
	push(x, 5);
	_div(x);
	TEST_ASSERT_EQUAL_INT(6, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 9);
	push(x, 2);
	_mod(x);
	TEST_ASSERT_EQUAL_INT(T(x), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 13);
	push(x, 7);
	_gt(x);
	TEST_ASSERT_EQUAL_INT(1, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 7);
	push(x, 13);
	_gt(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 7);
	push(x, 7);
	_gt(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 13);
	push(x, 7);
	_lt(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 7);
	push(x, 13);
	_lt(x);
	TEST_ASSERT_EQUAL_INT(1, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 7);
	push(x, 7);
	_lt(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 13),
	push(x, 7);
	_eq(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 7);
	push(x, 13);
	_eq(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 7);
	push(x, 7);
	_eq(x);
	TEST_ASSERT_EQUAL_INT(1, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 13);
	push(x, 7);
	_neq(x);
	TEST_ASSERT_EQUAL_INT(1, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 7);
	push(x, 13);
	_neq(x);
	TEST_ASSERT_EQUAL_INT(1, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 7);
	push(x, 7);
	_neq(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 0);
	push(x, 0);
	_and(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 0);
	push(x, 1);
	_and(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 1);
	push(x, 0);
	_and(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 1);
	push(x, 1);
	_and(x);
	TEST_ASSERT_EQUAL_INT(1, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 0);
	push(x, 0);
	_or(x);
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 0);
	push(x, 1);
	_or(x);
	TEST_ASSERT_EQUAL_INT(1, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 1);
	push(x, 0);
	_or(x);
	TEST_ASSERT_EQUAL_INT(1, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);

	push(x, 1);
	push(x, 1);
	_or(x);
	TEST_ASSERT_EQUAL_INT(1, T(x));
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	pop(x);
}

void test_interpreter_atom() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C code = ATOM(x, 13, ATOM(x, 7, 0));
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(7, T(x));
	TEST_ASSERT_EQUAL_INT(13, S(x));
}

void test_interpreter_primitive() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C code = ATOM(x, 13, ATOM(x, 7, PRIMITIVE(x, &_add, PRIMITIVE(x, &_dup, 0))));
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(20, T(x));
	TEST_ASSERT_EQUAL_INT(20, S(x));
}

void test_interpreter_branch() {
	C size = 256;
	B block[size];
	X* x = init(block, size);

	C code = BRANCH(x, ATOM(x, 7, 0), ATOM(x, 13, 0), 0);

	push(x, 1);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(7, T(x));
	pop(x);

	push(x, 0);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(13, T(x));
}

void test_interpreter_continued_branch() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C code = BRANCH(x, ATOM(x, 7, 0), ATOM(x, 13, 0), ATOM(x, 21, 0));

	push(x, 1);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(21, T(x));
	TEST_ASSERT_EQUAL_INT(7, S(x));
	pop(x);
	pop(x);

	push(x, 0);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(21, T(x));
	TEST_ASSERT_EQUAL_INT(13, S(x));
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
		
	push(x, 0);
	push(x, 5);
	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(0, T(x));
	TEST_ASSERT_EQUAL_INT(10, S(x));
}

void test_interpreter_word() {
	C size = 512;
	B block[size];
	X* x = init(block, size);

	C square = PRIMITIVE(x, &_dup, PRIMITIVE(x, &_mul, 0));

	// Tail call
	C code = ATOM(x, 5, WORD(x, square, 0));

	inner(x, code);
	TEST_ASSERT_EQUAL_INT(1, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(25, T(x));
	pop(x);

	// Non tail call
	code = ATOM(x, 3, WORD(x, square, ATOM(x, 13, 0)));

	inner(x, code);
	TEST_ASSERT_EQUAL_INT(2, depth(K(x)));
	TEST_ASSERT_EQUAL_INT(13, T(x));
	TEST_ASSERT_EQUAL_INT(9, S(x));
}

//////void test_allot() {
//////	C size = 256;
//////	B block[size];
//////	X* ctx = init(block, size);
//////
//////	C free_nodes = (size - sizeof(X)) / sizeof(PAIR);
//////
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_INT(0, ctx->here - BOTTOM(ctx));
//////	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
//////	allot(ctx, 3);
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_INT(3, ctx->here - BOTTOM(ctx));
//////	TEST_ASSERT_EQUAL_INT(BOTTOM(ctx) + 3 + RESERVED(ctx), ctx->there);
//////	allot(ctx, 13);
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_INT(16, ctx->here - BOTTOM(ctx));
//////	allot(ctx, 1);
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_INT(17, ctx->here - BOTTOM(ctx));
//////	TEST_ASSERT_LESS_THAN(free_nodes, depth(ctx->free));
//////
//////	C fnodes = depth(ctx->free);
//////	B* here = ctx->here;
//////	C reserved = RESERVED(ctx);
//////
//////	// Ensure reserved memory is 0 to allow next tests to pass
//////	allot(ctx, RESERVED(ctx));
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved, ctx->here);
//////	TEST_ASSERT_EQUAL_INT(0, RESERVED(ctx));
//////	TEST_ASSERT_EQUAL_INT(fnodes, depth(ctx->free));
//////
//////	allot(ctx, sizeof(C) * 8);
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*8, ctx->here);
//////	TEST_ASSERT_EQUAL_INT(fnodes - 4, depth(ctx->free));
//////
//////	allot(ctx, -(sizeof(C) * 4));
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*4, ctx->here);
//////	TEST_ASSERT_EQUAL_INT(fnodes - 2, depth(ctx->free));
//////	allot(ctx, -(sizeof(C) - 1));
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*3 + 1, ctx->here);
//////	TEST_ASSERT_EQUAL_INT(fnodes - 2, depth(ctx->free));
//////	allot(ctx, -(sizeof(C) + 1));
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(C)*2, ctx->here);
//////	TEST_ASSERT_EQUAL_INT(fnodes - 1, depth(ctx->free));
//////
//////	allot(ctx, 2048);
//////	TEST_ASSERT_EQUAL_INT(ERR_NOT_ENOUGH_MEMORY, ctx->err);
//////}
//////
//////void test_align() {
//////	C size = 256;
//////	B block[size];
//////	X* ctx = init(block, size);
//////
//////	// This ensures here will be aligned with a pair 
//////	allot(ctx, RESERVED(ctx));
//////
//////	allot(ctx, 1);
//////	TEST_ASSERT_NOT_EQUAL_INT(ctx->here, ALIGN(ctx->here, sizeof(C)));
//////
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////
//////	align(ctx);
//////	TEST_ASSERT_EQUAL_INT(ctx->here, ALIGN(ctx->here, sizeof(C)));
//////
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
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
////////	X* ctx = init(block, size);
////////
////////	C free_nodes = (size - sizeof(X)) / sizeof(PAIR);
////////
////////	// TODO
////////}
//////
//////void test_header_body_reveal() {
//////	C size = 2048;
//////	B block[size];
//////	X* ctx = init(block, size);
//////
//////	B* here = ctx->here;
//////	PAIR* w = header(ctx, "test", 4);
//////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//////	TEST_ASSERT_EQUAL_PTR(ctx->here, ALIGN(here, sizeof(C)) + sizeof(C) + 4 + 1);
//////	TEST_ASSERT(IS(ATOM, w));
//////	TEST_ASSERT_EQUAL_INT(4, COUNT(NFA(w)));
//////	TEST_ASSERT_EQUAL_STRING("test", NFA(w));
//////	TEST_ASSERT_EQUAL_INT(((B*)REF(REF(w))) + 4, DFA(w));
//////	TEST_ASSERT_EQUAL_INT(0, CFA(w));
//////
//////	TEST_ASSERT_NULL(ctx->dict);
//////
//////	PAIR* cfa = cons(ctx, ATOM, 7, cons(ctx, ATOM, 13, 0));
//////	body(ctx, w, cfa);
//////
//////	TEST_ASSERT_EQUAL_INT(cfa, CFA(w));
//////	TEST_ASSERT_EQUAL_INT(7, CFA(w)->value);
//////	TEST_ASSERT_EQUAL_INT(13, NEXT(CFA(w))->value);
//////
//////	reveal(ctx, w);
//////
//////	TEST_ASSERT_EQUAL_PTR(w, ctx->dict);
//////	TEST_ASSERT_NULL(NEXT(ctx->dict));
//////
//////	TEST_ASSERT_FALSE(IS_IMMEDIATE(w));
//////
//////	_immediate(ctx);
//////
//////	TEST_ASSERT_TRUE(IS_IMMEDIATE(w));
//////}
//////
////////void test_find() {
////////	C size = 512;
////////	B block[size];
////////	X* ctx = init(block, size);
////////
////////	PAIR* dup = reveal(ctx, header(ctx, "dup", 3));
////////	PAIR* swap = reveal(ctx, header(ctx, "swap", 4));
////////	PAIR* test = reveal(ctx, header(ctx, "test", 4));
////////
////////	TEST_ASSERT_EQUAL_PTR(dup, find(ctx, "dup", 3));
////////	TEST_ASSERT_EQUAL_PTR(swap, find(ctx, "swap", 4));
////////	TEST_ASSERT_EQUAL_PTR(test, find(ctx, "test", 4));
////////	TEST_ASSERT_EQUAL_PTR(0, find(ctx, "nop", 3));
////////
////////	TEST_ASSERT_EQUAL_PTR(test, ctx->dict);
////////	TEST_ASSERT_EQUAL_PTR(swap, NEXT(ctx->dict));
////////	TEST_ASSERT_EQUAL_PTR(dup, NEXT(NEXT(ctx->dict)));
////////	TEST_ASSERT_EQUAL_PTR(0, NEXT(NEXT(NEXT(ctx->dict))));
////////}
////////
////////void test_dodo_initialization() {
////////	C size = 4096;
////////	B block[size];
////////	X* ctx = dodo(init(block, size));
////////
////////	TEST_ASSERT_EQUAL_PTR(&_add, CFA(find(ctx, "+", 1))->value);
////////}
////////
////////void test_fib() {
////////	C size = 8192;
////////	B block[size];
////////	X* ctx = init(block, size);
////////
////////	// : fib dup 1 > if 1- dup 1- recurse swap recurse + then ;
////////
////////	ctx->ip =
////////		cons(ctx, (C)&_dup, T_PRIMITIVE,
////////		cons(ctx, 1, ATOM,
////////		cons(ctx, (C)&_gt, T_PRIMITIVE,
////////		cons(ctx,
////////			0,
////////			T_BRANCH,
////////			cons(ctx, 1, ATOM,
////////			cons(ctx, (C)&_sub, T_PRIMITIVE,
////////			cons(ctx, (C)&_dup, T_PRIMITIVE,
////////			cons(ctx, 1, ATOM,
////////			cons(ctx, (C)&_sub, T_PRIMITIVE,
////////			cons(ctx, (C)&_rec, T_PRIMITIVE,
////////			cons(ctx, (C)&_swap, T_PRIMITIVE,
////////			cons(ctx, (C)&_rec, T_PRIMITIVE,	
////////			cons(ctx, (C)&_add, T_PRIMITIVE, 0)))))))))))));
////////
////////		ctx->dstack = cons(ctx, 6, ATOM, 0);
////////		ctx->rstack = cons(ctx, (C)ctx->ip, T_WORD, 0);
////////
////////		inner(ctx);
////////
////////		//printf("%ld\n", ctx->dstack->value);
////////}
//////
int main() {
	UNITY_BEGIN();

	RUN_TEST(test_depth);

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_push);
	RUN_TEST(test_pop);
	RUN_TEST(test_cons);

	RUN_TEST(test_dup);
	RUN_TEST(test_swap);
	RUN_TEST(test_over);
	RUN_TEST(test_rot);
	RUN_TEST(test_drop);
	RUN_TEST(test_binops);

	RUN_TEST(test_interpreter_atom);
	RUN_TEST(test_interpreter_primitive);
	RUN_TEST(test_interpreter_branch);
	RUN_TEST(test_interpreter_continued_branch);
	RUN_TEST(test_interpreter_recursion);
	RUN_TEST(test_interpreter_word);

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
