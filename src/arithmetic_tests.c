#include<stdlib.h>

#include "unity.h"
#include "arithmetic.h"

void setUp() {}

void tearDown() {}

void test_ARITHMETIC_add() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 11, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	add(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(18, CAR(TOS(ctx)));
}

void test_ARITHMETIC_sub() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));
	sub(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(4, CAR(TOS(ctx)));
}

void test_ARITHMETIC_mul() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 11, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	mul(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(77, CAR(TOS(ctx)));
}

void test_ARITHMETIC_division() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 11, AS(ATOM, cons(ctx, 77, AS(ATOM, 0))));
	division(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(7, CAR(TOS(ctx)));
}

void test_ARITHMETIC_division_by_zero() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 0, AS(ATOM, cons(ctx, 77, AS(ATOM, 0))));

	division(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_DIVISION_BY_ZERO, ctx->err);
}

void test_ARITHMETIC_mod() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));
	mod(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(4, CAR(TOS(ctx)));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_ARITHMETIC_add);
	RUN_TEST(test_ARITHMETIC_sub);
	RUN_TEST(test_ARITHMETIC_mul);
	RUN_TEST(test_ARITHMETIC_division);
	RUN_TEST(test_ARITHMETIC_division_by_zero);
	RUN_TEST(test_ARITHMETIC_mod);

	return UNITY_END();
}

