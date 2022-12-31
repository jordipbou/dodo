#include<stdint.h>

typedef int8_t BYTE;
typedef intptr_t CELL;		// Data stack item's size (could be 16, 32 or 64 bits)

typedef struct {
	CELL car;					// First item, also next on lists
	CELL cdr;					// Second item, also prev on free stack
} PAIR;

typedef struct {
	CELL status;								// Status and error codes
	CELL bottom, here;					// Pointers to lowest and highest allocated bytes
	CELL there, top;						// Pointers to lowest and highest nodes
	CELL dstack, free, rstack;	// Pointers to data, free and return stacks
	CELL dict;									// Pointer to last defined word
	CELL Ax;
} CTX;

typedef void (*FUNC)(CTX*);

#define S_OK								0

#define T_FREE							0
#define T_LIT								1
#define T_WORD							2
#define T_QUOT							3

#define NIL									0

#define ALIGN(addr, bound)	((CELL)addr + ((CELL)bound-1)) & ~((CELL)bound-1)

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;	
	ctx->bottom = ctx->here = (CELL)((BYTE*)ctx) + sizeof(CTX);
	ctx->there = (CELL)ALIGN(((BYTE*)ctx) + sizeof(CTX), sizeof(PAIR));
	ctx->top = (CELL)ALIGN((((BYTE*)ctx) + size) - sizeof(PAIR) - 1, sizeof(PAIR));

	for (CELL i = ctx->there; i <= ctx->top; i += sizeof(PAIR)) {
		PAIR* p = (PAIR*)i;
		p->car = i == ctx->there ? NIL : i - sizeof(PAIR);
		p->cdr = i == ctx->top ? NIL : i + sizeof(PAIR);
	}

	ctx->dstack = ctx->rstack = ctx->dict = NIL;
	ctx->free = ctx->top;

	ctx->status = S_OK;

	return ctx;
}

CELL depth(CELL p) { 
	CELL c = 0; 
	while (1) { 
		if (p == 0) { return c; } 
		else { c++; p = ((PAIR*)p)->car & -4; } 
	}
}

CELL cons(CTX* ctx, CELL car, CELL type, CELL cdr) {
	PAIR* p = (PAIR*)ctx->free;
	ctx->free = ((PAIR*)ctx->free)->car & -4;
	p->car = car | type;
	p->cdr = cdr;
	return (CELL)p;
}

void push(CTX* ctx, CELL n) { ctx->dstack = cons(ctx, ctx->dstack, T_LIT, n); }
void rpush(CTX* ctx, CELL n) { ctx->rstack = cons(ctx, ctx->rstack, T_LIT, n); }

CELL reclaim(CTX* ctx, CELL* src) {
	PAIR* p = (PAIR*)*src;
	CELL v = p->cdr;
	CELL n = p->car & -4;
	p->car = ctx->free;
	((PAIR*)ctx->free)->cdr = (CELL)p;
	ctx->free = (CELL)p;
	*src = n;
	return v;
}

CELL pop(CTX* ctx) { return reclaim(ctx, &ctx->dstack); }
CELL rpop(CTX* ctx) { return reclaim(ctx, &ctx->rstack); }

#define RESERVED(ctx)			(ctx->there - ctx->here)

void _allot(CTX* ctx) {
	CELL bytes = pop(ctx);
	if (bytes == 0) return;
	if (bytes < 0) {
		if ((ctx->here - bytes) > ctx->bottom) ctx->here -= bytes;
		else ctx->here = ctx->bottom;
		// TODO: Available pairs should be returned to end of free doubly linked list !!
	} else {
		while (RESERVED(ctx) < bytes) {
			if ((((PAIR*)ctx->there)->car & 3) == T_FREE) {
				((PAIR*)(((PAIR*)ctx->there)->cdr))->car = ((PAIR*)ctx->there)->car;
				ctx->there += sizeof(PAIR);	
			} else {
				// ERROR: Not enough contiguous nodes to free (Not enough memory)
			}
		}
		ctx->here += bytes;
	}
}

