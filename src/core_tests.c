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

void test_CORE_init_block() {
	CELL size = 32*sizeof(NODE);
	BYTE buffer[size];

	// Ensure its aligned to sizeof(NODE)
	TEST_ASSERT_EQUAL_INT(ALIGN(buffer, sizeof(NODE)), buffer);

	BLOCK* block = init_block(buffer, size, 0);

	TEST_ASSERT_EQUAL_INT(29, FREE(block));
	TEST_ASSERT_EQUAL_INT((((CELL)block->free) - ((CELL)block->there)) / sizeof(NODE), FREE(block));
}

void test_CORE_init_block_2() {
	CELL size = 32*sizeof(NODE);
	BYTE buffer[size];

	BLOCK* block = init_block(buffer, size, 3);

	TEST_ASSERT_EQUAL_PTR(buffer + 3 + sizeof(BLOCK), block->bottom);
	TEST_ASSERT_EQUAL_PTR(block->bottom, block->here);
	TEST_ASSERT_EQUAL_PTR(ALIGN(buffer + 3 + sizeof(BLOCK), sizeof(NODE)), block->there);
	TEST_ASSERT_EQUAL_PTR(ALIGN(buffer + size - 3 - sizeof(NODE) - 1, sizeof(NODE)), block->free);

	CELL free_nodes = (((CELL)block->free) - ((CELL)block->there)) / sizeof(NODE);

	TEST_ASSERT_EQUAL_INT(free_nodes, FREE(block));
	TEST_ASSERT_EQUAL_INT(free_nodes, length(block->there, 1) - 1);
}

void test_CORE_cons() {
	CELL size = 16*sizeof(NODE);
	BYTE buffer[size];
	BLOCK* block = init_block(buffer, size, 0);

	CELL free = FREE(block);

	NODE* list = cons(block, 7, AS(ATOM, cons(block, 11, AS(ATOM, cons(block, 13, AS(ATOM, 0))))));

	TEST_ASSERT_NOT_NULL(list);

	TEST_ASSERT_EQUAL_INT(free - 3, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 3, length(block->there, 1) - 1);
	TEST_ASSERT_EQUAL_INT(3, length(list, 0));
	TEST_ASSERT_EQUAL_INT(7, list->value);
	TEST_ASSERT_EQUAL_INT(11, NEXT(list)->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(NEXT(list))->value);

	NODE* p = cons(block, 7, AS(ATOM, 0));

	TEST_ASSERT_NOT_NULL(p);

	TEST_ASSERT_EQUAL_INT(free - 4, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 4, length(block->there, 1) - 1);
	TEST_ASSERT_EQUAL_INT(7, p->value);
	TEST_ASSERT_EQUAL_INT(0, NEXT(p));
}

void test_CORE_array() {
	CELL size = 16*sizeof(NODE);
	BYTE buffer[size];
	BLOCK* block = init_block(buffer, size, 0);

	CELL free = FREE(block);

	ARRAY* a = array(block, 0);

	TEST_ASSERT_NULL(a);

	a = array(block, 1);

	TEST_ASSERT_EQUAL_INT(2 * sizeof(NODE), a->size);
	TEST_ASSERT_EQUAL_INT(sizeof(NODE), SIZE(a));
	TEST_ASSERT_EQUAL_INT(free - 2, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 2, length(block->there, 1) - 1);

	ARRAY* b = array(block, 43);

	TEST_ASSERT_NOT_NULL(b);

	TEST_ASSERT_EQUAL_INT(4 * sizeof(NODE), b->size);
	TEST_ASSERT_EQUAL_INT(3 * sizeof(NODE), SIZE(b));
	TEST_ASSERT_EQUAL_INT(free - 6, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 6, length(block->there, 1) - 1);

	ARRAY* c = array(block, 20 * sizeof(NODE));

	TEST_ASSERT_NULL(c);

	TEST_ASSERT_EQUAL_INT(free - 6, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 6, length(block->there, 1) - 1);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_length);
	RUN_TEST(test_CORE_reverse);
	RUN_TEST(test_CORE_init_block);
	RUN_TEST(test_CORE_init_block_2);
	RUN_TEST(test_CORE_cons);
	RUN_TEST(test_CORE_array);

	return UNITY_END();
}

