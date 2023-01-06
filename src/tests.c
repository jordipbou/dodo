#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_block_initialization() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TEST_ASSERT_EQUAL_INT(ctx->bottom, ((BYTE*)ctx) + sizeof(CTX));
	TEST_ASSERT_EQUAL_INT(ctx->there, ALIGN(ctx->here, sizeof(PAIR)));
	TEST_ASSERT_EQUAL_INT(NIL, ((PAIR*)ctx->top)->value);
	TEST_ASSERT_EQUAL_INT(NIL, ((PAIR*)ctx->there)->next);
	TEST_ASSERT_EQUAL_INT(ctx->top, ctx->free);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->err);
}

void test_free_depth() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

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
}

void test_align() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	// TODO
}

void test_cons() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);

	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	PAIR* p = ncons(ctx, 13, NIL);
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(NIL, NEXT(p));
	TEST_ASSERT_EQUAL_INT(13, p->value);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	PAIR* p2 = ncons(ctx, 17, p);
	TEST_ASSERT_EQUAL_INT(free_nodes - 2, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(p, NEXT(p2));
	TEST_ASSERT_EQUAL_INT(17, p2->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(p2)->value);
	TEST_ASSERT_EQUAL_INT(2, depth(p2));
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
}

void test_binops() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	PAIR* p = ncons(ctx, 13, ncons(ctx, 7, NIL));
	TEST_ASSERT_EQUAL_INT(2, depth(p));

	p = add(ctx, p);
	TEST_ASSERT_EQUAL_INT(p->value, 20);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = sub(ctx, ncons(ctx, 7, ncons(ctx, 13, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 6);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = mul(ctx, ncons(ctx, 7, ncons(ctx, 13, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 91);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = div(ctx, ncons(ctx, 5, ncons(ctx, 30, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 6);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = mod(ctx, ncons(ctx, 2, ncons(ctx, 9, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);


	p = gt(ctx, ncons(ctx, 7, ncons(ctx, 13, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = gt(ctx, ncons(ctx, 13, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = gt(ctx, ncons(ctx, 7, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);


	p = lt(ctx, ncons(ctx, 7, ncons(ctx, 13, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = lt(ctx, ncons(ctx, 13, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = lt(ctx, ncons(ctx, 7, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);


	p = eq(ctx, ncons(ctx, 7, ncons(ctx, 13, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = eq(ctx, ncons(ctx, 13, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = eq(ctx, ncons(ctx, 7, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);


	p = neq(ctx, ncons(ctx, 7, ncons(ctx, 13, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = neq(ctx, ncons(ctx, 13, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = neq(ctx, ncons(ctx, 7, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);


	p = gte(ctx, ncons(ctx, 7, ncons(ctx, 13, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = gte(ctx, ncons(ctx, 13, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = gte(ctx, ncons(ctx, 7, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);


	p = lte(ctx, ncons(ctx, 7, ncons(ctx, 13, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = lte(ctx, ncons(ctx, 13, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = lte(ctx, ncons(ctx, 7, ncons(ctx, 7, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);


	p = and(ctx, ncons(ctx, 0, ncons(ctx, 0, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = and(ctx, ncons(ctx, 1, ncons(ctx, 0, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = and(ctx, ncons(ctx, 0, ncons(ctx, 1, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = and(ctx, ncons(ctx, 1, ncons(ctx, 1, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);


	p = or(ctx, ncons(ctx, 0, ncons(ctx, 0, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 0);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = or(ctx, ncons(ctx, 1, ncons(ctx, 0, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = or(ctx, ncons(ctx, 0, ncons(ctx, 1, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

	p = or(ctx, ncons(ctx, 1, ncons(ctx, 1, NIL)));
	TEST_ASSERT_EQUAL_INT(p->value, 1);
	TEST_ASSERT_EQUAL_INT(1, depth(p));
	reclaim(ctx, p);

}

void test_create() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	BYTE* here = ctx->here;
	PAIR* w = create(ctx, "test", 4, NIL);
	TEST_ASSERT_EQUAL_INT(T_WORD, TYPE(w));
	TEST_ASSERT_EQUAL_INT(4, *((CELL*)(NFA(w)->value)));
	TEST_ASSERT_EQUAL_STRING("test", ((BYTE*)NFA(w)->value) + sizeof(CELL));
	TEST_ASSERT_EQUAL_INT(here + sizeof(CELL) + 4 + 1, DFA(w)->value);
	TEST_ASSERT_EQUAL_INT(here + sizeof(CELL) + 4 + 1, CFA(w)->value);
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

	inner(ctx, cons(ctx, (CELL)&_add, T_PRIM, NIL));

	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(20, ctx->dstack->value);

	PAIR* square = create(ctx, "square", 6, cons(ctx, (CELL)&_dup, T_PRIM, cons(ctx, (CELL)&_mul, T_PRIM, NIL)));

	inner(ctx, CFA(square));

	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(400, ctx->dstack->value);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_free_depth);
	RUN_TEST(test_allot);
	RUN_TEST(test_align);
	RUN_TEST(test_cons);
	RUN_TEST(test_reclaim);
	RUN_TEST(test_binops);

	RUN_TEST(test_create);

	RUN_TEST(test_inner_interpreter);

	return UNITY_END();
}

//void setUp() {}
//
//void tearDown() {}
//
//void test_block_initialization() {
//	CELL size = 256;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	TEST_ASSERT_EQUAL_INT(ctx->bottom, ((BYTE*)ctx) + sizeof(CTX));
//	TEST_ASSERT_EQUAL_INT(ctx->there, ALIGN(ctx->here, sizeof(PAIR)));
//	TEST_ASSERT_EQUAL_INT(NIL, ((PAIR*)ctx->top)->cdr);
//	TEST_ASSERT_EQUAL_INT(NIL, ((PAIR*)ctx->there)->car);
//	TEST_ASSERT_EQUAL_INT(NIL, ctx->dstack);
//	TEST_ASSERT_EQUAL_INT(ctx->top, ctx->free);
//	TEST_ASSERT_EQUAL_INT(NIL, ctx->rstack);
//	TEST_ASSERT_EQUAL_INT(NIL, ctx->dict);
//	TEST_ASSERT_EQUAL_INT(S_OK, ctx->status);
//}
//
//void test_free() {
//	CELL size = 256;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);
//	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
//}
//
//void test_stack() {
//	CELL size = 2048;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	CELL free_nodes = ((size - sizeof(CTX)) / sizeof(PAIR));
//
//	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
//	TEST_ASSERT_EQUAL_INT(0, depth(ctx->dstack));
//	push(ctx, 13);
//	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
//	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	//push(ctx, 17);
//	//TEST_ASSERT_EQUAL_INT(free_nodes - 2, depth(ctx->free));
//	//TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
//	//CELL val = pop(ctx);
//	//TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
//	//TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
//	//TEST_ASSERT_EQUAL_INT(17, val);
//	//val = pop(ctx);
//	//TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
//	//TEST_ASSERT_EQUAL_INT(0, depth(ctx->dstack));
//	//TEST_ASSERT_EQUAL_INT(13, val);
//}
//
//void test_allot() {
//	CELL size = 2048;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	CELL free_nodes = ((size - sizeof(CTX)) / sizeof(PAIR));
//
//	TEST_ASSERT_EQUAL_INT(0, ctx->here - ctx->bottom);
//	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
//	push(ctx, 3);
//	_allot(ctx);
//	TEST_ASSERT_EQUAL_INT(3, ctx->here - ctx->bottom);
//	TEST_ASSERT_EQUAL_INT(ctx->bottom + 3 + RESERVED(ctx), ctx->there);
//	push(ctx, 13);
//	_allot(ctx);
//	TEST_ASSERT_EQUAL_INT(16, ctx->here - ctx->bottom);
//	push(ctx, 1);
//	_allot(ctx);
//	TEST_ASSERT_EQUAL_INT(17, ctx->here - ctx->bottom);
//	TEST_ASSERT_LESS_THAN(free_nodes, depth(ctx->free));
//}
//
//////void test_store_and_fetch() {
//////	CELL size = 2048;
//////	BYTE block[size];
//////	CTX* c = init(block, size);
//////
//////	HALF here = c->here;
//////	allot(c, sizeof(CELL));
//////	push(c, 13);
//////	push(c, here);
//////	_store(c);
//////	TEST_ASSERT_EQUAL_INT(13, *((CELL*)(((BYTE*)c) + here)));
//////	TEST_ASSERT_EQUAL_INT(0, DEPTH(c));
//////	_fetch(c);
//////	TEST_ASSERT_EQUAL_INT(1, DEPTH(c));
//////	TEST_ASSERT_EQUAL_INT(13, pop(c));
//////}
//////
//////void test_header() {
//////	CELL size = 2048;
//////	BYTE block[size];
//////	CTX* c = init(block, size);
//////
//////	HALF here = c->here;
//////	_header(c, "+", T_F_PRIMITIVE);
//////	NODE word = NODE_AT(c, here);
//////	TEST_ASSERT_EQUAL_INT(here + sizeof(NODE), c->here);
//////	TEST_ASSERT_EQUAL_INT(c->dict, word.next);
//////	TEST_ASSERT_EQUAL_INT(T_F_PRIMITIVE, word.type);
//////	TEST_ASSERT_EQUAL_INT(1, word.len);
//////	TEST_ASSERT_EQUAL_INT8('+', word.data[0]);
//////	TEST_ASSERT_EQUAL_INT8(0, word.data[1]);
//////	TEST_ASSERT_EQUAL_INT8(0, word.data[2]);
//////	TEST_ASSERT_EQUAL_INT8(0, word.data[3]);
//////	TEST_ASSERT_EQUAL_INT8(0, word.data[4]);
//////	TEST_ASSERT_EQUAL_INT8(0, word.data[5]);
//////}
//////
//////void test_inner() {
//////	CELL size = 2048;
//////	BYTE block[size];
//////	CTX* c = init(block, size);
//////
//////	push(c, 7);
//////	push(c, 11);
//////	// TODO: Create a word definition for add primitive
//////	// TODO: Execute word and test stack has only the value 18
//////}
//
//int main() {
//	UNITY_BEGIN();
//
//	RUN_TEST(test_block_initialization);
//	RUN_TEST(test_free);
//	RUN_TEST(test_stack);
//	//RUN_TEST(test_allot);
//
//
//	//RUN_TEST(test_store_and_fetch);
//
//	//RUN_TEST(test_header);
//
//	//RUN_TEST(test_inner);
//
//	return UNITY_END();
//}
