#include<stdlib.h>
#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_pair_types() {
	PAIR* p = malloc(sizeof(PAIR));

	// TODO
}

void test_depth() {
	// TODO
}

void test_block_initialization() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TEST_ASSERT_EQUAL_INT(NIL, ctx->err);

	TEST_ASSERT_EQUAL_INT(BOTTOM(ctx), ((BYTE*)ctx) + sizeof(CTX));
	TEST_ASSERT_EQUAL_INT(BOTTOM(ctx), ctx->here);
	TEST_ASSERT_EQUAL_INT(ctx->there, ALIGN(ctx->here, sizeof(PAIR)));
	TEST_ASSERT_EQUAL_INT(
		ALIGN(((BYTE*)ctx) + size - sizeof(PAIR) - 1, sizeof(PAIR)), 
		ctx->top);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->top->prev);
	TEST_ASSERT_EQUAL_INT(NIL, NEXT(ctx->there));
	TEST_ASSERT_EQUAL_INT(ctx->top, ctx->free);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->stacks.next);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->stacks.stack);

	TEST_ASSERT_EQUAL_INT(NIL, ctx->dict);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);
	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));

	for (PAIR* p = ctx->there; p <= ctx->top; p++) {
		TEST_ASSERT(IS(T_FREE, p));
	}
}

void test_allot() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(0, ctx->here - BOTTOM(ctx));
	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	allot(ctx, 3);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(3, ctx->here - BOTTOM(ctx));
	TEST_ASSERT_EQUAL_INT(BOTTOM(ctx) + 3 + RESERVED(ctx), ctx->there);
	allot(ctx, 13);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(16, ctx->here - BOTTOM(ctx));
	allot(ctx, 1);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(17, ctx->here - BOTTOM(ctx));
	TEST_ASSERT_LESS_THAN(free_nodes, depth(ctx->free));

	CELL fnodes = depth(ctx->free);
	BYTE* here = ctx->here;
	CELL reserved = RESERVED(ctx);

	// Ensure reserved memory is 0 to allow next tests to pass
	allot(ctx, RESERVED(ctx));
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_PTR(here + reserved, ctx->here);
	TEST_ASSERT_EQUAL_INT(0, RESERVED(ctx));
	TEST_ASSERT_EQUAL_INT(fnodes, depth(ctx->free));

	allot(ctx, sizeof(CELL) * 8);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(CELL)*8, ctx->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, depth(ctx->free));

	allot(ctx, -(sizeof(CELL) * 4));
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(CELL)*4, ctx->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 2, depth(ctx->free));
	allot(ctx, -(sizeof(CELL) - 1));
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(CELL)*3 + 1, ctx->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 2, depth(ctx->free));
	allot(ctx, -(sizeof(CELL) + 1));
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_PTR(here + reserved + sizeof(CELL)*2, ctx->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 1, depth(ctx->free));

	allot(ctx, 2048);
	TEST_ASSERT_EQUAL_INT(ERR_NOT_ENOUGH_MEMORY, ctx->err);
}

void test_align() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	// This ensures here will be aligned with a pair 
	allot(ctx, RESERVED(ctx));

	allot(ctx, 1);
	TEST_ASSERT_NOT_EQUAL_INT(ctx->here, ALIGN(ctx->here, sizeof(CELL)));

	TEST_ASSERT_EQUAL_INT(0, ctx->err);

	align(ctx);
	TEST_ASSERT_EQUAL_INT(ctx->here, ALIGN(ctx->here, sizeof(CELL)));

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
}

void test_cons() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	PAIR* p = cons(ctx, T_LIST, 13, NIL);
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(NIL, NEXT(p));
	TEST_ASSERT_EQUAL_INT(13, p->value);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	TEST_ASSERT(IS(T_LIST, p));
	PAIR* p2 = cons(ctx, T_ATOM, 17, p);
	TEST_ASSERT_EQUAL_INT(free_nodes - 2, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(p, NEXT(p2));
	TEST_ASSERT_EQUAL_INT(17, p2->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(p2)->value);
	TEST_ASSERT_EQUAL_INT(2, depth(p2));

	while (depth(ctx->free) > 0) { cons(ctx, T_ATOM, 1, NIL); }

	TEST_ASSERT_EQUAL_INT(0, depth(ctx->free));
	PAIR* p3 = cons(ctx, T_ATOM, 21, NIL);
	TEST_ASSERT_EQUAL_PTR(NIL, p3);
	TEST_ASSERT_EQUAL_INT(ERR_NOT_ENOUGH_MEMORY, ctx->err);
}

