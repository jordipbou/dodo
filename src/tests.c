#include <stdlib.h>

#include "unity.h"
#include "vm.h"

void setUp(void) {}

void tearDown(void) {}

void test_block_initialization(void) {
	CELL data_size = 1024;
	CELL code_size = 1024;

	CTX* ctx = init(data_size, code_size);
	TEST_ASSERT_NOT_NULL(ctx);

	TEST_ASSERT_GREATER_OR_EQUAL_INT(data_size, ctx->dsize);
	TEST_ASSERT_GREATER_OR_EQUAL_INT(code_size, ctx->csize);

	TEST_ASSERT_EQUAL_INT(0, ctx->Fx);
	TEST_ASSERT_EQUAL_INT(0, ctx->Lx);

	TEST_ASSERT_NOT_NULL(ctx->code);
	
	deinit(ctx);
}

void test_compile_byte(void) {
	CTX* ctx = init(1024, 1024);

	TEST_ASSERT_EQUAL_INT(0, ctx->chere);

	compile_byte(ctx, 0xC3);
	TEST_ASSERT_EQUAL_INT(1, ctx->chere);

	TEST_ASSERT_EQUAL_INT8(0xC3, *(ctx->code));

	compile_byte(ctx, 0xB8);
	TEST_ASSERT_EQUAL_INT(2, ctx->chere);

	TEST_ASSERT_EQUAL_INT8(0xC3, *(ctx->code));
	TEST_ASSERT_EQUAL_INT8(0xB8, *(ctx->code + 1));

	deinit(ctx);
}

void test_compile_bytes(void) {
	CTX* ctx = init(1024, 1024);

	TEST_ASSERT_EQUAL_INT(0, ctx->chere);

	compile_bytes(ctx, "\xB8\x11\xFF\x00\xFF", 5);
	TEST_ASSERT_EQUAL_INT(5, ctx->chere);

	TEST_ASSERT_EQUAL_INT8(0xB8, *(ctx->code));
	TEST_ASSERT_EQUAL_INT8(0x11, *(ctx->code + 1));
	TEST_ASSERT_EQUAL_INT8(0xFF, *(ctx->code + 2));
	TEST_ASSERT_EQUAL_INT8(0x00, *(ctx->code + 3));
	TEST_ASSERT_EQUAL_INT8(0xFF, *(ctx->code + 4));

	compile_bytes(ctx, "\xC7\x47", 2);
	TEST_ASSERT_EQUAL_INT(7, ctx->chere);

	TEST_ASSERT_EQUAL_INT8(0xC7, *(ctx->code + 5));
	TEST_ASSERT_EQUAL_INT8(0x47, *(ctx->code + 6));

	deinit(ctx);
}

void test_compile_literal(void) {
	CTX* ctx = init(1024, 1024);

	TEST_ASSERT_EQUAL_INT(0, ctx->chere);

	compile_cell(ctx, 41378);
	TEST_ASSERT_EQUAL_INT(sizeof(CELL), ctx->chere);

	TEST_ASSERT_EQUAL_INT64(41378, *((CELL*)(ctx->code)));

	compile_cell(ctx, 13);
	TEST_ASSERT_EQUAL_INT(2*sizeof(CELL), ctx->chere);

	TEST_ASSERT_EQUAL_INT64(13, *((CELL*)(ctx->code + sizeof(CELL))));

	compile_halfcell(ctx, 19923);
	TEST_ASSERT_EQUAL_INT(2*sizeof(CELL) + sizeof(HALF), ctx->chere);

	TEST_ASSERT_EQUAL_INT32(19923, *((HALF*)(ctx->code + 2*sizeof(CELL))));

	compile_halfcell(ctx, -367);
	TEST_ASSERT_EQUAL_INT(2*sizeof(CELL) + 2*sizeof(HALF), ctx->chere);

	TEST_ASSERT_EQUAL_INT32(-367, *((HALF*)(ctx->code + 2*sizeof(CELL) + sizeof(HALF))));

	deinit(ctx);
}

