#include<stdlib.h>
#include<string.h>

#include "unity.h"
#include "core.h"

void setUp() {}

void tearDown() {}

void test_CORE_node() {
	NODE* node = malloc(sizeof(NODE)*2);

	ANODE* anode = (ANODE*)node;

	anode->next = node;
	anode->cells = anode->data;
	anode->type = STRING;
	anode->size = 1;
	anode->length = 6;
	anode->data[0] = 2048;

	TEST_ASSERT_EQUAL_PTR(node, anode->next);
	TEST_ASSERT_EQUAL_PTR(anode->data, anode->ref);
	TEST_ASSERT_EQUAL_INT(STRING, anode->type);
	TEST_ASSERT_EQUAL_INT(1, anode->size);
	TEST_ASSERT_EQUAL_INT(6, anode->length);
	TEST_ASSERT_EQUAL_INT(2048, anode->data[0]);
}

void test_CORE_length() {
	NODE* n1 = malloc(sizeof(NODE));
	NODE* n2 = malloc(sizeof(NODE));
	NODE* n3 = malloc(sizeof(NODE));

	n1->next = n2;
	n2->next = n3;
	n3->next = 0;

	TEST_ASSERT_EQUAL_PTR(3, length(n1, 0));
	TEST_ASSERT_EQUAL_PTR(2, length(n2, 0));
	TEST_ASSERT_EQUAL_PTR(1, length(n3, 0));
	TEST_ASSERT_EQUAL_PTR(0, length(0, 0));
}

void test_CORE_reverse() {
	NODE* p1 = malloc(sizeof(NODE));
	NODE* p2 = malloc(sizeof(NODE));
	NODE* p3 = malloc(sizeof(NODE));
	
	p1->next = p2;
	p2->next = p3;
	p3->next = 0;

	NODE* r = reverse(p1, 0);

	TEST_ASSERT_EQUAL_INT(p3, r);
	TEST_ASSERT_EQUAL_INT(p2, NEXT(r));
	TEST_ASSERT_EQUAL_INT(p1, NEXT(NEXT(r)));
}

void test_CORE_init_free() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	TEST_ASSERT_EQUAL_PTR((NODE*)ALIGN(block + size - 2*sizeof(CELL) - 1, 2*sizeof(CELL)), free);
	TEST_ASSERT_EQUAL_INT(16, length(free, 0));
	TEST_ASSERT_EQUAL_INT(16, length(last, 1));
}

void test_CORE_cons() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	NODE* list = cons(&free, 7, AS(ATOM, cons(&free, 11, AS(ATOM, cons(&free, 13, AS(ATOM, 0))))));

	TEST_ASSERT_EQUAL_INT(13, length(free, 0));
	TEST_ASSERT_EQUAL_INT(13, length(last, 1));
	TEST_ASSERT_EQUAL_INT(3, length(list, 0));
	TEST_ASSERT_EQUAL_INT(7, list->value);
	TEST_ASSERT_EQUAL_INT(11, NEXT(list)->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(NEXT(list))->value);

	NODE* p = cons(&free, 7, AS(ATOM, 0));

	TEST_ASSERT_EQUAL_INT(12, length(free, 0));
	TEST_ASSERT_EQUAL_INT(12, length(last, 1));
	TEST_ASSERT_EQUAL_INT(7, p->value);
	TEST_ASSERT_EQUAL_INT(0, NEXT(p));
}

void test_CORE_cons_2() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	TEST_ASSERT_EQUAL_PTR(saved_free - 15, last);

	NODE* n1 = cons(&free, 7, (NODE*)11);

	TEST_ASSERT_EQUAL_PTR(saved_free, n1);
	TEST_ASSERT_EQUAL_PTR(saved_free - 1, free);
	TEST_ASSERT_EQUAL_INT(15, length(free, 0));
	TEST_ASSERT_EQUAL_INT(15, length(last, 1));
	TEST_ASSERT_EQUAL_INT(7, n1->value);
	TEST_ASSERT_EQUAL_PTR(11, n1->next);

	ANODE* n2 = ncons(&free, 2, (NODE*)17);

	TEST_ASSERT_EQUAL_INT(13, length(free, 0));
	TEST_ASSERT_EQUAL_INT(13, length(last, 1));
	TEST_ASSERT_EQUAL_INT(saved_free - 3, free);
	TEST_ASSERT_EQUAL_INT(saved_free - 2, n2);
	TEST_ASSERT_EQUAL_INT(1, n2->size);
	TEST_ASSERT_EQUAL_PTR(17, n2->next);

	ANODE* n3 = ncons(&free, 3, 0);

	TEST_ASSERT_EQUAL_INT(10, length(free, 0));
	TEST_ASSERT_EQUAL_INT(10, length(last, 1));
	TEST_ASSERT_EQUAL_INT(saved_free - 6, free);
	TEST_ASSERT_EQUAL_INT(saved_free - 5, n3);
	TEST_ASSERT_EQUAL_INT(2, n3->size);

	ANODE* n4 = ncons(&free, 9, 0);

	TEST_ASSERT_EQUAL_INT(1, length(free, 0));
	TEST_ASSERT_EQUAL_INT(1, length(last, 1));
}

