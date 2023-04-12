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
	TEST_ASSERT_EQUAL_STRING("{ #7 P:11 { #13 #17 } #19 } ", dump_list(buf, list));

	TEST_ASSERT_EQUAL_INT(10, length(free, 0));
	TEST_ASSERT_EQUAL_INT(10, length(last, 1));

	NODE* cloned = clone(&free, list);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ #7 P:11 { #13 #17 } #19 } ", dump_list(buf, cloned));

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
	TEST_ASSERT_EQUAL_STRING("{ { #17 #13 { #11 #7 } } } ", dump_list(buf, S(ctx)));

	duplicate(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING(
		"{ { #17 #13 { #11 #7 } } { #17 #13 { #11 #7 } } } ",
		dump_list(buf, S(ctx)));

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
	TEST_ASSERT_EQUAL_STRING("{ #7 #11 } ", dump_list(buf, S(ctx)));
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
	TEST_ASSERT_EQUAL_STRING("{ { #7 #5 } #11 } ", dump_list(buf, S(ctx)));
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
	TEST_ASSERT_EQUAL_STRING("{ #11 #7 #11 } ", dump_list(buf, S(ctx)));

	S(ctx) = 
		cons(&ctx->fstack, 7, AS(ATOM, 
		cons(&ctx->fstack, (CELL)
			cons(&ctx->fstack, 11, AS(ATOM,
			cons(&ctx->fstack, 13, AS(ATOM, 0)))),
		AS(LIST, 0))));

	over(ctx);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ { #11 #13 } #7 { #11 #13 } } ", dump_list(buf, S(ctx)));
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
	TEST_ASSERT_EQUAL_STRING("{ #13 #7 #11 } ", dump_list(buf, S(ctx)));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_length);
	RUN_TEST(test_CORE_init_free);
	RUN_TEST(test_CORE_cons);
	RUN_TEST(test_CORE_reclaim);
	RUN_TEST(test_CORE_reclaim_list);
	RUN_TEST(test_CORE_clone);

	RUN_TEST(test_CONTEXT_data_stack);

	RUN_TEST(test_STACK_duplicate_atom);
	RUN_TEST(test_STACK_duplicate_list);
	RUN_TEST(test_STACK_swap_1);
	RUN_TEST(test_STACK_swap_2);
	RUN_TEST(test_STACK_drop);
	RUN_TEST(test_STACK_over);
	RUN_TEST(test_STACK_rot);

	return UNITY_END();
}

