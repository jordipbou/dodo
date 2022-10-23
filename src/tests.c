#include <stdlib.h>

#include "unity.h"
#include "vm.h"

void setUp(void) {}

void tearDown(void) {}

void test_list_length(void) {
	cell_t list[8];
	list[0] = (cell_t)&list[2];
	list[2] = (cell_t)&list[4];
	list[4] = (cell_t)&list[6];
	list[6] = (cell_t)NULL;
	TEST_ASSERT_EQUAL_INT(4, length(list));
}

void test_block_initialization(void) {
	cell_t b[32];
	init_block(b, 32);
	cell_t l = (32 - HEADER_SIZE / 2) / 2 - (32 - HEADER_SIZE / 2) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((cell_t *)FREE_HEAD(b)));
	TEST_ASSERT(b + SIZE(b) > (cell_t *)FREE_HEAD(b));
	TEST_ASSERT(FREE_HEAD(b) >= LOWEST_ASSIGNED(b));
	TEST_ASSERT(LOWEST_ASSIGNED(b) > FREE_TAIL(b));
	TEST_ASSERT(FREE_TAIL(b) >= HERE(b));
	TEST_ASSERT((cell_t *)HERE(b) >= b + HEADER_SIZE);
}

void test_cons_and_reclaim(void) {
	cell_t b[32];
	init_block(b, 32);
	cell_t l = (32 - HEADER_SIZE / 2) / 2 - (32 - HEADER_SIZE / 2) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((cell_t *)FREE_HEAD(b)));
	cell_t *i1 = cons(b, 0, 0);
	TEST_ASSERT_EQUAL_INT(l - 1, length((cell_t *)FREE_HEAD(b)));
	cell_t *i2 = cons(b, 0, 0);
	TEST_ASSERT_EQUAL_INT(l - 2, length((cell_t *)FREE_HEAD(b)));
	reclaim(b, i1);
	TEST_ASSERT_EQUAL_INT(l - 1, length((cell_t *)FREE_HEAD(b)));
	reclaim(b, i2);
	TEST_ASSERT_EQUAL_INT(l, length((cell_t *)FREE_HEAD(b)));

	TEST_ASSERT_EQUAL(i1, (cell_t *)CDR(i2));
	TEST_ASSERT_EQUAL(i2, (cell_t *)FREE_HEAD(b));
}

void test_stack(void) {
	cell_t b[32];
	init_block(b, 32);

	scp_t *s = malloc(sizeof(scp_t));
	s->block = b;
	s->dstack = s->rstack = NULL;
	TEST_ASSERT_EQUAL_INT(0, s->ddepth);
	TEST_ASSERT_EQUAL_INT(0, s->rdepth);
	push(s, 7);
	TEST_ASSERT_EQUAL_INT(1, s->ddepth);
	TEST_ASSERT_EQUAL_INT(0, s->rdepth);
	push(s, 11);
	TEST_ASSERT_EQUAL_INT(2, s->ddepth);
	TEST_ASSERT_EQUAL_INT(0, s->rdepth);
	TEST_ASSERT_EQUAL_INT(11, pop(s));
	TEST_ASSERT_EQUAL_INT(1, s->ddepth);
	TEST_ASSERT_EQUAL_INT(0, s->rdepth);
	TEST_ASSERT_EQUAL_INT(7, pop(s));
	TEST_ASSERT_EQUAL_INT(0, s->ddepth);
	TEST_ASSERT_EQUAL_INT(0, s->rdepth);
}

void fib(ctx_t *c) {
	DUP(c);
	LIT(c, 1);	
	GT(c);
	if (c->T != 0) {
		c->T = c->S; c->S = pop(c->scope);
		DEC(c);
		DUP(c);
		DEC(c);
		fib(c);
		SWAP(c);
		fib(c);
		ADD(c);
	} else {
		c->T = c->S; c->S = pop(c->scope);
	}
}

void test_fib(void) {
	cell_t b[262000];
	ctx_t *c = malloc(sizeof(ctx_t));
	scp_t *s = malloc(sizeof(scp_t));
	s->block = init_block(b, 262000);
	c->scope = s;
	c->T = c->S = 0;

	c->T = 25;
	fib(c);

	TEST_ASSERT_EQUAL_INT(75025, c->T);
}

void test_reserve(void) {
	cell_t b[32];
	init_block(b, 32);
	cell_t l = (32 - HEADER_SIZE / 2) / 2 - (32 - HEADER_SIZE / 2) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((cell_t *)FREE_HEAD(b)));
	cell_t res = reserve(b, 2);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(l - 2, length((cell_t *)FREE_HEAD(b)));
}

void test_allot(void) {
	cell_t b[32];
	init_block(b, 32);
	cell_t l = (32 - HEADER_SIZE / 2) / 2 - (32 - HEADER_SIZE / 2) % 2;
	TEST_ASSERT_EQUAL_INT(l, length((cell_t *)FREE_HEAD(b)));
	cell_t h = HERE(b);
	cell_t res = allot(b, 10);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(l - 1, length((cell_t *)FREE_HEAD(b)));
	TEST_ASSERT_EQUAL_INT(h + 10, HERE(b));
	res = allot(b, 2*sizeof(cell_t) + 2);
	TEST_ASSERT_EQUAL_INT(0, res);
	TEST_ASSERT_EQUAL_INT(l - 2, length((cell_t *)FREE_HEAD(b)));
	TEST_ASSERT_EQUAL_INT(h + 10 + 2*sizeof(cell_t) + 2, HERE(b));
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
