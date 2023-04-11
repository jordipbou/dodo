#include<stdlib.h>
#include<string.h>

#include "unity.h"
#include "core.h"

void setUp() {}

void tearDown() {}

void test_CORE_length() {
	NODE* n1 = malloc(sizeof(NODE));
	NODE* n2 = malloc(sizeof(NODE));
	NODE* n3 = malloc(sizeof(NODE));

	n1->next = n2;
	n2->next = n3;
	n3->next = 0;

	TEST_ASSERT_EQUAL_PTR(3, length(n1));
	TEST_ASSERT_EQUAL_PTR(2, length(n2));
	TEST_ASSERT_EQUAL_PTR(1, length(n3));
	TEST_ASSERT_EQUAL_PTR(0, length(0));
}

CELL inverse_length(NODE* list) {
	CELL count = 0;
	while (list) {
		count++;
		list = REF(list);
	}
	return count;
}

void test_CORE_inverse_length() {
	NODE* n1 = malloc(sizeof(NODE));
	NODE* n2 = malloc(sizeof(NODE));
	NODE* n3 = malloc(sizeof(NODE));

	n3->ref = n2;
	n2->ref = n1;
	n1->ref = 0;

	TEST_ASSERT_EQUAL_PTR(3, inverse_length(n3));
	TEST_ASSERT_EQUAL_PTR(2, inverse_length(n2));
	TEST_ASSERT_EQUAL_PTR(1, inverse_length(n1));
	TEST_ASSERT_EQUAL_PTR(0, inverse_length(0));
}

void test_CORE_init_free() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	TEST_ASSERT_EQUAL_PTR((NODE*)ALIGN(block + size - 2*sizeof(CELL) - 1, 2*sizeof(CELL)), free);
	TEST_ASSERT_EQUAL_INT(16, length(free));
	TEST_ASSERT_EQUAL_INT(16, inverse_length(last));
}

void test_CORE_cons() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	TEST_ASSERT_EQUAL_PTR(saved_free - 15, last);

	NODE* n1 = CONS1(&free, 7, (NODE*)11);

	TEST_ASSERT_EQUAL_PTR(saved_free, n1);
	TEST_ASSERT_EQUAL_PTR(saved_free - 1, free);
	TEST_ASSERT_EQUAL_INT(15, length(free));
	TEST_ASSERT_EQUAL_INT(15, inverse_length(last));
	TEST_ASSERT_EQUAL_INT(7, n1->value);
	TEST_ASSERT_EQUAL_PTR(11, n1->next);

	NODE* n2 = CONS2(&free, 0, 0, (NODE*)17);

	TEST_ASSERT_EQUAL_INT(13, length(free));
	TEST_ASSERT_EQUAL_INT(13, inverse_length(last));
	TEST_ASSERT_EQUAL_INT(saved_free - 3, free);
	TEST_ASSERT_EQUAL_INT(saved_free - 2, n2);
	TEST_ASSERT_EQUAL_INT(1, n2->size);
	TEST_ASSERT_EQUAL_PTR(17, n2->next);

	NODE* n3 = CONSn(&free, 3, 0);

	TEST_ASSERT_EQUAL_INT(10, length(free));
	TEST_ASSERT_EQUAL_INT(10, inverse_length(last));
	TEST_ASSERT_EQUAL_INT(saved_free - 6, free);
	TEST_ASSERT_EQUAL_INT(saved_free - 5, n3);
	TEST_ASSERT_EQUAL_INT(2, n3->size);

	NODE* n4 = CONSn(&free, 9, 0);

	TEST_ASSERT_EQUAL_INT(1, length(free));
	TEST_ASSERT_EQUAL_INT(1, inverse_length(last));
}

void test_CORE_reclaim() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	NODE* n1 = CONS1(&free, 7, AS(ATOM, 0));

	NODE* t1 = reclaim(&free, n1);

	TEST_ASSERT_EQUAL_INT(16, length(free));
	TEST_ASSERT_EQUAL_INT(16, inverse_length(last));
	TEST_ASSERT_EQUAL_PTR(0, t1);
	TEST_ASSERT_EQUAL_PTR(saved_free, free);

	n1 = CONSn(&free, 3, AS(ARRAY, 0));

	t1 = reclaim(&free, n1);

	TEST_ASSERT_EQUAL_INT(16, length(free));
	TEST_ASSERT_EQUAL_INT(16, inverse_length(last));
	TEST_ASSERT_EQUAL_PTR(0, t1);
	TEST_ASSERT_EQUAL_PTR(saved_free, free);

	n1 = 
		CONS1(&free, (CELL)AS(LIST,
			CONS1(&free, 7, AS(ATOM,
			CONS2(&free, 0, 0, AS(ARRAY, 0))))),
		AS(COLL, 0));

	t1 = reclaim(&free, n1);

	TEST_ASSERT_EQUAL_INT(16, length(free));
	TEST_ASSERT_EQUAL_INT(16, inverse_length(last));
	TEST_ASSERT_EQUAL_PTR(0, t1);
}

