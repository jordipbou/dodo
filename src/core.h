#ifndef __DODO_CORE__
#define __DODO_CORE__

#include <stdint.h>

typedef int8_t BYTE;
typedef intptr_t CELL;

typedef struct T_NODE {
	struct T_NODE* next;
	union {
		CELL value;
		struct T_NODE* ref;
	};
} NODE;

typedef struct {
	CELL size;
	CELL length;
	CELL data[];
} ARRAY;

#define SIZE(array)								(array->size - sizeof(NODE))

typedef struct {
	CELL size;
	BYTE* here;
	NODE* there;
	NODE* free;
	BYTE* bottom[];
} BLOCK;

#define NEXT(node)								((NODE*)(((CELL)(node)->next) & -8))
#define TYPE(node)								(((CELL)node->next) & 7)

#define AS(type, next)						((NODE*)(((CELL)(next)) | type))

enum Types { ATOM, REF, LIST, STRING, PRIM, CODE, WORD, TYPE };

CELL length(NODE* list, CELL dir) {
	CELL count = 0;
	if (dir) while (list) { count++; list = list->ref; }
	else while (list) { count++; list = NEXT(list); }
	return count;
}

NODE* reverse(NODE* list, NODE* acc) {
	if (list) {
		NODE* tail = NEXT(list);
		list->next = AS(LIST, acc);
		return reverse(tail, list);
	} else {
		return acc;
	}
}

#define ALIGN(addr, bound)				((((CELL)addr) + (bound - 1)) & ~(bound - 1))
#define FREE(block)								(length(block->free, 0) - 1)

BLOCK* init_block(BYTE* block, CELL size, CELL start) {
	NODE* node;
	BLOCK* b = (BLOCK*)(block + start);

	b->size = size - start;
	b->here = (BYTE*)(((BYTE*)b) + sizeof(BLOCK));
	b->there = (NODE*)ALIGN(b->here, sizeof(NODE));
	b->free = (NODE*)ALIGN(b + (size - start) - sizeof(NODE) - 1, sizeof(NODE));

	for (node = b->there; node <= b->free; node++) {
		node->next = node == b->there ? 0 : node - 1;
		node->ref = node == b->free ? 0 : node + 1;
	}

	return b;
}

NODE* cons(BLOCK* block, CELL value, NODE* next) {
	NODE* node;

	if (block->free == block->there) return 0;

	node = block->free;
	block->free = NEXT(block->free);
	node->next = next;
	node->value = value;
	block->free->ref = 0;

	return node;
}

ARRAY* array(BLOCK* block, CELL req_size) {
	NODE** pred = &block->free;
	NODE* node = block->free;
	CELL size = (req_size / sizeof(NODE) + ((req_size % sizeof(NODE)) == 0 ? 0 : 1)) + 1;
	CELL t;
	ARRAY* arr;
	if (req_size == 0) return 0;
	while (node && node != block->there) {
		t = size;
		while (--t && node && node != block->there) {
			if ((node->ref == (node + 1) || node->ref == 0)) {
				node = node->next;
			} else {
				break;
			}
		}
		if (t == 0) {
			*pred = NEXT(node);
			NEXT(node)->ref = *pred == block->free ? 0 : *pred;

			arr = (ARRAY*)node;	
			arr->size = size * sizeof(NODE);

			return arr;
		}
		pred = &(node->next);
		node = NEXT(node);
	}
	return 0;
}

//NODE* reclaim(NODE** free, NODE* node) {
//	CELL nodes = 1;
//	NODE* tail = NEXT(node);
//
//	if (TYPE(node) == LIST) {
//		while (node->ref) {
//			node->ref = reclaim(free, node->ref);
//		}
//	}
//
//	if (TYPE(node) == ARRAY || TYPE(node) == FLOW) {
//		nodes = ((ANODE*)node)->size + 1;
//	}
//
//	while (nodes) {
//		node->next = *free;
//		node->ref = 0;
//		(*free)->ref = node;
//		*free = node;
//		node++;
//		nodes--;
//	}
//
//	return tail;
//}
//
//NODE* clone(NODE** free, NODE* node, CELL follow) {
//	CELL i;
//	ANODE* anode;
//	if (!node) return 0;
//	if (TYPE(node) == LIST) {
//		return cons(
//			free, 
//			(CELL)clone(free, node->ref, 1), 
//			AS(LIST, follow ? clone(free, NEXT(node), 1) : 0));
//	} else {
//		if (TYPE(node) == ARRAY || TYPE(node) == FLOW) {
//			anode = (ANODE*)node;
//			ANODE* n = ncons(
//				free, 
//				anode->size + 1, 
//				AS(TYPE(anode), follow ? clone(free, NEXT(node), 1) : 0));
//			n->type = anode->type;
//			n->length = anode->length;
//			memcpy(n->data, anode->data, ((2*anode->size)-1)*sizeof(CELL));
//			return (NODE*)n;
//		} else {
//			return cons(free, node->value, AS(TYPE(node), follow ? clone(free, NEXT(node), 1) : 0));	
//		}
//	}
//}

#endif
