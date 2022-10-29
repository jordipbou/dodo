#include <stdlib.h>

#include "unity.h"
#include "vm.h"

C length(P* l) {
	for (C a = 0;; a++) { if (!l) { return a; } else { l = l->cdr; } }
}

void setUp(void) {}

void tearDown(void) {}

void test_is_aligned(void) {
	TEST_ASSERT_TRUE(IS_ALIGNED(12345 << 3));
	TEST_ASSERT_FALSE(IS_ALIGNED(12345));
}

void test_cells(void) {
	TEST_ASSERT_EQUAL_INT(0, AS_CELLS(0));
	TEST_ASSERT_EQUAL_INT(1, AS_CELLS(1));
	TEST_ASSERT_EQUAL_INT(1, AS_CELLS(8));
	TEST_ASSERT_EQUAL_INT(2, AS_CELLS(9));
	TEST_ASSERT_EQUAL_INT(2, AS_CELLS(16));
	TEST_ASSERT_EQUAL_INT(3, AS_CELLS(17));
	TEST_ASSERT_EQUAL_INT(3, AS_CELLS(24));
	TEST_ASSERT_EQUAL_INT(4, AS_CELLS(25));
}

void test_bytes(void) {
	TEST_ASSERT_EQUAL_INT(0, AS_BYTES(AS_CELLS(0)));
	TEST_ASSERT_EQUAL_INT(8, AS_BYTES(AS_CELLS(1)));
	TEST_ASSERT_EQUAL_INT(8, AS_BYTES(AS_CELLS(8)));
	TEST_ASSERT_EQUAL_INT(16, AS_BYTES(AS_CELLS(9)));
	TEST_ASSERT_EQUAL_INT(16, AS_BYTES(AS_CELLS(16)));
	TEST_ASSERT_EQUAL_INT(24, AS_BYTES(AS_CELLS(17)));
	TEST_ASSERT_EQUAL_INT(24, AS_BYTES(AS_CELLS(24)));
	TEST_ASSERT_EQUAL_INT(32, AS_BYTES(AS_CELLS(25)));
}

void test_list_length(void) {
	P list[4];
	TEST_ASSERT_EQUAL_INT(0, length(NULL));
	list[0].cdr = NULL;
	TEST_ASSERT_EQUAL_INT(1, length(list));
	list[0].cdr = &list[1];
	list[1].cdr = NULL;
	TEST_ASSERT_EQUAL_INT(2, length(list));
	list[1].cdr = &list[2];
	list[2].cdr = NULL;
	TEST_ASSERT_EQUAL_INT(3, length(list));
	list[2].cdr = &list[3];
	list[3].cdr = NULL;
	TEST_ASSERT_EQUAL_INT(4, length(list));
	TEST_ASSERT_EQUAL_INT(3, length(list + 1));
	TEST_ASSERT_EQUAL_INT(2, length(list + 2));
	TEST_ASSERT_EQUAL_INT(1, length(list + 3));
}

void test_equals_strings(void) {
	S* s1 = malloc(3*sizeof(C));
	S* s2 = malloc(3*sizeof(C));

	s1->str[0] = s2->str[0] = 0;
	s1->str[1] = s2->str[1] = 0;
	s1->str[2] = s2->str[2] = 0;
	s1->str[3] = s2->str[3] = 0;
	s1->str[4] = s2->str[4] = 0;
	s1->str[5] = s2->str[5] = 0;

	s1->len = 0;
	s2->len = 0;
	TEST_ASSERT_EQUAL_INT(1, equals(s1, s2));

	s1->str[0] = 'h';
	s1->str[1] = 'e';
	s1->str[2] = 'l';
	s1->str[3] = 'l';
	s1->str[4] = 'o';
	s1->str[5] = 0;

	s2->str[0] = 'c';
	s2->str[1] = 'a';
	s2->str[2] = 't';
	s2->str[3] = 0;
	s2->str[4] = 0;
	s2->str[5] = 0;

	s1->len = 5;
	s2->len = 3;
	TEST_ASSERT_EQUAL_INT(0, equals(s1, s2));

	s2->str[0] = 'h';
	s2->str[1] = 'e';
	s2->str[2] = 'l';
	s2->str[3] = 'l';
	s2->str[4] = 'o';
	s2->str[5] = 0;

	s2->len = 5;
	TEST_ASSERT_EQUAL_INT(1, equals(s1, s2));

	s1->str[5] = ' ';
	s1->str[6] = '1';
	s1->str[7] = 0;

	s2->str[5] = ' ';
	s2->str[6] = '2';
	s2->str[7] = 0;

	s1->len = 7;
	s2->len = 7;
	TEST_ASSERT_EQUAL_INT(0, equals(s1, s2));
}

void test_block_initialization(void) {
	int size = 32;
	C b[size];
	TEST_ASSERT_NOT_NULL(init(b, size));

	H* h = (H*)b;
	TEST_ASSERT_EQUAL_INT(size, h->size);
	TEST_ASSERT_EQUAL_INT(0, h->err);
	TEST_ASSERT_EQUAL_INT(&(h->data[0]), h->ip);
	TEST_ASSERT_NULL(h->sp);
	TEST_ASSERT_NULL(h->rp);
	TEST_ASSERT_NULL(h->dp);
	TEST_ASSERT_EQUAL_PTR(&(h->data[0]), h->here);
	TEST_ASSERT_EQUAL_PTR(&(h->data[0]), h->ftail);
	TEST_ASSERT_EQUAL_PTR(b + size - 2, h->lowest);
	TEST_ASSERT_EQUAL_PTR(b + size - 2, h->fhead);
	
	TEST_ASSERT_EQUAL_INT((b + size - (C*)(&(h->data[0]))) / 2, length(h->fhead));
}