void test_compile_next(void) {
	CTX* ctx = init(1024, 1024);
	
	TEST_ASSERT_EQUAL_INT(0, ctx->chere);

	compile_next(ctx);

	TEST_ASSERT_EQUAL_INT(8, ctx->chere);

	BYTE* NEXT = CALL(ctx->code, ctx);

	TEST_ASSERT_EQUAL_PTR(ctx->code + 8, NEXT);

	compile_next(ctx);

	TEST_ASSERT_EQUAL_INT(16, ctx->chere);

	NEXT = CALL(NEXT, ctx);

	TEST_ASSERT_EQUAL_PTR(ctx->code + 16, NEXT);

	deinit(ctx);
}

void generic_cfunc(CTX* ctx) {
}

void test_compile_cfunc(void) {
	CTX* ctx = init(1024, 1024);

	TEST_ASSERT_EQUAL_INT(0, ctx->chere);

	compile_cfunc(ctx, &generic_cfunc);

	TEST_ASSERT_EQUAL_INT(22, ctx->chere);

	BYTE* NEXT = CALL(ctx->code, ctx);

	TEST_ASSERT_EQUAL_PTR(&generic_cfunc, ctx->Fx);
	TEST_ASSERT_EQUAL_INT(ctx->code + 22, NEXT);

	deinit(ctx);
}

void test_compile_push(void) {
	CTX* ctx = init(1024, 1024);

	TEST_ASSERT_EQUAL_INT(0, ctx->chere);

	compile_push(ctx, &generic_cfunc, 13);

	TEST_ASSERT_EQUAL_INT(36, ctx->chere);

	BYTE* NEXT = CALL(ctx->code, ctx);

	TEST_ASSERT_EQUAL_PTR(&generic_cfunc, ctx->Fx);
	TEST_ASSERT_EQUAL_INT(13, ctx->Lx);
	TEST_ASSERT_EQUAL_INT(ctx->code + 36, NEXT);

	deinit(ctx);
}

