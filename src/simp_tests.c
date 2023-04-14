#include<stdlib.h>
#include<string.h>

#include "unity.h"
#include "simp.h"

void setUp() {}

void tearDown() {}

// CORE

void test_CORE_length() {
	NODE* n1 = malloc(sizeof(NODE));
	NODE* n2 = malloc(sizeof(NODE));
	NODE* n3 = malloc(sizeof(NODE));

	n1->next = n2;
	n2->next = n3;
	n3->next = 0;

	TEST_ASSERT_EQUAL_PTR(3, length(n1, 0));
	TEST_ASSERT_EQUAL_PTR(2, length(n2, 0));
	TEST_ASSERT_EQUAL_PTR(1, length(n3, 0));
	TEST_ASSERT_EQUAL_PTR(0, length(0, 0));
}

void test_CORE_reverse() {
	NODE* p1 = malloc(sizeof(NODE));
	NODE* p2 = malloc(sizeof(NODE));
	NODE* p3 = malloc(sizeof(NODE));
	
	p1->next = p2;
	p2->next = p3;
	p3->next = 0;

	NODE* r = reverse(p1, 0);

	TEST_ASSERT_EQUAL_INT(p3, r);
	TEST_ASSERT_EQUAL_INT(p2, NEXT(r));
	TEST_ASSERT_EQUAL_INT(p1, NEXT(NEXT(r)));
}

void test_CORE_init_free() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	TEST_ASSERT_EQUAL_PTR((NODE*)ALIGN(block + size - 2*sizeof(CELL) - 1, 2*sizeof(CELL)), free);
	TEST_ASSERT_EQUAL_INT(16, length(free, 0));
	TEST_ASSERT_EQUAL_INT(16, length(last, 1));
}

void test_CORE_cons() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	NODE* list = cons(&free, 7, AS(ATOM, cons(&free, 11, AS(ATOM, cons(&free, 13, AS(ATOM, 0))))));

	TEST_ASSERT_EQUAL_INT(13, length(free, 0));
	TEST_ASSERT_EQUAL_INT(13, length(last, 1));
	TEST_ASSERT_EQUAL_INT(3, length(list, 0));
	TEST_ASSERT_EQUAL_INT(7, list->value);
	TEST_ASSERT_EQUAL_INT(11, NEXT(list)->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(NEXT(list))->value);

	NODE* p = cons(&free, 7, AS(ATOM, 0));

	TEST_ASSERT_EQUAL_INT(12, length(free, 0));
	TEST_ASSERT_EQUAL_INT(12, length(last, 1));
	TEST_ASSERT_EQUAL_INT(7, p->value);
	TEST_ASSERT_EQUAL_INT(0, NEXT(p));
}

void test_CORE_reclaim() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	NODE* list = cons(&free, 7, AS(ATOM, cons(&free, 11, AS(ATOM, cons(&free, 13, AS(ATOM, 0))))));
	NODE* tail = reclaim(&free, list);

	TEST_ASSERT_EQUAL_INT(14, length(free, 0));
	TEST_ASSERT_EQUAL_INT(14, length(last, 1));
	TEST_ASSERT_EQUAL_INT(2, length(tail, 0));
	TEST_ASSERT_EQUAL_INT(11, tail->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(tail)->value);

	tail = reclaim(&free, reclaim(&free, tail));

	TEST_ASSERT_EQUAL_INT(16, length(free, 0));
	TEST_ASSERT_EQUAL_INT(16, length(last, 1));
	TEST_ASSERT_EQUAL_INT(0, length(tail, 0));
}