void test_CORE_cons_3() {
	CELL size = 10*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	NODE* n1 = free;
	NODE* n2 = free - 1;
	NODE* n3 = free - 2;
	NODE* n4 = free - 3;
	NODE* n5 = free - 4;
	NODE* n6 = free - 5;
	NODE* n7 = free - 6;
	NODE* n8 = free - 7;
	NODE* n9 = free - 8;
	NODE* n10 = free - 9;

	ANODE* n = ncons(&free, 4, 0);

	TEST_ASSERT_EQUAL_INT(3, n->size);
	TEST_ASSERT_EQUAL_PTR(n4, n);
	TEST_ASSERT_EQUAL_INT((((CELL)n3) + sizeof(CELL)), &n->data[0]);
	TEST_ASSERT_EQUAL_PTR(n2, &n->data[1]);
	TEST_ASSERT_EQUAL_INT((((CELL)n2) + sizeof(CELL)), &n->data[2]);
	TEST_ASSERT_EQUAL_PTR(n1, &n->data[3]);
	TEST_ASSERT_EQUAL_INT((((CELL)n1) + sizeof(CELL)), &n->data[4]);

	CELL a[6] = { 7, 11, 13, 17, 19, 23 };
	memcpy(n->data, a, 6*sizeof(CELL));
	TEST_ASSERT_EQUAL_INT(7, n->data[0]);
	TEST_ASSERT_EQUAL_INT(11, n->data[1]);
	TEST_ASSERT_EQUAL_INT(13, n->data[2]);
	TEST_ASSERT_EQUAL_INT(17, n->data[3]);
	TEST_ASSERT_EQUAL_INT(19, n->data[4]);
	TEST_ASSERT_EQUAL_INT(23, n->data[5]);

	NODE* m = cons(&free, 0, 0);

	TEST_ASSERT_EQUAL_PTR(n5, m);

	ANODE* l = ncons(&free, 3, 0);

	TEST_ASSERT_EQUAL_PTR(n8, l);

	strcpy(l->bytes, "test string");

	TEST_ASSERT_EQUAL_STRING("test string", l->bytes);
}	

void test_CORE_reclaim() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	NODE* list = cons(&free, 7, AS(ATOM, cons(&free, 11, AS(ATOM, cons(&free, 13, AS(ATOM, 0))))));
	NODE* tail = reclaim(&free, list);

	TEST_ASSERT_EQUAL_INT(14, length(free, 0));
	TEST_ASSERT_EQUAL_INT(14, length(last, 1));
	TEST_ASSERT_EQUAL_INT(2, length(tail, 0));
	TEST_ASSERT_EQUAL_INT(11, tail->value);
	TEST_ASSERT_EQUAL_INT(13, NEXT(tail)->value);

	tail = reclaim(&free, reclaim(&free, tail));

	TEST_ASSERT_EQUAL_INT(16, length(free, 0));
	TEST_ASSERT_EQUAL_INT(16, length(last, 1));
	TEST_ASSERT_EQUAL_INT(0, length(tail, 0));
}

