#include<stdlib.h>
#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

// TAGGED POINTER BASED TYPING INFORMATION

void test_basic_types() {
	CELL pair = (CELL)malloc(sizeof(CELL) * 2);

	CDR(pair) = AS(ATOM, 8);
	TEST_ASSERT_EQUAL_INT(8, NEXT(pair));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_EQUAL(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL(LIST, TYPE(pair));
	CDR(pair) = AS(LIST, 8);
	TEST_ASSERT_EQUAL_INT(8, NEXT(pair));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL(ATOM, TYPE(pair));
	TEST_ASSERT_EQUAL(LIST, TYPE(pair));
}

// CONTEXT

#define free_nodes(ctx)			(((ctx->size - sizeof(CTX)) / (2*sizeof(CELL))) - 1)

void test_block_size() {
	CELL size = 2;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TEST_ASSERT_EQUAL_PTR(0, ctx);
}

void test_block_initialization() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);
		
	TEST_ASSERT_NOT_EQUAL(0, ctx);

	TEST_ASSERT_EQUAL_INT(0, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx), FREE(ctx));

	TEST_ASSERT_EQUAL_INT(((BYTE*)ctx) + sizeof(CTX), BOTTOM(ctx));
	TEST_ASSERT_EQUAL_INT(BOTTOM(ctx), ctx->here);
	TEST_ASSERT_EQUAL_INT(ALIGN(ctx->here, 2*sizeof(CELL)), ctx->there);
	TEST_ASSERT_EQUAL_INT(ALIGN(((BYTE*)ctx) + size - 2*sizeof(CELL) - 1, 2*sizeof(CELL)), TOP(ctx));
	TEST_ASSERT_EQUAL_INT(TOP(ctx), ctx->free);

	TEST_ASSERT_EQUAL_INT(0, ctx->latest);
	TEST_ASSERT_EQUAL_INT(0, ctx->stack);
	TEST_ASSERT_EQUAL_INT(0, ctx->rstack);
	TEST_ASSERT_EQUAL_INT(0, ctx->cpile);
	TEST_ASSERT_EQUAL_INT(0, ctx->state);
	TEST_ASSERT_EQUAL_PTR(0, ctx->tib);
	TEST_ASSERT_EQUAL_INT(0, ctx->token);
	TEST_ASSERT_EQUAL_INT(0, ctx->in);
}

// LIST CREATION AND DESTRUCTION (AUTOMATIC MEMORY MANAGEMENT)

void test_cons() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL list = cons(ctx, 7, cons(ctx, 11, cons(ctx, 13, 0)));

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 3, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(3, length(list));
	TEST_ASSERT_EQUAL_INT(7, CAR(list));
	TEST_ASSERT_EQUAL_INT(11, CAR(CDR(list)));
	TEST_ASSERT_EQUAL_INT(13, CAR(CDR(CDR(list))));

	CELL pair = cons(ctx, 7, 0);

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 4, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(0, CDR(pair));
	
	while (ctx->free != ctx->there) { cons(ctx, 1, 0); }

	CELL p3 = cons(ctx, 13, 0);
	TEST_ASSERT_EQUAL_INT(0, p3);
}

void test_clone() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL list = 
		cons(ctx, 7, AS(ATOM, 
		cons(ctx, 11, AS(PRIM, 
		cons(ctx, 
			cons(ctx, 13, AS(ATOM,
			cons(ctx, 17, AS(ATOM, 0)))), AS(LIST,
		cons(ctx, 19, AS(ATOM, 0))))))));

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 6, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(4, length(list));
	TEST_ASSERT_EQUAL(ATOM, TYPE(list));
	TEST_ASSERT_EQUAL_INT(7, CAR(list));
	TEST_ASSERT_EQUAL(PRIM, TYPE(NEXT(list)));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(list)));
	TEST_ASSERT_EQUAL(LIST, TYPE(NEXT(NEXT(list))));
	TEST_ASSERT_EQUAL_INT(2, length(NEXT(NEXT(list))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(CAR(NEXT(NEXT(list)))));
	TEST_ASSERT_EQUAL_INT(13, CAR(CAR(NEXT(NEXT(list)))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(CAR(NEXT(NEXT(list))))));
	TEST_ASSERT_EQUAL_INT(17, CAR(NEXT(CAR(NEXT(NEXT(list))))));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(NEXT(list)))));
	TEST_ASSERT_EQUAL_INT(19, CAR(NEXT(NEXT(NEXT(list)))));

	CELL c = clone(ctx, list);

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 12, FREE(ctx));
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

	TEST_ASSERT_NOT_EQUAL_INT(list, c);
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(list), NEXT(c));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(NEXT(list)), NEXT(NEXT(c)));
	TEST_ASSERT_NOT_EQUAL_INT(CAR(NEXT(NEXT(list))), CAR(NEXT(NEXT(c))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(CAR(NEXT(NEXT(list)))), NEXT(CAR(NEXT(NEXT(c)))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(NEXT(NEXT(list))), NEXT(NEXT(NEXT(c))));
}

void test_reclaim() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL list = cons(ctx, 7, cons(ctx, 11, cons(ctx, 13, 0)));
	CELL tail = reclaim(ctx, list);

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 2, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(2, length(tail));
	TEST_ASSERT_EQUAL_INT(11, CAR(tail));
	TEST_ASSERT_EQUAL_INT(13, CAR(CDR(tail)));
}

void test_reclaim_list() {
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

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 6, FREE(ctx));
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

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 5, FREE(ctx));
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

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 1, FREE(ctx));
	TEST_ASSERT_EQUAL(ATOM, TYPE(tail));
	TEST_ASSERT_EQUAL_INT(17, CAR(tail));

	tail = reclaim(ctx, tail);

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx), FREE(ctx));
	TEST_ASSERT_EQUAL_INT(0, tail);
}

void test_reverse() {
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

void test_length() {
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

// INNER INTERPRETER

void test_execute_atom() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL xlist = cons(ctx, 13, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	execute(ctx, xlist);
	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(ctx->stack)));
}

void test_execute_list() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL xlist = 
		cons(ctx, 
			cons(ctx, 7, AS(ATOM, 
			cons(ctx, 11, AS(ATOM, 
			cons(ctx, 13, AS(ATOM, 0)))))), 
		AS(LIST, 0));

	execute(ctx, xlist);

	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL(LIST, TYPE(ctx->stack));
	TEST_ASSERT_EQUAL_INT(3, length(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(NEXT(CAR(ctx->stack)))));
	TEST_ASSERT_NOT_EQUAL_INT(xlist, ctx->stack);
	TEST_ASSERT_NOT_EQUAL_INT(CAR(xlist), CAR(ctx->stack));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(CAR(xlist)), NEXT(CAR(ctx->stack)));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(NEXT(CAR(xlist))), NEXT(NEXT(CAR(ctx->stack))));
}

CELL test_add_t(CTX* ctx) {
	CELL a = CAR(ctx->stack);
	CELL b = CAR(NEXT(ctx->stack));
	ctx->stack = reclaim(ctx, reclaim(ctx, ctx->stack));
	ctx->stack = cons(ctx, b + a, AS(ATOM, 0));

	return 0;
}


CELL test_dup_t(CTX* ctx) {
	CELL a = CAR(ctx->stack);
	ctx->stack = cons(ctx, a, AS(ATOM, ctx->stack));

	return 0;
}


void test_execute_primitive() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL xlist = 
		cons(ctx, 13, AS(ATOM, 
		cons(ctx, 7, AS(ATOM, 
		cons(ctx, (CELL)&test_add_t, AS(PRIM, 
		cons(ctx, (CELL)&test_dup_t, AS(PRIM, 0))))))));

	execute(ctx, xlist);

	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(20, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(20, CAR(NEXT(ctx->stack)));
}