void test_CORE_reclaim_list() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	NODE* list = 
		cons(&free, 5, AS(ATOM,
		cons(&free, (CELL)
			cons(&free, 7, AS(ATOM,
			cons(&free, 11, AS(ATOM,
			cons(&free, 13, AS(ATOM, 0)))))), AS(LIST, 
		cons(&free, 17, AS(ATOM, 0))))));

	TEST_ASSERT_EQUAL_INT(10, length(free, 0));
	TEST_ASSERT_EQUAL_INT(10, length(last, 1));
	TEST_ASSERT_EQUAL(ATOM, TYPE(list));
	TEST_ASSERT_EQUAL_INT(5, list->value);
	TEST_ASSERT_EQUAL(LIST, TYPE(NEXT(list)));
	TEST_ASSERT_EQUAL_INT(3, length(NEXT(list)->ref, 0));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(list)->ref));
	TEST_ASSERT_EQUAL_INT(7, NEXT(list)->ref->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(list)->ref)));
	TEST_ASSERT_EQUAL_INT(11, NEXT(NEXT(list)->ref)->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(NEXT(list)->ref))));
	TEST_ASSERT_EQUAL_INT(13, NEXT(NEXT(NEXT(list)->ref))->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(list))));
	TEST_ASSERT_EQUAL_INT(17, NEXT(NEXT(list))->value);

	NODE* tail = reclaim(&free, list);

	TEST_ASSERT_EQUAL_INT(11, length(free, 0));
	TEST_ASSERT_EQUAL_INT(11, length(last, 1));
	TEST_ASSERT_EQUAL(LIST, TYPE(tail));
	TEST_ASSERT_EQUAL_INT(3, length(tail->ref, 0));
	TEST_ASSERT_EQUAL(ATOM, TYPE(tail->ref));
	TEST_ASSERT_EQUAL_INT(7, tail->ref->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(tail->ref)));
	TEST_ASSERT_EQUAL_INT(11, NEXT(tail->ref)->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(tail->ref))));
	TEST_ASSERT_EQUAL_INT(13, NEXT(NEXT(tail->ref))->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(tail)));
	TEST_ASSERT_EQUAL_INT(17, NEXT(tail)->value);

	tail = reclaim(&free, tail);

	TEST_ASSERT_EQUAL_INT(15, length(free, 0));
	TEST_ASSERT_EQUAL_INT(15, length(last, 1));
	TEST_ASSERT_EQUAL(ATOM, TYPE(tail));
	TEST_ASSERT_EQUAL_INT(17, tail->value);

	tail = reclaim(&free, tail);

	TEST_ASSERT_EQUAL_INT(16, length(free, 0));
	TEST_ASSERT_EQUAL_INT(16, length(last, 1));
	TEST_ASSERT_EQUAL_INT(0, tail);
}

void test_CORE_clone() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];
	BYTE buf[255];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	NODE* list = 
		cons(&free, 7, AS(ATOM, 
		cons(&free, 11, AS(PRIM, 
		cons(&free, (CELL)
			cons(&free, 13, AS(ATOM,
			cons(&free, 17, AS(ATOM, 0)))), 
		AS(LIST,
		cons(&free, 19, AS(ATOM, 0))))))));

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#7 P:11 { #13 #17 } #19 ", print(buf, list));

	TEST_ASSERT_EQUAL_INT(10, length(free, 0));
	TEST_ASSERT_EQUAL_INT(10, length(last, 1));

	NODE* cloned = clone(&free, list);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#7 P:11 { #13 #17 } #19 ", print(buf, cloned));

	TEST_ASSERT_EQUAL_INT(4, length(free, 0));
	TEST_ASSERT_EQUAL_INT(4, length(last, 1));

	TEST_ASSERT_NOT_EQUAL(list, cloned);
	TEST_ASSERT_NOT_EQUAL(NEXT(list), NEXT(cloned));
	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(list)), NEXT(NEXT(cloned)));
	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(list))->ref, NEXT(NEXT(cloned))->ref);
	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(NEXT(list))->ref), NEXT(NEXT(NEXT(cloned))->ref));
	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(NEXT(list))), NEXT(NEXT(NEXT(cloned))));
}

// CONTEXT

