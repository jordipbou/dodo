#include<stdint.h>

#define NIL				0

typedef int8_t		BYTE;
typedef int16_t		WORD;
typedef intptr_t	CELL;		// 16, 32 or 64 bits depending on system
typedef CELL			tCELL;	// tagged cell

typedef struct _PAIR {
	tCELL	next;
	union {
		CELL value;
		struct _PAIR* prev;
		struct _PAIR* stack;
		tCELL ref;
	};
} PAIR;

typedef struct {
	CELL err;
	BYTE* here;
	PAIR* there, * top, * free, * dict;
	PAIR stacks;
} CTX;

typedef void (*FUNC)(CTX*);

#define T_FREE								0
#define T_ATOM								1
#define T_PRIMITIVE						2
#define T_LIST								3
#define ST_BRANCH							0
#define ST_RECURSION					1
#define ST_WORD								2
#define ST_QUOTATION					3

#define TYPE(t_cell)					((CELL)((t_cell) & 3))
#define AS(type, link)				((tCELL)((CELL)(link) | type))
#define IS(type, pair)				(TYPE(pair->next) == type)
#define NEXT(pair)						((PAIR*)(((CELL)pair->next) & -4))
#define REF(pair)							((PAIR*)(((CELL)pair->ref) & -4))

#define BOTTOM(ctx)					(((BYTE*)ctx) + sizeof(CTX))

#define ALIGN(addr, bound)	(((CELL)addr + ((CELL)bound-1)) & ~((CELL)bound-1))
#define PALIGN(addr)				((PAIR*)(ALIGN(addr, sizeof(PAIR))))
#define RESERVED(ctx)				(((CELL)ctx->there) - ((CELL)ctx->here))

#define ERR_NOT_ENOUGH_MEMORY		-1

CELL depth(PAIR* p) {	CELL c = 0; while (p != NIL) { c++; p = NEXT(p); } return c; }

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;	
	ctx->here = BOTTOM(ctx);
	ctx->there = PALIGN(BOTTOM(ctx));
	ctx->free = ctx->top = PALIGN(block + size - sizeof(PAIR) - 1);

	for (PAIR* p = ctx->there; p <= ctx->top; p++) {
		p->prev = p == ctx->top ? NIL : p + 1;
		p->next = AS(T_FREE, p == ctx->there ? NIL : p - 1);
	}

	ctx->stacks.next = NIL;
	ctx->stacks.ref = NIL;
	ctx->dict = NIL;
	ctx->err = NIL;

	return ctx;
}

PAIR* alloc(CTX* ctx, CELL type, CELL value, PAIR* next) {
	if (ctx->free == NIL) { ctx->err = ERR_NOT_ENOUGH_MEMORY;	return NIL;	}
	PAIR* p = ctx->free;
	ctx->free = NEXT(p);
	p->next = AS(type, next);
	p->value = value;
	return p;
}

PAIR* reclaim(CTX* ctx, PAIR* p) {
	if (p == NIL) return NIL;
	if (IS(T_LIST, p)) { PAIR* i = REF(p); while (i != NIL) { i = reclaim(ctx, i); }	}
	PAIR* tail = NEXT(p);
	p->next = AS(T_FREE, ctx->free);
	p->value = NIL;
	if (p->next != NIL) { NEXT(p)->prev = p; }
	ctx->free = p;
	return tail;
}

PAIR* clone(CTX* ctx, PAIR* list) {
	if (list == NIL) { 
		return NIL; 
	} else if (IS(T_ATOM, list)) { 
		return alloc(ctx, T_ATOM, list->value, clone(ctx, NEXT(list))); 
	}	else { 
		return alloc(ctx, T_LIST, (CELL)clone(ctx, REF(list)), clone(ctx, NEXT(list)));
	}
}

void allot(CTX* ctx, CELL bytes) {
	if (bytes == 0) { 
		return;
	} else if (bytes < 0) {
		if ((ctx->here + bytes) > BOTTOM(ctx)) ctx->here += bytes;
		else ctx->here = BOTTOM(ctx);
		while ((ctx->there - 1) >= PALIGN(ctx->here)) { reclaim(ctx, --ctx->there); }
	} else /* bytes > 0 */ {
		while (RESERVED(ctx) < bytes && ctx->there < ctx->top) {
			if (IS(T_FREE, ctx->there)) {
				if (ctx->there->prev != NIL) {
					ctx->there->prev->next = AS(T_FREE, NEXT(ctx->there));
				}
				ctx->there++;
			} else {
				ctx->err = ERR_NOT_ENOUGH_MEMORY;
				return;
			}
		}
		if (RESERVED(ctx) >= bytes)	ctx->here += bytes;
		else ctx->err = ERR_NOT_ENOUGH_MEMORY;
	}
}

