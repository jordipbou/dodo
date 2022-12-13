#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_block_initialization() {
	CELL size = 2048;
	BYTE block[size];
	CTX* c = init(block, size);

	TEST_ASSERT_EQUAL_INT((size - sizeof(CTX)) / sizeof(NODE) - 1, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(0, DEPTH(c));
}

void test_allot() {
	CELL size = 2048;
	BYTE block[size];
	CTX* c = init(block, size);

	CELL nodes = ((size - sizeof(CTX)) / sizeof(NODE)) - 1;

	TEST_ASSERT_EQUAL_INT(0, RESERVED(c));
	TEST_ASSERT_EQUAL_INT(0, c->here - c->bottom);
	TEST_ASSERT_EQUAL_INT(nodes, FREE_NODES(c));
	allot(c, 3);
	TEST_ASSERT_EQUAL_INT(13, RESERVED(c));
	TEST_ASSERT_EQUAL_INT(3, c->here - c->bottom);
	TEST_ASSERT_EQUAL_INT(nodes - 1, FREE_NODES(c));
	allot(c, 13);
	TEST_ASSERT_EQUAL_INT(0, RESERVED(c));
	TEST_ASSERT_EQUAL_INT(16, c->here - c->bottom);
	TEST_ASSERT_EQUAL_INT(nodes - 1, FREE_NODES(c));
}

void test_stack() {
	CELL size = 2048;
	BYTE block[size];
	CTX* c = init(block, size);

	CELL nodes = ((size - sizeof(CTX)) / sizeof(NODE)) - 1;

	TEST_ASSERT_EQUAL_INT(nodes, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(0, DEPTH(c));
	push(c, 13);
	TEST_ASSERT_EQUAL_INT(nodes - 1, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(1, DEPTH(c));
	push(c, 17);
	TEST_ASSERT_EQUAL_INT(nodes - 2, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(2, DEPTH(c));
	CELL val = pop(c);
	TEST_ASSERT_EQUAL_INT(nodes - 1, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(1, DEPTH(c));
	TEST_ASSERT_EQUAL_INT(17, val);
	val = pop(c);
	TEST_ASSERT_EQUAL_INT(nodes, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(0, DEPTH(c));
	TEST_ASSERT_EQUAL_INT(13, val);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_allot);

	RUN_TEST(test_stack);

	return UNITY_END();
}
