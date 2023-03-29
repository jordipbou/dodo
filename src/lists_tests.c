#include<stdlib.h>

#include "unity.h"
#include "lists.h"

void setUp() {}

void tearDown() {}

void test_LISTS_empty() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	empty(ctx);

	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(0, length(CAR(TOS(ctx))));
}

void test_LISTS_list_to_stack() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	list_to_stack(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, ctx->err);
	TEST_ASSERT_EQUAL_INT(0, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->dstack));

	ctx->err = 0;

	TOS(ctx) = cons(ctx, 0, AS(ATOM, 0));

	list_to_stack(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_LIST, ctx->err);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->dstack));
}

void test_LISTS_list_to_stack_2() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	empty(ctx);

	list_to_stack(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(0, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(2, length(ctx->dstack));
}

void test_LISTS_list_to_stack_3() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0)))), AS(LIST, 0));

	list_to_stack(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(2, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(7, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(2, length(ctx->dstack));
}

void test_LISTS_stack_to_list() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	stack_to_list(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(1, length(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(0, length(CAR(TOS(ctx))));
}

void test_LISTS_stack_to_list_2() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	empty(ctx);
	list_to_stack(ctx);
	stack_to_list(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(1, length(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(0, length(CAR(TOS(ctx))));
}

void test_LISTS_stack_to_list_3() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	empty(ctx);
	list_to_stack(ctx);
	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));
	stack_to_list(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(1, length(ctx->dstack));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(TOS(ctx)))));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_LISTS_empty);
	RUN_TEST(test_LISTS_list_to_stack);
	RUN_TEST(test_LISTS_list_to_stack_2);
	RUN_TEST(test_LISTS_list_to_stack_3);
	RUN_TEST(test_LISTS_stack_to_list);
	RUN_TEST(test_LISTS_stack_to_list_2);
	RUN_TEST(test_LISTS_stack_to_list_3);

	return UNITY_END();
}

