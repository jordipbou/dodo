#include <stdlib.h>

#include "unity.h"
#include "vm.h"

void setUp(void) {}

void tearDown(void) {}

void test_is_aligned(void) {
	TEST_ASSERT_TRUE(IS_ALIGNED(12345 << 3));
	TEST_ASSERT_FALSE(IS_ALIGNED(12345));
}

void test_as_cells(void) {
	TEST_ASSERT_EQUAL_INT(0, AS_CELLS(0));
	TEST_ASSERT_EQUAL_INT(1, AS_CELLS(1));
	TEST_ASSERT_EQUAL_INT(1, AS_CELLS(8));
	TEST_ASSERT_EQUAL_INT(2, AS_CELLS(9));
	TEST_ASSERT_EQUAL_INT(2, AS_CELLS(16));
	TEST_ASSERT_EQUAL_INT(3, AS_CELLS(17));
	TEST_ASSERT_EQUAL_INT(3, AS_CELLS(24));
	TEST_ASSERT_EQUAL_INT(4, AS_CELLS(25));
}

void test_as_bytes(void) {
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

void test_find_word(void) {
	S* s1 = malloc(sizeof(S) + 8);
	S* s2 = malloc(sizeof(S) + 8);
	S* s3 = malloc(sizeof(S) + 8);
	S* s4 = malloc(sizeof(S) + 8);

	s1->len = 3;
	s1->str[0] = 'd'; s1->str[1] = 'u'; s1->str[2] = 'p';

	s2->len = 4;
	s2->str[0] = 's'; s2->str[1] = 'w'; s2->str[2] = 'a'; s2->str[3] = 'p';

	s3->len = 1;
	s3->str[0] = '+';

	s4->len = 2;
	s4->str[0] = '>'; s4->str[1] = 'r'; s4->str[2] = 0;

	W* w1 = malloc(sizeof(W));
	W* w2 = malloc(sizeof(W));
	W* w3 = malloc(sizeof(W));
	W* w4 = malloc(sizeof(W));

	w4->cdr = w3;
	w3->cdr = w2;
	w2->cdr = w1;

	w1->name = s1;
	w2->name = s2;
	w3->name = s3;
	w4->name = s4;

	TEST_ASSERT_EQUAL_PTR(w4, find(w4, ">r"));
	TEST_ASSERT_EQUAL_PTR(w3, find(w4, "+"));
	TEST_ASSERT_EQUAL_PTR(w2, find(w4, "swap"));
	TEST_ASSERT_EQUAL_PTR(w1, find(w4, "dup"));
	TEST_ASSERT_NULL(find(w3, ">r"));
}

void test_block_initialization(void) {
	C data_size = 1024;
	C code_size = 1024;

	H* h = init(data_size, code_size);
	TEST_ASSERT_NOT_NULL(h);

	TEST_ASSERT_GREATER_OR_EQUAL_INT(data_size, h->dsize);
	TEST_ASSERT_GREATER_OR_EQUAL_INT(code_size, h->csize);
	TEST_ASSERT_EQUAL_INT(0, h->err);
	TEST_ASSERT_EQUAL_INT(&(h->data[0]), h->ip);
	TEST_ASSERT_NULL(h->sp);
	TEST_ASSERT_NULL(h->rp);
	TEST_ASSERT_NULL(h->dp);
	TEST_ASSERT_EQUAL_PTR(&(h->data[0]), h->here);
	TEST_ASSERT_EQUAL_PTR(&(h->data[0]), h->ftail);
	TEST_ASSERT_EQUAL_PTR(((C*)h) + h->dsize - 2, h->lowest);
	TEST_ASSERT_EQUAL_PTR(((C*)h) + h->dsize - 2, h->fhead);
	
	TEST_ASSERT_EQUAL_INT(
		(((C*)h) + h->dsize - (C*)(&(h->data[0]))) / 2, 
		length(h->fhead));

	deinit(h);
}

void test_allot(void) {
	int data_size = 1024, code_size = 1024;;
	H* bl = init(data_size, code_size);
	TEST_ASSERT_NOT_NULL(bl);
	int fpairs = (((C*)bl) + bl->dsize - (C*)(&(bl->data[0]))) / 2;
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

	// TODO: This one should fail!!
	//TEST_ASSERT_EQUAL_INT(-1, allot(bl, bl->dsize));
	TEST_ASSERT_EQUAL_INT(-1, allot(bl, 65536));
	// TODO: Correctly test extrem cases
	//TEST_ASSERT_EQUAL_INT(fpairs - 3, length(bl->fhead));
	//TEST_ASSERT_EQUAL_INT(35, bl->here - &(bl->data[0]));
	//TEST_ASSERT_EQUAL_INT(125, FREE(bl));

	deinit(bl);
}

void test_cons_and_reclaim(void) {
	int data_size = 1024, code_size = 1024;
	H* bl = init(data_size, code_size);
	TEST_ASSERT_NOT_NULL(bl);
	int fpairs = (((C*)bl) + bl->dsize - (C*)(&(bl->data[0]))) / 2;

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
	
	deinit(bl);
}

void test_stack(void) {
	H* bl = init(1024, 1024);
	TEST_ASSERT_NOT_NULL(bl);

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

	deinit(bl);
}

//void fib(H* bl) {
//	DUP(bl);
//	LIT(bl, 1);	
//	GT(bl);
//	if (pop(bl) != 0) {
//		DEC(bl);
//		DUP(bl);
//		DEC(bl);
//		fib(bl);
//		SWAP(bl);
//		fib(bl);
//		ADD(bl);
//	}
//}
//
//void test_fib(void) {
//	C b[262000];
//	H* bl = init(b, 262000);
//
//	push(bl, 25);
//	fib(bl);
//
//	TEST_ASSERT_EQUAL_INT(75025, pop(bl));
//}

void test_compile_str(void) {
	H* bl = init(1024, 1024);
	TEST_ASSERT_NOT_NULL(bl);

	C h = bl->here;
	S* s = (S*)h;
	unsigned char str[] = "Testing compile string!";
	compile_str(bl, str);
	TEST_ASSERT_EQUAL_INT(s->len, strlen(str));
	TEST_ASSERT_EQUAL_INT(0, strcmp(s->str, str));
	TEST_ASSERT_EQUAL_INT(8 + AS_CELLS(strlen(str)), bl->here - h);

	deinit(bl);
}

void test_compile_code(void) {
	H* bl = init(1024, 1024);
	TEST_ASSERT_NOT_NULL(bl);

	D* d = compile_code(bl, "test", 4);
	TEST_ASSERT_EQUAL_PTR(bl->code, d->code);

	TEST_ASSERT_EQUAL_INT8('t', d->code[0]);
	TEST_ASSERT_EQUAL_INT8('e', d->code[1]);
	TEST_ASSERT_EQUAL_INT8('s', d->code[2]);
	TEST_ASSERT_EQUAL_INT8('t', d->code[3]);

	D* d2 = compile_code(bl, "demo", 4);
	TEST_ASSERT_EQUAL_PTR(bl->code + 4, d2->code);

	TEST_ASSERT_EQUAL_INT8('d', d2->code[0]);
	TEST_ASSERT_EQUAL_INT8('e', d2->code[1]);
	TEST_ASSERT_EQUAL_INT8('m', d2->code[2]);
	TEST_ASSERT_EQUAL_INT8('o', d2->code[3]);

	// Test executing code: 
	// mov rax,13
	// ret
	D* d3 = compile_code(bl, "\x48\xC7\xC0\x0D\x00\x00\x00\xC3", 8);

	int res = ((int (*)(void))(d3->code))();
	TEST_ASSERT_EQUAL_INT(13, res);

	deinit(bl);
}

//void test_dictionary(void) {
//	C b[32];
//	H* bl = init(b, 32);
//	TEST_ASSERT_EQUAL_INT(0, length(bl->dp));
//	W* w = malloc(sizeof(W));
//	w->cdr = bl->dp;
//	bl->dp = (P*)w;
//	TEST_ASSERT_EQUAL_INT(1, length(bl->dp));
//	W* w2 = malloc(sizeof(W));
//	w2->cdr = bl->dp;
//	bl->dp = (P*)w2;
//	TEST_ASSERT_EQUAL_INT(2, length(bl->dp));
//}

void test_error_code(void) {
	H* bl = init(1024, 1024);

	// mov QWORD PTR [rdi], 11
	// ret
	D* d = compile_code(bl, "\x48\xC7\x07\x0B\x00\x00\x00\xC3", 8);

	((void (*)(H*))(d->code))(bl);

	TEST_ASSERT_EQUAL_INT(11, bl->err);

	deinit(bl);
}

void test_append_ripret(void) {
	H* bl = init(1024, 1024);

	D* d = compile_code(bl, "", 0);
	append_ripret(bl, d);

	C rip = ((C (*)())(d->code))();

	TEST_ASSERT_EQUAL_PTR(bl->code + 11, rip);

	deinit(bl);
}

void test_asm_c_asm(void) {
	H* bl = init(1024, 1024);

	D* d = compile_code(bl, 

	// mov QWORD PTR [rdi], 11
	// ret
	D* d = compile_code(bl, "\x48\xC7\x07\x0B\x00\x00\x00", 7);
	append_ripret(bl, d);

	C res = ((C (*)(H*))(d->code))(bl);

	TEST_ASSERT_EQUAL_INT(11, bl->err);



	// Test executing code: 
	// mov rax,13
	// ret
	D* d3 = compile_code(bl, "\x48\xC7\xC0\x0D\x00\x00\x00\xC3", 8);

	C res = ((C (*)(void))(d3->code))();
	TEST_ASSERT_EQUAL_INT(13, res);
	
	

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_is_aligned);
	RUN_TEST(test_as_cells);
	RUN_TEST(test_as_bytes);
	
	RUN_TEST(test_list_length);

	RUN_TEST(test_find_word);

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_allot);
	RUN_TEST(test_cons_and_reclaim);
	RUN_TEST(test_stack);

	//RUN_TEST(test_fib);
	//RUN_TEST(test_dictionary);

	RUN_TEST(test_compile_str);
	RUN_TEST(test_compile_code);

	RUN_TEST(test_error_code);
	RUN_TEST(test_append_ripret);

	return UNITY_END();
}