void align(CTX* ctx) { allot(ctx, ALIGN(ctx->here, sizeof(CELL)) - ((CELL)ctx->here)); }

#define TOS(ctx)				(ctx->stacks.stack->value)
#define SOS(ctx)				(NEXT(ctx->stacks.stack)->value)
#define POP(ctx)				(ctx->stacks.stack = reclaim(ctx, ctx->stacks.stack))
#define PUSH(ctx, t, v)	(ctx->stacks.stack = alloc(ctx, t, v, ctx->stacks.stack))

void inner(CTX* ctx, PAIR* xlist) {
	PAIR* ip = xlist;
	while(ctx->err == NIL && ip != NIL) {
		switch(TYPE(ip->next)) {
			case T_ATOM: PUSH(ctx, T_ATOM, ip->value); ip = NEXT(ip);	break;
			case T_PRIMITIVE:	((FUNC)ip->value)(ctx);	ip = NEXT(ip); break;
			case T_LIST:
				switch(TYPE(ip->ref)) {
					case ST_BRANCH:	ip = TOS(ctx) ? NEXT(ip) : REF(ip); POP(ctx); break;
					case ST_RECURSION: inner(ctx, xlist);	ip = NEXT(ip); break;
					case ST_WORD:	inner(ctx, REF(ip)); ip = NEXT(ip);	break;
					case ST_QUOTATION: /* TODO */	break;
				}
		}
	}
}

void _add(CTX* ctx) { SOS(ctx) = SOS(ctx) + TOS(ctx); POP(ctx); }
void _sub(CTX* ctx) { SOS(ctx) = SOS(ctx) - TOS(ctx); POP(ctx); }
void _mul(CTX* ctx) { SOS(ctx) = SOS(ctx) * TOS(ctx); POP(ctx); }
void _div(CTX* ctx) { SOS(ctx) = SOS(ctx) / TOS(ctx); POP(ctx); }
void _mod(CTX* ctx) { SOS(ctx) = SOS(ctx) % TOS(ctx); POP(ctx); }

void _gt(CTX* ctx) { SOS(ctx) = SOS(ctx) > TOS(ctx); POP(ctx); }
void _lt(CTX* ctx) { SOS(ctx) = SOS(ctx) < TOS(ctx); POP(ctx); }
void _eq(CTX* ctx) { SOS(ctx) = SOS(ctx) == TOS(ctx); POP(ctx); }
void _neq(CTX* ctx) { SOS(ctx) = SOS(ctx) != TOS(ctx); POP(ctx); }

void _and(CTX* ctx) { SOS(ctx) = SOS(ctx) && TOS(ctx); POP(ctx); }
void _or(CTX* ctx) { SOS(ctx) = SOS(ctx) || TOS(ctx); POP(ctx); }
void _not(CTX* ctx) { TOS(ctx) = !TOS(ctx); }

void _dup(CTX* ctx) { 
	if (IS(T_LIST, ctx->stacks.stack)) {
		PUSH(ctx, T_LIST, (CELL)clone(ctx, REF(ctx->stacks.stack)));
	} else {
		PUSH(ctx, TYPE(ctx->stacks.stack->next), TOS(ctx));
	}
}

void _swap(CTX* ctx) {
	PAIR* fos = ctx->stacks.stack;
	PAIR* sos = NEXT(ctx->stacks.stack);
	PAIR* tos = NEXT(NEXT(ctx->stacks.stack));
	ctx->stacks.stack = sos;
	ctx->stacks.stack->next = AS(TYPE(sos->next), fos);
	NEXT(ctx->stacks.stack)->next = AS(TYPE(fos->next), tos);
}

// ----------------------------------------------------------------------------

