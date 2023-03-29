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

	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, ctx->err);

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

void test_ERROR_expected_atom() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(ATOM, 0));
	EA(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(LIST, 0));
	EA(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_ATOM, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(PRIM, 0));
	EA(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_ATOM, error_var);
	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(WORD, 0));
	EA(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_ATOM, error_var);
}

void test_ERROR_expected_2_atoms() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(ATOM, 0));
	E2A(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_2_ATOMS, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));
	E2A(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(LIST, 0))));
	E2A(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_2_ATOMS, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(LIST, cons(ctx, 11, AS(ATOM, 0))));
	E2A(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_2_ATOMS, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(LIST, cons(ctx, 11, AS(LIST, 0))));
	E2A(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_2_ATOMS, error_var);

}

void test_ERROR_expected_list() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(ATOM, 0));
	EL(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_LIST, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(LIST, 0));
	EL(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(PRIM, 0));
	EL(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_LIST, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(WORD, 0));
	EL(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_LIST, error_var);
}

void test_ERROR_expected_word() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(ATOM, 0));
	EW(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_WORD, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(LIST, 0));
	EW(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_WORD, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(PRIM, 0));
	EW(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_WORD, error_var);

	error_var = 0;
	TOS(ctx) = cons(ctx, 7, AS(WORD, 0));
	EW(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_ERROR_stack_overflow);
	RUN_TEST(test_ERROR_stack_underflow);
	RUN_TEST(test_ERROR_expected_atom);
	RUN_TEST(test_ERROR_expected_2_atoms);
	RUN_TEST(test_ERROR_expected_list);
	RUN_TEST(test_ERROR_expected_word);

	return UNITY_END();
}