void test_allot(void) {
	int size = 32;
	C b[size];
	H* bl = init(b, size);
	TEST_ASSERT_NOT_NULL(bl);
	int fpairs = (b + size - (C*)(&(bl->data[0]))) / 2;
	TEST_ASSERT_EQUAL_INT(fpairs, length(bl->fhead));
	
	TEST_ASSERT_EQUAL_INT(0, FREE(bl));

	TEST_ASSERT_EQUAL_INT(0, allot(bl, 2));
	TEST_ASSERT_EQUAL_INT(fpairs - 1, length(bl->fhead));
	TEST_ASSERT_EQUAL_INT(2, bl->here - &(bl->data[0]));
	TEST_ASSERT_EQUAL_INT(14, FREE(bl));

	TEST_ASSERT_EQUAL_INT(0, allot(bl, 33));
	TEST_ASSERT_EQUAL_INT(fpairs - 3, length(bl->fhead));
	TEST_ASSERT_EQUAL_INT(35, bl->here - &(bl->data[0]));
	TEST_ASSERT_EQUAL_INT(13 , FREE(bl));

	TEST_ASSERT_EQUAL_INT(-1, allot(bl, 512));
	// TODO: Correctly test extrem cases
	//TEST_ASSERT_EQUAL_INT(fpairs - 3, length(bl->fhead));
	//TEST_ASSERT_EQUAL_INT(35, bl->here - &(bl->data[0]));
	//TEST_ASSERT_EQUAL_INT(125, FREE(bl));
}

void test_cons_and_reclaim(void) {
	int size = 32;
	C b[size];
	H* bl = init(b, size);
	TEST_ASSERT_NOT_NULL(bl);
	int fpairs = (b + size - (C*)(&(bl->data[0]))) / 2;

	TEST_ASSERT_EQUAL_INT(fpairs, length(bl->fhead));

	P *i1 = cons(bl, 0, 0);
	TEST_ASSERT_EQUAL_INT(fpairs - 1, length(bl->fhead));

	P *i2 = cons(bl, 0, 0);
	TEST_ASSERT_EQUAL_INT(fpairs - 2, length(bl->fhead));

	reclaim(bl, i1);
	TEST_ASSERT_EQUAL_INT(fpairs - 1, length(bl->fhead));

	reclaim(bl, i2);
	TEST_ASSERT_EQUAL_INT(fpairs, length(bl->fhead));

	TEST_ASSERT_EQUAL(i1, i2->cdr);
	TEST_ASSERT_EQUAL(i2, bl->fhead);
}

void test_stack(void) {
	C b[32];
	H* bl = init(b, 32);
	TEST_ASSERT_EQUAL_INT(0, length(bl->sp));
	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
	push(bl, 7);
	TEST_ASSERT_EQUAL_INT(1, length(bl->sp));
	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
	push(bl, 11);
	TEST_ASSERT_EQUAL_INT(2, length(bl->sp));
	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
	TEST_ASSERT_EQUAL_INT(11, pop(bl));
	TEST_ASSERT_EQUAL_INT(1, length(bl->sp));
	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
	TEST_ASSERT_EQUAL_INT(7, pop(bl));
	TEST_ASSERT_EQUAL_INT(0, length(bl->sp));
	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
}

void fib(H* bl) {
	DUP(bl);
	LIT(bl, 1);	
	GT(bl);
	if (pop(bl) != 0) {
		DEC(bl);
		DUP(bl);
		DEC(bl);
		fib(bl);
		SWAP(bl);
		fib(bl);
		ADD(bl);
	}
}

void test_fib(void) {
	C b[262000];
	H* bl = init(b, 262000);

	push(bl, 25);
	fib(bl);

	TEST_ASSERT_EQUAL_INT(75025, pop(bl));
}

void test_dictionary(void) {
	C b[32];
	H* bl = init(b, 32);
	TEST_ASSERT_EQUAL_INT(0, length(bl->dp));
	W* w = malloc(sizeof(W));
	w->cdr = bl->dp;
	bl->dp = (P*)w;
	TEST_ASSERT_EQUAL_INT(1, length(bl->dp));
	W* w2 = malloc(sizeof(W));
	w2->cdr = bl->dp;
	bl->dp = (P*)w2;
	TEST_ASSERT_EQUAL_INT(2, length(bl->dp));
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_is_aligned);
	RUN_TEST(test_cells);
	RUN_TEST(test_bytes);
	
	RUN_TEST(test_list_length);

	RUN_TEST(test_equals_strings);

	RUN_TEST(test_block_initialization);
	RUN_TEST(test_allot);

	RUN_TEST(test_cons_and_reclaim);
	RUN_TEST(test_stack);
	RUN_TEST(test_fib);
	RUN_TEST(test_dictionary);

	return UNITY_END();
}
