#ifndef __DODO__
#define __DODO__

#include<stdint.h>

typedef int8_t BYTE;
#if INTPTR_MAX == INT64_MAX
	typedef int32_t HALF;
#else
	typedef int16_t HALF;
#endif
typedef intptr_t CELL;

typedef struct T_NODE {
	struct T_NODE* next;
	union {
		CELL value;
		struct T_NODE* ref;
		struct {
			HALF size;		// Always in cells, to ensure correct reclaiming
			HALF length;
		};
	};
	CELL data[];
} NODE;

#define CELLS(node)					(node->data)
#define BYTES(node)					((BYTE*)node->data)

#define NEXT(node)					((NODE*)(((CELL)node->next) & -8))
#define TYPE(node)					(((CELL)node->next) & 7)

#define REF(node)						((NODE*)(((CELL)node->ref) & -8))
#define SUBTYPE(node)				(((CELL)node->ref) & 7)

#define AS(type, node)			((NODE*)(((CELL)node) | type))

enum Types { ATOM, PRIM, PARRAY, STRING, BARRAY, CARRAY, COLL };
enum CollTypes { LIST, WORD, USER };

#define IS_ARRAY(node)			(TYPE(node) == PARRAY || TYPE(node) == STRING || TYPE(node) == BARRAY || TYPE(node) == CARRAY)

CELL length(NODE* list) {
	CELL count = 0;
	while (list) {
		count++;
		list = NEXT(list);
	}
	return count;
}

#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))

NODE* init_free(BYTE* block, CELL size, CELL start) {
	NODE* node;

	NODE* there = (NODE*)ALIGN((block + start), 2*sizeof(CELL));
	NODE* top = (NODE*)ALIGN((block + size - 2*sizeof(CELL) - 1), 2*sizeof(CELL));

	for (node = there; node <= top; node ++) {
		node->next = node == there ? 0 : node - 1;
		node->ref = node == top ? 0 : node + 1;
	}

	return top;
}

NODE* cons(NODE** free, CELL n, CELL value, NODE* next) {
	NODE* pred = (NODE*)free;
	NODE* node = *free;
	while (node) {
		CELL t = n;
		while (--t) {
			if (node->ref == (node + 1) || node->ref == 0) {
				node = node->next;
			} else {
				break;
			}
		}
		if (t == 0) {
			pred->next = node->next;
			node->next->ref = pred == (NODE*)free ? 0 : pred;
			node->next = next;
			node->value = value;
			if (n > 1) node->size = n;
			return node;
		}
		node = node->next;
	}
}

NODE* reclaim(NODE** free, NODE* node) {
	CELL nodes = 1;
	NODE* tail = NEXT(node);

	if (TYPE(node) == COLL && (SUBTYPE(node) == LIST || SUBTYPE(node) == USER)) {
		while (node->ref) {
			node->ref = AS(SUBTYPE(node), reclaim(free, node->ref));
		}
	}

	if (IS_ARRAY(node)) {
		nodes = node->size;
	}

	while (nodes) {
		node->next = *free;
		node->ref = 0;
		(*free)->ref = node;
		*free = node;
		node++;
		nodes--;
	}

	return tail;
}

#endif