void test_CONTEXT_data_stack() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	NODE* main_stack = 0;
	NODE* second_stack = 0;
	NODE** data_stack = &main_stack;

	*data_stack = cons(&free, 7, AS(ATOM, *data_stack));

	TEST_ASSERT_EQUAL_INT(1, length(*data_stack, 0));
	TEST_ASSERT_EQUAL_INT(1, length(main_stack, 0));
	TEST_ASSERT_EQUAL_INT(0, length(second_stack, 0));
	TEST_ASSERT_EQUAL_INT(15, length(free, 0));
	TEST_ASSERT_EQUAL_INT(15, length(last, 1));

	data_stack = &second_stack;

	*data_stack = 
		cons(&free, (CELL)
			cons(&free, 11, AS(ATOM, 
			cons(&free, 13, AS(ATOM, 0)))), 
		AS(LIST, 0));

	TEST_ASSERT_EQUAL_INT(1, length(*data_stack, 0));
	TEST_ASSERT_EQUAL_INT(1, length(main_stack, 0));
	TEST_ASSERT_EQUAL_INT(1, length(second_stack, 0));
	TEST_ASSERT_EQUAL_INT(12, length(free, 0));
	TEST_ASSERT_EQUAL_INT(12, length(last, 1));

	data_stack = &((*data_stack)->ref);

	TEST_ASSERT_EQUAL_INT(2, length(*data_stack, 0));

	*data_stack = cons(&free, 17, AS(ATOM, *data_stack));

	TEST_ASSERT_EQUAL_INT(3, length(*data_stack, 0));
	TEST_ASSERT_EQUAL_INT(1, length(main_stack, 0));
	TEST_ASSERT_EQUAL_INT(1, length(second_stack, 0));
	TEST_ASSERT_EQUAL_INT(11, length(free, 0));
	TEST_ASSERT_EQUAL_INT(11, length(last, 1));
}

// STACK PRIMITIVES

void test_STACK_duplicate_atom() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free = length(ctx->fstack, 0);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, S(ctx)));

	duplicate(ctx);

	TEST_ASSERT_EQUAL_INT(2, length(S(ctx), 0));
	TEST_ASSERT_EQUAL_INT(free - 2, length(ctx->fstack, 0));
	TEST_ASSERT_EQUAL_INT(free - 2, length(ctx->there, 1));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(S(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(S(ctx))));
	TEST_ASSERT_EQUAL_INT(7, S(ctx)->value);
	TEST_ASSERT_EQUAL_INT(7, NEXT(S(ctx))->value);

	S(ctx) = cons(&ctx->fstack, 11, AS(ATOM, S(ctx)));

	duplicate(ctx);

	TEST_ASSERT_EQUAL_INT(4, length(S(ctx), 0));
	TEST_ASSERT_EQUAL_INT(free - 4, length(ctx->fstack, 0));
	TEST_ASSERT_EQUAL_INT(free - 4, length(ctx->there, 1));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(S(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(S(ctx))));
	TEST_ASSERT_EQUAL_INT(11, S(ctx)->value);
	TEST_ASSERT_EQUAL_INT(11, NEXT(S(ctx))->value);
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(NEXT(S(ctx)))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(NEXT(NEXT(S(ctx))))));
	TEST_ASSERT_EQUAL_INT(7, NEXT(NEXT(S(ctx)))->value);
	TEST_ASSERT_EQUAL_INT(7, NEXT(NEXT(NEXT(S(ctx))))->value);
}

void test_STACK_duplicate_list() {
	BYTE buf[255];
	
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free = length(ctx->fstack, 0);

	S(ctx) = 
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 17, AS(ATOM,
			cons(&ctx->fstack, 13, AS(ATOM,
			cons(&ctx->fstack, (CELL)
				cons(&ctx->fstack, 11, AS(ATOM,
				cons(&ctx->fstack, 7, AS(ATOM, 0)))),
			AS(LIST, 0)))))),
		AS(LIST, S(ctx)));

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ #17 #13 { #11 #7 } } ", print(buf, S(ctx)));

	duplicate(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING(
		"{ #17 #13 { #11 #7 } } { #17 #13 { #11 #7 } } ",
		print(buf, S(ctx)));

	TEST_ASSERT_EQUAL_INT(free - 12, length(ctx->fstack, 0));
	TEST_ASSERT_EQUAL_INT(free - 12, length(ctx->there, 1));
}