void test_reclaim() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	PAIR* p = cons(ctx, T_ATOM, 21, cons(ctx, T_ATOM, 17, cons(ctx, T_ATOM, 13, NIL)));
	TEST_ASSERT_EQUAL_INT(3, depth(p));
	TEST_ASSERT_EQUAL_INT(free_nodes - 3, depth(ctx->free));
	p = reclaim(ctx, p);
	TEST_ASSERT_EQUAL_INT(2, depth(p));
	TEST_ASSERT_EQUAL_INT(free_nodes - 2, depth(ctx->free));
	p = reclaim(ctx, p);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
	p = reclaim(ctx, p);
	TEST_ASSERT_EQUAL_INT(0, depth(p));
	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	p = reclaim(ctx, p);
	TEST_ASSERT_EQUAL_INT(0, depth(p));
	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));

	p = cons(ctx, T_LIST, (CELL)cons(ctx, T_ATOM, 13, cons(ctx, T_ATOM, 17, NIL)), cons(ctx, T_ATOM, 21, NIL));
	TEST_ASSERT_EQUAL_INT(2, depth(p));
	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)p->value));
	TEST_ASSERT_EQUAL_INT(free_nodes - 4, depth(ctx->free));
	p = reclaim(ctx, p);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
}

//void test_inner_interpreter_atom_primitive_and_branch() {
//	CELL size = 2048;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	inner(ctx);
//
//	TEST_ASSERT_EQUAL_INT(0, depth(ctx->stacks.stack));
//
//	ctx->ip = cons(ctx, T_ATOM, 13, cons(ctx, T_ATOM, 7, NIL));
//	inner(ctx);
//
//	TEST_ASSERT_EQUAL_INT(2, depth(ctx->stacks.stack));
//	TEST_ASSERT_EQUAL_INT(7, ctx->stacks.stack->value);
//	TEST_ASSERT_EQUAL_INT(13, NEXT(ctx->stacks.stack)->value);
//
//	ctx->ip = cons(ctx, T_PRIMITIVE, (CELL)&_add, NIL);
//	inner(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
//	TEST_ASSERT_EQUAL_INT(20, ctx->stacks.stack->value);
//
//	// TODO: Test branch
//}

//void test_inner_interpreter_words() {
	//PAIR* cfa = pcons(ctx, &_dup, pcons(ctx, &_mul, NIL));
	//reveal(ctx, body(ctx, header(ctx, "square", 6, T_PRIM), cfa));

	//TEST_ASSERT_EQUAL_PTR(cfa, CFA(ctx->dict));

	//ctx->ip = CFA(ctx->dict);
	//inner(ctx);

	//TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	//TEST_ASSERT_EQUAL_INT(400, ctx->stacks.stack->value);

	//ctx->ip = 
	//	ncons(ctx, 1, 
	//		bcons(ctx, 
	//			ncons(ctx, 7, NIL), 
	//			ncons(ctx, 13, NIL), 
	//			ncons(ctx, 5, pcons(ctx, &_add, NIL))));

	//inner(ctx);

	//TEST_ASSERT_EQUAL_INT(12, ctx->stacks.stack->value);

	//ctx->ip = 
	//	ncons(ctx, 0, 
	//		bcons(ctx, 
	//			ncons(ctx, 7, NIL), 
	//			ncons(ctx, 13, NIL), 
	//			ncons(ctx, 5, pcons(ctx, &_add, NIL))));

	//inner(ctx);

	//TEST_ASSERT_EQUAL_INT(18, ctx->stacks.stack->value);

	//// TODO: Test a list node in CFA
//}

//void test_append() {
//}
//
//void test_copy() {
//	CELL size = 256;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);
//
//	// TODO
//}

