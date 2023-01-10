#include<stdint.h>

typedef int8_t		BYTE;
typedef intptr_t	CELL;

typedef struct _PAIR {
	struct _PAIR* next;
	CELL value;	
} PAIR;

typedef struct {
	CELL err;
	BYTE* bottom, * here;
	PAIR* there, * top;
	PAIR* dstack, * free, * rstack;
	PAIR* dict;
} CTX;

typedef void (*FUNC)(CTX*);

#define NIL									0
#define ALIGN(addr, bound)	(((CELL)addr + ((CELL)bound-1)) & ~((CELL)bound-1))

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;	
	ctx->bottom = ctx->here = ((BYTE*)ctx) + sizeof(CTX);
	ctx->there = (PAIR*)(ALIGN(ctx->here, sizeof(PAIR)));
	ctx->free = ctx->top = (PAIR*)(ALIGN(block + size - sizeof(PAIR) - 1, sizeof(PAIR)));

	for (PAIR* p = ctx->there; p <= ctx->top; p++) {
		p->next = p == ctx->there ? NIL : p - 1;
		p->value = (CELL)(p == ctx->top ? NIL : p + 1);
	}

	ctx->dict = ctx->rstack = ctx->dstack = NIL;
	ctx->err = NIL;

	return ctx;
}

#define RESERVED(ctx)				(((CELL)ctx->there) - ((CELL)ctx->here))

#define TYPE(pair)					(((CELL)pair->next) & 7)
#define NEXT(pair)					((PAIR*)(((CELL)pair->next) & -8))

#define T_FREE							0
#define T_NUM								1
#define T_PRIM							2
#define T_WORD							3
#define T_LIST							4

#define E_NOT_ENOUGH_MEMORY		-1

void allot(CTX* ctx, CELL bytes) {
	if (bytes == 0) return;
	if (bytes < 0) {
		if ((ctx->here + bytes) > ctx->bottom) ctx->here += bytes;
		else ctx->here = ctx->bottom;
		while ((ctx->there - 1) >= (PAIR*)ALIGN(ctx->here, sizeof(PAIR))) {
			ctx->free->value = (CELL)--ctx->there;
			ctx->there->next = ctx->free;
			ctx->there->value = NIL;
			ctx->free = ctx->there;
		}
	} else {
		while (RESERVED(ctx) < bytes && ctx->there < ctx->top) {
			if (TYPE(ctx->there) == T_FREE) {
				if (ctx->there->value != NIL) {
					((PAIR*)ctx->there->value)->next = ctx->there->next;
				}
				ctx->there++;
			} else {
				ctx->err = E_NOT_ENOUGH_MEMORY;
				return;
			}
		}
		if (RESERVED(ctx) >= bytes)	ctx->here += bytes;
		else ctx->err = E_NOT_ENOUGH_MEMORY;
	}
}

void align(CTX* ctx) { allot(ctx, ALIGN(ctx->here, sizeof(CELL)) - ((CELL)ctx->here)); }

CELL depth(PAIR* p) {	
	CELL c = 0; 
	while (1) { if (p == NIL) { return c; } else { c++; p = NEXT(p); } }
}

PAIR* cons(CTX* x, CELL value, CELL type, PAIR* next) {
	if (x->free == NIL) return NIL;
	PAIR* p = x->free;
	x->free = p->next;
	p->next = (PAIR*)((CELL)next | type);
	p->value = value;
	return p;
}

#define ncons(ctx, value, next)		cons(ctx, value, T_NUM, next)
#define pcons(ctx, value, next)		cons(ctx, (CELL)value, T_PRIM, next)
#define wcons(ctx, value, next)		cons(ctx, (CELL)value, T_WORD, next)
#define lcons(ctx, value, next)		cons(ctx, (CELL)value, T_LIST, next)

PAIR* reclaim(CTX* x, PAIR* p) {
	if (p == NIL) return NIL;
	if (TYPE(p) == T_LIST) {
		PAIR* item = (PAIR*)p->value;
		while (item != NIL) { item = reclaim(x, item); }
	}
	PAIR* tail = NEXT(p);
	p->next = x->free;
	p->value = NIL;
	if (p->next != NIL) { p->next->value = (CELL)p; }
	x->free = p;
	return tail;
}

PAIR* copy(CTX* ctx, PAIR* list) {
	if (list == NIL) {
		return NIL;
	} else if (TYPE(list) == T_LIST) {
		return cons(ctx, (CELL)copy(ctx, (PAIR*)list->value), T_LIST, copy(ctx, NEXT(list)));
	} else {
		return cons(ctx, list->value, TYPE(list), copy(ctx, NEXT(list)));
	}
}

#define TOS(ctx)			(ctx->dstack->value)
#define SOS(ctx)			(NEXT(ctx->dstack)->value)
#define POP(ctx)			(ctx->dstack = reclaim(ctx, ctx->dstack))

void add(CTX* ctx) { SOS(ctx) = SOS(ctx) + TOS(ctx); POP(ctx); }
void sub(CTX* ctx) { SOS(ctx) = SOS(ctx) - TOS(ctx); POP(ctx); }
void mul(CTX* ctx) { SOS(ctx) = SOS(ctx) * TOS(ctx); POP(ctx); }
void div(CTX* ctx) { SOS(ctx) = SOS(ctx) / TOS(ctx); POP(ctx); }
void mod(CTX* ctx) { SOS(ctx) = SOS(ctx) % TOS(ctx); POP(ctx); }

