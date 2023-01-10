#include<stdint.h>
#include<string.h>

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

#define TYPE(pair)					(pair == NIL ? NIL : ((CELL)pair->next) & 7)
#define NEXT(pair)					((PAIR*)(((CELL)pair->next) & -8))

#define T_FREE							0
#define T_NUM								1
#define T_PRIM							2
#define T_WORD							4
#define T_LIST							7

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

void and(CTX* ctx) { SOS(ctx) = SOS(ctx) && TOS(ctx); POP(ctx); }
void or(CTX* ctx) { SOS(ctx) = SOS(ctx) || TOS(ctx); POP(ctx); }
void not(CTX* ctx) { TOS(ctx) = !TOS(ctx); }

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

PAIR* header(CTX* ctx, BYTE* name, CELL nlen, CELL type) {
	align(ctx); 
	BYTE* str = ctx->here;
	*((CELL*)ctx->here) = nlen;
	allot(ctx, sizeof(CELL) + nlen + 1);

	for (CELL i = 0; i < nlen; i++) {
		str[sizeof(CELL) + i] = name[i];	
	}
	str[sizeof(CELL) + nlen] = 0;

	return cons(ctx, (CELL)ncons(ctx, (CELL)str, ncons(ctx, (CELL)ctx->here, NIL)), type, NIL);
}

#define NFA(word)						((PAIR*)(word->value))
#define DFA(word)						(NEXT(NFA(word)))
#define CFA(word)						(NEXT(DFA(word)))

PAIR* body(CTX* ctx, PAIR* word, PAIR* cfa) {
	PAIR* old_cfa = CFA(word);
	DFA(word)->next = (PAIR*)(((CELL)cfa) | T_NUM);
	while (old_cfa != NIL) { old_cfa = reclaim(ctx, old_cfa); }
	return word;
}

PAIR* reveal(CTX* ctx, PAIR* header) {
	header->next = ctx->dict;
	ctx->dict = header;
	return header;
}

#define T_IMMEDIATE							1
#define IS_IMMEDIATE(word)			(TYPE(word) & T_IMMEDIATE)

void immediate(CTX* ctx) {
	ctx->dict->next = (PAIR*)(((CELL)ctx->dict->next) | T_IMMEDIATE);
}

PAIR* find(CTX* ctx, BYTE* name, CELL nlen) {
	PAIR* word = ctx->dict;
	// TODO: Change strcmp here to custom string comparison (starting on name length)
	while (word != NIL && strcmp(name, ((BYTE*)(NFA(word)->value) + sizeof(CELL)))) {
		word = NEXT(word);
	}
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
					ctx->dstack = lcons(ctx, (CELL)copy(ctx, (PAIR*)ip->value), ctx->dstack);
					ip = NEXT(ip);
					break;
			}
		}
	}
}

CTX* dodo(CTX* ctx) {
	reveal(ctx, body(ctx, header(ctx, "+", 1, T_PRIM), pcons(ctx, &add, NIL)));
	reveal(ctx, body(ctx, header(ctx, "-", 1, T_PRIM), pcons(ctx, &sub, NIL)));
	reveal(ctx, body(ctx, header(ctx, "*", 1, T_PRIM), pcons(ctx, &mul, NIL)));
	reveal(ctx, body(ctx, header(ctx, "/", 1, T_PRIM), pcons(ctx, &div, NIL)));
	reveal(ctx, body(ctx, header(ctx, "mod", 3, T_PRIM), pcons(ctx, &mod, NIL)));

	reveal(ctx, body(ctx, header(ctx, ">", 1, T_PRIM), pcons(ctx, &gt, NIL)));
	reveal(ctx, body(ctx, header(ctx, "<", 1, T_PRIM), pcons(ctx, &lt, NIL)));
	reveal(ctx, body(ctx, header(ctx, "=", 1, T_PRIM), pcons(ctx, &eq, NIL)));
	reveal(ctx, body(ctx, header(ctx, "<>", 2, T_PRIM), pcons(ctx, &neq, NIL)));

	reveal(ctx, body(ctx, header(ctx, "and", 3, T_PRIM), pcons(ctx, &and, NIL)));
	reveal(ctx, body(ctx, header(ctx, "or", 2, T_PRIM), pcons(ctx, &or, NIL)));
	reveal(ctx, body(ctx, header(ctx, "invert", 3, T_PRIM), pcons(ctx, &not, NIL)));

	reveal(ctx, body(ctx, header(ctx, "dup", 3, T_PRIM), pcons(ctx, &dup, NIL)));
	reveal(ctx, body(ctx, header(ctx, "swap", 4, T_PRIM), pcons(ctx, &swap, NIL)));

	return ctx;
}

// TODO: Outer interpreter to be able to load a forth file 
