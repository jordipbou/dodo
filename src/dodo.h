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
	WORD len;
	union {
		CELL prev;		// Doubly linked list node
		CELL value;		// List node
		struct {			// Dictionary node
			HALF key;		// Pointer to key string (array) node
			HALF val;		// Pointer to value node
		};						
		BYTE data[8];	// Array node
	};
} NODE;

#define T_NUMBER				0
#define T_LIST					8
#define T_ARRAY					16	
#define T_DICT					32	

#define T_F_PRIMITIVE		64
#define T_F_DOCOL				128
#define T_F_DOVAR				256
#define T_F_DOCONST			512
#define T_F_IMMEDIATE		1024

#define T_FREE					16384
#define T_NIL						32768

// Virtual machine context

typedef struct {
	CELL size;						// Size of block
	HALF bottom, here;		// Pointers to lowest and highest allocated bytes
	HALF there, top;			// Pointers to lowest and highest nodes
	HALF dstack, rstack;	// Pointers to first node in data and return stacks
	HALF cstack, xstack;	// Pointers to first node in control and exception stacks
	HALF dict, TIB;				// Pointers to dictionary list and terminal input buffer
	NODE fstack;					// Doubly linked list of free nodes (also acts as NIL)
} CTX;

#define NIL		(offsetof(CTX, fstack) / sizeof(NODE))

CTX* init(BYTE* block, CELL size) {
	CTX* c = (CTX*)block;	
	c->size = size;

	c->bottom = c->here = (HALF)sizeof(CTX);
	c->there = (HALF)(sizeof(CTX) / sizeof(NODE));
	c->top = (HALF)((size / sizeof(NODE)) - 1);

	NODE* nodes = (NODE*)c;
	for (HALF i = c->there; i <= c->top; i++) {
		nodes[i].type = T_FREE;
		nodes[i].next = i - 1;
		nodes[i].prev = i + 1;
	}
	nodes[c->there].next = NIL;
	nodes[c->top].prev = NIL;

	c->dstack = NIL;
	c->rstack = NIL;
	c->cstack = NIL;
	c->xstack = NIL;

	c->fstack.next = c->top;
	c->fstack.prev = c->there;
	c->fstack.type = T_NIL;

	return c;
}

// Memory management

#define NODE_AT(c, idx)		(((NODE*)c)[idx])
#define T(c)							NODE_AT(c, c->dstack)
#define S(c)							NODE_AT(c, T(c).next)

#define RESERVED(c)			((c->there * sizeof(NODE)) - c->here)

CELL length(CTX* c, HALF node) { 
	NODE* nodes = (NODE*)c;
	for (CELL c = 0;; c++) {
		if (node == NIL) { return c; }
		else { node = nodes[node].next; }
	}
}

#define FREE_NODES(c)		(length(c, c->fstack.next))
#define DEPTH(c)				(length(c, c->dstack))

void allot(CTX* c, HALF bytes) {
	if (bytes == 0) return;
	if (bytes < 0) {
		// TODO: Nodes should be returned to end of free doubly linked list !!
		if ((c->here + bytes) > c->bottom) c->here += bytes;
		else c->here = c->bottom;
	} else {
		while (RESERVED(c) < bytes) {
			if (NODE_AT(c, c->there).type & T_FREE) {
				NODE_AT(c, NODE_AT(c, c->there).prev).next = NIL;
				c->there++;	
				c->fstack.prev = c->there;
			} else {
				// ERROR: Not enough contiguous nodes to free (Not enough memory)
			}
		}
		c->here += bytes;
	}
}

void hold(NODE* nodes, HALF* src, HALF* dest, CELL val) {
	HALF node = *src;
	*src = nodes[node].next;
	nodes[node].next = *dest;
	*dest = node;
	nodes[node].type = T_NUMBER;
	nodes[node].val = val;
}

CELL release(NODE* nodes, HALF* src, HALF* dest) {
	HALF node = *src;
	*src = nodes[node].next;
	nodes[node].next = *dest;
	*dest = node;
	return nodes[node].val;
}

#define push(c, val)		hold((NODE*)c, &c->fstack.next, &c->dstack, val)
#define pop(c)					release((NODE*)c, &c->dstack, &c->fstack.next)
#define rpush(c, val)		hold((NODE*)c, &c->fstack.next, &c->rstack, val)
#define rpop(c)					release((NODE*)c, &c->rstack, &c->fstack.next)
#define cpush(c, val)		hold((NODE*)c, &c->fstack.next, &c->cstack, val)
#define cpop(c)					release((NODE*)c, &c->cstack, &c->fstack.next)
#define xpush(c, val)		hold((NODE*)c, &c->fstack.next, &c->xstack)
#define xpop(c)					release((NODE*)c, &c->xstack, &c->fstack.next)

// Primitives

void _add(CTX* c) { CELL x = pop(c); T(c).val += x; }
void _dec(CTX* c) { T(c).val--; }

void _gt(CTX* c) { CELL x = pop(c); T(c).val = T(c).val > x; }

void _dup(CTX* c) { push(c, T(c).val); }
void _swap(CTX* c) { CELL t = S(c).val; S(c).val = T(c).val; T(c).val = t; }

void _docol(CTX* c) {
}

void _dovar(CTX* c) {
}

void _doconst(CTX* c) {
}

//void _create(CTX* c) {
//	NODE* entry = &(((BYTE*)c)[c->here]);
//	// Should take name from input string !!!!
//	HALF name_len = strlen(name);
//	allot(c, sizeof(NODE) + name_len - 7 + sizeof(CELL));
//	// TODO: Check memory error
//	entry->next = c->dict;
//	entry->type = T_ENTRY;
//	entry->flags = F_DOVAR;
//	entry->name_len = name_len;
//	for (CELL i = 0; i < entry->name_len; i++) {
//		entry->data[i] = name[i];
//	}
//	entry->data[entry->name_len] = 0;
//}
