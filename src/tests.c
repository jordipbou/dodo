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
	TEST_ASSERT_EQUAL_INT(NIL, ((PAIR*)ctx->top)->cdr);
	TEST_ASSERT_EQUAL_INT(NIL, ((PAIR*)ctx->there)->car);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->dstack);
	TEST_ASSERT_EQUAL_INT(ctx->top, ctx->free);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->rstack);
	TEST_ASSERT_EQUAL_INT(NIL, ctx->dict);
	TEST_ASSERT_EQUAL_INT(S_OK, ctx->status);
}

void test_free() {
	CELL size = 256;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = (size - sizeof(CTX)) / sizeof(PAIR);
	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
}

void test_stack() {
	CELL size = 2048;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = ((size - sizeof(CTX)) / sizeof(PAIR));

	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(0, depth(ctx->dstack));
	push(ctx, 13);
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	push(ctx, 17);
	TEST_ASSERT_EQUAL_INT(free_nodes - 2, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(2, depth(ctx->dstack));
	CELL val = pop(ctx);
	TEST_ASSERT_EQUAL_INT(free_nodes - 1, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(1, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(17, val);
	val = pop(ctx);
	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	TEST_ASSERT_EQUAL_INT(0, depth(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(13, val);
}

void test_allot() {
	CELL size = 2048;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free_nodes = ((size - sizeof(CTX)) / sizeof(PAIR));

	TEST_ASSERT_EQUAL_INT(0, ctx->here - ctx->bottom);
	TEST_ASSERT_EQUAL_INT(free_nodes, depth(ctx->free));
	push(ctx, 3);
	_allot(ctx);
	TEST_ASSERT_EQUAL_INT(3, ctx->here - ctx->bottom);
	TEST_ASSERT_EQUAL_INT(ctx->bottom + 3 + RESERVED(ctx), ctx->there);
	push(ctx, 13);
	_allot(ctx);
	TEST_ASSERT_EQUAL_INT(16, ctx->here - ctx->bottom);
	push(ctx, 1);
	_allot(ctx);
	TEST_ASSERT_EQUAL_INT(17, ctx->here - ctx->bottom);
	TEST_ASSERT_LESS_THAN(free_nodes, depth(ctx->free));
}

////void test_store_and_fetch() {
////	CELL size = 2048;
////	BYTE block[size];
////	CTX* c = init(block, size);
////
////	HALF here = c->here;
////	allot(c, sizeof(CELL));
////	push(c, 13);
////	push(c, here);
////	_store(c);
////	TEST_ASSERT_EQUAL_INT(13, *((CELL*)(((BYTE*)c) + here)));
////	TEST_ASSERT_EQUAL_INT(0, DEPTH(c));
////	_fetch(c);
////	TEST_ASSERT_EQUAL_INT(1, DEPTH(c));
////	TEST_ASSERT_EQUAL_INT(13, pop(c));
////}
////
////void test_header() {
////	CELL size = 2048;
////	BYTE block[size];
////	CTX* c = init(block, size);
////
////	HALF here = c->here;
////	_header(c, "+", T_F_PRIMITIVE);
////	NODE word = NODE_AT(c, here);
////	TEST_ASSERT_EQUAL_INT(here + sizeof(NODE), c->here);
////	TEST_ASSERT_EQUAL_INT(c->dict, word.next);
////	TEST_ASSERT_EQUAL_INT(T_F_PRIMITIVE, word.type);
////	TEST_ASSERT_EQUAL_INT(1, word.len);
////	TEST_ASSERT_EQUAL_INT8('+', word.data[0]);
////	TEST_ASSERT_EQUAL_INT8(0, word.data[1]);
////	TEST_ASSERT_EQUAL_INT8(0, word.data[2]);
////	TEST_ASSERT_EQUAL_INT8(0, word.data[3]);
////	TEST_ASSERT_EQUAL_INT8(0, word.data[4]);
////	TEST_ASSERT_EQUAL_INT8(0, word.data[5]);
////}
////
////void test_inner() {
////	CELL size = 2048;
////	BYTE block[size];
////	CTX* c = init(block, size);
////
////	push(c, 7);
////	push(c, 11);
////	// TODO: Create a word definition for add primitive
////	// TODO: Execute word and test stack has only the value 18
////}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_block_initialization);
	RUN_TEST(test_free);
	RUN_TEST(test_stack);
	RUN_TEST(test_allot);


	//RUN_TEST(test_store_and_fetch);

	//RUN_TEST(test_header);

	//RUN_TEST(test_inner);

	return UNITY_END();
}