void test_binops() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stacks.stack = cons(ctx, T_ATOM, 13, cons(ctx, T_ATOM, 7, NIL));
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->stacks.stack));

	_add(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 20);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 13, NIL));
	_sub(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 6);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 13, NIL));
	_mul(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 91);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 5, cons(ctx, T_ATOM, 30, NIL));
	_div(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 6);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 2, cons(ctx, T_ATOM, 9, NIL));
	_mod(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 13, NIL));
	_gt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 13, cons(ctx, T_ATOM, 7, NIL));
	_gt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 7, NIL));
	_gt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 13, NIL));
	_lt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 13, cons(ctx, T_ATOM, 7, NIL));
	_lt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 7, NIL));
	_lt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 13, NIL));
	_eq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 13, cons(ctx, T_ATOM, 7, NIL));
	_eq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 7, NIL));
	_eq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 13, NIL));
	_neq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 13, cons(ctx, T_ATOM, 7, NIL));
	_neq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 7, NIL));
	_neq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 0, cons(ctx, T_ATOM, 0, NIL));
	_and(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 1, cons(ctx, T_ATOM, 0, NIL));
	_and(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 0, cons(ctx, T_ATOM, 1, NIL));
	_and(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 1, cons(ctx, T_ATOM, 1, NIL));
	_and(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 0, cons(ctx, T_ATOM, 0, NIL));
	_or(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 1, cons(ctx, T_ATOM, 0, NIL));
	_or(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 0, cons(ctx, T_ATOM, 1, NIL));
	_or(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);

	ctx->stacks.stack = cons(ctx, T_ATOM, 1, cons(ctx, T_ATOM, 1, NIL));
	_or(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->stacks.stack));
	POP(ctx);
}

void test_dup() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, NIL);
	_dup(ctx);
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->stacks.stack));
	TEST_ASSERT_EQUAL_INT(7, TOS(ctx));
	TEST_ASSERT_EQUAL_INT(7, SOS(ctx));
}

void test_dup_list() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	PUSH(ctx, T_LIST, (CELL)cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 13, NIL)));

	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)ctx->stacks.stack->value));
	TEST_ASSERT_EQUAL_INT(free_nodes - 3, depth(ctx->free));

	_dup(ctx);

	TEST_ASSERT_EQUAL_INT(free_nodes - 6, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->stacks.stack));
	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)ctx->stacks.stack->value));
	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)NEXT(ctx->stacks.stack)->value));
	TEST_ASSERT_NOT_EQUAL_INT(ctx->stacks.stack, NEXT(ctx->stacks.stack));
	TEST_ASSERT_NOT_EQUAL_INT(ctx->stacks.stack->value, NEXT(ctx->stacks.stack)->value);
	TEST_ASSERT_NOT_EQUAL_INT(
		NEXT(((PAIR*)ctx->stacks.stack->value)), 
		NEXT(((PAIR*)(NEXT(ctx->stacks.stack)->value))));
}

void test_swap() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stacks.stack = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 13, NIL));
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->stacks.stack));
	TEST_ASSERT_EQUAL_INT(7, TOS(ctx));
	TEST_ASSERT_EQUAL_INT(13, SOS(ctx));
	_swap(ctx);
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->stacks.stack));
	TEST_ASSERT_EQUAL_INT(13, TOS(ctx));
	TEST_ASSERT_EQUAL_INT(7, SOS(ctx));
}

void test_header_body_reveal() {
	CELL size = 2048;
	BYTE block[size];
	CTX* ctx = init(block, size);

	BYTE* here = ctx->here;
	PAIR* w = header(ctx, "test", 4);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_PTR(ctx->here, ALIGN(here, sizeof(CELL)) + sizeof(CELL) + 4 + 1);
	TEST_ASSERT(IS(T_ATOM, w));
	TEST_ASSERT_EQUAL_INT(4, COUNT(NFA(w)));
	TEST_ASSERT_EQUAL_STRING("test", NFA(w));
	TEST_ASSERT_EQUAL_INT(((BYTE*)REF(REF(w))) + 4, DFA(w));
	TEST_ASSERT_EQUAL_INT(NIL, CFA(w));

	TEST_ASSERT_NULL(ctx->dict);

	PAIR* cfa = cons(ctx, T_ATOM, 7, cons(ctx, T_ATOM, 13, NIL));
	body(ctx, w, cfa);

	TEST_ASSERT_EQUAL_INT(cfa, CFA(w));
	TEST_ASSERT_EQUAL_INT(7, CFA(w)->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(CFA(w))->value);

	reveal(ctx, w);

	TEST_ASSERT_EQUAL_PTR(w, ctx->dict);
	TEST_ASSERT_NULL(NEXT(ctx->dict));

	TEST_ASSERT_FALSE(IS_IMMEDIATE(w));

	_immediate(ctx);

	TEST_ASSERT_TRUE(IS_IMMEDIATE(w));
}

