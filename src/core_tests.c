#include<stdlib.h>

#include "unity.h"
#include "core.h"

void setUp() {}

void tearDown() {}

#define f_nodes(ctx)	(((ctx->size - ALIGN(sizeof(CTX), 2*sizeof(CELL))) / (2*sizeof(CELL))) - 2)

void test_CORE_init() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);
		
	TEST_ASSERT_NOT_EQUAL(0, ctx);

	TEST_ASSERT_EQUAL(size, ctx->size);

	TEST_ASSERT_EQUAL_INT(0, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(f_nodes(ctx), ctx->ftotal);
	TEST_ASSERT_EQUAL_INT(ctx->ftotal, ctx->free);

	TEST_ASSERT_EQUAL_INT(((BYTE*)ctx) + sizeof(CTX), BOTTOM(ctx));
	TEST_ASSERT_EQUAL_INT(BOTTOM(ctx), ctx->here);
	TEST_ASSERT_EQUAL_INT(ALIGN(ctx->here, 2*sizeof(CELL)), ctx->there);
	TEST_ASSERT_EQUAL_INT(ALIGN(((BYTE*)ctx) + size - 2*sizeof(CELL) - 1, 2*sizeof(CELL)), TOP(ctx));
	TEST_ASSERT_EQUAL_INT(TOP(ctx), ctx->dstack);
	TEST_ASSERT_EQUAL_INT(TOP(ctx) - 2*sizeof(CELL), ctx->fstack);
	TEST_ASSERT_EQUAL_INT(0, ctx->rstack);
	TEST_ASSERT_EQUAL_INT(0, ctx->xstack);

	TEST_ASSERT_EQUAL_INT(0, ctx->latest);
	TEST_ASSERT_EQUAL_INT(0, TOS(ctx));
	//TEST_ASSERT_EQUAL_INT(0, R(ctx));
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(0, ctx->status);

	TEST_ASSERT_NULL(ctx->ibuf);
	TEST_ASSERT_EQUAL_INT(0, ctx->ipos);
}

void test_CORE_cons() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL list = cons(ctx, 7, cons(ctx, 11, cons(ctx, 13, 0)));

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 3, ctx->free);
	TEST_ASSERT_EQUAL_INT(3, length(list));
	TEST_ASSERT_EQUAL_INT(7, CAR(list));
	TEST_ASSERT_EQUAL_INT(11, CAR(CDR(list)));
	TEST_ASSERT_EQUAL_INT(13, CAR(CDR(CDR(list))));

	CELL p = cons(ctx, 7, 0);

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 4, ctx->free);
	TEST_ASSERT_EQUAL_INT(7, CAR(p));
	TEST_ASSERT_EQUAL_INT(0, CDR(p));
}

void test_CORE_reclaim() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL list = cons(ctx, 7, cons(ctx, 11, cons(ctx, 13, 0)));
	CELL tail = reclaim(ctx, list);

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 2, ctx->free);
	TEST_ASSERT_EQUAL_INT(2, length(tail));
	TEST_ASSERT_EQUAL_INT(11, CAR(tail));
	TEST_ASSERT_EQUAL_INT(13, CAR(CDR(tail)));

	tail = reclaim(ctx, reclaim(ctx, tail));

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx), ctx->free);
	TEST_ASSERT_EQUAL_INT(0, length(tail));
}

