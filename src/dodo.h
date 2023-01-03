#include<stdint.h>

typedef int8_t		BYTE;
typedef intptr_t	CELL;

typedef struct _PAIR {
	struct _PAIR* next;
	CELL value;
} PAIR;

typedef struct {
	BYTE* bottom, * here;
	PAIR* there, * top;
	PAIR* dstack, * free, * rstack;
} CTX;

typedef void (*FUNC)(CTX*);

#define NIL									0
#define ALIGN(addr, bound)	((CELL)addr + ((CELL)bound-1)) & ~((CELL)bound-1)

#define TYPE(pair)					(((CELL)pair->next) & 3)
#define UNTYPED(addr)				(PAIR*)(((CELL)addr) & -4)
PAIR* typed(PAIR* p, CELL t) { p->next = (PAIR*)((CELL)UNTYPED(p->next) | t); return p; }

#define T_FREE							0
#define T_NUM								1
#define T_WORD							2
#define T_LIST							3

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;	
	ctx->bottom = ctx->here = ((BYTE*)ctx) + sizeof(CTX);
	ctx->there = (PAIR*)(ALIGN(ctx->here, sizeof(PAIR)));
	ctx->free = ctx->top = (PAIR*)(ALIGN(block + size - sizeof(PAIR) - 1, sizeof(PAIR)));

	for (PAIR* p = ctx->there; p <= ctx->top; p++) {
		p->next = p == ctx->there ? NIL : p - 1;
		p->value = (CELL)(p == ctx->top ? NIL : p + 1);
	}

	ctx->dstack = NIL;

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

CELL depth(PAIR* p) {
	CELL c = 0; 
	while (1) { 
		if (p == NIL) { return c; } 
		else { c++; p = UNTYPED(p->next); }
	}
}

PAIR* cons(CTX* ctx, PAIR* next, CELL value) {
	PAIR* p = ctx->free;
	ctx->free = p->next;
	p->next = next;
	p->value = value;
	return p;
}

PAIR* reclaim(CTX* ctx, PAIR* p) {
	if (p == NIL) return NIL;				
	PAIR* tail = UNTYPED(p->next);
	p->next = ctx->free;
	p->value = NIL;
	ctx->free->value = (CELL)p;
	ctx->free = p;
	return tail;
}

#define BINOP(name, op) \
	PAIR* name(CTX* ctx, PAIR* list) { \
		PAIR* tail = UNTYPED(list->next); \
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

// TODO: How to deal with input buffer?

#define W_PRIMITIVE					1
#define W_COLON_DEFINITION	3

#define F_NON_IMMEDIATE			1
#define F_IMMEDIATE					3

#define NFA(w)		((PAIR*)(w->value))
#define CFA(w)		(UNTYPED(NFA(w)->next))
#define XT(w)			((PAIR*)(CFA(w)->value))
#define DFA(w)		(UNTYPED(CFA(w)->next))

PAIR* create(CTX* ctx, BYTE* name, CELL nlen) {
	BYTE* here = ctx->here;

	allot(ctx, sizeof(CELL) + nlen + 1);
	*((CELL*)here) = nlen;
	for (CELL i = 0; i < nlen; i++) {
		here[sizeof(CELL) + i] = name[i];	
	}
	here[sizeof(CELL) + nlen] = 0;

	// TODO: DFA should be CELL aligned !!

	PAIR* dfa = typed(cons(ctx, NIL, (CELL)ctx->here), T_NUM);
	// TODO: body should be [PUSH DFA] [RET] NIL
	PAIR* xt = typed(cons(ctx, NIL, dfa->value), T_NUM);
	PAIR* cfa = typed(cons(ctx, dfa, (CELL)xt), W_COLON_DEFINITION);
	PAIR* nfa = typed(cons(ctx, cfa, (CELL)here), F_NON_IMMEDIATE);
	PAIR* word = cons(ctx, NIL, (CELL)nfa);

	return typed(word, T_WORD);
}

void inner(CTX* ctx, PAIR* list) {
	while(list != NIL) {
		if (TYPE(list) == T_NUM) {
			ctx->dstack = typed(cons(ctx, ctx->dstack, list->value), T_NUM);
			list = UNTYPED(list->next);
		} else if (TYPE(list) == T_WORD) {
			if (TYPE(CFA(list)) == W_PRIMITIVE) {
				((FUNC)(CFA(list)->value))(ctx);
				list = UNTYPED(list->next);
			} else {
				ctx->rstack = typed(cons(ctx, ctx->rstack, (CELL)(UNTYPED(list->next))), T_NUM);
				list = (PAIR*)(CFA(list)->value);
			}
		} else if (TYPE(list) == T_LIST) {
			// TODO: execute or push the quotation?
		}
	}
}