void test_execute_call() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL xt = 
		cons(ctx, (CELL)&test_dup_t, AS(PRIM, 
		cons(ctx, (CELL)&test_add_t, AS(PRIM, 0))));
	CELL call = cons(ctx, xt, AS(CALL, 0));

	ctx->stack = cons(ctx, 5, AS(ATOM, ctx->stack));

	execute(ctx, call);

	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(10, CAR(ctx->stack));

	call = cons(ctx, xt, AS(CALL, cons(ctx, 13, AS(ATOM, 0))));

	execute(ctx, call);

	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(13, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(20, CAR(NEXT(ctx->stack)));
}

// IP PRIMITIVEs

void test_branch() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL xt = 
		cons(ctx, 1, AS(ATOM,
		cons(ctx, (CELL)&branch, AS(PRIM,
		cons(ctx, cons(ctx, 7, AS(ATOM, 0)), AS(LIST, 
		cons(ctx, cons(ctx, 11, AS(ATOM, 0)), AS(LIST,
		cons(ctx, 13, AS(ATOM, 0))))))))));

	execute(ctx, xt);

	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(13, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(ctx->stack)));

	ctx->stack = 0;

	xt = 
		cons(ctx, 0, AS(ATOM,
		cons(ctx, (CELL)&branch, AS(PRIM,
		cons(ctx, cons(ctx, 7, AS(ATOM, 0)), AS(LIST, 
		cons(ctx, cons(ctx, 11, AS(ATOM, 0)), AS(LIST, 0))))))));

	execute(ctx, xt);

	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(11, CAR(ctx->stack));

}

void test_jump() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL dest = cons(ctx, 7, AS(ATOM, 0));
	CELL src = cons(ctx, (CELL)&jump, AS(PRIM, cons(ctx, dest, AS(ATOM, 0))));

	execute(ctx, src);

	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
}

void test_zjump() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL dest = cons(ctx, 7, AS(ATOM, 0));
	CELL src = 
		cons(ctx, (CELL)&zjump, AS(PRIM, 
		cons(ctx, dest, AS(ATOM, 
		cons(ctx, 13, AS(ATOM, 0))))));

	ctx->stack = cons(ctx, 1, AS(ATOM, 0));

	execute(ctx, src);

	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(13, CAR(ctx->stack));

	ctx->stack = cons(ctx, 0, AS(ATOM, 0));

	execute(ctx, src);

	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
}

// STACK PRIMITIVES

void test_duplicate_atom() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 7, AS(ATOM, ctx->stack));
	duplicate(ctx);

	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 2, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(ctx->stack));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(ctx->stack)));

	ctx->stack = cons(ctx, 11, AS(ATOM, ctx->stack));
	duplicate(ctx);

	TEST_ASSERT_EQUAL_INT(4, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 4, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(ctx->stack));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(11, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(NEXT(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(NEXT(NEXT(ctx->stack)))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(NEXT(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(NEXT(NEXT(ctx->stack)))));
}

void test_duplicate_list() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = 
		cons(ctx, 
			cons(ctx, 17, AS(ATOM,
			cons(ctx, 13, AS(ATOM,
			cons(ctx, 
				cons(ctx, 11, AS(ATOM,
				cons(ctx, 7, AS(ATOM, 0)))),
				AS(LIST, 0)))))),
			AS(LIST, ctx->stack));

	// (17, 13, (11, 7))
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 6, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(ctx->stack));
	TEST_ASSERT_EQUAL_INT(3, length(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(17, CAR(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(CAR(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(NEXT(CAR(ctx->stack)))));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(NEXT(CAR(ctx->stack))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(NEXT(NEXT(CAR(ctx->stack))))));
	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(NEXT(NEXT(CAR(ctx->stack))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(NEXT(NEXT(CAR(ctx->stack)))))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(NEXT(NEXT(CAR(ctx->stack)))))));

	duplicate(ctx);

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 12, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));

	TEST_ASSERT_EQUAL_INT(LIST, TYPE(ctx->stack));
	TEST_ASSERT_EQUAL_INT(3, length(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(17, CAR(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(CAR(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(NEXT(CAR(ctx->stack)))));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(NEXT(CAR(ctx->stack))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(NEXT(NEXT(CAR(ctx->stack))))));
	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(NEXT(NEXT(CAR(ctx->stack))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(NEXT(NEXT(CAR(ctx->stack)))))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(NEXT(NEXT(CAR(ctx->stack)))))));

	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(3, length(CAR(NEXT(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(NEXT(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(17, CAR(CAR(NEXT(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(NEXT(ctx->stack)))));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(CAR(NEXT(ctx->stack)))));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(NEXT(CAR(NEXT(ctx->stack))))));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(NEXT(CAR(NEXT(ctx->stack)))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(CAR(NEXT(NEXT(CAR(NEXT(ctx->stack)))))));
	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(NEXT(NEXT(CAR(NEXT(ctx->stack)))))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(CAR(NEXT(NEXT(CAR(NEXT(ctx->stack))))))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(NEXT(NEXT(CAR(NEXT(ctx->stack))))))));

	TEST_ASSERT_NOT_EQUAL_INT(ctx->stack, NEXT(ctx->stack));
	TEST_ASSERT_NOT_EQUAL_INT(CAR(ctx->stack), CAR(NEXT(ctx->stack)));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(CAR(ctx->stack)), NEXT(CAR(NEXT(ctx->stack))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(NEXT(CAR(ctx->stack))), NEXT(NEXT(CAR(NEXT(ctx->stack)))));
	TEST_ASSERT_NOT_EQUAL_INT(CAR(NEXT(NEXT(CAR(ctx->stack)))), CAR(NEXT(NEXT(CAR(NEXT(ctx->stack))))));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(CAR(NEXT(NEXT(CAR(ctx->stack))))), NEXT(CAR(NEXT(NEXT(CAR(NEXT(ctx->stack)))))));
}

void test_swap_1() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 11, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(11, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(ctx->stack)));
	swap(ctx);
	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(ctx->stack)));
}

void test_swap_2() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 11, AS(ATOM, cons(ctx, cons(ctx, 7, AS(ATOM, cons(ctx, 5, AS(ATOM, 0)))), AS(LIST, 0))));
	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(ctx->stack));
	TEST_ASSERT_EQUAL_INT(11, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(NEXT(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(5, CAR(NEXT(CAR(NEXT(ctx->stack)))));
	swap(ctx);
	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(ctx->stack));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(5, CAR(NEXT(CAR(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(ctx->stack)));
}

void test_drop() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 7, AS(ATOM, ctx->stack));

	drop(ctx);

	TEST_ASSERT_EQUAL_INT(0, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx), FREE(ctx));

	ctx->stack = 
		cons(ctx,
			cons(ctx, 7, AS(ATOM,
			cons(ctx,
				cons(ctx, 11, AS(ATOM,
				cons(ctx, 13, AS(ATOM, 0)))),
			AS(LIST, 0)))),
		AS(LIST, ctx->stack));

	drop(ctx);

	TEST_ASSERT_EQUAL_INT(0, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx), FREE(ctx));
}

void test_over() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));

	over(ctx);

	TEST_ASSERT_EQUAL_INT(3, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(11, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(NEXT(ctx->stack))));

	ctx->stack = 
		cons(ctx, 7, AS(ATOM, 
		cons(ctx, 
			cons(ctx, 11, AS(ATOM,
			cons(ctx, 13, AS(ATOM, 0)))),
		AS(LIST, 0))));

	over(ctx);

	TEST_ASSERT_EQUAL_INT(3, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(ctx->stack));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(CAR(ctx->stack))));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(ctx->stack)));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(NEXT(ctx->stack)), ctx->stack);
	TEST_ASSERT_NOT_EQUAL_INT(CAR(NEXT(NEXT(ctx->stack))), CAR(ctx->stack));
	TEST_ASSERT_NOT_EQUAL_INT(NEXT(CAR(NEXT(NEXT(ctx->stack)))), NEXT(CAR(ctx->stack)));
}

