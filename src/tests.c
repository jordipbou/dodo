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
	C b[32];
	init_bl(b, 32);
	C l = (32 - HEADER_SIZE / 2) / 2 - (32 - HEADER_SIZE / 2) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((C *)FREE_HEAD(b)));
	TEST_ASSERT(b + SIZE(b) > (C *)FREE_HEAD(b));
	TEST_ASSERT(FREE_HEAD(b) >= LOWEST_ASSIGNED(b));
	TEST_ASSERT(LOWEST_ASSIGNED(b) > FREE_TAIL(b));
	TEST_ASSERT(FREE_TAIL(b) >= HERE(b));
	TEST_ASSERT((C *)HERE(b) >= b + HEADER_SIZE);
}

void test_cons_and_reclaim(void) {
	C b[32];
	init_bl(b, 32);
	C l = (32 - HEADER_SIZE / 2) / 2 - (32 - HEADER_SIZE / 2) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((C *)FREE_HEAD(b)));
	C *i1 = cons(b, 0, 0);
	TEST_ASSERT_EQUAL_INT(l - 1, length((C *)FREE_HEAD(b)));
	C *i2 = cons(b, 0, 0);
	TEST_ASSERT_EQUAL_INT(l - 2, length((C *)FREE_HEAD(b)));
	reclaim(b, i1);
	TEST_ASSERT_EQUAL_INT(l - 1, length((C *)FREE_HEAD(b)));
	reclaim(b, i2);
	TEST_ASSERT_EQUAL_INT(l, length((C *)FREE_HEAD(b)));

	TEST_ASSERT_EQUAL(i1, (C *)CDR(i2));
	TEST_ASSERT_EQUAL(i2, (C *)FREE_HEAD(b));
}

void test_stack(void) {
	C bl[32];
	init_bl(bl, 32);

	S *s = malloc(sizeof(S));
	s->ds = s->rs = NULL;
	TEST_ASSERT_EQUAL_INT(0, length(s->ds));
	TEST_ASSERT_EQUAL_INT(0, length(s->rs));
	push(bl, s, 7);
	TEST_ASSERT_EQUAL_INT(1, length(s->ds));
	TEST_ASSERT_EQUAL_INT(0, length(s->rs));
	push(bl, s, 11);
	TEST_ASSERT_EQUAL_INT(2, length(s->ds));
	TEST_ASSERT_EQUAL_INT(0, length(s->rs));
	TEST_ASSERT_EQUAL_INT(11, pop(bl, s));
	TEST_ASSERT_EQUAL_INT(1, length(s->ds));
	TEST_ASSERT_EQUAL_INT(0, length(s->rs));
	TEST_ASSERT_EQUAL_INT(7, pop(bl, s));
	TEST_ASSERT_EQUAL_INT(0, length(s->ds));
	TEST_ASSERT_EQUAL_INT(0, length(s->rs));
}

void fib(X *c) {
	DUP(c);
	LIT(c, 1);	
	GT(c);
	if (pop(c->bl, c->sc) != 0) {
		DEC(c);
		DUP(c);
		DEC(c);
		fib(c);
		SWAP(c);
		fib(c);
		ADD(c);
	}
}

void test_fib(void) {
	C b[262000];
	X *c = malloc(sizeof(X));
	S *s = malloc(sizeof(S));
	c->bl = init_bl(b, 262000);
	c->sc = s;

	push(c->bl, c->sc, 25);
	fib(c);

	TEST_ASSERT_EQUAL_INT(75025, pop(c->bl, c->sc));
}

void test_reserve(void) {
	C b[32];
	init_bl(b, 32);
	C l = (32 - HEADER_SIZE / 2) / 2 - (32 - HEADER_SIZE / 2) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((C *)FREE_HEAD(b)));
	C res = reserve(b, 2);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(l - 2, length((C *)FREE_HEAD(b)));
}

void test_allot(void) {
	C b[32];
	init_bl(b, 32);
	C l = (32 - HEADER_SIZE / 2) / 2 - (32 - HEADER_SIZE / 2) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((C *)FREE_HEAD(b)));
	C h = HERE(b);
	C res = allot(b, 10);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(l - 1, length((C *)FREE_HEAD(b)));
	TEST_ASSERT_EQUAL_INT(h + 10, HERE(b));
	res = allot(b, 2*sizeof(C) + 2);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(l - 2, length((C *)FREE_HEAD(b)));
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
