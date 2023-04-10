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

void test_CORE_ncons() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	TEST_ASSERT_EQUAL_PTR(saved_free - 15, last);

	NODE* n1 = cons(&free, 1, 7, (NODE*)11);

	TEST_ASSERT_EQUAL_PTR(saved_free, n1);
	TEST_ASSERT_EQUAL_PTR(saved_free - 1, free);
	TEST_ASSERT_EQUAL_INT(15, length(free));
	TEST_ASSERT_EQUAL_INT(15, inverse_length(last));
	TEST_ASSERT_EQUAL_INT(7, n1->value);
	TEST_ASSERT_EQUAL_PTR(11, n1->next);

	NODE* n2 = cons(&free, 2, 0, (NODE*)17); 

	TEST_ASSERT_EQUAL_INT(13, length(free));
	TEST_ASSERT_EQUAL_INT(13, inverse_length(last));
	TEST_ASSERT_EQUAL_INT(saved_free - 3, free);
	TEST_ASSERT_EQUAL_INT(saved_free - 2, n2);
	TEST_ASSERT_EQUAL_INT(2, n2->size);
	TEST_ASSERT_EQUAL_PTR(17, n2->next);

	NODE* n3 = cons(&free, 3, 0, 0);

	TEST_ASSERT_EQUAL_INT(10, length(free));
	TEST_ASSERT_EQUAL_INT(10, inverse_length(last));
	TEST_ASSERT_EQUAL_INT(saved_free - 6, free);
	TEST_ASSERT_EQUAL_INT(saved_free - 5, n3);
	TEST_ASSERT_EQUAL_INT(3, n3->size);

	NODE* n4 = cons(&free, 9, 0, 0);

	TEST_ASSERT_EQUAL_INT(1, length(free));
	TEST_ASSERT_EQUAL_INT(1, inverse_length(last));
}

void test_CORE_reclaim() {
	CELL size = 16*2*sizeof(CELL);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	NODE* n1 = cons(&free, 1, 7, AS(ATOM, 0));

	NODE* t1 = reclaim(&free, n1);

	TEST_ASSERT_EQUAL_INT(16, length(free));
	TEST_ASSERT_EQUAL_INT(16, inverse_length(last));
	TEST_ASSERT_EQUAL_PTR(0, t1);
	TEST_ASSERT_EQUAL_PTR(saved_free, free);

	n1 = cons(&free, 3, 7, AS(CARRAY, 0));

	t1 = reclaim(&free, n1);

	TEST_ASSERT_EQUAL_INT(16, length(free));
	TEST_ASSERT_EQUAL_INT(16, inverse_length(last));
	TEST_ASSERT_EQUAL_PTR(0, t1);
	TEST_ASSERT_EQUAL_PTR(saved_free, free);

	n1 =
		cons(&free, 1, (CELL)AS(LIST,
			cons(&free, 1, 7, AS(ATOM,
			cons(&free, 2, 0, AS(CARRAY, 0))))),
		AS(COLL, 0));

	t1 = reclaim(&free, n1);

	TEST_ASSERT_EQUAL_INT(16, length(free));
	TEST_ASSERT_EQUAL_INT(16, inverse_length(last));
	TEST_ASSERT_EQUAL_PTR(0, t1);
}

// TODO: Add complex tests of consing and reclaiming different sizes to check if
// everything is correctly reclaimed

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_length);
	RUN_TEST(test_CORE_inverse_length);
	RUN_TEST(test_CORE_init_free);

	RUN_TEST(test_CORE_ncons);
	RUN_TEST(test_CORE_reclaim);

	return UNITY_END();
}