void test_CORE_reclaim_list() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	NODE* list = 
		cons(&free, 5, AS(ATOM,
		cons(&free, (CELL)
			cons(&free, 7, AS(ATOM,
			cons(&free, 11, AS(ATOM,
			cons(&free, 13, AS(ATOM, 0)))))), AS(LIST, 
		cons(&free, 17, AS(ATOM, 0))))));

	TEST_ASSERT_EQUAL_INT(10, length(free, 0));
	TEST_ASSERT_EQUAL_INT(10, length(last, 1));
	TEST_ASSERT_EQUAL(ATOM, TYPE(list));
	TEST_ASSERT_EQUAL_INT(5, list->value);
	TEST_ASSERT_EQUAL(LIST, TYPE(NEXT(list)));
	TEST_ASSERT_EQUAL_INT(3, length(NEXT(list)->ref, 0));
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(list)->ref));
	TEST_ASSERT_EQUAL_INT(7, NEXT(list)->ref->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(list)->ref)));
	TEST_ASSERT_EQUAL_INT(11, NEXT(NEXT(list)->ref)->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(NEXT(list)->ref))));
	TEST_ASSERT_EQUAL_INT(13, NEXT(NEXT(NEXT(list)->ref))->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(list))));
	TEST_ASSERT_EQUAL_INT(17, NEXT(NEXT(list))->value);

	NODE* tail = reclaim(&free, list);

	TEST_ASSERT_EQUAL_INT(11, length(free, 0));
	TEST_ASSERT_EQUAL_INT(11, length(last, 1));
	TEST_ASSERT_EQUAL(LIST, TYPE(tail));
	TEST_ASSERT_EQUAL_INT(3, length(tail->ref, 0));
	TEST_ASSERT_EQUAL(ATOM, TYPE(tail->ref));
	TEST_ASSERT_EQUAL_INT(7, tail->ref->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(tail->ref)));
	TEST_ASSERT_EQUAL_INT(11, NEXT(tail->ref)->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(NEXT(tail->ref))));
	TEST_ASSERT_EQUAL_INT(13, NEXT(NEXT(tail->ref))->value);
	TEST_ASSERT_EQUAL(ATOM, TYPE(NEXT(tail)));
	TEST_ASSERT_EQUAL_INT(17, NEXT(tail)->value);

	tail = reclaim(&free, tail);

	TEST_ASSERT_EQUAL_INT(15, length(free, 0));
	TEST_ASSERT_EQUAL_INT(15, length(last, 1));
	TEST_ASSERT_EQUAL(ATOM, TYPE(tail));
	TEST_ASSERT_EQUAL_INT(17, tail->value);

	tail = reclaim(&free, tail);

	TEST_ASSERT_EQUAL_INT(16, length(free, 0));
	TEST_ASSERT_EQUAL_INT(16, length(last, 1));
	TEST_ASSERT_EQUAL_INT(0, tail);
}

void test_CORE_cons_reclaim() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	NODE* n1 = cons(&free, 7, AS(ATOM, 0));
	NODE* n2 = cons(&free, 11, AS(ATOM, 0));
	NODE* n3 = cons(&free, 13, AS(ATOM, 0));
	NODE* n4 = cons(&free, 17, AS(ATOM, 0));

	reclaim(&free, n3);
	reclaim(&free, n2);

	ANODE* n5 = ncons(&free, 2, AS(ARRAY, 0));

	TEST_ASSERT_NOT_EQUAL_INT(0, n5);
	TEST_ASSERT_EQUAL_INT(12, length(free, 0));
	TEST_ASSERT_EQUAL_INT(12, length(last, 1));

	TEST_ASSERT_EQUAL_PTR(n3, n5);
}

void test_CORE_cons_reclaim_2() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];

	NODE* free = init_free(block, size, 0);
	NODE* saved_free = free;
	NODE* last = (NODE*)block;

	NODE* n1 = cons(&free, 7, AS(ATOM, 0));
	NODE* n2 = cons(&free, 11, AS(ATOM, 0));
	NODE* n3 = cons(&free, 13, AS(ATOM, 0));
	NODE* n4 = cons(&free, 17, AS(ATOM, 0));

	reclaim(&free, n2);
	reclaim(&free, n3);

	ANODE* n5 = ncons(&free, 2, AS(ARRAY, 0));

	TEST_ASSERT_EQUAL_INT(12, length(free, 0));
	TEST_ASSERT_EQUAL_INT(12, length(last, 1));

	TEST_ASSERT_EQUAL_PTR(n2, n5);
}