//void test_find() {
//	CELL size = 512;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	PAIR* dup = reveal(ctx, header(ctx, "dup", 3));
//	PAIR* swap = reveal(ctx, header(ctx, "swap", 4));
//	PAIR* test = reveal(ctx, header(ctx, "test", 4));
//
//	TEST_ASSERT_EQUAL_PTR(dup, find(ctx, "dup", 3));
//	TEST_ASSERT_EQUAL_PTR(swap, find(ctx, "swap", 4));
//	TEST_ASSERT_EQUAL_PTR(test, find(ctx, "test", 4));
//	TEST_ASSERT_EQUAL_PTR(NIL, find(ctx, "nop", 3));
//
//	TEST_ASSERT_EQUAL_PTR(test, ctx->dict);
//	TEST_ASSERT_EQUAL_PTR(swap, NEXT(ctx->dict));
//	TEST_ASSERT_EQUAL_PTR(dup, NEXT(NEXT(ctx->dict)));
//	TEST_ASSERT_EQUAL_PTR(NIL, NEXT(NEXT(NEXT(ctx->dict))));
//}
//
//void test_dodo_initialization() {
//	CELL size = 4096;
//	BYTE block[size];
//	CTX* ctx = dodo(init(block, size));
//
//	TEST_ASSERT_EQUAL_PTR(&_add, CFA(find(ctx, "+", 1))->value);
//}
//
//void test_fib() {
//	CELL size = 8192;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	// : fib dup 1 > if 1- dup 1- recurse swap recurse + then ;
//
//	ctx->ip =
//		cons(ctx, (CELL)&_dup, T_PRIM,
//		cons(ctx, 1, T_ATOM,
//		cons(ctx, (CELL)&_gt, T_PRIM,
//		cons(ctx,
//			NIL,
//			T_BRANCH,
//			cons(ctx, 1, T_ATOM,
//			cons(ctx, (CELL)&_sub, T_PRIM,
//			cons(ctx, (CELL)&_dup, T_PRIM,
//			cons(ctx, 1, T_ATOM,
//			cons(ctx, (CELL)&_sub, T_PRIM,
//			cons(ctx, (CELL)&_rec, T_PRIM,
//			cons(ctx, (CELL)&_swap, T_PRIM,
//			cons(ctx, (CELL)&_rec, T_PRIM,	
//			cons(ctx, (CELL)&_add, T_PRIM, NIL)))))))))))));
//
//		ctx->stacks.stack = cons(ctx, 6, T_ATOM, NIL);
//		ctx->rstack = cons(ctx, (CELL)ctx->ip, T_WORD, NIL);
//
//		inner(ctx);
//
//		//printf("%ld\n", ctx->stacks.stack->value);
//}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_pair_types);
	RUN_TEST(test_depth);

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_allot);
	RUN_TEST(test_align);
	RUN_TEST(test_cons);
	RUN_TEST(test_reclaim);
	
	//RUN_TEST(test_inner_interpreter_atom_primitive_and_branch);

	//RUN_TEST(test_fib);

	//RUN_TEST(test_append);
	//RUN_TEST(test_copy);

	RUN_TEST(test_binops);
	RUN_TEST(test_dup);
	RUN_TEST(test_dup_list);
	RUN_TEST(test_swap);

	RUN_TEST(test_header_body_reveal);
	//RUN_TEST(test_find);

	//RUN_TEST(test_dodo_initialization);

	return UNITY_END();
}