//#ifdef _WIN32
//  #include <conio.h>
//#else
//	#include <unistd.h>
//	#include <termios.h>
//#endif
//
//#define NFA(word)						((PAIR*)(word->value))
//#define DFA(word)						(NEXT(NFA(word)))
//#define CFA(word)						(NEXT(DFA(word)))
//
//PAIR* header(CTX* ctx, BYTE* name, CELL nlen, CELL type) {
//	align(ctx); 
//	BYTE* str = ctx->here;
//	allot(ctx, sizeof(CELL) + nlen + 1);
//	*((CELL*)str) = nlen;
//
//	for (CELL i = 0; i < nlen; i++) {
//		str[sizeof(CELL) + i] = name[i];	
//	}
//	str[sizeof(CELL) + nlen] = 0;
//
//	return walloc(ctx, (CELL)nalloc(ctx, (CELL)str, nalloc(ctx, (CELL)ctx->here, NIL)), NIL);
//}
//
//PAIR* body(CTX* ctx, PAIR* word, PAIR* cfa) {
//	PAIR* old_cfa = CFA(word);
//	DFA(word)->next = (PAIR*)(((CELL)cfa) | T_ATOM);
//	while (old_cfa != NIL) { old_cfa = reclaim(ctx, old_cfa); }
//	return word;
//}
//
//PAIR* reveal(CTX* ctx, PAIR* header) {
//	header->next = ctx->dict;
//	ctx->dict = header;
//	return header;
//}
//
//#define T_IMMEDIATE							1
//#define IS_IMMEDIATE(word)			(TYPE(word) & T_IMMEDIATE)
//
//void _immediate(CTX* ctx) {
//	ctx->dict->next = (PAIR*)(((CELL)ctx->dict->next) | T_IMMEDIATE);
//}
//
//PAIR* find(CTX* ctx, BYTE* name, CELL nlen) {
//	PAIR* word = ctx->dict;
//	// TODO: Change strcmp here to custom string comparison (starting on name length)
//	while (word != NIL && strcmp(name, ((BYTE*)(NFA(word)->value) + sizeof(CELL)))) {
//		word = NEXT(word);
//	}
//	return word;
//}
//
//// Source code for getch is taken from:
//// Crossline readline (https://github.com/jcwangxp/Crossline).
//// It's a fantastic readline cross-platform replacement, but only getch was
//// needed and there's no need to include everything else.
//#ifdef _WIN32	// Windows
//int dodo_getch (void) {	fflush (stdout); return _getch(); }
//#else
//int dodo_getch ()
//{
//	char ch = 0;
//	struct termios old_term, cur_term;
//	fflush (stdout);
//	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
//	cur_term = old_term;
//	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
//	cur_term.c_cc[VMIN] = 1;
//	cur_term.c_cc[VTIME] = 0;
//	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
//	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
//	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
//	return ch;
//}
//#endif
//
//void _key(CTX* ctx) { ctx->dstack = nalloc(ctx, dodo_getch(), ctx->dstack); }
//void _emit(CTX* ctx) { 
//	CELL K = TOS(ctx); 
//	POP(ctx); 
//	if (K == 127) { printf ("\b \b"); } 
//	else { printf ("%c", (char)K); }
//}
//
//CTX* dodo(CTX* ctx) {
//	reveal(ctx, body(ctx, header(ctx, "+", 1, T_PRIM), palloc(ctx, &_add, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "-", 1, T_PRIM), palloc(ctx, &_sub, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "*", 1, T_PRIM), palloc(ctx, &_mul, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "/", 1, T_PRIM), palloc(ctx, &_div, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "mod", 3, T_PRIM), palloc(ctx, &_mod, NIL)));
//
//	reveal(ctx, body(ctx, header(ctx, ">", 1, T_PRIM), palloc(ctx, &_gt, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "<", 1, T_PRIM), palloc(ctx, &_lt, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "=", 1, T_PRIM), palloc(ctx, &_eq, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "<>", 2, T_PRIM), palloc(ctx, &_neq, NIL)));
//
//	reveal(ctx, body(ctx, header(ctx, "and", 3, T_PRIM), palloc(ctx, &_and, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "or", 2, T_PRIM), palloc(ctx, &_or, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "invert", 3, T_PRIM), palloc(ctx, &_not, NIL)));
//
//	reveal(ctx, body(ctx, header(ctx, "dup", 3, T_PRIM), palloc(ctx, &_dup, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "swap", 4, T_PRIM), palloc(ctx, &_swap, NIL)));
//
//	reveal(ctx, body(ctx, header(ctx, "key", 3, T_PRIM), palloc(ctx, &_key, NIL)));
//	reveal(ctx, body(ctx, header(ctx, "emit", 4, T_PRIM), palloc(ctx, &_emit, NIL)));
//
//	return ctx;
//}