//void test_CORE_clone() {
//	CELL size = 16*sizeof(NODE);
//	BYTE block[size];
//	BYTE buf[255];
//
//	NODE* free = init_free(block, size, 0);
//	NODE* last = (NODE*)block;
//
//	NODE* list = 
//		cons(&free, 7, AS(ATOM, 
//		cons(&free, 11, AS(PRIM, 
//		cons(&free, (CELL)
//			cons(&free, 13, AS(ATOM,
//			cons(&free, 17, AS(ATOM, 0)))), 
//		AS(LIST,
//		cons(&free, 19, AS(ATOM, 0))))))));
//
//	buf[0] = 0;
//	TEST_ASSERT_EQUAL_STRING("#7 P:11 { #13 #17 } #19 ", print(buf, list, 1, 0));
//
//	TEST_ASSERT_EQUAL_INT(10, length(free, 0));
//	TEST_ASSERT_EQUAL_INT(10, length(last, 1));
//
//	NODE* cloned = clone(&free, list, 1);
//
//	buf[0] = 0;
//	TEST_ASSERT_EQUAL_STRING("#7 P:11 { #13 #17 } #19 ", print(buf, cloned, 1, 0));
//
//	TEST_ASSERT_EQUAL_INT(4, length(free, 0));
//	TEST_ASSERT_EQUAL_INT(4, length(last, 1));
//
//	TEST_ASSERT_NOT_EQUAL(list, cloned);
//	TEST_ASSERT_NOT_EQUAL(NEXT(list), NEXT(cloned));
//	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(list)), NEXT(NEXT(cloned)));
//	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(list))->ref, NEXT(NEXT(cloned))->ref);
//	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(NEXT(list))->ref), NEXT(NEXT(NEXT(cloned))->ref));
//	TEST_ASSERT_NOT_EQUAL(NEXT(NEXT(NEXT(list))), NEXT(NEXT(NEXT(cloned))));
//}

void test_CORE_clone_atom() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];
	BYTE buf[255];

	NODE* free = init_free(block, size, 0);
	NODE* last = (NODE*)block;

	NODE* n = cons(&free, 7, AS(ATOM, 0));
	
	NODE* n2 = clone(&free, n, 0);

	TEST_ASSERT_EQUAL_INT(14, length(free, 0));

	NODE* stack = cons(&free, 11, AS(ATOM, 0));

	TEST_ASSERT_EQUAL_INT(13, length(free, 0));

	stack = LINK(clone(&free, stack, 0), stack);

	TEST_ASSERT_EQUAL_INT(12, length(free, 0));
	TEST_ASSERT_EQUAL_INT(2, length(stack, 0));
	TEST_ASSERT_EQUAL_INT(11, stack->value);
	TEST_ASSERT_EQUAL_INT(11, NEXT(stack)->value);
}

void test_CORE_clone_array() {
	CELL size = 16*sizeof(NODE);
	BYTE block[size];
	NODE* free = init_free(block, size, 0);

	CELL array[] = {0, 1, 2, 3};

	ANODE* n1 = ncons(&free, 3, AS(FLOW, 0));
	memcpy(n1->data, array, 4);

	TEST_ASSERT_EQUAL_INT(FLOW, TYPE(n1));

	ANODE* n2 = (ANODE*)clone(&free, (NODE*)n1, 1);

	TEST_ASSERT_NOT_EQUAL(n1, n2);
	TEST_ASSERT_EQUAL_INT(FLOW, TYPE(n1));
	TEST_ASSERT_EQUAL_INT(TYPE(n1), TYPE(n2));
	TEST_ASSERT_EQUAL_INT(n1->size, n2->size);
	TEST_ASSERT_EQUAL_MEMORY(n1->data, n2->data, (2*sizeof(NODE)) - sizeof(NODE));

	TEST_ASSERT_EQUAL_INT(10, length(free, 0));
}

int main() {
	UNITY_BEGIN();

	RUN_TEST(test_CORE_node);

	RUN_TEST(test_CORE_length);
	RUN_TEST(test_CORE_reverse);

	RUN_TEST(test_CORE_init_free);
	RUN_TEST(test_CORE_cons);
	RUN_TEST(test_CORE_cons_2);
	RUN_TEST(test_CORE_cons_3);
	RUN_TEST(test_CORE_reclaim);
	RUN_TEST(test_CORE_reclaim_list);
	RUN_TEST(test_CORE_cons_reclaim);
	RUN_TEST(test_CORE_cons_reclaim_2);
	//RUN_TEST(test_CORE_clone);
	RUN_TEST(test_CORE_clone_atom);
	RUN_TEST(test_CORE_clone_array);

	return UNITY_END();
}

