#include "dodo.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_block_initialization() {
	BYTE block[2048];
	CTX* c = init(block, 2048);

	TEST_ASSERT_EQUAL_INT(124, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(0, DEPTH(c));
}

void test_allot() {
	BYTE block[2048];
	CTX* c = init(block, 2048);

	TEST_ASSERT_EQUAL_INT(0, RESERVED(c));
	TEST_ASSERT_EQUAL_INT(0, c->here - c->bottom);
	TEST_ASSERT_EQUAL_INT(124, FREE_NODES(c));
	allot(c, 3);
	TEST_ASSERT_EQUAL_INT(13, RESERVED(c));
	TEST_ASSERT_EQUAL_INT(3, c->here - c->bottom);
	TEST_ASSERT_EQUAL_INT(123, FREE_NODES(c));
	allot(c, 13);
	TEST_ASSERT_EQUAL_INT(0, RESERVED(c));
	TEST_ASSERT_EQUAL_INT(16, c->here - c->bottom);
	TEST_ASSERT_EQUAL_INT(123, FREE_NODES(c));
}

void test_stacks() {
	BYTE block[2048];
	CTX* c = init(block, 2048);

	TEST_ASSERT_EQUAL_INT(124, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(0, DEPTH(c));
	push(c, 13);
	TEST_ASSERT_EQUAL_INT(123, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(1, DEPTH(c));
	push(c, 17);
	TEST_ASSERT_EQUAL_INT(122, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(2, DEPTH(c));
	CELL val = pop(c);
	TEST_ASSERT_EQUAL_INT(123, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(1, DEPTH(c));
	TEST_ASSERT_EQUAL_INT(17, val);
	val = pop(c);
	TEST_ASSERT_EQUAL_INT(124, FREE_NODES(c));
	TEST_ASSERT_EQUAL_INT(0, DEPTH(c));
	TEST_ASSERT_EQUAL_INT(13, val);
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_allot);

	RUN_TEST(test_stacks);

	return UNITY_END();
}
