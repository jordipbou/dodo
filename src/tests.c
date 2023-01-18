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

	TEST_ASSERT_EQUAL_INT(ctx->bottom, ((BYTE*)ctx) + sizeof(CTX));
	TEST_ASSERT_EQUAL_INT(ctx->bottom, ctx->here);
	TEST_ASSERT_EQUAL_INT(ctx->there, ALIGN(ctx->here, sizeof(PAIR)));
	TEST_ASSERT_EQUAL_INT(
		ALIGN(((BYTE*)ctx) + size - sizeof(PAIR) - 1, sizeof(PAIR)), 
		ctx->top);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->top->prev);
	TEST_ASSERT_EQUAL_INT(NIL, NEXT(ctx->there));
	TEST_ASSERT_EQUAL_INT(ctx->top, ctx->free);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->dstack);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->rstack);

	TEST_ASSERT_EQUAL_INT(NIL, ctx->dict);

	TEST_ASSERT_EQUAL_INT(NIL, ctx->ip);

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
	TEST_ASSERT_EQUAL_INT(0, ctx->here - ctx->bottom);
	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	allot(ctx, 3);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(3, ctx->here - ctx->bottom);
	TEST_ASSERT_EQUAL_INT(ctx->bottom + 3 + RESERVED(ctx), ctx->there);
	allot(ctx, 13);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(16, ctx->here - ctx->bottom);
	allot(ctx, 1);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(17, ctx->here - ctx->bottom);
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

void test_alloc() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	PAIR* p = alloc(ctx, T_LIST, 13, NIL);
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(NIL, NEXT(p));
	TEST_ASSERT_EQUAL_INT(13, p->value);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	TEST_ASSERT(IS(T_LIST, p));
	PAIR* p2 = alloc(ctx, T_ATOM, 17, p);
	TEST_ASSERT_EQUAL_INT(free_nodes - 2, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(p, NEXT(p2));
	TEST_ASSERT_EQUAL_INT(17, p2->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(p2)->value);
	TEST_ASSERT_EQUAL_INT(2, depth(p2));

	while (depth(ctx->free) > 0) { alloc(ctx, T_ATOM, 1, NIL); }

	TEST_ASSERT_EQUAL_INT(0, depth(ctx->free));
	PAIR* p3 = alloc(ctx, T_ATOM, 21, NIL);
	TEST_ASSERT_EQUAL_PTR(NIL, p3);
	TEST_ASSERT_EQUAL_INT(ERR_NOT_ENOUGH_MEMORY, ctx->err);
}

void test_reclaim() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	PAIR* p = alloc(ctx, T_ATOM, 21, alloc(ctx, T_ATOM, 17, alloc(ctx, T_ATOM, 13, NIL)));
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

	p = alloc(ctx, T_LIST, (CELL)alloc(ctx, T_ATOM, 13, alloc(ctx, T_ATOM, 17, NIL)), alloc(ctx, T_ATOM, 21, NIL));
	TEST_ASSERT_EQUAL_INT(2, depth(p));
	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)p->value));
	TEST_ASSERT_EQUAL_INT(free_nodes - 4, depth(ctx->free));
	p = reclaim(ctx, p);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
}

void test_inner_interpreter_atom_primitive_and_branch() {
	CELL size = 2048;
	BYTE block[size];
	CTX* ctx = init(block, size);

	inner(ctx);

	TEST_ASSERT_EQUAL_INT(0, depth(ctx->dstack));

	ctx->ip = alloc(ctx, T_ATOM, 13, alloc(ctx, T_ATOM, 7, NIL));
	inner(ctx);

	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(7, ctx->dstack->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(ctx->dstack)->value);

	ctx->ip = alloc(ctx, T_PRIMITIVE, (CELL)&_add, NIL);
	inner(ctx);

	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(20, ctx->dstack->value);

	// TODO: Test branch
}

