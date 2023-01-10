#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

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
	TEST_ASSERT_EQUAL_INT(NIL, ((PAIR*)ctx->top)->value);
	TEST_ASSERT_EQUAL_INT(NIL, ((PAIR*)ctx->there)->next);
	TEST_ASSERT_EQUAL_INT(ctx->top, ctx->free);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->dstack);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->rstack);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->dict);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
}

void test_allot() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(0, ctx->here - ctx->bottom);
	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	allot(ctx, 3);
	TEST_ASSERT_EQUAL_INT(3, ctx->here - ctx->bottom);
	TEST_ASSERT_EQUAL_INT(ctx->bottom + 3 + RESERVED(ctx), ctx->there);
	allot(ctx, 13);
	TEST_ASSERT_EQUAL_INT(16, ctx->here - ctx->bottom);
	allot(ctx, 1);
	TEST_ASSERT_EQUAL_INT(17, ctx->here - ctx->bottom);
	TEST_ASSERT_LESS_THAN(free_nodes, depth(ctx->free));

	CELL fnodes = depth(ctx->free);

	allot(ctx, 64);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, depth(ctx->free));

	allot(ctx, -32);
	TEST_ASSERT_EQUAL_INT(fnodes - 2, depth(ctx->free));
	allot(ctx, -7);
	TEST_ASSERT_EQUAL_INT(fnodes - 2, depth(ctx->free));
	allot(ctx, -9);
	TEST_ASSERT_EQUAL_INT(fnodes - 1, depth(ctx->free));

	allot(ctx, 2048);
	TEST_ASSERT_EQUAL_INT(E_NOT_ENOUGH_MEMORY, ctx->err);
}

void test_align() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	// TODO: ?	
}

void test_cons() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	PAIR* p = cons(ctx, 13, T_WORD, NIL);
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(NIL, NEXT(p));
	TEST_ASSERT_EQUAL_INT(13, p->value);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	TEST_ASSERT_EQUAL_INT(T_WORD, TYPE(p));
	PAIR* p2 = ncons(ctx, 17, p);
	TEST_ASSERT_EQUAL_INT(free_nodes - 2, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(p, NEXT(p2));
	TEST_ASSERT_EQUAL_INT(17, p2->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(p2)->value);
	TEST_ASSERT_EQUAL_INT(2, depth(p2));

	while (depth(ctx->free) > 0) { ncons(ctx, 1, NIL); }

	TEST_ASSERT_EQUAL_INT(0, depth(ctx->free));
	PAIR* p3 = ncons(ctx, 21, NIL);
	TEST_ASSERT_EQUAL_PTR(NIL, p3);
	TEST_ASSERT_EQUAL_INT(NIL, TYPE(p3));
}

void test_reclaim() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	PAIR* p = ncons(ctx, 21, ncons(ctx, 17, ncons(ctx, 13, NIL)));
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

	p = lcons(ctx, ncons(ctx, 13, ncons(ctx, 17, NIL)), ncons(ctx, 21, NIL));
	TEST_ASSERT_EQUAL_INT(2, depth(p));
	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)p->value));
	TEST_ASSERT_EQUAL_INT(free_nodes - 4, depth(ctx->free));
	p = reclaim(ctx, p);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
}

void test_copy() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	// TODO
}

void test_binops() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->dstack = ncons(ctx, 13, ncons(ctx, 7, NIL));
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));

	add(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 20);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 13, NIL));
	sub(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 6);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 13, NIL));
	mul(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 91);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 5, ncons(ctx, 30, NIL));
	div(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 6);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 2, ncons(ctx, 9, NIL));
	mod(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 13, NIL));
	gt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 13, ncons(ctx, 7, NIL));
	gt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 7, NIL));
	gt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 13, NIL));
	lt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 13, ncons(ctx, 7, NIL));
	lt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 7, NIL));
	lt(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 13, NIL));
	eq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 13, ncons(ctx, 7, NIL));
	eq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 7, NIL));
	eq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 13, NIL));
	neq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 13, ncons(ctx, 7, NIL));
	neq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 7, NIL));
	neq(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 0, ncons(ctx, 0, NIL));
	and(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 1, ncons(ctx, 0, NIL));
	and(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 0, ncons(ctx, 1, NIL));
	and(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 1, ncons(ctx, 1, NIL));
	and(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 0, ncons(ctx, 0, NIL));
	or(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 0);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 1, ncons(ctx, 0, NIL));
	or(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 0, ncons(ctx, 1, NIL));
	or(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);

	ctx->dstack = ncons(ctx, 1, ncons(ctx, 1, NIL));
	or(ctx);
	TEST_ASSERT_EQUAL_INT(TOS(ctx), 1);
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	POP(ctx);
}

void test_dup() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->dstack = ncons(ctx, 7, NIL);
	dup(ctx);
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(7, TOS(ctx));
	TEST_ASSERT_EQUAL_INT(7, SOS(ctx));
}

