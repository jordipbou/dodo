#include<stdlib.h>

#include "unity.h"
#include "bit.h"

void setUp() {}

void tearDown() {}

void test_BIT_and() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 11, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	and(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(3, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, 0, AS(ATOM, cons(ctx, 0, AS(ATOM, 0))));
	and(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, -1, AS(ATOM, cons(ctx, 0, AS(ATOM, 0))));
	and(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, 0, AS(ATOM, cons(ctx, -1, AS(ATOM, 0))));
	and(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, -1, AS(ATOM, cons(ctx, -1, AS(ATOM, 0))));	
	and(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(-1, CAR(TOS(ctx)));
}

void test_BIT_or() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));
	or(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(15, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, 0, AS(ATOM, cons(ctx, 0, AS(ATOM, 0))));
	or(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, -1, AS(ATOM, cons(ctx, 0, AS(ATOM, 0))));
	or(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(-1, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, 0, AS(ATOM, cons(ctx, -1, AS(ATOM, 0))));
	or(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(-1, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, -1, AS(ATOM, cons(ctx, -1, AS(ATOM, 0))));
	or(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(-1, CAR(TOS(ctx)));
}

void test_BIT_invert() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, 0));
	invert(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(-8, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, 0, AS(ATOM, 0));
	invert(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(-1, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, 1, AS(ATOM, 0));
	invert(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(-2, CAR(TOS(ctx)));

	TOS(ctx) = cons(ctx, -1, AS(ATOM, 0));
	invert(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(0, CAR(TOS(ctx)));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_BIT_and);
	RUN_TEST(test_BIT_or);
	RUN_TEST(test_BIT_invert);

	return UNITY_END();
}

