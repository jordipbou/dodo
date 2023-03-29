#include<stdlib.h>

#include "unity.h"
#include "error.h"

void setUp() {}

void tearDown() {}

CELL error_var = 0;

void error_handler(CTX* ctx) {
	error_var = ctx->err;
	ctx->err = 0;
}

void test_ERROR_stack_overflow() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->xstack = 
		cons(ctx,
			cons(ctx, 0, AS(ATOM,
			cons(ctx, (CELL)&error_handler, AS(PRIM, 0)))),
		AS(LIST, 0));

	error_var = 0;
	OF(ctx, ctx->ftotal + 1);
	
	TEST_ASSERT_EQUAL_INT(ERR_STACK_OVERFLOW, error_var);

	error_var = 0;
	OF(ctx, 1);

	TEST_ASSERT_EQUAL_INT(0, error_var);
}

void test_ERROR_stack_underflow() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->xstack = 
		cons(ctx,
			cons(ctx, 0, AS(ATOM,
			cons(ctx, (CELL)&error_handler, AS(PRIM, 0)))),
		AS(LIST, 0));

	error_var = 0;
	UF1(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(ATOM, 0));
	UF1(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);

	error_var = 0;
	UF2(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, error_var);

	error_var = 0;
	UF3(ctx);
	
	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));
	UF1(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);

	error_var = 0;
	UF2(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);

	error_var = 0;
	UF3(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))))));
	UF1(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);

	error_var = 0;
	UF2(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);

	error_var = 0;
	UF3(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);
}

void test_ERROR_no_handler_found() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

		ctx->xstack = 
		cons(ctx,
			cons(ctx, ERR_STACK_UNDERFLOW, AS(ATOM,
			cons(ctx, (CELL)&error_handler, AS(PRIM, 0)))),
		AS(LIST, 0));

	// It should just return and not get stuck in an infinite loop.

	ERR(ctx, 1, ERR_STACK_OVERFLOW);	
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_ERROR_stack_overflow);
	RUN_TEST(test_ERROR_stack_underflow);
	RUN_TEST(test_ERROR_no_handler_found);

	return UNITY_END();
}

