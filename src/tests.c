#include <stdlib.h>

#include "unity.h"
#include "vm.h"

void setUp(void) {}

void tearDown(void) {}

void test_list_length(void) {
	C list[8];
	list[0] = (C)&list[2];
	list[2] = (C)&list[4];
	list[4] = (C)&list[6];
	list[6] = (C)NULL;
	TEST_ASSERT_EQUAL_INT(4, length(list));
}

void test_block_initialization(void) {
	int size = 32;
	C b[size];
	init_bl(b, size);
	C l = (size - Hsz) / 2 - (size - Hsz) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((C *)FH(b)));
	TEST_ASSERT(b + SZ(b) > (C *)FH(b));
	TEST_ASSERT(FH(b) >= LA(b));
	TEST_ASSERT(LA(b) > FT(b));
	TEST_ASSERT(FT(b) >= HERE(b));
	TEST_ASSERT((C *)HERE(b) >= b + Hsz);
}

void test_cons_and_reclaim(void) {
	int size = 32;
	C b[size];
	init_bl(b, size);
	C l = (size - Hsz) / 2 - (size - Hsz) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((C *)FH(b)));
	C *i1 = cons(b, 0, 0);
	TEST_ASSERT_EQUAL_INT(l - 1, length((C *)FH(b)));
	C *i2 = cons(b, 0, 0);
	TEST_ASSERT_EQUAL_INT(l - 2, length((C *)FH(b)));
	reclaim(b, i1);
	TEST_ASSERT_EQUAL_INT(l - 1, length((C *)FH(b)));
	reclaim(b, i2);
	TEST_ASSERT_EQUAL_INT(l, length((C *)FH(b)));

	TEST_ASSERT_EQUAL(i1, (C *)CDR(i2));
	TEST_ASSERT_EQUAL(i2, (C *)FH(b));
}

void test_stack(void) {
	C bl[32];
	init_bl(bl, 32);
	TEST_ASSERT_EQUAL_INT(0, length((C*)(DS(bl))));
	TEST_ASSERT_EQUAL_INT(0, length((C*)(RS(bl))));
	push(bl, 7);
	TEST_ASSERT_EQUAL_INT(1, length((C*)(DS(bl))));
	TEST_ASSERT_EQUAL_INT(0, length((C*)(RS(bl))));
	push(bl, 11);
	TEST_ASSERT_EQUAL_INT(2, length((C*)(DS(bl))));
	TEST_ASSERT_EQUAL_INT(0, length((C*)(RS(bl))));
	TEST_ASSERT_EQUAL_INT(11, pop(bl));
	TEST_ASSERT_EQUAL_INT(1, length((C*)(DS(bl))));
	TEST_ASSERT_EQUAL_INT(0, length((C*)(RS(bl))));
	TEST_ASSERT_EQUAL_INT(7, pop(bl));
	TEST_ASSERT_EQUAL_INT(0, length((C*)(DS(bl))));
	TEST_ASSERT_EQUAL_INT(0, length((C*)(RS(bl))));
}

void fib(C* bl) {
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
	C bl[262000];
	init_bl(bl, 262000);

	push(bl, 25);
	fib(bl);

	TEST_ASSERT_EQUAL_INT(75025, pop(bl));
}

void test_reserve(void) {
	int size = 32;
	C b[size];
	init_bl(b, size);
	C l = (size - Hsz) / 2 - (size - Hsz) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((C *)FH(b)));
	C res = reserve(b, 2);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(l - 2, length((C *)FH(b)));
}

void test_allot(void) {
	int size = 32;
	C b[size];
	init_bl(b, size);
	C l = (size - Hsz) / 2 - (size - Hsz) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((C *)FH(b)));
	C h = HERE(b);
	C res = allot(b, 10);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(l - 1, length((C *)FH(b)));
	TEST_ASSERT_EQUAL_INT(h + 10, HERE(b));
	res = allot(b, 2*sizeof(C) + 2);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(l - 2, length((C *)FH(b)));
	TEST_ASSERT_EQUAL_INT(h + 10 + 2*sizeof(C) + 2, HERE(b));
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