void test_rot() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))))));

	rot(ctx);

	TEST_ASSERT_EQUAL_INT(3, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(13, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(NEXT(ctx->stack))));
}

// ARITHMETIC PRIMITIVES

void test_add() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 11, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	add(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(18, CAR(ctx->stack));
}

void test_sub() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));
	sub(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(4, CAR(ctx->stack));
}

void test_mul() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 11, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	mul(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(77, CAR(ctx->stack));
}

void test_division() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 11, AS(ATOM, cons(ctx, 77, AS(ATOM, 0))));
	division(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
}

void test_mod() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 11, AS(ATOM, 0))));
	mod(ctx);
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(4, CAR(ctx->stack));
}

// COMPARISON OPERATORS

void test_gt() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))));
	gt(ctx);
	TEST_ASSERT_EQUAL_INT(1, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));

	ctx->stack = cons(ctx, 13, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	gt(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	gt(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
}

void test_lt() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))));
	lt(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));

	ctx->stack = cons(ctx, 13, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	lt(ctx);
	TEST_ASSERT_EQUAL_INT(1, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	lt(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
}

void test_eq() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 13, AS(ATOM, 0))));
	eq(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));

	ctx->stack = cons(ctx, 13, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	eq(ctx);
	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));

	ctx->stack = cons(ctx, 7, AS(ATOM, cons(ctx, 7, AS(ATOM, 0))));
	eq(ctx);
	TEST_ASSERT_EQUAL_INT(1, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
}

// PARSING

void test_parse_token() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);
	BYTE* str = "   test  ";

	ctx->tib = str;
	ctx->token = 0;
	ctx->in = 0;

	parse_token(ctx);

	TEST_ASSERT_EQUAL_INT(4, ctx->in - ctx->token);
	TEST_ASSERT_EQUAL_INT((CELL)str + 3, ctx->tib + ctx->token);

	parse_token(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->in - ctx->token);
	TEST_ASSERT_EQUAL_INT((CELL)str + 9, ctx->tib + ctx->token);

	BYTE* str2 = "";
	ctx->tib = str2;
	ctx->token = 0;
	ctx->in = 0;

	parse_token(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->in - ctx->token);
	TEST_ASSERT_EQUAL_INT((CELL)str2, ctx->tib + ctx->token);

	BYTE* str3 = ": name    word1 ;";
	ctx->tib = str3;
	ctx->token = 0;
	ctx->in = 0;

	parse_token(ctx);

	TEST_ASSERT_EQUAL_INT(1, ctx->in - ctx->token);
	TEST_ASSERT(!strncmp(":", ctx->tib + ctx->token, ctx->in - ctx->token));
	TEST_ASSERT_EQUAL_INT((CELL)str3, ctx->tib + ctx->token);

	parse_token(ctx);

	TEST_ASSERT_EQUAL_INT(4, ctx->in - ctx->token);
	TEST_ASSERT(!strncmp("name", ctx->tib + ctx->token, ctx->in - ctx->token));
	TEST_ASSERT_EQUAL_INT((CELL)str3 + 2, ctx->tib + ctx->token);

	parse_token(ctx);

	TEST_ASSERT_EQUAL_INT(5, ctx->in - ctx->token);
	TEST_ASSERT(!strncmp("word1", ctx->tib + ctx->token, ctx->in - ctx->token));
	TEST_ASSERT_EQUAL_INT((CELL)str3 + 10, ctx->tib + ctx->token);

	parse_token(ctx);

	TEST_ASSERT_EQUAL_INT(1, ctx->in - ctx->token);
	TEST_ASSERT(!strncmp(";", ctx->tib + ctx->token, ctx->in - ctx->token));
	TEST_ASSERT_EQUAL_INT((CELL)str3 + 16, ctx->tib + ctx->token);

	parse_token(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->in - ctx->token);
	TEST_ASSERT_EQUAL_INT((CELL)str3 + 17, ctx->tib + ctx->token);

	parse_token(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->in - ctx->token);
	TEST_ASSERT_EQUAL_INT((CELL)str3 + 17, ctx->tib + ctx->token);
}

void test_find_token() {
	CELL size = 4096;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->latest =	
		cons(ctx, cons(ctx, (CELL)"test2", AS(ATOM, 0)), AS(LIST,
		cons(ctx, cons(ctx, (CELL)"dup", AS(ATOM, 0)), AS(CALL,
		cons(ctx, cons(ctx, (CELL)"join", AS(ATOM, 0)), AS(LIST,
		cons(ctx, cons(ctx, (CELL)"test", AS(ATOM, 0)), AS(LIST, 0))))))));

	ctx->tib = "dup";
	ctx->token = 0;
	ctx->in = 3;

	CELL word = find_token(ctx);

	TEST_ASSERT_NOT_EQUAL_INT(0, word);
	TEST_ASSERT_EQUAL_INT(1, IMMEDIATE(word));
	TEST_ASSERT_EQUAL_INT(NEXT(ctx->latest), word);

	ctx->tib = "   join  ";
	ctx->token = 0;
	ctx->in = 0;

	parse_token(ctx);
	word = find_token(ctx);

	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(word));
	TEST_ASSERT_EQUAL_INT(NEXT(NEXT(ctx->latest)), word);

	ctx->tib = "test";
	ctx->token = 0;
	ctx->in = 0;

	parse_token(ctx);
	word = find_token(ctx);

	TEST_ASSERT_EQUAL_STRING("test", (BYTE*)NFA(word));
	TEST_ASSERT_EQUAL_INT(NEXT(NEXT(NEXT(ctx->latest))), word);
}

// OUTER INTERPRETER

void test_evaluate_numbers() {
	CELL size = 4096;
	BYTE block[size];
	CTX* ctx = init(block, size);

	evaluate(ctx, "   7 11    13  ");

	TEST_ASSERT_EQUAL_INT(3, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(13, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(NEXT(ctx->stack))));
}

void test_evaluate_words() {
	CELL size = 4096;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->latest = 
		cons(ctx,
			cons(ctx, (CELL)"test", AS(ATOM,
			cons(ctx, 7, AS(ATOM,
			cons(ctx, 11, AS(ATOM,
			cons(ctx, 13, AS(ATOM, 0)))))))),
		AS(LIST, 0));

	evaluate(ctx, " test   ");

	TEST_ASSERT_EQUAL_INT(3, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(13, CAR(ctx->stack));
	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(ctx->stack)));
	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(NEXT(ctx->stack))));
}

// CONTIGUOUS MEMORY