void test_CORE_cons_reclaim() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	NODE* n1 = CONS1(&free, 7, AS(ATOM, 0));
	NODE* n2 = CONS1(&free, 11, AS(ATOM, 0));
	NODE* n3 = CONS1(&free, 13, AS(ATOM, 0));
	NODE* n4 = CONS1(&free, 17, AS(ATOM, 0));

	reclaim(&free, n3);
	reclaim(&free, n2);

	NODE* n5 = CONS2(&free, 19, 23, AS(PLIT, 0));

	TEST_ASSERT_EQUAL_INT(12, length(free));
	TEST_ASSERT_EQUAL_INT(12, inverse_length(last));

	TEST_ASSERT_EQUAL_PTR(n3, n5);
}

void test_CORE_cons_reclaim_2() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	NODE* n1 = CONS1(&free, 7, AS(ATOM, 0));
	NODE* n2 = CONS1(&free, 11, AS(ATOM, 0));
	NODE* n3 = CONS1(&free, 13, AS(ATOM, 0));
	NODE* n4 = CONS1(&free, 17, AS(ATOM, 0));

	reclaim(&free, n2);
	reclaim(&free, n3);

	NODE* n5 = CONS2(&free, 19, 23, AS(PLIT, 0));

	TEST_ASSERT_EQUAL_INT(12, length(free));
	TEST_ASSERT_EQUAL_INT(12, inverse_length(last));

	TEST_ASSERT_EQUAL_PTR(n2, n5);
}

void test_CORE_clone_atom() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	NODE* n1 = CONS1(&free, 7, AS(ATOM, 0));

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(TYPE(n1), TYPE(n2));
	TEST_ASSERT_EQUAL_INT(n1->value, n2->value);

	TEST_ASSERT_EQUAL_INT(14, length(free));
}

void test_CORE_clone_prim() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	NODE* n1 = CONS1(&free, 7, AS(PRIM, 0));

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(TYPE(n1), TYPE(n2));
	TEST_ASSERT_EQUAL_INT(n1->value, n2->value);

	TEST_ASSERT_EQUAL_INT(14, length(free));
}

void test_CORE_clone_plit() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	NODE* n1 = CONS2(&free, 7, 11, AS(PLIT, 0));

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(TYPE(n1), TYPE(n2));
	TEST_ASSERT_EQUAL_INT(n1->size, n2->size);
	TEST_ASSERT_EQUAL_INT(n1->data[0], n2->data[0]);
	TEST_ASSERT_EQUAL_INT(n1->data[1], n2->data[1]);

	TEST_ASSERT_EQUAL_INT(12, length(free));
}

void test_CORE_clone_string() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	NODE* n1 = CONSn(&free, 2, AS(PLIT, 0));
	strcpy(BYTES(n1), "test string");

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(TYPE(n1), TYPE(n2));
	TEST_ASSERT_EQUAL_INT(n1->size, n2->size);
	TEST_ASSERT_EQUAL_STRING(BYTES(n1), BYTES(n2));

	TEST_ASSERT_EQUAL_INT(12, length(free));
}

void test_CORE_clone_barray() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	BYTE array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

	NODE* n1 = CONSn(&free, 3, AS(PLIT, 0));
	memcpy(BYTES(n1), array, 20);

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(TYPE(n1), TYPE(n2));
	TEST_ASSERT_EQUAL_INT(n1->size, n2->size);
	TEST_ASSERT_EQUAL_STRING(BYTES(n1), BYTES(n2));

	TEST_ASSERT_EQUAL_INT(10, length(free));
}

void test_CORE_clone_array() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	CELL array[] = {0, 1, 2, 3};

	NODE* n1 = CONSn(&free, 3, AS(PLIT, 0));
	memcpy(n1->data, array, 4);

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(TYPE(n1), TYPE(n2));
	TEST_ASSERT_EQUAL_INT(n1->size, n2->size);
	TEST_ASSERT_EQUAL_MEMORY(n1->data, n2->data, 4*sizeof(CELL));

	TEST_ASSERT_EQUAL_INT(10, length(free));
}

void test_CORE_clone_list() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	NODE* n1 = 
		CONS1(&free, (CELL)AS(LIST,
			CONS1(&free, 7, AS(ATOM, 
			CONS1(&free, 11, AS(ATOM, 0))))),
		AS(COLL, 0));

	TEST_ASSERT_EQUAL_INT(13, length(free));

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_EQUAL_INT(10, length(free));

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(COLL, TYPE(n2));
	TEST_ASSERT_EQUAL_INT(LIST, SUBTYPE(n2));
	TEST_ASSERT_NOT_EQUAL(n1->ref, n2->ref);
	TEST_ASSERT_EQUAL_INT(2, length(REF(n2)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(REF(n2)));
	TEST_ASSERT_EQUAL(7, REF(n2)->value);
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(REF(n2))));
	TEST_ASSERT_EQUAL(11, NEXT(REF(n2))->value);
}

