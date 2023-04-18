#ifndef __DODO_CORE__
#define __DODO_CORE__

#include <stdint.h>

typedef int8_t BYTE;
#if INTPTR_MAX == INT64_MAX
	typedef int16_t QUARTER;
	typedef int32_t HALF;
#else
	typedef int8_t QUARTER;
	typedef int16_t HALF;
#endif
typedef intptr_t CELL;

typedef struct T_NODE {
	struct T_NODE* next;
	union {
		CELL value;
		struct T_NODE* ref;
	};
} NODE;

typedef struct {
	struct T_NODE* next;
	union {
		CELL value;
		CELL* cells;
		BYTE* bytes;
		struct T_NODE* ref;
	};
	QUARTER type;
	QUARTER size;
	HALF length;
	CELL data[];
} ANODE;

#define NEXT(node)								((NODE*)(((CELL)(node)->next) & -8))
#define TYPE(node)								(((CELL)node->next) & 7)

#define REST(node)								((ARRAY_NODE*)(((NODE*)node) + 1))

#define AS(type, next)						((NODE*)(((CELL)(next)) | type))

NODE* LINK(NODE* node, NODE* link) { 
	node->next = AS(TYPE(node), link); 
	return node; 
}

enum Types { ATOM, ARRAY, LIST, PRIM, FLOW, WORD, IWORD, CUSTOM };
enum ATypes { CARRAY, BARRAY, STRING };

CELL length(NODE* list, CELL dir) {
	CELL count = 0;
	if (dir) while (list) { count++; list = list->ref; }
	else while (list) { count++; list = NEXT(list); }
	return count;
}

NODE* reverse(NODE* list, NODE* acc) {
	if (list) {
		NODE* tail = NEXT(list);
		LINK(list, acc);
		return reverse(tail, list);
	} else {
		return acc;
	}
}

#define ALIGN(addr, bound)				((((CELL)addr) + (bound - 1)) & ~(bound - 1))

NODE* init_free(BYTE* block, CELL size, CELL start) {
	NODE* node;

	NODE* there = (NODE*)ALIGN((block + start), sizeof(NODE));
	NODE* top = (NODE*)ALIGN((block + size - sizeof(NODE) - 1), sizeof(NODE));

	for (node = there; node <= top; node ++) {
		node->next = node == there ? 0 : node - 1;
		node->ref = node == top ? 0 : node + 1;
	}

	return top;
}

ANODE* ncons(NODE** free, CELL n, NODE* next) {
	NODE** pred = free;
	NODE* node = *free;
	ANODE* anode;
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
			*pred = NEXT(node);
			(*pred)->ref = 0;
	
			anode = (ANODE*)node;

			anode->next = next;
			anode->cells = anode->data;
			anode->size = n - 1;
			anode->length = 0;

			return anode;
		}

		pred = &node;
		node = node->next;
	}
}

NODE* cons(NODE** free, CELL value, NODE* next) {
	NODE* node;

	if (*free == 0) return 0;

	node = *free;
	*free = NEXT(*free);
	node->next = next;
	node->value = value;
	(*free)->ref = 0;

	return node;
}

NODE* reclaim(NODE** free, NODE* node) {
	CELL nodes = 1;
	NODE* tail = NEXT(node);

	if (TYPE(node) == LIST) {
		while (node->ref) {
			node->ref = reclaim(free, node->ref);
		}
	}

	if (TYPE(node) == ARRAY || TYPE(node) == FLOW) {
		nodes = ((ANODE*)node)->size + 1;
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

NODE* clone(NODE** free, NODE* node, CELL follow) {
	CELL i;
	ANODE* anode;
	if (!node) return 0;
	if (TYPE(node) == LIST) {
		return cons(
			free, 
			(CELL)clone(free, node->ref, 1), 
			AS(LIST, follow ? clone(free, NEXT(node), 1) : 0));
	} else {
		if (TYPE(node) == ARRAY || TYPE(node) == FLOW) {
			anode = (ANODE*)node;
			ANODE* n = ncons(
				free, 
				anode->size + 1, 
				AS(TYPE(anode), follow ? clone(free, NEXT(node), 1) : 0));
			n->type = anode->type;
			n->length = anode->length;
			memcpy(n->data, anode->data, ((2*anode->size)-1)*sizeof(CELL));
			return (NODE*)n;
		} else {
			return cons(free, node->value, AS(TYPE(node), follow ? clone(free, NEXT(node), 1) : 0));	
		}
	}
}

#endif
