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

void test_STACK_over_strings() {
	BYTE buf[255];
	BYTE block[512];
	CTX* x = init(block, 512);

	S(x) = 
		cons(x, (CELL)string(x, "first", 5), AS(REF,
		cons(x, (CELL)string(x, "second", 6), AS(REF, 0))));

	over(x);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("\"second\" \"first\" \"second\" ", print(buf, S(x), 1, x));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_reverse);

	RUN_TEST(test_STACK_over_strings);

	return UNITY_END();
}