void test_CORE_clone_word() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	NODE* l = CONS1(&free, 7, AS(ATOM, 0));
	NODE* n1 = CONS1(&free, (CELL)AS(WORD, l), AS(COLL, 0));

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(TYPE(n1), TYPE(n2));
	TEST_ASSERT_EQUAL_INT(SUBTYPE(n1), SUBTYPE(n2));
	TEST_ASSERT_EQUAL_INT(n1->value, n2->value);

	TEST_ASSERT_EQUAL_INT(13, length(free));
}

void test_CORE_clone_dual() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	NODE* l = CONS1(&free, 7, AS(ATOM, 0));
	NODE* n1 = CONS1(&free, (CELL)AS(DUAL, l), AS(COLL, 0));

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(TYPE(n1), TYPE(n2));
	TEST_ASSERT_EQUAL_INT(SUBTYPE(n1), SUBTYPE(n2));
	TEST_ASSERT_EQUAL_INT(n1->value, n2->value);

	TEST_ASSERT_EQUAL_INT(13, length(free));
}

void test_CORE_clone_user() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	NODE* n1 = 
		CONS1(&free, (CELL)AS(USER,
			CONS1(&free, 7, AS(ATOM, 
			CONS1(&free, 11, AS(ATOM, 0))))),
		AS(COLL, 0));

	TEST_ASSERT_EQUAL_INT(13, length(free));

	NODE* n2 = clone(&free, n1);

	TEST_ASSERT_EQUAL_INT(10, length(free));

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(COLL, TYPE(n2));
	TEST_ASSERT_EQUAL_INT(USER, SUBTYPE(n2));
	TEST_ASSERT_NOT_EQUAL(n1->ref, n2->ref);
	TEST_ASSERT_EQUAL_INT(2, length(REF(n2)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(REF(n2)));
	TEST_ASSERT_EQUAL(7, REF(n2)->value);
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(REF(n2))));
	TEST_ASSERT_EQUAL(11, NEXT(REF(n2))->value);
}

#define f_nodes(ctx)	(((ctx->size - ALIGN(sizeof(CTX), sizeof(NODE))) / (sizeof(NODE))) - 1)

void test_CONTEXT_init() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	TEST_ASSERT_EQUAL_INT(f_nodes(ctx), length(ctx->fstack) - 1);
}

// STACK PRIMITIVES

void test_STACK_duplicate_atom() {
	CELL size = 512;
	BYTE block[size];
	CTX* ctx = init(block, size);

	S(ctx) = CONS1(&F(ctx), 7, AS(ATOM, S(ctx)));

	duplicate(ctx);

	TEST_ASSERT_EQUAL_INT(2, length(S(ctx)));
	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 1, length(F(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(S(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(S(ctx))));
	TEST_ASSERT_EQUAL_INT(7, S(ctx)->value);
	TEST_ASSERT_EQUAL_INT(7, NEXT(S(ctx))->value);

	S(ctx) = CONS1(&F(ctx), 11, AS(ATOM, S(ctx)));

	duplicate(ctx);

	TEST_ASSERT_EQUAL_INT(4, length(S(ctx)));
	TEST_ASSERT_EQUAL_INT(f_nodes(ctx) - 3, length(F(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(S(ctx)));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(S(ctx))));
	TEST_ASSERT_EQUAL_INT(11, S(ctx)->value);
	TEST_ASSERT_EQUAL_INT(11, NEXT(S(ctx))->value);
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(NEXT(S(ctx)))));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(NEXT(NEXT(NEXT(S(ctx))))));
	TEST_ASSERT_EQUAL_INT(7, NEXT(NEXT(S(ctx)))->value);
	TEST_ASSERT_EQUAL_INT(7, NEXT(NEXT(NEXT(S(ctx))))->value);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_length);
	RUN_TEST(test_CORE_inverse_length);
	RUN_TEST(test_CORE_init_free);

	RUN_TEST(test_CORE_cons);
	RUN_TEST(test_CORE_reclaim);
	RUN_TEST(test_CORE_cons_reclaim);
	RUN_TEST(test_CORE_cons_reclaim_2);

	RUN_TEST(test_CORE_clone_atom);
	RUN_TEST(test_CORE_clone_prim);
	RUN_TEST(test_CORE_clone_plit);
	RUN_TEST(test_CORE_clone_string);
	RUN_TEST(test_CORE_clone_barray);
	RUN_TEST(test_CORE_clone_array);
	RUN_TEST(test_CORE_clone_list);
	RUN_TEST(test_CORE_clone_word);
	RUN_TEST(test_CORE_clone_dual);
	RUN_TEST(test_CORE_clone_user);

	RUN_TEST(test_CONTEXT_init);

	RUN_TEST(test_STACK_duplicate_atom);

	return UNITY_END();
}

