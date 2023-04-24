#include<stdlib.h>
#include<string.h>

#include "unity.h"
#include "dodo.h"

void setUp() {}

void tearDown() {}

CELL ilength(NODE* n) { CELL a = 0; while (n) { a++; n = (NODE*)n->val; } return a; }
#define iFREE(x)		(ilength(x->there) - 1)

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

void test_CORE_cons() {
	NODE* a, * b;
	BYTE buffer[16*sizeof(NODE)];
	CTX* x = init(buffer, 16*sizeof(NODE));

	CELL free = FREE(x);

	a = cons(x, 7, AS(ATOM, cons(x, 11, AS(ATOM, cons(x, 13, AS(ATOM, 0))))));

	TEST_ASSERT_NOT_NULL(a);

	TEST_ASSERT_EQUAL_INT(free - 3, FREE(x));
	TEST_ASSERT_EQUAL_INT(free - 3, iFREE(x));
	TEST_ASSERT_EQUAL_INT(3, length(a));
	TEST_ASSERT_EQUAL_INT(7, a->val);
	TEST_ASSERT_EQUAL_INT(11, N(a)->val);
	TEST_ASSERT_EQUAL_INT(13, N(N(a))->val);

	b = cons(x, 7, AS(ATOM, 0));

	TEST_ASSERT_NOT_NULL(b);

	TEST_ASSERT_EQUAL_INT(free - 4, FREE(x));
	TEST_ASSERT_EQUAL_INT(free - 4, iFREE(x));
	TEST_ASSERT_EQUAL_INT(7, b->val);
	TEST_ASSERT_EQUAL_INT(0, N(b));
}

void test_STACK_over_strings() {
	BYTE buf[255];
	BYTE buffer[512];
	CTX* x = init(buffer, 512);

	S(x) = 
		cons(x, (CELL)string(x, "first", 5), AS(REF,
		cons(x, (CELL)string(x, "second", 6), AS(REF, 0))));

	over(x);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("\"second\" \"first\" \"second\" ", print(buf, S(x), 1, x));
}

void test_LIST_list_to_stack() {
	BYTE buf[255];
	BYTE buffer[512];
	CTX* x = init(buffer, 512);

	S(x) =
		cons(x, (CELL)
			cons(x, (CELL)
				cons(x, 13, AS(ATOM,
				cons(x, 17, AS(ATOM, 0)))),
			AS(LIST,
			cons(x, 7, AS(ATOM,
			cons(x, 11, AS(ATOM, 0)))))),
		AS(LIST, 0));

	list_to_stack(x);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("{ #13 #17 } #7 #11 ", print(buf, S(x), 1, x));

	list_to_stack(x);

	buf[0] = 0;
	TEST_ASSERT_EQUAL_STRING("#13 #17 ", print(buf, S(x), 1, x));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_reverse);
	RUN_TEST(test_CORE_cons);

	RUN_TEST(test_STACK_over_strings);

	RUN_TEST(test_LIST_list_to_stack);

	return UNITY_END();
}