void test_allot() {
	CELL size = 1024;
	BYTE block[size];
	CTX* ctx = init(block, size);

	BYTE* here = ctx->here;
	ctx->stack = cons(ctx, 0, AS(ATOM, ctx->stack));
	CELL result = allot(ctx);
	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_INT(here, ctx->here);

	ctx->stack = cons(ctx, 13, AS(ATOM, ctx->stack));
	result = allot(ctx);
	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_INT(here + 13, ctx->here);

	CELL fnodes = FREE(ctx);
	here = ctx->here;
	CELL reserved = RESERVED(ctx);

	// Ensure reserved memory is 0 to allow next tests to pass
	ctx->stack = cons(ctx, RESERVED(ctx), AS(ATOM, ctx->stack));
	result = allot(ctx);
	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_PTR(here + reserved, ctx->here);
	TEST_ASSERT_EQUAL_INT(0, RESERVED(ctx));
	TEST_ASSERT_EQUAL_INT(fnodes, FREE(ctx));

	here = ctx->here;

	ctx->stack = cons(ctx, 8*sizeof(CELL), AS(ATOM, ctx->stack));
	result = allot(ctx);
	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_PTR(here + 8*sizeof(CELL), ctx->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(0, RESERVED(ctx));

	here = ctx->here;

	ctx->stack = cons(ctx, 2*sizeof(CELL) - 3, AS(ATOM, ctx->stack));
	result = allot(ctx);
	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_PTR(here + 2*sizeof(CELL) - 3, ctx->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 5, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(3, RESERVED(ctx));

	ctx->stack = cons(ctx, -(2*sizeof(CELL) - 3), AS(ATOM, ctx->stack));
	result = allot(ctx);
	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_PTR(here, ctx->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(0, RESERVED(ctx));

	ctx->stack = cons(ctx, -1, AS(ATOM, ctx->stack));
	result = allot(ctx);
	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_PTR(here - 1, ctx->here);
	TEST_ASSERT_EQUAL_INT(fnodes - 4, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(1, RESERVED(ctx));

	here = ctx->here;
	reserved = RESERVED(ctx);
	fnodes = FREE(ctx);

	ctx->stack = cons(ctx, 2048, AS(ATOM, ctx->stack));
	result = allot(ctx);
	TEST_ASSERT_EQUAL_INT(ERR_NOT_ENOUGH_MEMORY, result);
	TEST_ASSERT_EQUAL_PTR(here, ctx->here);
	TEST_ASSERT_EQUAL_INT(reserved, RESERVED(ctx));
	TEST_ASSERT_EQUAL_INT(fnodes, FREE(ctx));

	ctx->stack = cons(ctx, -2048, AS(ATOM, ctx->stack));
	result = allot(ctx);
	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_PTR(BOTTOM(ctx), ctx->here);
	TEST_ASSERT_EQUAL_INT((CELL)ALIGN(ctx->here, 2*sizeof(CELL)), ctx->there);
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx), FREE(ctx));
}

void test_compile_str() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL result;

	ctx->tib = " test string\"  ";
	ctx->in = 0;
	result = compile_str(ctx);

	TEST_ASSERT_EQUAL_INT(0, result);

	BYTE* addr = (BYTE*)CAR(ctx->stack);

	TEST_ASSERT_EQUAL_STRING("test string", addr);
	TEST_ASSERT_EQUAL_INT(11, *((CELL*)(addr - sizeof(CELL))));
}

////  LIST FUNCTIONS

void test_append() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL dest = 0;
	CELL result = 0;

	result = append(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, result);

	ctx->stack = cons(ctx, (CELL)&dest, AS(ATOM, ctx->stack));
	ctx->stack = cons(ctx, 7, AS(ATOM, ctx->stack));

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 2, FREE(ctx));

	result = append(ctx);

	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_INT(1, length(dest));
	TEST_ASSERT_EQUAL_INT(7, CAR(dest));
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 1, FREE(ctx));

	ctx->stack = reclaim(ctx, ctx->stack);
	dest = reclaim(ctx, dest);

	TEST_ASSERT_EQUAL_INT(0, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx), FREE(ctx));

	ctx->stack = cons(ctx, cons(ctx, 7, AS(ATOM, 0)), AS(LIST, ctx->stack));
	ctx->stack = cons(ctx, 11, AS(ATOM, ctx->stack));

	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 3, FREE(ctx));

	result = append(ctx);

	TEST_ASSERT_EQUAL_INT(0, result);
	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 3, FREE(ctx));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(ctx->stack));
	TEST_ASSERT_EQUAL_INT(2, length(CAR(ctx->stack)));
}
//void test_depth() {
//	CELL p1 = (CELL)malloc(sizeof(CELL) * 2);
//	CELL p2 = (CELL)malloc(sizeof(CELL) * 2);
//	CELL p3 = (CELL)malloc(sizeof(CELL) * 2);
//	CELL p4 = (CELL)malloc(sizeof(CELL) * 2);
//	CELL p5 = (CELL)malloc(sizeof(CELL) * 2);
//
//	CDR(p1) = AS(ATOM, p2);
//	CDR(p2) = AS(LIST, p3);
//	CAR(p2) = p4;
//	CDR(p3) = AS(PRIM, 0);
//	CDR(p4) = p5;
//	CDR(p5) = 0;
//
//	TEST_ASSERT_EQUAL_INT(3, length(p1));
//	TEST_ASSERT_EQUAL_INT(2, length(CAR(p2)));
//	TEST_ASSERT_EQUAL_INT(5, depth(p1));
//	TEST_ASSERT_EQUAL_INT(4, depth(p2));
//	TEST_ASSERT_EQUAL_INT(2, depth(p4));
//	TEST_ASSERT_EQUAL_INT(1, depth(p3));
//	TEST_ASSERT_EQUAL_INT(1, depth(p5));
//	TEST_ASSERT_EQUAL_INT(0, depth(0));
//}
//
// CONTIGUOUS MEMORY

//void test_align() {
//	CELL size = 512;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	// This ensures here will be aligned with a pair 
//	CELL result = allot(ctx, RESERVED(ctx));
//
//	// And this ensures that it will not be aligned
//	result = allot(ctx, 1);
//	TEST_ASSERT_NOT_EQUAL_INT(ALIGN(ctx->here, sizeof(CELL)), ctx->here);
//	TEST_ASSERT_EQUAL_INT(0, result);
//
//	result = align(ctx);
//	TEST_ASSERT_EQUAL_INT(ALIGN(ctx->here, sizeof(CELL)), ctx->here);
//	TEST_ASSERT_EQUAL_INT(0, result);
//}
//
//// WORD DEFINITIONS
//
//void test_header() {
//	CELL size = 512;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	BYTE* here = ctx->here;
//
//	CELL h = header(ctx, "test");
//
//	TEST_ASSERT_EQUAL_STRING("test", NFA(h));
//	TEST_ASSERT_EQUAL_PTR(ctx->here, DFA(h));
//	TEST_ASSERT_EQUAL_INT(0, XT(h));
//	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(h));
//}
//
//void test_body() {
//	CELL size = 512;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	CELL h = header(ctx, "test");
//
//	CELL w = body(h, cons(ctx, 11, AS(ATOM, cons(ctx, 7, AS(ATOM, 0)))));
//
//	execute(ctx, XT(w));
//
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(ctx->stack)));
//}
//
//void test_reveal() {
//	CELL size = 512;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	CELL h = header(ctx, "test1");
//	CELL h2 = header(ctx, "test2");
//
//	CELL d = reveal(ctx, h);
//
//	TEST_ASSERT_EQUAL_INT(h, d);
//	TEST_ASSERT_EQUAL_INT(h, ctx->latest);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->latest));
//
//	d = reveal(ctx, h2);
//
//	TEST_ASSERT_EQUAL_INT(h2, d);
//	TEST_ASSERT_EQUAL_INT(h2, ctx->latest);
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->latest));
//	TEST_ASSERT_EQUAL_INT(h, NEXT(ctx->latest));
//}
//
//void test_immediate() {
//	CELL size = 512;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	CELL h = reveal(ctx, header(ctx, "test1"));
//
//	TEST_ASSERT_EQUAL_INT(0, IMMEDIATE(ctx->latest));
//
//	immediate(h);
//
//	TEST_ASSERT_EQUAL_INT(1, IMMEDIATE(ctx->latest));
//}