void test_CORE_reclaim_list() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL list = 
		cons(ctx, 5, AS(ATOM,
		cons(ctx, 
			cons(ctx, 7,  AS(ATOM,
			cons(ctx, 11, AS(ATOM,
			cons(ctx, 13, AS(ATOM, 0)))))), AS(LIST, 
		cons(ctx, 17, AS(ATOM, 0))))));

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 6, ctx->free);
	TEST_ASSERT_EQUAL(ATOM, TYPE(list));
	TEST_ASSERT_EQUAL_INT(5, CAR(list));
	TEST_ASSERT_EQUAL(LIST, TYPE(NEXT(list)));
	TEST_ASSERT_EQUAL_INT(3, length(CAR(NEXT(list))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(CAR(NEXT(list))));
	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(NEXT(list))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(CAR(NEXT(list)))));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(NEXT(list)))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(CAR(NEXT(list))))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(NEXT(CAR(NEXT(list))))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(list))));
	TEST_ASSERT_EQUAL_INT(17, CAR(NEXT(NEXT(list))));

	CELL tail = reclaim(ctx, list);

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 5, ctx->free);
	TEST_ASSERT_EQUAL(LIST, TYPE(tail));
	TEST_ASSERT_EQUAL_INT(3, length(CAR(tail)));
	TEST_ASSERT_EQUAL(ATOM, TYPE(CAR(tail)));
	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(tail)));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(CAR(tail))));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(tail))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(CAR(tail)))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(NEXT(CAR(tail)))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(tail)));
	TEST_ASSERT_EQUAL_INT(17, CAR(NEXT(tail)));

	tail = reclaim(ctx, tail);

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 1, ctx->free);
	TEST_ASSERT_EQUAL(ATOM, TYPE(tail));
	TEST_ASSERT_EQUAL_INT(17, CAR(tail));

	tail = reclaim(ctx, tail);

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx), ctx->free);
	TEST_ASSERT_EQUAL_INT(0, tail);
}

void test_CORE_clone() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL l = 
		cons(ctx, 7, AS(ATOM, 
		cons(ctx, 11, AS(PRIM, 
		cons(ctx, 
			cons(ctx, 13, AS(ATOM,
			cons(ctx, 17, AS(ATOM, 0)))), AS(LIST,
		cons(ctx, 19, AS(ATOM, 0))))))));

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 6, ctx->free);
	TEST_ASSERT_EQUAL_INT(4, length(l));
	TEST_ASSERT_EQUAL(ATOM, TYPE(l));
	TEST_ASSERT_EQUAL_INT(7, CAR(l));
	TEST_ASSERT_EQUAL(PRIM, TYPE(NEXT(l)));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(l)));
	TEST_ASSERT_EQUAL(LIST, TYPE(NEXT(NEXT(l))));
	TEST_ASSERT_EQUAL_INT(2, length(NEXT(NEXT(l))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(CAR(NEXT(NEXT(l)))));
	TEST_ASSERT_EQUAL_INT(13, CAR(CAR(NEXT(NEXT(l)))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(CAR(NEXT(NEXT(l))))));
	TEST_ASSERT_EQUAL_INT(17, CAR(NEXT(CAR(NEXT(NEXT(l))))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(NEXT(l)))));
	TEST_ASSERT_EQUAL_INT(19, CAR(NEXT(NEXT(NEXT(l)))));

	CELL c = clone(ctx, l);

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 12, ctx->free);
	TEST_ASSERT_EQUAL_INT(4, length(c));
	TEST_ASSERT_EQUAL(ATOM, TYPE(c));
	TEST_ASSERT_EQUAL_INT(7, CAR(c));
	TEST_ASSERT_EQUAL(PRIM, TYPE(NEXT(c)));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(c)));
	TEST_ASSERT_EQUAL(LIST, TYPE(NEXT(NEXT(c))));
	TEST_ASSERT_EQUAL_INT(2, length(NEXT(NEXT(c))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(CAR(NEXT(NEXT(c)))));
	TEST_ASSERT_EQUAL_INT(13, CAR(CAR(NEXT(NEXT(c)))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(CAR(NEXT(NEXT(c))))));
	TEST_ASSERT_EQUAL_INT(17, CAR(NEXT(CAR(NEXT(NEXT(c))))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(NEXT(c)))));
	TEST_ASSERT_EQUAL_INT(19, CAR(NEXT(NEXT(NEXT(c)))));

	TEST_ASSERT_NOT_EQUAL_INT(l, c);
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(l), NEXT(c));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(NEXT(l)), NEXT(NEXT(c)));
	TEST_ASSERT_NOT_EQUAL_INT(CAR(NEXT(NEXT(l))), CAR(NEXT(NEXT(c))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(CAR(NEXT(NEXT(l)))), NEXT(CAR(NEXT(NEXT(c)))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(NEXT(NEXT(l))), NEXT(NEXT(NEXT(c))));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_init);
	RUN_TEST(test_CORE_cons);
	RUN_TEST(test_CORE_reclaim);
	RUN_TEST(test_CORE_reclaim_list);
	RUN_TEST(test_CORE_clone);

	return UNITY_END();
}