void test_STACK_swap_1() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 11, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, 0))));

	swap(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#7 #11 ", print(buf, S(ctx)));
}

void test_STACK_swap_2() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = 
		cons(&ctx->fstack, 11, AS(ATOM, 
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 7, AS(ATOM, 
			cons(&ctx->fstack, 5, AS(ATOM, 0)))), 
		AS(LIST, 0))));

	swap(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ #7 #5 } #11 ", print(buf, S(ctx)));
}

void test_STACK_drop() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free = length(ctx->fstack, 0);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, S(ctx)));

	drop(ctx);

	TEST_ASSERT_EQUAL_INT(0, length(S(ctx), 0));
	TEST_ASSERT_EQUAL_INT(free, length(ctx->fstack, 0));

	S(ctx) = 
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 7, AS(ATOM,
			cons(&ctx->fstack, (CELL)
				cons(&ctx->fstack, 11, AS(ATOM,
				cons(&ctx->fstack, 13, AS(ATOM, 0)))),
			AS(LIST, 0)))),
		AS(LIST, S(ctx)));

	drop(ctx);

	TEST_ASSERT_EQUAL_INT(0, length(S(ctx), 0));
	TEST_ASSERT_EQUAL_INT(free, length(ctx->fstack, 0));
}

void test_STACK_over() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 11, AS(ATOM, 0))));

	over(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#11 #7 #11 ", print(buf, S(ctx)));

	S(ctx) = 
		cons(&ctx->fstack, 7, AS(ATOM, 
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 11, AS(ATOM,
			cons(&ctx->fstack, 13, AS(ATOM, 0)))),
		AS(LIST, 0))));

	over(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ #11 #13 } #7 { #11 #13 } ", print(buf, S(ctx)));
}

void test_STACK_rot() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = 
		cons(&ctx->fstack, 7, AS(ATOM, 
		cons(&ctx->fstack, 11, AS(ATOM, 
		cons(&ctx->fstack, 13, AS(ATOM, 0))))));

	rot(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#13 #7 #11 ", print(buf, S(ctx)));
}

// ARITHMETIC PRIMITIVES

void test_ARITHMETIC_add() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 11, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, 0))));
	add(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#18 ", print(buf, S(ctx)));
}

void test_ARITHMETIC_incr() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, 0));
	incr(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#8 ", print(buf, S(ctx)));
}

void test_ARITHMETIC_sub() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 11, AS(ATOM, 0))));
	sub(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#4 ", print(buf, S(ctx)));
}

void test_ARITHMETIC_decr() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, 0));
	decr(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#6 ", print(buf, S(ctx)));
}

void test_ARITHMETIC_mul() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 11, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, 0))));
	mul(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#77 ", print(buf, S(ctx)));
}

void test_ARITHMETIC_division() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 11, AS(ATOM, cons(&ctx->fstack, 77, AS(ATOM, 0))));
	division(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#7 ", print(buf, S(ctx)));
}

void test_ARITHMETIC_division_by_zero() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 0, AS(ATOM, cons(&ctx->fstack, 77, AS(ATOM, 0))));
	division(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_DIVISION_BY_ZERO, ctx->err);
}

void test_ARITHMETIC_mod() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 11, AS(ATOM, 0))));
	mod(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#4 ", print(buf, S(ctx)));
}

// COMPARISON PRIMITIVES

void test_COMPARISON_gt() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 13, AS(ATOM, 0))));
	gt(ctx);

	S(ctx) = cons(&ctx->fstack, 13, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, S(ctx)))));
	gt(ctx);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, S(ctx)))));
	gt(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#0 #0 #1 ", print(buf, S(ctx)));
}

