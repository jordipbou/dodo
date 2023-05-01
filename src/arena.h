#ifndef DODO_ARENA
#define DODO_ARENA

#include "types.h"

typedef struct {
	CELL size;
	CELL bottom;
	CELL here;
	CELL top;
	BYTE block[1];
} ARENA;

#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))

ARENA* init_arena(BYTE* block, CELL size, CELL size_of_header) {
	ARENA* arena = (ARENA*)block;

	arena->size = size;

	arena->bottom = arena + ALIGN(arena + size_of_header, sizeof(NODE));
	arena->here = arena->bottom;
	arena->top = arena + size;

	return arena;
}

NODE* cons(ALLOCATOR* a, CELL value, CELL next) {
	ARENA* arena = (ARENA*)a;
	NODE* node;

	if (arena->here + sizeof(NODE) >= arena->top) return 0;

	node->value = value;
	node->next = next;

	arena->here += sizeof(NODE);

	return node;
}

NODE* reclaim(ALLOCATOR* a, NODE* node) {
	return next(node);
}

NODE* clone(ALLOCATOR* a, NODE* node, CELL follow) {
	return cons_arena(a, node->value, as(type(node), follow ? clone_arena(a, next(n), 1) : 0));
}

#endif
