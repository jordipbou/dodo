#include<stdlib.h>

#include "unity.h"
#include "core.h"

void setUp() {}

void tearDown() {}

// PAIRS

void test_PAIRS_pairs() {
	CELL pair = (CELL)malloc(2*sizeof(CELL));

	CAR(pair) = 7;	
	CDR(pair) = AS(ATOM, 8);
	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(8, NEXT(pair));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
	CDR(pair) = AS(LIST, 16);
	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(16, NEXT(pair));
	TEST_ASSERT_NOT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
	CDR(pair) = AS(PRIM, 24);
	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(24, NEXT(pair));
	TEST_ASSERT_NOT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
	CDR(pair) = AS(WORD, 32);
	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(32, NEXT(pair));
	TEST_ASSERT_NOT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_EQUAL_INT(WORD, TYPE(pair));
}

void test_PAIRS_link_pairs() {
	CELL pair = (CELL)malloc(sizeof(CELL) * 2);

	CAR(pair) = 7;
	CDR(pair) = AS(ATOM, 16);

	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(16, NEXT(pair));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
	
	LINK(pair, 32);

	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(32, NEXT(pair));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
}

void test_PAIRS_length() {
	CELL p1 = (CELL)malloc(2*sizeof(CELL));
	CELL p2 = (CELL)malloc(2*sizeof(CELL));
	CELL p3 = (CELL)malloc(2*sizeof(CELL));

	CDR(p1) = AS(ATOM, p2);
	CDR(p2) = p3;
	CDR(p3) = 0;

	TEST_ASSERT_EQUAL_INT(0, length(0));
	TEST_ASSERT_EQUAL_INT(1, length(p3));
	TEST_ASSERT_EQUAL_INT(2, length(p2));
	TEST_ASSERT_EQUAL_INT(3, length(p1));
}

void test_PAIRS_reverse() {
	CELL p1 = (CELL)malloc(2*sizeof(CELL));
	CELL p2 = (CELL)malloc(2*sizeof(CELL));
	CELL p3 = (CELL)malloc(2*sizeof(CELL));
	
	CDR(p1) = p2;
	CDR(p2) = p3;
	CDR(p3) = 0;

	CELL r = reverse(p1, 0);

	TEST_ASSERT_EQUAL_INT(p3, r);
	TEST_ASSERT_EQUAL_INT(p2, NEXT(r));
	TEST_ASSERT_EQUAL_INT(p1, NEXT(NEXT(r)));
}

// CONTEXT

#define f_nodes(ctx)	(((ctx->size - ALIGN(sizeof(CTX), 2*sizeof(CELL))) / (2*sizeof(CELL))) - 2)

void test_CONTEXT_init() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);
		
	TEST_ASSERT_NOT_EQUAL(0, ctx);

	TEST_ASSERT_EQUAL(size, ctx->size);

	TEST_ASSERT_EQUAL_INT(0, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(f_nodes(ctx), ctx->ftotal);
	TEST_ASSERT_EQUAL_INT(ctx->ftotal, ctx->free);
	TEST_ASSERT_EQUAL_INT(0, ctx->fmax);

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

	TEST_ASSERT_EQUAL_INT(0, ctx->ip);
}

void test_CONTEXT_cons() {
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

void test_CONTEXT_reclaim() {
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

void test_CONTEXT_reclaim_list() {
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

void test_CONTEXT_clone() {
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

// ERRORS

CELL error_var = 0;

void error_handler(CTX* ctx) {
	error_var = ctx->err;
	ctx->err = 0;
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

void test_ERROR_expected_list() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->xstack = 
		cons(ctx,
			cons(ctx, 0, AS(ATOM,
			cons(ctx, (CELL)&error_handler, AS(PRIM, 0)))),
		AS(LIST, 0));

	TOS(ctx) = cons(ctx, 7, AS(ATOM, 0));

	error_var = 0;
	EL(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_LIST, error_var);

	TOS(ctx) = cons(ctx, 7, AS(LIST, 0));

	error_var = 0;
	EL(ctx);

	TEST_ASSERT_EQUAL_INT(0, error_var);
}

// STACK PRIMITIVES
void test_STACK_duplicate_atom() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TOS(ctx) = cons(ctx, 7, AS(ATOM, TOS(ctx)));
	duplicate(ctx);

	TEST_ASSERT_EQUAL_INT(2, length(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ctx->ftotal - 2, ctx->free);
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(TOS(ctx))));
	TEST_ASSERT_EQUAL_INT(7, CAR(TOS(ctx)));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(TOS(ctx))));

	TOS(ctx) = cons(ctx, 11, AS(ATOM, TOS(ctx)));
	duplicate(ctx);

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

void test_STACK_duplicate_list() {
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

	duplicate(ctx);

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

// ARITHMETIC PRIMITIVES
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

// COMPARISON PRIMITIVES
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

// BIT PRIMITIVES
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

// LISTS
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

	// PAIRS
	RUN_TEST(test_PAIRS_pairs);
	RUN_TEST(test_PAIRS_link_pairs);
	RUN_TEST(test_PAIRS_length);
	RUN_TEST(test_PAIRS_reverse);

	// CONTEXT
	RUN_TEST(test_CONTEXT_init);
	RUN_TEST(test_CONTEXT_cons);
	RUN_TEST(test_CONTEXT_reclaim);
	RUN_TEST(test_CONTEXT_reclaim_list);
	RUN_TEST(test_CONTEXT_clone);

	// ERRORS
	RUN_TEST(test_ERROR_no_handler_found);
	RUN_TEST(test_ERROR_stack_overflow);
	RUN_TEST(test_ERROR_stack_underflow);
	RUN_TEST(test_ERROR_expected_list);

	// STACK PRIMITIVES
	RUN_TEST(test_STACK_duplicate_atom);
	RUN_TEST(test_STACK_duplicate_list);
	RUN_TEST(test_STACK_swap_1);
	RUN_TEST(test_STACK_swap_2);
	RUN_TEST(test_STACK_drop);
	RUN_TEST(test_STACK_over);
	RUN_TEST(test_STACK_rot);

	// ARITHMETIC PRIMITIVES
	RUN_TEST(test_ARITHMETIC_add);
	RUN_TEST(test_ARITHMETIC_sub);
	RUN_TEST(test_ARITHMETIC_mul);
	RUN_TEST(test_ARITHMETIC_division);
	RUN_TEST(test_ARITHMETIC_division_by_zero);
	RUN_TEST(test_ARITHMETIC_mod);

	// COMPARISON PRIMITIVES
	RUN_TEST(test_COMPARISON_gt);
	RUN_TEST(test_COMPARISON_lt);
	RUN_TEST(test_COMPARISON_eq);
	RUN_TEST(test_COMPARISON_neq);

	// BIT PRIMITIVES
	RUN_TEST(test_BIT_and);
	RUN_TEST(test_BIT_or);
	RUN_TEST(test_BIT_invert);

	// LISTS
	RUN_TEST(test_LISTS_empty);
	RUN_TEST(test_LISTS_list_to_stack);
	RUN_TEST(test_LISTS_list_to_stack_2);
	RUN_TEST(test_LISTS_list_to_stack_3);
	RUN_TEST(test_LISTS_stack_to_list);
	RUN_TEST(test_LISTS_stack_to_list_2);
	RUN_TEST(test_LISTS_stack_to_list_3);

	return UNITY_END();
}