void test_COMPARISON_lt() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 13, AS(ATOM, 0))));
	lt(ctx);

	S(ctx) = cons(&ctx->fstack, 13, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, S(ctx)))));
	lt(ctx);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, S(ctx)))));
	lt(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#0 #1 #0 ", print(buf, S(ctx)));
}

void test_COMPARISON_eq() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 13, AS(ATOM, 0))));
	eq(ctx);

	S(ctx) = cons(&ctx->fstack, 13, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, S(ctx)))));
	eq(ctx);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, S(ctx)))));
	eq(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#1 #0 #0 ", print(buf, S(ctx)));
}

void test_COMPARISON_neq() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 13, AS(ATOM, 0))));
	neq(ctx);

	S(ctx) = cons(&ctx->fstack, 13, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, S(ctx)))));
	neq(ctx);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, S(ctx)))));
	neq(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#0 #1 #1 ", print(buf, S(ctx)));
}

// BIT PRIMITIVES

void test_BIT_and() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 11, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, 0))));
	and(ctx);

	S(ctx) = cons(&ctx->fstack, 0, AS(ATOM, cons(&ctx->fstack, 0, AS(ATOM, S(ctx)))));
	and(ctx);

	S(ctx) = cons(&ctx->fstack, -1, AS(ATOM, cons(&ctx->fstack, 0, AS(ATOM, S(ctx)))));
	and(ctx);

	S(ctx) = cons(&ctx->fstack, 0, AS(ATOM, cons(&ctx->fstack, -1, AS(ATOM, S(ctx)))));
	and(ctx);

	S(ctx) = cons(&ctx->fstack, -1, AS(ATOM, cons(&ctx->fstack, -1, AS(ATOM, S(ctx)))));	
	and(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#-1 #0 #0 #0 #3 ", print(buf, S(ctx)));
}

void test_BIT_or() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, cons(&ctx->fstack, 11, AS(ATOM, 0))));
	or(ctx);

	S(ctx) = cons(&ctx->fstack, 0, AS(ATOM, cons(&ctx->fstack, 0, AS(ATOM, S(ctx)))));
	or(ctx);

	S(ctx) = cons(&ctx->fstack, -1, AS(ATOM, cons(&ctx->fstack, 0, AS(ATOM, S(ctx)))));
	or(ctx);

	S(ctx) = cons(&ctx->fstack, 0, AS(ATOM, cons(&ctx->fstack, -1, AS(ATOM, S(ctx)))));
	or(ctx);

	S(ctx) = cons(&ctx->fstack, -1, AS(ATOM, cons(&ctx->fstack, -1, AS(ATOM, S(ctx)))));
	or(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#-1 #-1 #-1 #0 #15 ", print(buf, S(ctx)));
}

void test_BIT_invert() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = cons(&ctx->fstack, 7, AS(ATOM, 0));
	invert(ctx);

	S(ctx) = cons(&ctx->fstack, 0, AS(ATOM, S(ctx)));
	invert(ctx);

	S(ctx) = cons(&ctx->fstack, 1, AS(ATOM, S(ctx)));
	invert(ctx);

	S(ctx) = cons(&ctx->fstack, -1, AS(ATOM, S(ctx)));
	invert(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#0 #-2 #-1 #-8 ", print(buf, S(ctx)));
}

// LISTS

void test_LISTS_empty() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	empty(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ } ", print(buf, S(ctx)));
}

void test_LISTS_list_to_stack() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	list_to_stack(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_STACK_UNDERFLOW, ctx->err);

	ctx->err = 0;
	S(ctx) = cons(&ctx->fstack, 0, AS(ATOM, 0));
	list_to_stack(ctx);

	TEST_ASSERT_EQUAL_INT(ERR_EXPECTED_LIST, ctx->err);
}

void test_LISTS_list_to_stack_2() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	empty(ctx);
	list_to_stack(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("", print(buf, S(ctx)));
}

