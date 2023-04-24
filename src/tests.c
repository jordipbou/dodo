#include<stdlib.h>
#include<string.h>

#include "unity.h"
#include "dodo.h"

void setUp() {}

void tearDown() {}

void test_CORE_reverse() {
	NODE* p1, * p2, * p3, * r;

	p1 = malloc(sizeof(NODE));
	p2 = malloc(sizeof(NODE));
	p3 = malloc(sizeof(NODE));
	
	p1->next = (CELL)p2;
	p2->next = (CELL)p3;
	p3->next = 0;

	r = reverse(p1, 0);

	TEST_ASSERT_EQUAL_INT(p3, r);
	TEST_ASSERT_EQUAL_INT(p2, N(r));
	TEST_ASSERT_EQUAL_INT(p1, N(N(r)));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_reverse);

	return UNITY_END();
}

