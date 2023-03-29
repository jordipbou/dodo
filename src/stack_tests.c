#include<stdlib.h>

#include "unity.h"
#include "stack.h"

void setUp() {}

void tearDown() {}

void test_STACK_dupl_atom() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, TOS(ctx)));
	dupl(ctx);

	TEST_ASSERT_EQUAL_INT(2, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ctx->ftotal - 2, ctx->free);
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(7, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(TOS(ctx))));

	TOS(ctx) = cons(ctx, 11, AS(ATOM, TOS(ctx)));
	dupl(ctx);

	TEST_ASSERT_EQUAL_INT(4, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ctx->ftotal - 4, ctx->free);
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(11, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(NEXT(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(NEXT(NEXT(TOS(ctx))))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(NEXT(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(NEXT(NEXT(TOS(ctx))))));
}

void test_STACK_dupl_list() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = 
		cons(ctx, 
			cons(ctx, 17, AS(ATOM,
			cons(ctx, 13, AS(ATOM,
			cons(ctx, 
				cons(ctx, 11, AS(ATOM,
				cons(ctx, 7, AS(ATOM, 0)))),
				AS(LIST, 0)))))),
			AS(LIST, TOS(ctx)));

	// (17, 13, (11, 7))
	TEST_ASSERT_EQUAL_INT(ctx->ftotal - 6, ctx->free);
	TEST_ASSERT_EQUAL_INT(1, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(3, length(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(17, CAR(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(CAR(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(NEXT(CAR(TOS(ctx))))));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(NEXT(CAR(TOS(ctx)))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(NEXT(NEXT(CAR(TOS(ctx)))))));
	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(NEXT(NEXT(CAR(TOS(ctx)))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(NEXT(NEXT(CAR(TOS(ctx))))))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(NEXT(NEXT(CAR(TOS(ctx))))))));

	dupl(ctx);

	TEST_ASSERT_EQUAL_INT(ctx->ftotal - 12, ctx->free);
	TEST_ASSERT_EQUAL_INT(2, length(TOS(ctx)));

	TEST_ASSERT_EQUAL_INT(LIST, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(3, length(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(17, CAR(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(CAR(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(NEXT(CAR(TOS(ctx))))));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(NEXT(CAR(TOS(ctx)))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(NEXT(NEXT(CAR(TOS(ctx)))))));
	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(NEXT(NEXT(CAR(TOS(ctx)))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(NEXT(NEXT(CAR(TOS(ctx))))))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(NEXT(NEXT(CAR(TOS(ctx))))))));

	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(3, length(CAR(NEXT(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(NEXT(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(17, CAR(CAR(NEXT(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(NEXT(TOS(ctx))))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(CAR(NEXT(TOS(ctx))))));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(NEXT(CAR(NEXT(TOS(ctx)))))));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(NEXT(CAR(NEXT(TOS(ctx))))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(NEXT(NEXT(CAR(NEXT(TOS(ctx))))))));
	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(NEXT(NEXT(CAR(NEXT(TOS(ctx))))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(NEXT(NEXT(CAR(NEXT(TOS(ctx)))))))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(NEXT(NEXT(CAR(NEXT(TOS(ctx)))))))));

	TEST_ASSERT_NOT_EQUAL_INT(TOS(ctx), NEXT(TOS(ctx)));
	TEST_ASSERT_NOT_EQUAL_INT(CAR(TOS(ctx)), CAR(NEXT(TOS(ctx))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(CAR(TOS(ctx))), NEXT(CAR(NEXT(TOS(ctx)))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(NEXT(CAR(TOS(ctx)))), NEXT(NEXT(CAR(NEXT(TOS(ctx))))));
	TEST_ASSERT_NOT_EQUAL_INT(CAR(NEXT(NEXT(CAR(TOS(ctx))))), CAR(NEXT(NEXT(CAR(NEXT(TOS(ctx)))))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(CAR(NEXT(NEXT(CAR(TOS(ctx)))))), NEXT(CAR(NEXT(NEXT(CAR(NEXT(TOS(ctx))))))));
}

void test_STACK_swap_1() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 11, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	TEST_ASSERT_EQUAL_INT(2, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(11, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(TOS(ctx))));
	swap(ctx);
	TEST_ASSERT_EQUAL_INT(2, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(7, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(TOS(ctx))));
}

void test_STACK_swap_2() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 11, AS(ATOM, cons(ctx, cons(ctx, 7, AS(ATOM, cons(ctx, 5, AS(ATOM, 0)))), AS(LIST, 0))));
	TEST_ASSERT_EQUAL_INT(2, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(11, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(NEXT(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(5, CAR(NEXT(CAR(NEXT(TOS(ctx))))));
	swap(ctx);
	TEST_ASSERT_EQUAL_INT(2, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(5, CAR(NEXT(CAR(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(TOS(ctx))));
}

void test_STACK_drop() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, TOS(ctx)));

	drop(ctx);

	TEST_ASSERT_EQUAL_INT(0, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ctx->ftotal, ctx->free);

	TOS(ctx) = 
		cons(ctx,
			cons(ctx, 7, AS(ATOM,
			cons(ctx,
				cons(ctx, 11, AS(ATOM,
				cons(ctx, 13, AS(ATOM, 0)))),
			AS(LIST, 0)))),
		AS(LIST, TOS(ctx)));

	drop(ctx);

	TEST_ASSERT_EQUAL_INT(0, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ctx->ftotal, ctx->free);
}

void test_STACK_over() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));

	over(ctx);

	TEST_ASSERT_EQUAL_INT(3, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(11, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(NEXT(TOS(ctx)))));

	TOS(ctx) = 
		cons(ctx, 7, AS(ATOM, 
		cons(ctx, 
			cons(ctx, 11, AS(ATOM,
			cons(ctx, 13, AS(ATOM, 0)))),
		AS(LIST, 0))));

	over(ctx);

	TEST_ASSERT_EQUAL_INT(3, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(CAR(TOS(ctx)))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(TOS(ctx))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(NEXT(TOS(ctx))), TOS(ctx));
	TEST_ASSERT_NOT_EQUAL_INT(CAR(NEXT(NEXT(TOS(ctx)))), CAR(TOS(ctx)));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(CAR(NEXT(NEXT(TOS(ctx))))), NEXT(CAR(TOS(ctx))));
}

void test_STACK_rot() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))))));

	rot(ctx);

	TEST_ASSERT_EQUAL_INT(3, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(13, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(NEXT(TOS(ctx)))));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_STACK_dupl_atom);
	RUN_TEST(test_STACK_dupl_list);
	RUN_TEST(test_STACK_swap_1);
	RUN_TEST(test_STACK_swap_2);
	RUN_TEST(test_STACK_drop);
	RUN_TEST(test_STACK_over);
	RUN_TEST(test_STACK_rot);

	return UNITY_END();
}