void test_dup_list() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	ctx->dstack = lcons(ctx, ncons(ctx, 7, ncons(ctx, 13, NIL)), NIL);

	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)ctx->dstack->value));
	TEST_ASSERT_EQUAL_INT(free_nodes - 3, depth(ctx->free));

	dup(ctx);

	TEST_ASSERT_EQUAL_INT(free_nodes - 6, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)ctx->dstack->value));
	TEST_ASSERT_EQUAL_INT(2, depth((PAIR*)NEXT(ctx->dstack)->value));
	TEST_ASSERT_NOT_EQUAL_INT(ctx->dstack, NEXT(ctx->dstack));
	TEST_ASSERT_NOT_EQUAL_INT(ctx->dstack->value, NEXT(ctx->dstack)->value);
	TEST_ASSERT_NOT_EQUAL_INT(
		NEXT(((PAIR*)ctx->dstack->value)), 
		NEXT(((PAIR*)(NEXT(ctx->dstack)->value))));
}

void test_swap() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->dstack = ncons(ctx, 7, ncons(ctx, 13, NIL));
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(7, TOS(ctx));
	TEST_ASSERT_EQUAL_INT(13, SOS(ctx));
	swap(ctx);
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(13, TOS(ctx));
	TEST_ASSERT_EQUAL_INT(7, SOS(ctx));
}

void test_header_body_reveal() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	BYTE* here = ctx->here;
	PAIR* w = header(ctx, "test", 4, T_WORD);
	TEST_ASSERT_EQUAL_INT(T_WORD, TYPE(w));
	TEST_ASSERT_EQUAL_INT(4, *((CELL*)(NFA(w)->value)));
	TEST_ASSERT_EQUAL_STRING("test", ((BYTE*)NFA(w)->value) + sizeof(CELL));
	TEST_ASSERT_EQUAL_INT(here + sizeof(CELL) + 4 + 1, DFA(w)->value);
	TEST_ASSERT_EQUAL_INT(NIL, CFA(w));

	TEST_ASSERT_NULL(ctx->dict);

	PAIR* cfa = ncons(ctx, 7, ncons(ctx, 13, NIL));
	body(ctx, w, cfa);

	TEST_ASSERT_EQUAL_INT(cfa, CFA(w));
	TEST_ASSERT_EQUAL_INT(7, CFA(w)->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(CFA(w))->value);

	reveal(ctx, w);

	TEST_ASSERT_EQUAL_PTR(w, ctx->dict);
	TEST_ASSERT_NULL(ctx->dict->next);

	TEST_ASSERT_FALSE(IS_IMMEDIATE(w));

	immediate(ctx);

	TEST_ASSERT_TRUE(IS_IMMEDIATE(w));
}

void test_find() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	PAIR* dup = reveal(ctx, header(ctx, "dup", 3, T_PRIM));
	PAIR* swap = reveal(ctx, header(ctx, "swap", 4, T_PRIM));
	PAIR* test = reveal(ctx, header(ctx, "test", 4, T_WORD));

	TEST_ASSERT_EQUAL_PTR(dup, find(ctx, "dup", 3));
	TEST_ASSERT_EQUAL_PTR(swap, find(ctx, "swap", 4));
	TEST_ASSERT_EQUAL_PTR(test, find(ctx, "test", 4));
	TEST_ASSERT_EQUAL_PTR(NIL, find(ctx, "nop", 3));

	TEST_ASSERT_EQUAL_PTR(test, ctx->dict);
	TEST_ASSERT_EQUAL_PTR(swap, NEXT(ctx->dict));
	TEST_ASSERT_EQUAL_PTR(dup, NEXT(NEXT(ctx->dict)));
	TEST_ASSERT_EQUAL_PTR(NIL, NEXT(NEXT(NEXT(ctx->dict))));
}

void test_inner_interpreter() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	inner(ctx, NIL);

	TEST_ASSERT_EQUAL_INT(0, depth(ctx->dstack));

	inner(ctx, ncons(ctx, 13, ncons(ctx, 7, NIL)));

	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(7, ctx->dstack->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(ctx->dstack)->value);

	inner(ctx, cons(ctx, (CELL)&add, T_PRIM, NIL));

	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(20, ctx->dstack->value);

	PAIR* cfa = pcons(ctx, &dup, pcons(ctx, &mul, NIL));
	reveal(ctx, body(ctx, header(ctx, "square", 6, T_PRIM), cfa));

	TEST_ASSERT_EQUAL_PTR(cfa, CFA(ctx->dict));

	inner(ctx, CFA(ctx->dict));

	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(400, ctx->dstack->value);

	// TODO: Test a list node in CFA
}

void test_dodo_initialization() {
	CELL size = 4096;
	BYTE block[size];
	CTX* ctx = dodo(init(block, size));

	TEST_ASSERT_EQUAL_PTR(&add, CFA(find(ctx, "+", 1))->value);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_allot);
	RUN_TEST(test_align);
	RUN_TEST(test_cons);
	RUN_TEST(test_reclaim);
	RUN_TEST(test_copy);

	RUN_TEST(test_binops);
	RUN_TEST(test_dup);
	RUN_TEST(test_dup_list);
	RUN_TEST(test_swap);

	RUN_TEST(test_header_body_reveal);
	RUN_TEST(test_find);

	RUN_TEST(test_inner_interpreter);

	RUN_TEST(test_dodo_initialization);

	return UNITY_END();
}
