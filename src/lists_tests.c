#include<stdlib.h>

#include "unity.h"
#include "lists.h"

void setUp() {}

void tearDown() {}

void test_LISTS_pairs() {
	CELL pair = (CELL)malloc(2*sizeof(CELL));

	CAR(pair) = 7;	
	CDR(pair) = AS(ATOM, 8);
	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(8, NEXT(pair));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
	CDR(pair) = AS(LIST, 16);
	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(16, NEXT(pair));
	TEST_ASSERT_NOT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
	CDR(pair) = AS(PRIM, 24);
	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(24, NEXT(pair));
	TEST_ASSERT_NOT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
	CDR(pair) = AS(WORD, 32);
	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(32, NEXT(pair));
	TEST_ASSERT_NOT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_EQUAL_INT(WORD, TYPE(pair));
}

void test_LISTS_link_pairs() {
	CELL pair = (CELL)malloc(sizeof(CELL) * 2);

	CAR(pair) = 7;
	CDR(pair) = AS(ATOM, 16);

	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(16, NEXT(pair));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
	
	LINK(pair, 32);

	TEST_ASSERT_EQUAL_INT(7, CAR(pair));
	TEST_ASSERT_EQUAL_INT(32, NEXT(pair));
	TEST_ASSERT_EQUAL_INT(ATOM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(LIST, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(PRIM, TYPE(pair));
	TEST_ASSERT_NOT_EQUAL_INT(WORD, TYPE(pair));
}

void test_LISTS_length() {
	CELL p1 = (CELL)malloc(2*sizeof(CELL));
	CELL p2 = (CELL)malloc(2*sizeof(CELL));
	CELL p3 = (CELL)malloc(2*sizeof(CELL));

	CDR(p1) = AS(ATOM, p2);
	CDR(p2) = p3;
	CDR(p3) = 0;

	TEST_ASSERT_EQUAL_INT(0, length(0));
	TEST_ASSERT_EQUAL_INT(1, length(p3));
	TEST_ASSERT_EQUAL_INT(2, length(p2));
	TEST_ASSERT_EQUAL_INT(3, length(p1));
}

void test_LISTS_reverse() {
	CELL p1 = (CELL)malloc(2*sizeof(CELL));
	CELL p2 = (CELL)malloc(2*sizeof(CELL));
	CELL p3 = (CELL)malloc(2*sizeof(CELL));
	
	CDR(p1) = p2;
	CDR(p2) = p3;
	CDR(p3) = 0;

	CELL r = reverse(p1, 0);

	TEST_ASSERT_EQUAL_INT(p3, r);
	TEST_ASSERT_EQUAL_INT(p2, NEXT(r));
	TEST_ASSERT_EQUAL_INT(p1, NEXT(NEXT(r)));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_LISTS_pairs);
	RUN_TEST(test_LISTS_link_pairs);
	RUN_TEST(test_LISTS_length);
	RUN_TEST(test_LISTS_reverse);

	return UNITY_END();
}

