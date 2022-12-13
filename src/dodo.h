#include<stdint.h>
#include<stddef.h>

// Datatypes

typedef int8_t BYTE;
typedef int16_t WORD;		
typedef int32_t HALF;		// Used mainly as index inside bytes and nodes arrays
typedef int64_t CELL;

typedef struct {
	HALF next;
	WORD type;
	WORD len;				//Array length
	union {
		CELL prev;		// Doubly linked list for free nodes
		CELL value;		// List node
		BYTE data[8];	// Array node
	};
} NODE;

#define T_NUMBER				0
#define T_LIST					8
#define T_ARRAY					16	
#define T_DICT					32	

#define T_F_PRIMITIVE		512	
#define T_F_DOCOL				1024
#define T_F_DOVAR				2048
#define T_F_DOCONST			4096
#define T_F_IMMEDIATE		8192

#define T_FREE					16384
#define T_NIL						32768

// Virtual machine context

typedef struct {
	CELL size;						// Size of block
	HALF bottom, here;		// Pointers to lowest and highest allocated bytes
	HALF there, top;			// Pointers to lowest and highest nodes
	HALF dstack, rstack;	// Pointers to data and return stacks
} CTX;

#define NIL									0
#define ALIGN(addr, bound)	((CELL)addr + ((CELL)bound-1)) & ~((CELL)bound-1)

CTX* init(BYTE* block, CELL size) {
	CTX* c = (CTX*)block;	
	// TODO: Size must be NODE aligned (floor aligned, as block is fixed size)
	c->size = size;

	c->bottom = c->here = (HALF)sizeof(CTX);
	// TODO: c->there must be NODE aligned, independent of CTX size
	c->there = (HALF)(sizeof(CTX) / sizeof(NODE));
	c->top = (HALF)((size / sizeof(NODE)) - 1);

	NODE* nodes = (NODE*)c;
	for (HALF i = c->there; i <= c->top; i++) {
		nodes[i].type = T_FREE;
		nodes[i].next = i + 1;
		nodes[i].prev = i - 1;
	}
	nodes[c->there].prev = NIL;
	nodes[c->top].next = NIL;

	c->dstack = c->top;
	c->rstack = NIL;

	return c;
}

// Memory management

#define NODE_AT(c, idx)		(((NODE*)c)[idx])
#define STACK(c)					NODE_AT(c, c->dstack)
#define T(c)							NODE_AT(c, STACK(c).next)
#define S(c)							NODE_AT(c, T(c).next)

#define RESERVED(c)			((c->there * sizeof(NODE)) - c->here)

#define DIR_NEXT				1
#define DIR_PREV				0

CELL length(CTX* c, CELL direction, HALF node) { 
	NODE* nodes = (NODE*)c;
	for (CELL c = 0;; c++) {
		if (node == NIL) { return c; }
		else { 
			if (direction == DIR_NEXT) {
				node = nodes[node].next;
			} else {
				node = nodes[node].prev;
			}
		}
	}
}

#define FREE_NODES(c)		(length(c, DIR_PREV, NODE_AT(c, c->dstack).prev))
#define DEPTH(c)				(length(c, DIR_NEXT, NODE_AT(c, c->dstack).next))

void allot(CTX* c, HALF bytes) {
	if (bytes == 0) return;
	if (bytes < 0) {
		// TODO: Nodes should be returned to end of free doubly linked list !!
		if ((c->here + bytes) > c->bottom) c->here += bytes;
		else c->here = c->bottom;
	} else {
		while (RESERVED(c) < bytes) {
			if (NODE_AT(c, c->there).type & T_FREE) {
				NODE_AT(c, NODE_AT(c, c->there).next).prev = NODE_AT(c, c->there).prev;
				c->there++;	
			} else {
				// ERROR: Not enough contiguous nodes to free (Not enough memory)
			}
		}
		c->here += bytes;
	}
}

void push(CTX* c, CELL n) {
	c->dstack = STACK(c).prev;
	T(c).value = n;
	T(c).type = T_NUMBER;
}

CELL pop(CTX* c) {
	CELL n = T(c).value;
	T(c).prev = c->dstack;
	T(c).type = T_FREE;
	c->dstack = STACK(c).next;

	return n;
}

// TODO: hold function to add node to stacks/lists other than data stack
// TODO: release function to return node to free doubly linked list
//void hold(NODE* nodes, HALF* src, HALF* dest, CELL val) {
//	HALF node = *src;
//	*src = nodes[node].next;
//	nodes[node].next = *dest;
//	*dest = node;
//	nodes[node].type = T_NUMBER;
//	nodes[node].value = val;
//}
//
//CELL release(NODE* nodes, HALF* src, HALF* dest) {
//	HALF node = *src;
//	*src = nodes[node].next;
//	nodes[node].next = *dest;
//	*dest = node;
//	return nodes[node].value;
//}

// TODO: rpush and rpop
//#define rpush(c, val)		hold((NODE*)c, &c->fstack.next, &c->rstack, val)
//#define rpop(c)					release((NODE*)c, &c->rstack, &c->fstack.next)

// Primitives

void _add(CTX* c) { CELL x = pop(c); T(c).value += x; }
void _dec(CTX* c) { T(c).value--; }

void _gt(CTX* c) { CELL x = pop(c); T(c).value = T(c).value > x; }

void _dup(CTX* c) { push(c, T(c).value); }
void _swap(CTX* c) { CELL t = S(c).value; S(c).value = T(c).value; T(c).value = t; }
