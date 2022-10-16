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
	TEST_ASSERT_EQUAL_INT(14, length((cell_t *)FREE_HEAD(b)));
}

void test_take_and_put(void) {
	cell_t b[32];
	init_block(b, 32);
	TEST_ASSERT_EQUAL_INT(14, length((cell_t *)FREE_HEAD(b)));
	cell_t *i1 = take(b);
	TEST_ASSERT_EQUAL_INT(13, length((cell_t *)FREE_HEAD(b)));
	cell_t *i2 = take(b);
	TEST_ASSERT_EQUAL_INT(12, length((cell_t *)FREE_HEAD(b)));
	put(b, i1);
	TEST_ASSERT_EQUAL_INT(13, length((cell_t *)FREE_HEAD(b)));
	put(b, i2);
	TEST_ASSERT_EQUAL_INT(14, length((cell_t *)FREE_HEAD(b)));

	TEST_ASSERT_EQUAL(i1, (cell_t *)CDR(i2));
	TEST_ASSERT_EQUAL(i2, (cell_t *)FREE_HEAD(b));
}

void test_stack(void) {
	cell_t b[32];
	init_block(b, 32);

	scp_t *s = malloc(sizeof(scp_t));
	s->block = b;
	s->dstack = s->rstack = NULL;
	push(s, 7);
	push(s, 11);
	TEST_ASSERT_EQUAL_INT(11, pop(s));
	TEST_ASSERT_EQUAL_INT(7, pop(s));
}

void fib(ctx_t *c) {
	DUP(c);
	LIT1(c);	
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

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_list_length);
	RUN_TEST(test_block_initialization);
	RUN_TEST(test_take_and_put);
	RUN_TEST(test_stack);
	RUN_TEST(test_fib);
	return UNITY_END();
}