void test_LISTS_list_to_stack_3() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = 
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 7, AS(ATOM, 
			cons(&ctx->fstack, 11, AS(ATOM, 0)))), 
		AS(LIST, 0));

	list_to_stack(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#7 #11 ", print(buf, S(ctx)));
}

void test_LISTS_reverse_list() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = 
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 7, AS(ATOM,
			cons(&ctx->fstack, 11, AS(ATOM,
			cons(&ctx->fstack, 13, AS(ATOM, 0)))))),
		AS(LIST, 0));

	reverse_list(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ #13 #11 #7 } ", print(buf, S(ctx)));
}

void test_LISTS_main_stack() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) =
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 7, AS(ATOM,
			cons(&ctx->fstack, 11, AS(ATOM, 0)))),
		AS(LIST,
		cons(&ctx->fstack, 13, AS(ATOM, 0))));

	list_to_stack(ctx);
	S(ctx) = cons(&ctx->fstack, 17, AS(ATOM, S(ctx)));
	main_stack(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ #17 #7 #11 } #13 ", print(buf, S(ctx)));
}

// INNER INTERPRETER

void test_EXEC_incrIP_1() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->ip = 0;
	CELL free = length(ctx->fstack, 0);

	// TODO: dump_context to compare prev and post context strings

	incrIP(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->ip);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);
	TEST_ASSERT_EQUAL_INT(0, length(S(ctx), 0));
	TEST_ASSERT_EQUAL_INT(free, length(ctx->fstack, 0));
}

void test_EXEC_incrIP_2() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	NODE* i1 = cons(&ctx->fstack, 11, AS(ATOM, 0));
	NODE* i2 = cons(&ctx->fstack, 7, AS(ATOM, i1));

	ctx->ip = i2;

	CELL free = length(ctx->fstack, 0);

	incrIP(ctx);

	TEST_ASSERT_EQUAL_INT(i1, ctx->ip);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);

	incrIP(ctx);

	TEST_ASSERT_EQUAL_INT(0, ctx->ip);
	TEST_ASSERT_EQUAL_INT(0, ctx->err);

	TEST_ASSERT_EQUAL_INT(free, length(ctx->fstack, 0));
}

void test_EXEC_step_nothing() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	CELL free = length(ctx->fstack, 0);

	NODE* ip = step(ctx);

	TEST_ASSERT_EQUAL_INT(0, length(S(ctx), 0));
	TEST_ASSERT_EQUAL_INT(0, length(ctx->ip, 0));
	TEST_ASSERT_EQUAL_INT(0, ip);
	TEST_ASSERT_EQUAL_INT(free, length(ctx->fstack, 0));
}

void test_EXEC_step_atom() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->ip = cons(&ctx->fstack, 13, AS(ATOM, cons(&ctx->fstack, 7, AS(ATOM, 0))));

	CELL free = length(ctx->fstack, 0);

	step(ctx); 

	TEST_ASSERT_EQUAL_INT(1, length(S(ctx), 0));
	TEST_ASSERT_EQUAL_INT(1, length(ctx->ip, 0));
	TEST_ASSERT_EQUAL_INT(13, S(ctx)->value);

	step(ctx);

	TEST_ASSERT_EQUAL_INT(2, length(S(ctx), 0));
	TEST_ASSERT_EQUAL_INT(0, length(ctx->ip, 0));
	TEST_ASSERT_EQUAL_INT(7, S(ctx)->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(S(ctx))->value);

	TEST_ASSERT_EQUAL_INT(free - 2, length(ctx->fstack, 0));
}

void test_EXEC_step_list() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->ip =
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 7, AS(ATOM, 
			cons(&ctx->fstack, 11, AS(ATOM, 
			cons(&ctx->fstack, 13, AS(ATOM, 0)))))), 
		AS(LIST, 0));

	CELL free = length(ctx->fstack, 0);

	step(ctx);

	TEST_ASSERT_EQUAL_INT(0, length(ctx->ip, 0));
	TEST_ASSERT_EQUAL_INT(0, length(ctx->rstack, 0));

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ #7 #11 #13 } ", print(buf, S(ctx)));

	TEST_ASSERT_EQUAL_INT(free - 4, length(ctx->fstack, 0));
}

