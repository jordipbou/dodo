#include<stdint.h>

typedef int8_t		BYTE;
typedef intptr_t	CELL;

typedef struct _PAIR {
	struct _PAIR* next;
	CELL value;
} PAIR;

#define TYPE(pair)					(((CELL)pair->next) & 7)
#define NEXT(pair)					((PAIR*)(((CELL)pair->next) & -8))

#define T_FREE							0
#define T_NUM								1
#define T_PRIM							2
#define T_WORD							3
#define T_LIST							4

typedef struct {
	CELL err;
	BYTE* bottom, * here;
	PAIR* there, * top;
	PAIR* dstack, * free, * rstack;
} CTX;

typedef void (*FUNC)(CTX*);

#define NIL									0
#define ALIGN(addr, bound)	((CELL)addr + ((CELL)bound-1)) & ~((CELL)bound-1)

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;	
	ctx->bottom = ctx->here = ((BYTE*)ctx) + sizeof(CTX);
	ctx->there = (PAIR*)(ALIGN(ctx->here, sizeof(PAIR)));
	ctx->free = ctx->top = (PAIR*)(ALIGN(block + size - sizeof(PAIR) - 1, sizeof(PAIR)));

	for (PAIR* p = ctx->there; p <= ctx->top; p++) {
		p->next = p == ctx->there ? NIL : p - 1;
		p->value = (CELL)(p == ctx->top ? NIL : p + 1);
	}

	ctx->rstack = ctx->dstack = NIL;
	ctx->err = NIL;

	return ctx;
}

#define RESERVED(ctx)			(((CELL)ctx->there) - ((CELL)ctx->here))

void allot(CTX* ctx, CELL bytes) {
	if (bytes == 0) return;
	if (bytes < 0) {
		if ((ctx->here - bytes) > ctx->bottom) ctx->here -= bytes;
		else ctx->here = ctx->bottom;
		// TODO: Available pairs should be returned to end of free doubly linked list !!
	} else {
		while (RESERVED(ctx) < bytes) {
			if (TYPE(ctx->there) == T_FREE) {
				((PAIR*)ctx->there->value)->next = ctx->there->next;
				ctx->there++;
			} else {
				// ERROR: Not enough contiguous nodes to free (Not enough memory)
			}
		}
		ctx->here += bytes;
	}
}

void align(CTX* ctx) { 
	allot(ctx, ALIGN(ctx->here, sizeof(CELL)) - ((CELL)ctx->here)); 
}

CELL depth(PAIR* p) {	
	CELL c = 0; 
	while (1) { if (p == NIL) { return c; } else { c++; p = NEXT(p); } }
}

PAIR* cons(CTX* ctx, CELL value, CELL type, PAIR* next) {
	PAIR* p = ctx->free;
	ctx->free = p->next;
	p->next = (PAIR*)((CELL)next | type);
	p->value = value;
	return p;
}

#define ncons(ctx, value, next)		cons(ctx, value, T_NUM, next)
#define pcons(ctx, value, next)		cons(ctx, value, T_PRIM, next)
#define wcons(ctx, value, next)		cons(ctx, value, T_WORD, next)
#define lcons(ctx, value, next)		cons(ctx, value, T_LIST, next)

PAIR* reclaim(CTX* ctx, PAIR* p) {
	if (p == NIL) return NIL;				
	PAIR* tail = NEXT(p);
	p->next = ctx->free;
	p->value = NIL;
	ctx->free->value = (CELL)p;
	ctx->free = p;
	return tail;
}

// --------------------------------------------------------------------------

// TODO: I don't really see a benefit of using different operators for lists
// and for ctx->dstack, as don't know how to use them inside DODO. Better to
// use a stack of stacks, or just a default stack (dstack) and all operations
// work on there.

#define BINOP(name, op) \
	PAIR* name(CTX* ctx, PAIR* list) { \
		PAIR* tail = NEXT(list); \
		tail->value = tail->value op list->value; \
		return reclaim(ctx, list); \
	} \
	void _##name(CTX* ctx) { ctx->dstack = name(ctx, ctx->dstack); }

BINOP(add, +);
BINOP(sub, -);
BINOP(mul, *);
BINOP(div, /);
BINOP(mod, %);

BINOP(gt, >);
BINOP(lt, <);
BINOP(eq, ==);
BINOP(neq, !=);
BINOP(gte, >=);
BINOP(lte, <=);

BINOP(and, &&);
BINOP(or, ||);

// TODO: Dup it's not deep copying if its a list !!!

PAIR* dup(CTX* ctx, PAIR* list) { return cons(ctx, list->value, TYPE(list), list); }
void _dup(CTX* ctx) { ctx->dstack = dup(ctx, ctx->dstack); }

#define W_PRIMITIVE					1
#define W_COLON_DEF					3

#define F_NON_IMMEDIATE			1
#define F_IMMEDIATE					3

#define NFA(w)		((PAIR*)(w->value))
#define DFA(w)		(NEXT(NFA(w)))
#define CFA(w)		(NEXT(DFA(w)))

PAIR* create(CTX* ctx, BYTE* name, CELL nlen, PAIR* body) {
	BYTE* here = ctx->here;

	allot(ctx, sizeof(CELL) + nlen + 1);

	*((CELL*)here) = nlen;
	for (CELL i = 0; i < nlen; i++) {
		here[sizeof(CELL) + i] = name[i];	
	}
	here[sizeof(CELL) + nlen] = 0;

	align(ctx);

	PAIR* cfa = body == NIL ? cons(ctx, (CELL)ctx->here, W_COLON_DEF, NIL) : body;
	PAIR* dfa = ncons(ctx, (CELL)ctx->here, cfa);
	// TODO: nfa should not point to here, but be a string node itself that
	// can be freed?
	PAIR* nfa = cons(ctx, (CELL)here, F_NON_IMMEDIATE, dfa);
	PAIR* word = wcons(ctx, (CELL)nfa, NIL);

	return word;
}

void inner(CTX* ctx, PAIR* ip) {
	while(ctx->err == NIL) {
		if (ip == NIL) {
			if (ctx->rstack == NIL) return;
			else {
				ip = (PAIR*)ctx->rstack->value;
				ctx->rstack = reclaim(ctx, ctx->rstack);
			}
		} else {
			switch (TYPE(ip)) {
				case T_NUM: 
					ctx->dstack = ncons(ctx, ip->value, ctx->dstack);
					ip = NEXT(ip);
					break;
				case T_WORD: 
					ctx->rstack = ncons(ctx, (CELL)NEXT(ip), ctx->rstack);
					ip = (PAIR*)ip->value;
					break;
				case T_PRIM:
					((FUNC)ip->value)(ctx);
					ip = NEXT(ip);
					break;
				case T_LIST: 
					// TODO: List should be deep copied here, shouldn't it?
					// No, it should just be moved!!
					ctx->dstack = cons(ctx, ip->value, T_LIST, ctx->dstack);
					break;
			}
		}
	}
}
