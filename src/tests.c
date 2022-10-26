#include <stdlib.h>

#include "unity.h"
#include "vm.h"

void setUp(void) {}

void tearDown(void) {}

void test_list_length(void) {
	P list[4];
	list[0].cdr = &list[1];
	list[1].cdr = &list[2];
	list[2].cdr = &list[3];
	list[3].cdr = NULL;
	TEST_ASSERT_EQUAL_INT(4, length(list));
}

void test_block_initialization(void) {
	int size = 32;
	C b[size];
	TEST_ASSERT_NOT_NULL(init_bl(b, size));

	H* h = (H*)b;
	TEST_ASSERT_EQUAL_INT(size, h->size);
	TEST_ASSERT_EQUAL_INT(0, h->err);
	TEST_ASSERT_EQUAL_INT(b + szHEADER, h->ip);
	TEST_ASSERT_NULL(h->sp);
	TEST_ASSERT_NULL(h->rp);
	TEST_ASSERT_NULL(h->dp);
	TEST_ASSERT_EQUAL_PTR(b + szHEADER, h->here);
	TEST_ASSERT_EQUAL_PTR(b + szHEADER, h->ftail);
	TEST_ASSERT_EQUAL_PTR(b + size - szPAIR, h->lowest);
	TEST_ASSERT_EQUAL_PTR(b + size - szPAIR, h->fhead);
	
	TEST_ASSERT_EQUAL_INT((size - szHEADER) / 2, length(h->fhead));
}

void test_cons_and_reclaim(void) {
	int size = 32;
	C b[size];
	H* bl = init_bl(b, size);
	TEST_ASSERT_NOT_NULL(bl);
	int fpairs = (size - szHEADER) / 2;

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
	H* bl = init_bl(b, 32);
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
	H* bl = init_bl(b, 262000);

	push(bl, 25);
	fib(bl);

	TEST_ASSERT_EQUAL_INT(75025, pop(bl));
}

void test_reserve(void) {
	int size = 32;
	C b[size];
	H* bl = init_bl(b, size);
	int fpairs = (size - szHEADER) / 2;
	TEST_ASSERT_EQUAL_INT(fpairs, length(bl->fhead));
	C res = reserve(bl, 2);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(fpairs - 2, length(bl->fhead));
}

void test_allot(void) {
	int size = 32;
	C b[size];
	H* bl = init_bl(b, size);
	int fpairs = (size - szHEADER) / 2;
	TEST_ASSERT_EQUAL_INT(fpairs, length(bl->fhead));
	B* h = bl->here;
	C res = allot(bl, 10);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(fpairs - 1, length(bl->fhead));
	TEST_ASSERT_EQUAL_PTR(h + 10, bl->here);
	res = allot(bl, 2*sizeof(C) + 2);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(fpairs - 2, length(bl->fhead));
	TEST_ASSERT_EQUAL_PTR(h + 10 + 2*sizeof(C) + 2, bl->here);
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_list_length);
	RUN_TEST(test_block_initialization);
	RUN_TEST(test_cons_and_reclaim);
	RUN_TEST(test_stack);
	RUN_TEST(test_fib);
	RUN_TEST(test_reserve);
	RUN_TEST(test_allot);
	return UNITY_END();
}