//void test_compile_bytes(void) {
//	H* bl = init(1024, 1024);
//	TEST_ASSERT_NOT_NULL(bl);
//
//	S* s = create_code(bl);
//	TEST_ASSERT_NOT_NULL(compile_bytes(bl, s, "test", 4));
//	TEST_ASSERT_EQUAL_PTR(bl->code, s->data.code);
//
//	TEST_ASSERT_EQUAL_INT8('t', s->data.code[0]);
//	TEST_ASSERT_EQUAL_INT8('e', s->data.code[1]);
//	TEST_ASSERT_EQUAL_INT8('s', s->data.code[2]);
//	TEST_ASSERT_EQUAL_INT8('t', s->data.code[3]);
//
//	S* s2 = create_code(bl);
//	TEST_ASSERT_NOT_NULL(compile_bytes(bl, s2, "demo", 4));
//	TEST_ASSERT_EQUAL_PTR(bl->code + 4, s2->data.code);
//
//	TEST_ASSERT_EQUAL_INT8('d', s2->data.code[0]);
//	TEST_ASSERT_EQUAL_INT8('e', s2->data.code[1]);
//	TEST_ASSERT_EQUAL_INT8('m', s2->data.code[2]);
//	TEST_ASSERT_EQUAL_INT8('o', s2->data.code[3]);
//
//	// Test executing code: 
//	// mov rax,13
//	// ret
//	S* s3 = create_code(bl);
//	compile_bytes(bl, s3, "\x48\xC7\xC0\x0D\x00\x00\x00\xC3", 8);
//
//	int res = ((int (*)(void))(s3->data.code))();
//	TEST_ASSERT_EQUAL_INT(13, res);
//
//	deinit(bl);
//}
//
//void test_is_aligned(void) {
//	TEST_ASSERT_TRUE(IS_ALIGNED(12345 << 3));
//	TEST_ASSERT_FALSE(IS_ALIGNED(12345));
//}
//
//void test_as_cells(void) {
//	TEST_ASSERT_EQUAL_INT(0, AS_CELLS(0));
//	TEST_ASSERT_EQUAL_INT(1, AS_CELLS(1));
//	TEST_ASSERT_EQUAL_INT(1, AS_CELLS(8));
//	TEST_ASSERT_EQUAL_INT(2, AS_CELLS(9));
//	TEST_ASSERT_EQUAL_INT(2, AS_CELLS(16));
//	TEST_ASSERT_EQUAL_INT(3, AS_CELLS(17));
//	TEST_ASSERT_EQUAL_INT(3, AS_CELLS(24));
//	TEST_ASSERT_EQUAL_INT(4, AS_CELLS(25));
//}
//
//void test_as_bytes(void) {
//	TEST_ASSERT_EQUAL_INT(0, AS_BYTES(AS_CELLS(0)));
//	TEST_ASSERT_EQUAL_INT(8, AS_BYTES(AS_CELLS(1)));
//	TEST_ASSERT_EQUAL_INT(8, AS_BYTES(AS_CELLS(8)));
//	TEST_ASSERT_EQUAL_INT(16, AS_BYTES(AS_CELLS(9)));
//	TEST_ASSERT_EQUAL_INT(16, AS_BYTES(AS_CELLS(16)));
//	TEST_ASSERT_EQUAL_INT(24, AS_BYTES(AS_CELLS(17)));
//	TEST_ASSERT_EQUAL_INT(24, AS_BYTES(AS_CELLS(24)));
//	TEST_ASSERT_EQUAL_INT(32, AS_BYTES(AS_CELLS(25)));
//}
//
//void test_list_length(void) {
//	P list[4];
//	TEST_ASSERT_EQUAL_INT(0, length(NULL));
//	list[0].cdr = NULL;
//	TEST_ASSERT_EQUAL_INT(1, length(list));
//	list[0].cdr = &list[1];
//	list[1].cdr = NULL;
//	TEST_ASSERT_EQUAL_INT(2, length(list));
//	list[1].cdr = &list[2];
//	list[2].cdr = NULL;
//	TEST_ASSERT_EQUAL_INT(3, length(list));
//	list[2].cdr = &list[3];
//	list[3].cdr = NULL;
//	TEST_ASSERT_EQUAL_INT(4, length(list));
//	TEST_ASSERT_EQUAL_INT(3, length(list + 1));
//	TEST_ASSERT_EQUAL_INT(2, length(list + 2));
//	TEST_ASSERT_EQUAL_INT(1, length(list + 3));
//}
//
//void test_find_word(void) {
//	S* s1 = malloc(sizeof(S) + 8);
//	S* s2 = malloc(sizeof(S) + 8);
//	S* s3 = malloc(sizeof(S) + 8);
//	S* s4 = malloc(sizeof(S) + 8);
//
//	s1->len = 3;
//	s1->data.str[0] = 'd'; s1->data.str[1] = 'u'; s1->data.str[2] = 'p';
//
//	s2->len = 4;
//	s2->data.str[0] = 's'; s2->data.str[1] = 'w'; 
//	s2->data.str[2] = 'a'; s2->data.str[3] = 'p';
//
//	s3->len = 1;
//	s3->data.str[0] = '+';
//
//	s4->len = 2;
//	s4->data.str[0] = '>'; s4->data.str[1] = 'r'; s4->data.str[2] = 0;
//
//	W* w1 = malloc(sizeof(W));
//	W* w2 = malloc(sizeof(W));
//	W* w3 = malloc(sizeof(W));
//	W* w4 = malloc(sizeof(W));
//
//	w4->cdr = (P*)w3;
//	w3->cdr = (P*)w2;
//	w2->cdr = (P*)w1;
//
//	w1->name = s1;
//	w2->name = s2;
//	w3->name = s3;
//	w4->name = s4;
//
//	TEST_ASSERT_EQUAL_PTR(w4, find(w4, ">r"));
//	TEST_ASSERT_EQUAL_PTR(w3, find(w4, "+"));
//	TEST_ASSERT_EQUAL_PTR(w2, find(w4, "swap"));
//	TEST_ASSERT_EQUAL_PTR(w1, find(w4, "dup"));
//	TEST_ASSERT_NULL(find(w3, ">r"));
//}
//
//void test_allot(void) {
//	int data_size = 1024, code_size = 1024;;
//	H* bl = init(data_size, code_size);
//	TEST_ASSERT_NOT_NULL(bl);
//	int fpairs = (((C*)bl) + bl->dsize - (C*)(&(bl->data[0]))) / 2;
//	TEST_ASSERT_EQUAL_INT(fpairs, length(bl->fhead));
//	
//	TEST_ASSERT_EQUAL_INT(0, FREE(bl));
//
//	TEST_ASSERT_EQUAL_INT(0, allot(bl, 2));
//	TEST_ASSERT_EQUAL_INT(fpairs - 1, length(bl->fhead));
//	TEST_ASSERT_EQUAL_INT(2, bl->here - &(bl->data[0]));
//	TEST_ASSERT_EQUAL_INT(14, FREE(bl));
//
//	TEST_ASSERT_EQUAL_INT(0, allot(bl, 33));
//	TEST_ASSERT_EQUAL_INT(fpairs - 3, length(bl->fhead));
//	TEST_ASSERT_EQUAL_INT(35, bl->here - &(bl->data[0]));
//	TEST_ASSERT_EQUAL_INT(13 , FREE(bl));
//
//	// TODO: This one should fail!!
//	//TEST_ASSERT_EQUAL_INT(-1, allot(bl, bl->dsize));
//	TEST_ASSERT_EQUAL_INT(-1, allot(bl, 65536));
//	// TODO: Correctly test extrem cases
//	//TEST_ASSERT_EQUAL_INT(fpairs - 3, length(bl->fhead));
//	//TEST_ASSERT_EQUAL_INT(35, bl->here - &(bl->data[0]));
//	//TEST_ASSERT_EQUAL_INT(125, FREE(bl));
//
//	deinit(bl);
//}
//
//void test_cons_and_reclaim(void) {
//	int data_size = 1024, code_size = 1024;
//	H* bl = init(data_size, code_size);
//	TEST_ASSERT_NOT_NULL(bl);
//	int fpairs = (((C*)bl) + bl->dsize - (C*)(&(bl->data[0]))) / 2;
//
//	TEST_ASSERT_EQUAL_INT(fpairs, length(bl->fhead));
//
//	P *i1 = cons(bl, 0, 0);
//	TEST_ASSERT_EQUAL_INT(fpairs - 1, length(bl->fhead));
//
//	P *i2 = cons(bl, 0, 0);
//	TEST_ASSERT_EQUAL_INT(fpairs - 2, length(bl->fhead));
//
//	reclaim(bl, i1);
//	TEST_ASSERT_EQUAL_INT(fpairs - 1, length(bl->fhead));
//
//	reclaim(bl, i2);
//	TEST_ASSERT_EQUAL_INT(fpairs, length(bl->fhead));
//
//	TEST_ASSERT_EQUAL(i1, i2->cdr);
//	TEST_ASSERT_EQUAL(i2, bl->fhead);
//	
//	deinit(bl);
//}
//
//void test_stack(void) {
//	H* bl = init(1024, 1024);
//	TEST_ASSERT_NOT_NULL(bl);
//
//	TEST_ASSERT_EQUAL_INT(0, length(bl->sp));
//	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
//	push(bl, 7);
//	TEST_ASSERT_EQUAL_INT(1, length(bl->sp));
//	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
//	push(bl, 11);
//	TEST_ASSERT_EQUAL_INT(2, length(bl->sp));
//	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
//	TEST_ASSERT_EQUAL_INT(11, pop(bl));
//	TEST_ASSERT_EQUAL_INT(1, length(bl->sp));
//	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
//	TEST_ASSERT_EQUAL_INT(7, pop(bl));
//	TEST_ASSERT_EQUAL_INT(0, length(bl->sp));
//	TEST_ASSERT_EQUAL_INT(0, length(bl->rp));
//
//	deinit(bl);
//}
//
////void fib(H* bl) {
////	DUP(bl);
////	LIT(bl, 1);	
////	GT(bl);
////	if (pop(bl) != 0) {
////		DEC(bl);
////		DUP(bl);
////		DEC(bl);
////		fib(bl);
////		SWAP(bl);
////		fib(bl);
////		ADD(bl);
////	}
////}
////
////void test_fib(void) {
////	C b[262000];
////	H* bl = init(b, 262000);
////
////	push(bl, 25);
////	fib(bl);
////
////	TEST_ASSERT_EQUAL_INT(75025, pop(bl));
////}
//
//void test_store_str(void) {
//	H* bl = init(1024, 1024);
//	TEST_ASSERT_NOT_NULL(bl);
//
//	B* h = bl->here;
//	S* s = (S*)h;
//	unsigned char str[] = "Testing compile string!";
//	store_str(bl, str);
//	TEST_ASSERT_EQUAL_INT(s->len, strlen(str));
//	TEST_ASSERT_EQUAL_INT(0, strcmp(s->data.str, str));
//	TEST_ASSERT_EQUAL_INT(8 + AS_CELLS(strlen(str)), bl->here - h);
//
//	deinit(bl);
//}
//
////void test_dictionary(void) {
////	C b[32];
////	H* bl = init(b, 32);
////	TEST_ASSERT_EQUAL_INT(0, length(bl->dp));
////	W* w = malloc(sizeof(W));
////	w->cdr = bl->dp;
////	bl->dp = (P*)w;
////	TEST_ASSERT_EQUAL_INT(1, length(bl->dp));
////	W* w2 = malloc(sizeof(W));
////	w2->cdr = bl->dp;
////	bl->dp = (P*)w2;
////	TEST_ASSERT_EQUAL_INT(2, length(bl->dp));
////}
//
//void test_error_code(void) {
//	H* bl = init(1024, 1024);
//
//	// mov QWORD PTR [rdi+16], 11
//	// ret
//	S* s = create_code(bl);
//	compile_bytes(bl, s, "\x48\xC7\x47\x10\x0B\x00\x00\x00\xC3", 9);
//
//	((void (*)(H*))(s->data.code))(bl);
//
//	TEST_ASSERT_EQUAL_INT(11, bl->err);
//
//	deinit(bl);
//}
//
//void test_asm_c_asm(void) {
//	H* bl = init(1024, 1024);
//	TEST_ASSERT_NOT_NULL(bl);
//
//	S* s = create_code(bl);
//	// mov QWORD PTR [rdi+16], 11
//	TEST_ASSERT_NOT_NULL(compile_bytes(bl, s, "\x48\xC7\x47\x10\x0B\x00\x00\x00", 8));
//	TEST_ASSERT_NOT_NULL(compile_cfunc(bl, s, 8));
//	// mov QWORD PTR [rdi+16], 57
//	// ret
//	TEST_ASSERT_NOT_NULL(compile_bytes(bl, s, "\x48\xC7\x47\x10\x39\x00\x00\x00\xC3", 9));
//
//	C res = ((C (*)(H*))(s->data.code))(bl);
//
//	TEST_ASSERT_EQUAL_INT(11, bl->err);
//	TEST_ASSERT_EQUAL_INT(res, 8);
//
//	TEST_ASSERT_EQUAL_PTR(s->data.code + 8 + 25, bl->ip);
//
//	((C (*)(H*))(bl->ip))(bl);
//
//	TEST_ASSERT_EQUAL_INT(57, bl->err);
//
//	deinit(bl);
//}
	
int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_block_initialization);

	RUN_TEST(test_compile_byte);
	RUN_TEST(test_compile_bytes);
	RUN_TEST(test_compile_literal);

	RUN_TEST(test_compile_next);
	RUN_TEST(test_compile_cfunc);
	RUN_TEST(test_compile_push);


	//RUN_TEST(test_is_aligned);
	//RUN_TEST(test_as_cells);
	//RUN_TEST(test_as_bytes);
	//
	//RUN_TEST(test_list_length);

	//RUN_TEST(test_find_word);

	//RUN_TEST(test_allot);
	//RUN_TEST(test_cons_and_reclaim);
	//RUN_TEST(test_stack);

	////RUN_TEST(test_fib);
	////RUN_TEST(test_dictionary);

	//RUN_TEST(test_store_str);

	//RUN_TEST(test_error_code);
	//RUN_TEST(test_asm_c_asm);

	return UNITY_END();
}