//void _node_align(CTX* c) { c->here = c->there; }
//
//// TODO: align 
//
//#define T(c)								T_NODE(c)->value
//#define S(c)								S_NODE(c)->value
//
//void _dup(CTX* c) { push(c, T(c)); }
//void _swap(CTX* c) { CELL t = S(c); S(c) = T(c); T(c) = t; }
//
//void _add(CTX* c) { CELL x = pop(c); T(c) += x; }
//
//void _dec(CTX* c) { T(c)--; }
//
//void _gt(CTX* c) { CELL x = pop(c); T(c) = T(c) > x; }
//
//void _header(CTX* c) {
//	CELL name_length = pop(c);
//	BYTE* name_addr = (BYTE*)pop(c);
//
//	node_align(c);
//
//	NODE* name = NODE_AT(c, c->here);
//	push(c, sizeof(NODE)); allot(c);
//
//	name->next = NIL;
//	name->type = T_ARRAY;
//	name->len = name_length;
//
//	push(c, name_length - 6);
//	allot(c);
//
//	node_align(c);
//
//	for (CELL i = 0; i < ((((BYTE*)c) + c->here) - &name->data); i++) {
//		if (i < name_length) { name->data[i] = name_addr[i]; }
//		else { name->data[i] = 0; }
//	}
//
//	NODE* word = NODE_AT(c, c->here);
//	push(c, sizeof(NODE)); allot(c);
//	word->next = NIL;			// Will point to c->dict after reveal
//	word->type = T_DICT;
//	word->name = REL(name);
//	word->code = NIL;
//
//	// Data will start after word node, but must be allocated
//}
//
////void cons(CTX* c, HALF next) {
////	// TODO: Takes one node from free stack (the one previous to stack)
////	// TODO: Sets next and type to T_LIST
////	// TODO: Pushes address to stack
////}
////
////void forget(CTX* c, HALF addr) {
////	// TODO: Returns nodes recursively (takes type into account) to free stack
////}
////
////// Primitives
////
////// TODO: Check stack errors ?!
////
////void _add(CTX* c) { CELL x = pop(c); T(c).value += x; }
////void _sub(CTX* c) { CELL x = pop(c); T(c).value -= x; }
////void _mul(CTX* c) { CELL x = pop(c); T(c).value *= x; }
////void _div(CTX* c) { CELL x = pop(c); T(c).value /= x; }
////void _mod(CTX* c) { CELL x = pop(c); T(c).value %= x; }
////
////void _dec(CTX* c) { T(c).value--; }
////void _inc(CTX* c) { T(c).value++; }
////
////void _gt(CTX* c) { CELL x = pop(c); T(c).value = T(c).value > x; }
////void _lt(CTX* c) { CELL x = pop(c); T(c).value = T(c).value < x; }
////void _eq(CTX* c) { CELL x = pop(c); T(c).value = T(c).value == x; }
////void _neq(CTX *c) { CELL x = pop(c); T(c).value = T(c).value != x; }
////
////void _dup(CTX* c) { push(c, T(c).value); }
////void _swap(CTX* c) { CELL t = S(c).value; S(c).value = T(c).value; T(c).value = t; }
////
////void _ret(CTX* c) { c->status = S_RET; }
////
////void _store(CTX* c) { CELL i = pop(c); CELL x = pop(c); *((CELL*)(((BYTE*)c) + i)) = x; }
////void _fetch(CTX* c) { CELL i = pop(c); push(c, *((CELL*)(((BYTE*)c) + i))); }
////
////void _comma(CTX* c) { 
////	CELL x = pop(c); 
////	HALF here = c->here;
////	allot(c, sizeof(CELL));
////	*((CELL*)(c + here)) = x;
////}
////
////// Definition creation
////
////void _header(CTX* c, BYTE* name, HALF type) {
////	//align2(c);
////	NODE* word = (((NODE*)c) + c->here);
////	allot(c, sizeof(NODE));
////	word->next = c->dict;	
////	word->type = type;
////	word->len = strlen(name);
////	// TODO: Name is fixed to less than 6 here
////	for (CELL i = 0; i < 6; i++) {
////		if (i < word->len) {
////			word->data[i] = name[i];
////		} else {
////			word->data[i] = 0;
////		}
////	}
////}
////
////// Inner interpreter
////
////// DOUBT: Right now, I'm not using a return stack, as every call is made
////// directly in C, is that correct? Can a standard FORTH be implemented
////// that way?
////// It seems the only real problem will be with control flow words. Only!
////
////void _execute(CTX*c, NODE* word);
////
////#define DFA(w)		((CELL*)(ALIGN(w + sizeof(CELL) + w->len - 6, sizeof(CELL))))
////
////void _docol(CTX* c, CELL* dfa) {
////	NODE* nodes = (NODE*)c;
////	HALF* word = (HALF*)dfa;	
////	while (c->status == S_OK) {
////		_execute(c, &NODE_AT(c, *word));
////		word++;
////	}
////}
////
////void _dovar(CTX* c, CELL* dfa) {
////	push(c, (CELL)dfa);
////}
////
////void _doconst(CTX* c, CELL* dfa) {
////	push(c, *dfa);
////}
////
////void _execute(CTX* c, NODE* word) {
////	if (word->type && T_F_PRIMITIVE) {
////		((FUNC)(DFA(word)))(c);
////	} else if (word->type && T_F_DOCOL) { 
////		_docol(c, DFA(word));
////	} else if (word->type && T_F_DOVAR) {
////		_dovar(c, DFA(word));
////	} else if (word->type && T_F_DOCONST) {
////		_doconst(c, DFA(word));
////	}
////}