void test_EXEC_step_list_2() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->ip = 
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 7, AS(ATOM, 
			cons(&ctx->fstack, 11, AS(ATOM, 
			cons(&ctx->fstack, 13, AS(ATOM, 0)))))), 
		AS(LIST, 
		cons(&ctx->fstack, 17, AS(ATOM, 0))));

	CELL free = length(ctx->fstack, 0);

	while(step(ctx));

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#17 { #7 #11 #13 } ", print(buf, S(ctx)));

	TEST_ASSERT_EQUAL_INT(free - 5, length(ctx->fstack, 0));
}

void test_EXEC_step_primitive() {
	BYTE buf[255];
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	ctx->ip =
		cons(&ctx->fstack, 13, AS(ATOM, 
		cons(&ctx->fstack, 7, AS(ATOM, 
		cons(&ctx->fstack, (CELL)&add, AS(PRIM, 
		cons(&ctx->fstack, (CELL)&duplicate, AS(PRIM, 0))))))));

	CELL free = length(ctx->fstack, 0);

	step(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#13 ", print(buf, S(ctx)));

	step(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#7 #13 ", print(buf, S(ctx)));

	step(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#20 ", print(buf, S(ctx)));

	step(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#20 #20 ", print(buf, S(ctx)));

	TEST_ASSERT_EQUAL_INT(free - 2, length(ctx->fstack, 0));
}

//void test_EXEC_CALL_word() {
//	CELL size = 512;
//	BYTE block[size];
//	CTX* ctx = init(block, size);
//
//	CELL word = 
//		cons(ctx, AS(NDNN,
//			cons(ctx, (CELL)"test_word", AS(ATOM,
//			cons(ctx, (CELL)&duplicate, AS(PRIM, 
//			cons(ctx, (CELL)&add, AS(PRIM, 0))))))),
//		AS(WORD, 0));
//
//	CELL free = ctx->free;
//
//	CALL(ctx, XT(ctx, word));	
//
//	TEST_ASSERT_EQUAL_INT(0, length(ctx->rstack));
//	TEST_ASSERT_EQUAL_INT(XT(ctx, word), IP(ctx));
//	TEST_ASSERT_EQUAL_INT(PRIM, TYPE(IP(ctx)));
//	TEST_ASSERT_EQUAL_INT((CELL)&duplicate, CAR(IP(ctx)));
//
//	TEST_ASSERT_EQUAL_INT(free, ctx->free);
//}

int main() {
	UNITY_BEGIN();

	// CORE
	RUN_TEST(test_CORE_length);
	RUN_TEST(test_CORE_reverse);
	RUN_TEST(test_CORE_init_free);
	RUN_TEST(test_CORE_cons);
	RUN_TEST(test_CORE_reclaim);
	RUN_TEST(test_CORE_reclaim_list);
	RUN_TEST(test_CORE_clone);

	// CONTEXT
	RUN_TEST(test_CONTEXT_data_stack);

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
	RUN_TEST(test_ARITHMETIC_incr);
	RUN_TEST(test_ARITHMETIC_sub);
	RUN_TEST(test_ARITHMETIC_decr);
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
	RUN_TEST(test_LISTS_reverse_list);
	RUN_TEST(test_LISTS_main_stack);

	// INNER INTERPRETER
	RUN_TEST(test_EXEC_incrIP_1);
	RUN_TEST(test_EXEC_incrIP_2);
	RUN_TEST(test_EXEC_step_nothing);
	RUN_TEST(test_EXEC_step_atom);
	RUN_TEST(test_EXEC_step_list);
	RUN_TEST(test_EXEC_step_list_2);
	RUN_TEST(test_EXEC_step_primitive);

	return UNITY_END();
}