void gt(CTX* ctx) { SOS(ctx) = SOS(ctx) > TOS(ctx); POP(ctx); }
void lt(CTX* ctx) { SOS(ctx) = SOS(ctx) < TOS(ctx); POP(ctx); }
void eq(CTX* ctx) { SOS(ctx) = SOS(ctx) == TOS(ctx); POP(ctx); }
void neq(CTX* ctx) { SOS(ctx) = SOS(ctx) != TOS(ctx); POP(ctx); }
void gte(CTX* ctx) { SOS(ctx) = SOS(ctx) >= TOS(ctx); POP(ctx); }
void lte(CTX* ctx) { SOS(ctx) = SOS(ctx) <= TOS(ctx); POP(ctx); }

void and(CTX* ctx) { SOS(ctx) = SOS(ctx) && TOS(ctx); POP(ctx); }
void or(CTX* ctx) { SOS(ctx) = SOS(ctx) || TOS(ctx); POP(ctx); }

void dup(CTX* ctx) { 
	if (TYPE(ctx->dstack) == T_LIST) {
		ctx->dstack = cons(ctx, (CELL)copy(ctx, (PAIR*)TOS(ctx)), T_LIST, ctx->dstack);
	} else {
		ctx->dstack = cons(ctx, TOS(ctx), TYPE(ctx->dstack), ctx->dstack);
	}
}

void swap(CTX* ctx) {
	PAIR* sos = NEXT(ctx->dstack);
	ctx->dstack->next = (PAIR*)(((CELL)NEXT(sos)) | TYPE(ctx->dstack));
	sos->next = (PAIR*)(((CELL)ctx->dstack) | TYPE(sos));
	ctx->dstack = sos;
}

// CORRECTLY TESTED UNTIL HERE -----------------------------------------------

//#define W_PRIMITIVE					1
//#define W_COLON_DEF					3
//
//#define F_NON_IMMEDIATE			1
//#define F_IMMEDIATE					3
//
//PAIR* header(CTX* ctx, BYTE* name, CELL nlen, PAIR* body) {
//	BYTE* here = ctx->here;
//
//	allot(ctx, sizeof(CELL) + nlen + 1);
//
//	*((CELL*)here) = nlen;
//	for (CELL i = 0; i < nlen; i++) {
//		here[sizeof(CELL) + i] = name[i];	
//	}
//	here[sizeof(CELL) + nlen] = 0;
//
//	align(ctx);
//
//	PAIR* cfa = body == NIL ? cons(ctx, (CELL)ctx->here, W_COLON_DEF, NIL) : body;
//	// TODO: It's dfa needed or just pushing it on CFA its enough?
//	PAIR* dfa = ncons(ctx, (CELL)ctx->here, cfa);
//	// TODO: nfa should not point to here, but be a string node itself that
//	// can be freed?
//	PAIR* nfa = cons(ctx, (CELL)here, F_NON_IMMEDIATE, dfa);
//	PAIR* word = wcons(ctx, (CELL)nfa, NIL);
//
//	return word;
//}
//
//void reveal(CTX* ctx, PAIR* header) {
//	header->next = ctx->dict;
//	ctx->dict = header;
//}
//
//void create(CTX* ctx, BYTE* name, CELL nlen, PAIR* body) {
//	reveal(ctx, header(ctx, name, nlen, body));
//}
//
//#define NFA(word)						((PAIR*)(word->value))
//#define DFA(word)						(NEXT(NFA(word)))
//#define CFA(word)						(NEXT(DFA(word)))
//
//void body(CTX* ctx, PAIR* word, PAIR* cfa) {
//	PAIR* old_cfa = CFA(word);
//	DFA(word)->next = (PAIR*)(((CELL)cfa) | 1);
//	while (old_cfa != NIL) { old_cfa = reclaim(ctx, old_cfa); }
//}
//
//void inner(CTX* ctx, PAIR* ip) {
//	while(ctx->err == NIL) {
//		if (ip == NIL) {
//			if (ctx->rstack == NIL) return;
//			else {
//				ip = (PAIR*)ctx->rstack->value;
//				ctx->rstack = reclaim(ctx, ctx->rstack);
//			}
//		} else {
//			switch (TYPE(ip)) {
//				case T_NUM: 
//					ctx->dstack = ncons(ctx, ip->value, ctx->dstack);
//					ip = NEXT(ip);
//					break;
//				case T_WORD: 
//					ctx->rstack = ncons(ctx, (CELL)NEXT(ip), ctx->rstack);
//					ip = (PAIR*)ip->value;
//					break;
//				case T_PRIM:
//					((FUNC)ip->value)(ctx);
//					ip = NEXT(ip);
//					break;
//				case T_LIST: 
//					// TODO: List should be deep copied here, shouldn't it?
//					// No, it should just be moved!!
//					ctx->dstack = lcons(ctx, ip->value, ctx->dstack);
//					ip = NEXT(ip);
//					break;
//			}
//		}
//	}
//}