//void test_mlength() {
//	C p1 = (C)malloc(sizeof(C) * 2);
//	C p2 = (C)malloc(sizeof(C) * 2);
//	C p3 = (C)malloc(sizeof(C) * 2);
//
//	D(p1) = p2;
//	D(p2) = p3;
//	D(p3) = 0;
//
//	TEST_ASSERT(mlength(p1, 3));
//	TEST_ASSERT(mlength(p1, 2));
//	TEST_ASSERT(mlength(p1, 1));
//	TEST_ASSERT(mlength(p1, 0));
//	TEST_ASSERT(!mlength(p2, 3));
//	TEST_ASSERT(mlength(p2, 2));
//	TEST_ASSERT(mlength(p2, 1));
//	TEST_ASSERT(mlength(p2, 0));
//	TEST_ASSERT(!mlength(p3, 3));
//	TEST_ASSERT(!mlength(p3, 2));
//	TEST_ASSERT(mlength(p3, 1));
//	TEST_ASSERT(mlength(p3, 0));
//}
//
//void test_last() {
//	C p1 = (C)malloc(sizeof(C) * 2);
//	C p2 = (C)malloc(sizeof(C) * 2);
//	C p3 = (C)malloc(sizeof(C) * 2);
//
//	D(p1) = p2;
//	D(p2) = p3;
//	D(p3) = 0;
//
//	TEST_ASSERT_EQUAL_INT(0, last(0));
//	TEST_ASSERT_EQUAL_INT(p3, last(p1));
//	TEST_ASSERT_EQUAL_INT(p3, last(p2));
//	TEST_ASSERT_EQUAL_INT(p3, last(p3));
//}
//
//// BASIC STACK OPERATIONS
//
//void test_push() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 11);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0,CTX* ctx->err);
//	TEST_ASSERT(IS(ATOM,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(11, CAR(ctx->stack));
//	push(x, ATOM, 7);
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0,CTX* ctx->err);
//	TEST_ASSERT(IS(ATOM,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
//	TEST_ASSERT(IS(ATOM, NEXT(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(ctx->stack)));
//}
//
//void test_pop() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	C v = pop(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(7, v);
//	v = pop(ctx);
//	TEST_ASSERT_EQUAL_INT(0, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(11, v);
//}
//
//// COMPILATION PILE OPERATIONS
//
//void test_cppush() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	TEST_ASSERT_EQUAL_INT(0,CTX* ctx->cpile);
//	TEST_ASSERT_EQUAL_INT(0, length(x->cpile));
//
//	cppush(ctx);
//
//	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 1, FREE(ctx));
//	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
//	TEST_ASSERT(IS(LIST,CTX* ctx->cpile));
//	TEST_ASSERT_EQUAL_INT(0, length(CAR(x->cpile)));
//
//	cppush(ctx);
//
//	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 2, FREE(ctx));
//	TEST_ASSERT_EQUAL_INT(2, length(x->cpile));
//	TEST_ASSERT(IS(LIST,CTX* ctx->cpile));
//	TEST_ASSERT_EQUAL_INT(0, length(CAR(x->cpile)));
//	TEST_ASSERT(IS(LIST, NEXT(x->cpile)));
//	TEST_ASSERT_EQUAL_INT(0, length(CAR(NEXT(x->cpile))));
//}
//
//void test_cppop_1() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	cppush(ctx);
//	cppop(ctx);
//
//	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 1, FREE(ctx));
//	TEST_ASSERT_EQUAL_INT(0, length(x->cpile));
//	TEST_ASSERT_EQUAL_INT(0,CTX* ctx->cpile);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, length(CAR(ctx->stack)));
//}
//
//void test_cspush() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	cppush(ctx);
//	cspush(x, cons(x, 7, T(ATOM, 0)));
//
//	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 2, FREE(ctx));
//	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(x->cpile)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(x->cpile)));
//
//	cspush(x, cons(x, 11, T(ATOM, 0)));
//
//	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 3, FREE(ctx));
//	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
//	TEST_ASSERT_EQUAL_INT(2, length(CAR(x->cpile)));
//	TEST_ASSERT(IS(ATOM, CAR(x->cpile)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(x->cpile)));
//	TEST_ASSERT(IS(ATOM, NEXT(CAR(x->cpile))));
//	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(x->cpile))));
//
//	cppush(ctx);
//
//	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 4, FREE(ctx));
//	TEST_ASSERT_EQUAL_INT(2, length(x->cpile));
//	TEST_ASSERT_EQUAL_INT(0, length(CAR(x->cpile)));
//	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(x->cpile))));
//	TEST_ASSERT(IS(ATOM, CAR(NEXT(x->cpile))));
//	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(NEXT(x->cpile))));
//	TEST_ASSERT(IS(ATOM, NEXT(CAR(NEXT(x->cpile)))));
//	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(NEXT(x->cpile)))));
//
//	cspush(x, cons(x, 13, T(ATOM, 0)));
//
//	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 5, FREE(ctx));
//	TEST_ASSERT_EQUAL_INT(2, length(x->cpile));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(x->cpile)));
//	TEST_ASSERT(IS(ATOM, CAR(x->cpile)));
//	TEST_ASSERT_EQUAL_INT(13, CAR(CAR(x->cpile)));
//	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(x->cpile))));
//	TEST_ASSERT(IS(ATOM, CAR(NEXT(x->cpile))));
//	TEST_ASSERT_EQUAL_INT(11, CAR(CAR(NEXT(x->cpile))));
//	TEST_ASSERT(IS(ATOM, NEXT(CAR(NEXT(x->cpile)))));
//	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(CAR(NEXT(x->cpile)))));
//}
//
//void test_cppop_2() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	cppush(ctx);
//	cspush(x, cons(x, 7, T(ATOM, 0)));
//	cspush(x, cons(x, 11, T(ATOM, 0)));
//	cppush(ctx);
//	cspush(x, cons(x, 13, T(ATOM, 0)));
//	cppop(ctx);
//
//	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 5, FREE(ctx));
//	TEST_ASSERT_EQUAL_INT(1, length(x->cpile));
//	TEST_ASSERT_EQUAL_INT(3, length(CAR(x->cpile)));
//	TEST_ASSERT(IS(LIST, CAR(x->cpile)));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(A(x->cpile))));
//	TEST_ASSERT_EQUAL_INT(13, CAR(CAR(A(x->cpile))));
//	TEST_ASSERT(IS(ATOM, NEXT(CAR(x->cpile))));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(x->cpile))));
//	TEST_ASSERT(IS(ATOM, NEXT(NEXT(CAR(x->cpile)))));
//	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(NEXT(CAR(x->cpile)))));
//
//	cppop(ctx);
//
//	TEST_ASSERT_EQUAL_INT(free_nodes(ctx) - 5, FREE(ctx));
//	TEST_ASSERT_EQUAL_INT(0, length(x->cpile));
//	TEST_ASSERT_EQUAL_INT(0,CTX* ctx->cpile);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(3, length(CAR(ctx->stack)));
//	TEST_ASSERT(IS(LIST, CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(A(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(13, CAR(CAR(A(ctx->stack))));
//	TEST_ASSERT(IS(ATOM, NEXT(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
//	TEST_ASSERT(IS(ATOM, NEXT(NEXT(CAR(ctx->stack)))));
//	TEST_ASSERT_EQUAL_INT(7, CAR(NEXT(NEXT(CAR(ctx->stack)))));
//}
//
//void test_braces() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	_lbrace(ctx);
//	cspush(x, cons(x, 1, T(ATOM, 0)));
//	cspush(x, cons(x, 2, T(ATOM, 0)));
//	_lbrace(ctx);
//	cspush(x, cons(x, 3, T(ATOM, 0)));
//	cspush(x, cons(x, 4, T(ATOM, 0)));
//	_rbrace(ctx);
//	cspush(x, cons(x, 5, T(ATOM, 0)));
//	_lbrace(ctx);
//	cspush(x, cons(x, 6, T(ATOM, 0)));
//	_rbrace(ctx);
//	_rbrace(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0,CTX* ctx->cpile);
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(5, length(CAR(ctx->stack)));
//	TEST_ASSERT(IS(LIST, CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(A(ctx->stack))));
//	TEST_ASSERT(IS(ATOM, CAR(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(6, CAR(CAR(A(ctx->stack))));
//	TEST_ASSERT(IS(ATOM, NEXT(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(5, CAR(NEXT(CAR(ctx->stack))));
//	TEST_ASSERT(IS(LIST, NEXT(NEXT(CAR(ctx->stack)))));
//	TEST_ASSERT_EQUAL_INT(2, length(CAR(NEXT(NEXT(CAR(ctx->stack))))));
//	TEST_ASSERT(IS(ATOM, CAR(NEXT(NEXT(CAR(ctx->stack))))));
//	TEST_ASSERT_EQUAL_INT(4, CAR(CAR(NEXT(NEXT(CAR(ctx->stack))))));
//	TEST_ASSERT(IS(ATOM, NEXT(CAR(NEXT(NEXT(CAR(ctx->stack)))))));
//	TEST_ASSERT_EQUAL_INT(3, CAR(NEXT(CAR(NEXT(NEXT(CAR(ctx->stack)))))));
//	TEST_ASSERT(IS(ATOM, NEXT(NEXT(NEXT(CAR(ctx->stack))))));
//	TEST_ASSERT_EQUAL_INT(2, CAR(NEXT(NEXT(NEXT(CAR(ctx->stack))))));
//	TEST_ASSERT(IS(ATOM, NEXT(NEXT(NEXT(NEXT(CAR(ctx->stack)))))));
//	TEST_ASSERT_EQUAL_INT(1, CAR(NEXT(NEXT(NEXT(NEXT(CAR(ctx->stack)))))));
//}
//
//void test_empty() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	_empty(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//}
//
//void test_join_atom_atom_1() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(2, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
//}
//
//void test_join_atom_atom_2() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 13);
//
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(2, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(ctx->stack)));
//}
//
//void test_join_atom_empty_1() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	_empty(ctx);
//	push(x, ATOM, 7);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//}
//
//void test_join_atom_empty_2() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 13);
//
//	_empty(ctx);
//	push(x, ATOM, 7);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(ctx->stack)));
//}
//
//void test_join_atom_list_1() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 13);
//	push(x, ATOM, 11);
//	_join(ctx);
//	push(x, ATOM, 7);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(3, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(NEXT(CAR(ctx->stack)))));
//}
//
//void test_join_atom_list_2() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 17);
//
//	push(x, ATOM, 13);
//	push(x, ATOM, 11);
//	_join(ctx);
//	push(x, ATOM, 7);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(3, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(NEXT(CAR(ctx->stack)))));
//	TEST_ASSERT(IS(ATOM, NEXT(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(17, CAR(NEXT(ctx->stack)));
//}
//
//void test_join_list_atom_1() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 13);
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//	_join(ctx);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(3, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(NEXT(CAR(ctx->stack)))));
//}
//
//void test_join_list_atom_2() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 17);
//
//	push(x, ATOM, 13);
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//	_join(ctx);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(3, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(NEXT(CAR(ctx->stack)))));
//	TEST_ASSERT(IS(ATOM, NEXT(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(17, CAR(NEXT(ctx->stack)));
//}
//
//void test_join_list_list_1() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 17);
//	push(x, ATOM, 13);
//	_join(ctx);
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//	_join(ctx);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(4, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(NEXT(CAR(ctx->stack)))));
//	TEST_ASSERT_EQUAL_INT(17, CAR(NEXT(NEXT(NEXT(CAR(ctx->stack))))));
//}
//
//void test_join_list_list_2() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 19);
//
//	push(x, ATOM, 17);
//	push(x, ATOM, 13);
//	_join(ctx);
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//	_join(ctx);
//	_join(ctx);
//
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(4, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(NEXT(CAR(ctx->stack)))));
//	TEST_ASSERT_EQUAL_INT(17, CAR(NEXT(NEXT(NEXT(CAR(ctx->stack))))));
//	TEST_ASSERT(IS(ATOM, NEXT(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(19, CAR(NEXT(ctx->stack)));
//}
//
//void test_quote_1() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 7);
//	_quote(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//}
//
//void test_quote_2() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 11);
//
//	push(x, ATOM, 7);
//	_quote(ctx);
//
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(ctx->stack)));
//	TEST_ASSERT(IS(ATOM, NEXT(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(ctx->stack)));
//}
//
//void test_quote_3() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//	_join(ctx);
//	_quote(ctx);
//
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(ctx->stack)));
//	TEST_ASSERT(IS(LIST, CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(2, length(CAR(A(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(A(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(A(ctx->stack)))));
//}
//
//void test_quote_4() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 13);
//
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//	_join(ctx);
//	_quote(ctx);
//
//	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//	TEST_ASSERT(IS(LIST,CTX* ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, length(CAR(ctx->stack)));
//	TEST_ASSERT(IS(LIST, CAR(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(2, length(CAR(A(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(7, CAR(CAR(A(ctx->stack))));
//	TEST_ASSERT_EQUAL_INT(11, CAR(NEXT(CAR(A(ctx->stack)))));
//	TEST_ASSERT(IS(ATOM, NEXT(ctx->stack)));
//	TEST_ASSERT_EQUAL_INT(13, CAR(NEXT(ctx->stack)));
//}
//
//void test_neq() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 13);
//	push(x, ATOM, 7);
//	_neq(ctx);
//	TEST_ASSERT_EQUAL_INT(1, CAR(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 7);
//	push(x, ATOM, 13);
//	_neq(ctx);
//	TEST_ASSERT_EQUAL_INT(1, CAR(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 7);
//	push(x, ATOM, 7);
//	_neq(ctx);
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	pop(ctx);
//}
//
//void test_and() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 7);
//	push(x, ATOM, 11);
//	_and(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(3, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 0);
//	push(x, ATOM, 0);
//	_and(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 0);
//	push(x, ATOM, -1);
//	_and(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, -1);
//	push(x, ATOM, 0);
//	_and(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, -1);
//	push(x, ATOM, -1);
//	_and(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(-1, CAR(ctx->stack));
//	pop(ctx);
//}
//
//void test_or() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 11);
//	push(x, ATOM, 7);
//	_or(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(15, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 0);
//	push(x, ATOM, 0);
//	_or(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 0);
//	push(x, ATOM, -1);
//	_or(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(-1, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, -1);
//	push(x, ATOM, 0);
//	_or(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(-1, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, -1);
//	push(x, ATOM, -1);
//	_or(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(-1, CAR(ctx->stack));
//	pop(ctx);
//}
//
//void test_invert() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 7);
//	_invert(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(-8, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 0);
//	_invert(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(-1, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 1);
//	_invert(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(-2, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, -1);
//	_invert(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//	pop(ctx);
//}
//
//void test_not() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	push(x, ATOM, 7);
//	_not(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 0);
//	_not(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(1, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, 1);
//	_not(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//	pop(ctx);
//
//	push(x, ATOM, -1);
//	_not(ctx);
//	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//	TEST_ASSERT_EQUAL_INT(0, CAR(ctx->stack));
//	pop(ctx);
//}
//
//#define ATOM(x, n, d)							cons(x, n, T(ATOM, d))
//#define LIST(x, l, d)							cons(x, l, T(LIST, d))
//#define PRIM(x, p, d)				cons(x, (C)p, T(PRM, d))
//#define RECURSION(x, d)						PRIM(x, 0, d)
//#define JUMP(x, j, d)							cons(x, ATOM(x, j, 0), T(JMP, d))
//#define LAMBDA(x, w, d)						cons(x, cons(x, cons(x, w, T(LIST, 0)), T(LIST, 0)), T(JMP, d))
//#define CALL(x,CTX* ctxt, d)						cons(x, cons(x,CTX* ctxt, T(LIST, 0)), T(JMP, d))
//C BRANCH(X*CTX* ctx, C t, C f, C d) {
//	if (t) R(last(t), d); else t = d;
//	if (f) R(last(f), d); else f = d;
//	return cons(x, cons(x, t, T(LIST, cons(x, f, T(LIST, 0)))), T(JMP, d));
//}
//
//void test_allot_str() {
//	CELL size = 512;
//	BYTE block[size];
//	X*CTX* ctx = init(block, size);
//
//	B* here =CTX* ctx->here;
//
//	B* s = allot_str(x, 11);
//	strcpy(s, "test string");
//
//	TEST_ASSERT_EQUAL_PTR(here + sizeof(C), s);
//	TEST_ASSERT_EQUAL_STRING("test string", s);
//	TEST_ASSERT_EQUAL_INT(11, count(s));
//	TEST_ASSERT_EQUAL_PTR(here + sizeof(C) + 12,CTX* ctx->here);
//}
//
////void test_to_number() {
////	CELL size = 512;
////	BYTE block[size];
////	X*CTX* ctx = init(block, size);
////
////	x->ibuf = "256";
////	parse_token(ctx);
////	_to_number(ctx);
////	C n = pop(ctx);
////
////	TEST_ASSERT_EQUAL_INT(256, n);
////}
//
//////void test_clear_stack() {
//////	CELL size = 512;
//////	BYTE block[size];
//////	X*CTX* ctx = init(block, size);
//////
//////	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 3;
//////
//////	TEST_ASSERT_EQUAL_INT(free_nodes, height(ctx->free));
//////
//////	push(x, 13);
//////	push(x, 7);
//////	
//////	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(ctx->free));
//////
//////	_sclear(ctx);
//////
//////	TEST_ASSERT_EQUAL_INT(0, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(free_nodes, height(ctx->free));
//////}
//////
//////void test_push_stack() {
//////	CELL size = 512;
//////	BYTE block[size];
//////	X*CTX* ctx = init(block, size);
//////
//////	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 3;
//////
//////	push(x, 13);
//////	push(x, 7);
//////
//////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(ctx->free));
//////	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//////
//////	_spush(ctx);
//////
//////	TEST_ASSERT_EQUAL_INT(free_nodes - 3, height(ctx->free));
//////	TEST_ASSERT_EQUAL_INT(7, CAR(D(P(ctx))));
//////	TEST_ASSERT_EQUAL_INT(13, CAR(D(D(P(ctx)))));
//////	TEST_ASSERT_EQUAL_INT(2, height(P(ctx)));
//////	TEST_ASSERT_EQUAL_INT(R(ctx), CAR(P(ctx)));
//////	TEST_ASSERT_EQUAL_INT(0, length(ctx->stack));
//////}
//////
//////void test_drop_stack() {
//////	CELL size = 512;
//////	BYTE block[size];
//////	X*CTX* ctx = init(block, size);
//////
//////	C free_nodes = (size - sizeof(X)) / (2*sizeof(C)) - 3;
//////
//////	_sdrop(ctx);
//////
//////	TEST_ASSERT_EQUAL_INT(0, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(1, length(P(ctx)));
//////	TEST_ASSERT_EQUAL_INT(R(ctx), P(ctx));
//////	TEST_ASSERT_EQUAL_INT(free_nodes, height(ctx->free));
//////
//////	push(x, 13);
//////	push(x, 7);
//////
//////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(ctx->free));
//////	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(13,CTX* ctx->stack);
//////	TEST_ASSERT_EQUAL_INT(R(ctx), P(ctx));
//////
//////	_spush(ctx);
//////
//////	TEST_ASSERT_EQUAL_INT(free_nodes - 3, height(ctx->free));
//////
//////	push(x, 21);
//////
//////	TEST_ASSERT_EQUAL_INT(free_nodes - 4, height(ctx->free));
//////	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(21, CAR(ctx->stack));
//////
//////	_sdrop(ctx);
//////
//////	TEST_ASSERT_EQUAL_INT(free_nodes - 2, height(ctx->free));
//////	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(13,CTX* ctx->stack);
//////	TEST_ASSERT_EQUAL_INT(R(ctx), P(ctx));
//////}
//////
//////void test_drop() {
//////	CELL size = 512;
//////	BYTE block[size];
//////	X*CTX* ctx = init(block, size);
//////
//////	push(x, 21);
//////	push(x, 13);
//////	push(x, 7);
//////	TEST_ASSERT_EQUAL_INT(3, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(7, CAR(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(13,CTX* ctx->stack);
//////	TEST_ASSERT_EQUAL_INT(21, CAR(D(D(K(ctx)))));
//////	_drop(ctx);
//////	TEST_ASSERT_EQUAL_INT(2, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(13, CAR(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(21,CTX* ctx->stack);
//////	_drop(ctx);
//////	TEST_ASSERT_EQUAL_INT(1, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(21, CAR(ctx->stack));
//////	_drop(ctx);
//////	TEST_ASSERT_EQUAL_INT(0, length(ctx->stack));
//////	_drop(ctx);
//////	TEST_ASSERT_EQUAL_INT(ERR_UNDERFLOW,CTX* ctx->err);
//////}
//////
//////void test_rev() {
//////	CELL size = 512;
//////	BYTE block[size];
//////	X*CTX* ctx = init(block, size);
//////
//////	push(x, 21);
//////	push(x, 13);
//////	push(x, 7);
//////	push(x, 5);
//////	push(x, 3);
//////	TEST_ASSERT_EQUAL_INT(5, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(3, CAR(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(5,CTX* ctx->stack);
//////	TEST_ASSERT_EQUAL_INT(7, CAR(D(D(K(ctx)))));
//////	TEST_ASSERT_EQUAL_INT(13, CAR(D(D(D(K(ctx))))));
//////	TEST_ASSERT_EQUAL_INT(21, CAR(D(D(D(D(K(ctx)))))));
//////	_rev(ctx);
//////	TEST_ASSERT_EQUAL_INT(5, length(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(21, CAR(ctx->stack));
//////	TEST_ASSERT_EQUAL_INT(13,CTX* ctx->stack);
//////	TEST_ASSERT_EQUAL_INT(7, CAR(D(D(K(ctx)))));
//////	TEST_ASSERT_EQUAL_INT(5, CAR(D(D(D(K(ctx))))));
//////	TEST_ASSERT_EQUAL_INT(3, CAR(D(D(D(D(K(ctx)))))));
//////}
//////
////////////void test_stack_to_list() {
////////////}
////////////
//////////////void test_append() {
//////////////}
//////////////
//////////////void test_copy() {
//////////////	CELL size = 512;
//////////////	BYTE block[size];
//////////////	X* ctx = init(block, size);
//////////////
//////////////	C free_nodes = (size - sizeof(X)) / sizeof(PAIR);
//////////////
//////////////	// TODO
//////////////}
////////////
////////////void test_header_body_reveal() {
////////////	CELL size = 2048;
////////////	BYTE block[size];
////////////	X* ctx = init(block, size);
////////////
////////////	B* here = ctx->here;
////////////	PAIR* w = header(ctx, "test", 4);
////////////	TEST_ASSERT_EQUAL_INT(0, ctx->err);
////////////	TEST_ASSERT_EQUAL_PTR(ctx->here, ALIGN(here, sizeof(C)) + sizeof(C) + 4 + 1);
////////////	TEST_ASSERT(IS(ATOM, w));
////////////	TEST_ASSERT_EQUAL_INT(4, COUNT(NFA(w)));
////////////	TEST_ASSERT_EQUAL_STRING("test", NFA(w));
////////////	TEST_ASSERT_EQUAL_INT(((B*)REF(REF(w))) + 4, DFA(w));
////////////	TEST_ASSERT_EQUAL_INT(0, CFA(w));
////////////
////////////	TEST_ASSERT_NULL(ctx->dict);
////////////
////////////	PAIR* cfa = cons(ctx, ATOM, 7, cons(ctx, ATOM, 13, 0));
////////////	body(ctx, w, cfa);
////////////
////////////	TEST_ASSERT_EQUAL_INT(cfa, CFA(w));
////////////	TEST_ASSERT_EQUAL_INT(7, CFA(w)->value);
////////////	TEST_ASSERT_EQUAL_INT(13, NEXT(CFA(w))->value);
////////////
////////////	reveal(ctx, w);
////////////
////////////	TEST_ASSERT_EQUAL_PTR(w, ctx->dict);
////////////	TEST_ASSERT_NULL(NEXT(ctx->dict));
////////////
////////////	TEST_ASSERT_FALSE(IS_IMMEDIATE(w));
////////////
////////////	_immediate(ctx);
////////////
////////////	TEST_ASSERT_TRUE(IS_IMMEDIATE(w));
////////////}
////////////
//////////////void test_find() {
//////////////	CELL size = 512;
//////////////	BYTE block[size];
//////////////	X* ctx = init(block, size);
//////////////
//////////////	PAIR* dup = reveal(ctx, header(ctx, "dup", 3));
//////////////	PAIR* swap = reveal(ctx, header(ctx, "swap", 4));
//////////////	PAIR* test = reveal(ctx, header(ctx, "test", 4));
//////////////
//////////////	TEST_ASSERT_EQUAL_PTR(dup, find(ctx, "dup", 3));
//////////////	TEST_ASSERT_EQUAL_PTR(swap, find(ctx, "swap", 4));
//////////////	TEST_ASSERT_EQUAL_PTR(test, find(ctx, "test", 4));
//////////////	TEST_ASSERT_EQUAL_PTR(0, find(ctx, "nop", 3));
//////////////
//////////////	TEST_ASSERT_EQUAL_PTR(test, ctx->dict);
//////////////	TEST_ASSERT_EQUAL_PTR(swap, NEXT(ctx->dict));
//////////////	TEST_ASSERT_EQUAL_PTR(dup, NEXT(NEXT(ctx->dict)));
//////////////	TEST_ASSERT_EQUAL_PTR(0, NEXT(NEXT(NEXT(ctx->dict))));
//////////////}
//////////////
//////////////void test_dodo_initialization() {
//////////////	CELL size = 4096;
//////////////	BYTE block[size];
//////////////	X* ctx = dodo(init(block, size));
//////////////
//////////////	TEST_ASSERT_EQUAL_PTR(&_add, CFA(find(ctx, "+", 1))->value);
//////////////}
//////////////
//////////////void test_fib() {
//////////////	CELL size = 8192;
//////////////	BYTE block[size];
//////////////	X* ctx = init(block, size);
//////////////
//////////////	// : fib dup 1 > if 1- dup 1- recurse swap recurse + then ;
//////////////
//////////////	ctx->ip =
//////////////		cons(ctx, (C)&_dup, T_PRM,
//////////////		cons(ctx, 1, ATOM,
//////////////		cons(ctx, (C)&_gt, T_PRM,
//////////////		cons(ctx,
//////////////			0,
//////////////			T_JMP,
//////////////			cons(ctx, 1, ATOM,
//////////////			cons(ctx, (C)&_sub, T_PRM,
//////////////			cons(ctx, (C)&_dup, T_PRM,
//////////////			cons(ctx, 1, ATOM,
//////////////			cons(ctx, (C)&_sub, T_PRM,
//////////////			cons(ctx, (C)&_rec, T_PRM,
//////////////			cons(ctx, (C)&_swap, T_PRM,
//////////////			cons(ctx, (C)&_rec, T_PRM,	
//////////////			cons(ctx, (C)&_add, T_PRM, 0)))))))))))));
//////////////
//////////////		ctx->dictstack = cons(ctx, 6, ATOM, 0);
//////////////		ctx->rstack = cons(ctx, (C)ctx->ip, T_WORD, 0);
//////////////
//////////////		inner(ctx);
//////////////
//////////////		//printf("%ld\n", ctx->dictstack->value);
//////////////}
////////////
int main() {
	UNITY_BEGIN();

	// TAGGED POINTER BASED TYPING INFORMATION
	RUN_TEST(test_basic_types);

	// CONTEXT
	RUN_TEST(test_block_size);
	RUN_TEST(test_block_initialization);

	// LIST CREATION AND DESTRUCTION (AUTOMATIC MEMORY MANAGEMENT)
	RUN_TEST(test_cons);
	RUN_TEST(test_clone);
	RUN_TEST(test_reclaim);
	RUN_TEST(test_reclaim_list);
	RUN_TEST(test_reverse);
	RUN_TEST(test_length);

	// INNER INTERPRETER
	RUN_TEST(test_execute_atom);
	RUN_TEST(test_execute_list);
	RUN_TEST(test_execute_primitive);
	RUN_TEST(test_execute_call);

	// IP PRIMITIVES
	RUN_TEST(test_branch);
	RUN_TEST(test_jump);
	RUN_TEST(test_zjump);

	// STACK PRIMITIVES
	RUN_TEST(test_duplicate_atom);
	RUN_TEST(test_duplicate_list);
	RUN_TEST(test_swap_1);
	RUN_TEST(test_swap_2);
	RUN_TEST(test_drop);
	RUN_TEST(test_over);
	RUN_TEST(test_rot);

	// ARITHMETIC PRIMITIVES
	RUN_TEST(test_add);
	RUN_TEST(test_sub);
	RUN_TEST(test_mul);
	RUN_TEST(test_division);
	RUN_TEST(test_mod);

	// COMPARISON PRIMITIVES
	RUN_TEST(test_gt);
	RUN_TEST(test_lt);
	RUN_TEST(test_eq);

	// PARSING
	RUN_TEST(test_parse_token);
	RUN_TEST(test_find_token);

	// OUTER INTERPRETER
	RUN_TEST(test_evaluate_numbers);
	RUN_TEST(test_evaluate_words);

	// CONTIGUOUS MEMORY
	RUN_TEST(test_allot);
	RUN_TEST(test_compile_str);
	//RUN_TEST(test_align);

	//  LIST FUNCTIONS
	RUN_TEST(test_append);
	//RUN_TEST(test_depth);

	//// WORD DEFINITIONS
	//RUN_TEST(test_header);
	//RUN_TEST(test_body);
	//RUN_TEST(test_reveal);
	//RUN_TEST(test_immediate);

//	RUN_TEST(test_mlength);
//	RUN_TEST(test_last);
//
//	// COMPILATION PILE OPERATIONS
//	RUN_TEST(test_cppush);
//	RUN_TEST(test_cppop_1);
//	RUN_TEST(test_cspush);
//	RUN_TEST(test_cppop_2);
//
//	RUN_TEST(test_braces);
//
//	RUN_TEST(test_empty);
//
//	RUN_TEST(test_join_atom_atom_1);
//	RUN_TEST(test_join_atom_atom_2);
//	RUN_TEST(test_join_atom_empty_1);
//	RUN_TEST(test_join_atom_empty_2);
//	RUN_TEST(test_join_atom_list_1);
//	RUN_TEST(test_join_atom_list_2);
//	RUN_TEST(test_join_list_atom_1);
//	RUN_TEST(test_join_list_atom_2);
//	RUN_TEST(test_join_list_list_1);
//	RUN_TEST(test_join_list_list_2);
//
//	RUN_TEST(test_quote_1);
//	RUN_TEST(test_quote_2);
//	RUN_TEST(test_quote_3);
//	RUN_TEST(test_quote_4);
//
//	RUN_TEST(test_gt);
//	RUN_TEST(test_lt);
//	RUN_TEST(test_eq);
//	RUN_TEST(test_neq);
//
//	RUN_TEST(test_and);
//	RUN_TEST(test_or);
//	RUN_TEST(test_invert);
//	RUN_TEST(test_not);
//
//	//RUN_TEST(test_clear_stack);
//	//RUN_TEST(test_push_stack);
//	//RUN_TEST(test_drop_stack);
//
//
//	RUN_TEST(test_allot_str);
//
//	RUN_TEST(test_find);
//	//RUN_TEST(test_to_number);
//
////	//RUN_TEST(test_stack_to_list);
////
////	////RUN_TEST(test_fib);
////
////	////RUN_TEST(test_append);
////	////RUN_TEST(test_copy);
////
////
////	//RUN_TEST(test_header_body_reveal);
////	////RUN_TEST(test_find);
////
////	////RUN_TEST(test_dodo_initialization);
////
	return UNITY_END();
}