//void test_inner_interpreter_words() {
	//PAIR* cfa = palloc(ctx, &_dup, palloc(ctx, &_mul, NIL));
	//reveal(ctx, body(ctx, header(ctx, "square", 6, T_PRIM), cfa));

	//TEST_ASSERT_EQUAL_PTR(cfa, CFA(ctx->dict));

	//ctx->ip = CFA(ctx->dict);
	//inner(ctx);

	//TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	//TEST_ASSERT_EQUAL_INT(400, ctx->dstack->value);

	//ctx->ip = 
	//	nalloc(ctx, 1, 
	//		balloc(ctx, 
	//			nalloc(ctx, 7, NIL), 
	//			nalloc(ctx, 13, NIL), 
	//			nalloc(ctx, 5, palloc(ctx, &_add, NIL))));

	//inner(ctx);

	//TEST_ASSERT_EQUAL_INT(12, ctx->dstack->value);

	//ctx->ip = 
	//	nalloc(ctx, 0, 
	//		balloc(ctx, 
	//			nalloc(ctx, 7, NIL), 
	//			nalloc(ctx, 13, NIL), 
	//			nalloc(ctx, 5, palloc(ctx, &_add, NIL))));

	//inner(ctx);

	//TEST_ASSERT_EQUAL_INT(18, ctx->dstack->value);

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
//
//void test_binops() {
//	CELL size = 256;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	ctx->dstack = nalloc(ctx, 13, nalloc(ctx, 7, NIL));
//	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
//
//	_add(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 20);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 13, NIL));
//	_sub(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 6);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 13, NIL));
//	_mul(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 91);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 5, nalloc(ctx, 30, NIL));
//	_div(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 6);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 2, nalloc(ctx, 9, NIL));
//	_mod(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 13, NIL));
//	_gt(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 13, nalloc(ctx, 7, NIL));
//	_gt(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 7, NIL));
//	_gt(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 13, NIL));
//	_lt(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 13, nalloc(ctx, 7, NIL));
//	_lt(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 7, NIL));
//	_lt(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 13, NIL));
//	_eq(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 13, nalloc(ctx, 7, NIL));
//	_eq(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 7, NIL));
//	_eq(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 13, NIL));
//	_neq(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 13, nalloc(ctx, 7, NIL));
//	_neq(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 7, NIL));
//	_neq(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 0, nalloc(ctx, 0, NIL));
//	_and(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 1, nalloc(ctx, 0, NIL));
//	_and(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 0, nalloc(ctx, 1, NIL));
//	_and(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 1, nalloc(ctx, 1, NIL));
//	_and(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 0, nalloc(ctx, 0, NIL));
//	_or(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 1, nalloc(ctx, 0, NIL));
//	_or(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 0, nalloc(ctx, 1, NIL));
//	_or(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//
//	ctx->dstack = nalloc(ctx, 1, nalloc(ctx, 1, NIL));
//	_or(ctx);
//	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	POP(ctx);
//}
//
//void test_dup() {
//	CELL size = 256;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	ctx->dstack = nalloc(ctx, 7, NIL);
//	_dup(ctx);
//	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
//	TEST_ASSERT_EQUAL_INT(7, TOS(ctx));
//	TEST_ASSERT_EQUAL_INT(7, SOS(ctx));
//}
//
//void test_dup_list() {
//	CELL size = 256;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);
//
//	ctx->dstack = lalloc(ctx, nalloc(ctx, 7, nalloc(ctx, 13, NIL)), NIL);
//
//	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)ctx->dstack->value));
//	TEST_ASSERT_EQUAL_INT(free_nodes - 3, depth(ctx->free));
//
//	_dup(ctx);
//
//	TEST_ASSERT_EQUAL_INT(free_nodes - 6, depth(ctx->free));
//	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
//	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)ctx->dstack->value));
//	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)NEXT(ctx->dstack)->value));
//	TEST_ASSERT_NOT_EQUAL_INT(ctx->dstack, NEXT(ctx->dstack));
//	TEST_ASSERT_NOT_EQUAL_INT(ctx->dstack->value, NEXT(ctx->dstack)->value);
//	TEST_ASSERT_NOT_EQUAL_INT(
//		NEXT(((PAIR*)ctx->dstack->value)), 
//		NEXT(((PAIR*)(NEXT(ctx->dstack)->value))));
//}
//
//void test_swap() {
//	CELL size = 256;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	ctx->dstack = nalloc(ctx, 7, nalloc(ctx, 13, NIL));
//	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
//	TEST_ASSERT_EQUAL_INT(7, TOS(ctx));
//	TEST_ASSERT_EQUAL_INT(13, SOS(ctx));
//	_swap(ctx);
//	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
//	TEST_ASSERT_EQUAL_INT(13, TOS(ctx));
//	TEST_ASSERT_EQUAL_INT(7, SOS(ctx));
//}
//
//void test_header_body_reveal() {
//	CELL size = 2048;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	BYTE* here = ctx->here;
//	PAIR* w = header(ctx, "test", 4, T_WORD);
//	TEST_ASSERT_EQUAL_INT(0, ctx->err);
//	TEST_ASSERT_EQUAL_PTR(ctx->here, ALIGN(here, sizeof(CELL)) + sizeof(CELL) + 4 + 1);
//	TEST_ASSERT_EQUAL_INT(T_WORD, TYPE(w));
//	TEST_ASSERT_EQUAL_INT(4, *((CELL*)(NFA(w)->value)));
//	TEST_ASSERT_EQUAL_STRING("test", ((BYTE*)NFA(w)->value) + sizeof(CELL));
//	TEST_ASSERT_EQUAL_INT(((BYTE*)NFA(w)->value) + sizeof(CELL) + 5, DFA(w)->value);
//	TEST_ASSERT_EQUAL_INT(NIL, CFA(w));
//
//	TEST_ASSERT_NULL(ctx->dict);
//
//	PAIR* cfa = nalloc(ctx, 7, nalloc(ctx, 13, NIL));
//	body(ctx, w, cfa);
//
//	TEST_ASSERT_EQUAL_INT(cfa, CFA(w));
//	TEST_ASSERT_EQUAL_INT(7, CFA(w)->value);
//	TEST_ASSERT_EQUAL_INT(13, NEXT(CFA(w))->value);
//
//	reveal(ctx, w);
//
//	TEST_ASSERT_EQUAL_PTR(w, ctx->dict);
//	TEST_ASSERT_NULL(ctx->dict->next);
//
//	TEST_ASSERT_FALSE(IS_IMMEDIATE(w));
//
//	_immediate(ctx);
//
//	TEST_ASSERT_TRUE(IS_IMMEDIATE(w));
//}
//
//void test_find() {
//	CELL size = 512;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	PAIR* dup = reveal(ctx, header(ctx, "dup", 3, T_PRIM));
//	PAIR* swap = reveal(ctx, header(ctx, "swap", 4, T_PRIM));
//	PAIR* test = reveal(ctx, header(ctx, "test", 4, T_WORD));
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
//		alloc(ctx, (CELL)&_dup, T_PRIM,
//		alloc(ctx, 1, T_ATOM,
//		alloc(ctx, (CELL)&_gt, T_PRIM,
//		alloc(ctx,
//			NIL,
//			T_BRANCH,
//			alloc(ctx, 1, T_ATOM,
//			alloc(ctx, (CELL)&_sub, T_PRIM,
//			alloc(ctx, (CELL)&_dup, T_PRIM,
//			alloc(ctx, 1, T_ATOM,
//			alloc(ctx, (CELL)&_sub, T_PRIM,
//			alloc(ctx, (CELL)&_rec, T_PRIM,
//			alloc(ctx, (CELL)&_swap, T_PRIM,
//			alloc(ctx, (CELL)&_rec, T_PRIM,	
//			alloc(ctx, (CELL)&_add, T_PRIM, NIL)))))))))))));
//
//		ctx->dstack = alloc(ctx, 6, T_ATOM, NIL);
//		ctx->rstack = alloc(ctx, (CELL)ctx->ip, T_WORD, NIL);
//
//		inner(ctx);
//
//		//printf("%ld\n", ctx->dstack->value);
//}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_pair_types);
	RUN_TEST(test_depth);

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_allot);
	RUN_TEST(test_align);
	RUN_TEST(test_alloc);
	RUN_TEST(test_reclaim);
	
	RUN_TEST(test_inner_interpreter_atom_primitive_and_branch);

	//RUN_TEST(test_fib);

	//RUN_TEST(test_append);
	//RUN_TEST(test_copy);

	//RUN_TEST(test_binops);
	//RUN_TEST(test_dup);
	//RUN_TEST(test_dup_list);
	//RUN_TEST(test_swap);

	//RUN_TEST(test_header_body_reveal);
	//RUN_TEST(test_find);

	//RUN_TEST(test_dodo_initialization);

	return UNITY_END();
}
