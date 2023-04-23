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

	DATA* a = array(block, 0);

	TEST_ASSERT_NULL(a);

	a = array(block, 1);

	CELL sz = (((sizeof(DATA) + 1) / sizeof(NODE)) + (((sizeof(DATA) + 1) % sizeof(NODE)) == 0 ? 0 : 1)) * sizeof(NODE);

	TEST_ASSERT_EQUAL_INT(sz, a->size);
	TEST_ASSERT_EQUAL_INT(free - 2, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 2, length(block->there, 1) - 1);

	DATA* b = array(block, 43);

	TEST_ASSERT_NOT_NULL(b);

	sz = (((sizeof(DATA) + 43) / sizeof(NODE)) + (((sizeof(DATA) + 43) % sizeof(NODE)) == 0 ? 0 : 1)) * sizeof(NODE);

	TEST_ASSERT_EQUAL_INT(sz, b->size);
	TEST_ASSERT_EQUAL_INT(free - 7, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 7, length(block->there, 1) - 1);

	DATA* c = array(block, 20 * sizeof(NODE));

	TEST_ASSERT_NULL(c);

	TEST_ASSERT_EQUAL_INT(free - 7, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 7, length(block->there, 1) - 1);
}

void test_CORE_reclaim() {
	CELL size = 16*sizeof(NODE);
	BYTE buffer[size];
	BLOCK* block = init_block(buffer, size, 0);

	CELL free = FREE(block);

	NODE* list = cons(block, 7, AS(ATOM, cons(block, 11, AS(ATOM, cons(block, 13, AS(ATOM, 0))))));
	NODE* tail = reclaim(block, list);

	TEST_ASSERT_EQUAL_INT(free - 2, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 2, length(block->there, 1) - 1);
	TEST_ASSERT_EQUAL_INT(2, length(tail, 0));
	TEST_ASSERT_EQUAL_INT(11, tail->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(tail)->value);

	tail = reclaim(block, reclaim(block, tail));

	TEST_ASSERT_EQUAL_INT(free, FREE(block));
	TEST_ASSERT_EQUAL_INT(free, length(block->there, 1) - 1);
	TEST_ASSERT_EQUAL_INT(0, length(tail, 0));
}

void test_CORE_reclaim_list() {
	CELL size = 16*sizeof(NODE);
	BYTE buffer[size];
	BLOCK* block = init_block(buffer, size, 0);

	CELL free = FREE(block);

	NODE* list = 
		cons(block, 5, AS(ATOM,
		cons(block, (CELL)
			cons(block, 7, AS(ATOM,
			cons(block, 11, AS(ATOM,
			cons(block, 13, AS(ATOM, 0)))))), AS(LIST, 
		cons(block, 17, AS(ATOM, 0))))));

	TEST_ASSERT_EQUAL_INT(free - 6, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 6, length(block->there, 1) - 1);
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

	NODE* tail = reclaim(block, list);

	TEST_ASSERT_EQUAL_INT(free - 5, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 5, length(block->there, 1) - 1);
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

	tail = reclaim(block, tail);

	TEST_ASSERT_EQUAL_INT(free - 1, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 1, length(block->there, 1) - 1);
	TEST_ASSERT_EQUAL(ATOM, TYPE(tail));
	TEST_ASSERT_EQUAL_INT(17, tail->value);

	tail = reclaim(block, tail);

	TEST_ASSERT_EQUAL_INT(free, FREE(block));
	TEST_ASSERT_EQUAL_INT(free, length(block->there, 1) - 1);
	TEST_ASSERT_EQUAL_INT(0, tail);
}

void test_CORE_cons_reclaim() {
	CELL size = 16*sizeof(NODE);
	BYTE buffer[size];
	BLOCK* block = init_block(buffer, size, 0);

	CELL free = FREE(block);

	NODE* n1 = cons(block, 7, AS(ATOM, 0));
	NODE* n2 = cons(block, 11, AS(ATOM, 0));
	NODE* n3 = cons(block, 13, AS(ATOM, 0));
	NODE* n4 = cons(block, 17, AS(ATOM, 0));

	reclaim(block, n3);
	reclaim(block, n2);

	NODE* n5 = cons(block, (CELL)array(block, 2*sizeof(CELL)), AS(CODE, 0));

	TEST_ASSERT_NOT_EQUAL_INT(0, n5);
	TEST_ASSERT_EQUAL_INT(free - 6, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 6, length(block->there, 1) - 1);
}

void test_CORE_reclaim_data() {
}

void test_CORE_clone() {
	CELL size = 16*sizeof(NODE);
	BYTE buffer[size];
	BLOCK* block = init_block(buffer, size, 0);
	BYTE buf[255];

	CELL free = FREE(block);

	NODE* list = 
		cons(block, 7, AS(ATOM, 
		cons(block, 11, AS(PRIM, 
		cons(block, (CELL)
			cons(block, 13, AS(ATOM,
			cons(block, 17, AS(ATOM, 0)))), 
		AS(LIST,
		cons(block, 19, AS(ATOM, 0))))))));

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#7 P:11 { #13 #17 } #19 ", print(buf, list, 1, 0));

	TEST_ASSERT_EQUAL_INT(free - 6, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 6, length(block->there, 1) - 1);

	NODE* cloned = clone(block, list, 1);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#7 P:11 { #13 #17 } #19 ", print(buf, cloned, 1, 0));

	TEST_ASSERT_EQUAL_INT(free - 12, FREE(block));
	TEST_ASSERT_EQUAL_INT(free - 12, length(block->there, 1) - 1);

	TEST_ASSERT_NOT_EQUAL(list, cloned);
	TEST_ASSERT_NOT_EQUAL(NEXT(list), NEXT(cloned));
	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(list)), NEXT(NEXT(cloned)));
	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(list))->ref, NEXT(NEXT(cloned))->ref);
	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(NEXT(list))->ref), NEXT(NEXT(NEXT(cloned))->ref));
	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(NEXT(list))), NEXT(NEXT(NEXT(cloned))));
}

void test_EVALUATION_parse_token() {
}

void test_EVALUATION_compile() {
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_length);
	RUN_TEST(test_CORE_reverse);
	RUN_TEST(test_CORE_init_block);
	RUN_TEST(test_CORE_init_block_2);
	RUN_TEST(test_CORE_cons);
	RUN_TEST(test_CORE_array);
	RUN_TEST(test_CORE_reclaim);
	RUN_TEST(test_CORE_reclaim_list);
	RUN_TEST(test_CORE_cons_reclaim);
	RUN_TEST(test_CORE_clone);

	return UNITY_END();
}

