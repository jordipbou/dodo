#include<stdlib.h>

#include "unity.h"
#include "comparison.h"

void setUp() {}

void tearDown() {}

void test_COMPARISON_gt() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))));
	gt(ctx);
	TEST_ASSERT_EQUAL_INT(1, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));

	TOS(ctx) = cons(ctx, 13, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	gt(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	gt(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
}

void test_COMPARISON_lt() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))));
	lt(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));

	TOS(ctx) = cons(ctx, 13, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	lt(ctx);
	TEST_ASSERT_EQUAL_INT(1, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	lt(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
}

void test_COMPARISON_eq() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))));
	eq(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));

	TOS(ctx) = cons(ctx, 13, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	eq(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	eq(ctx);
	TEST_ASSERT_EQUAL_INT(1, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
}

void test_COMPARISON_neq() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))));
	neq(ctx);
	TEST_ASSERT_EQUAL_INT(1, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));

	TOS(ctx) = cons(ctx, 13, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	neq(ctx);
	TEST_ASSERT_EQUAL_INT(1, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	neq(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_COMPARISON_gt);
	RUN_TEST(test_COMPARISON_lt);
	RUN_TEST(test_COMPARISON_eq);
	RUN_TEST(test_COMPARISON_neq);

	return UNITY_END();
}

