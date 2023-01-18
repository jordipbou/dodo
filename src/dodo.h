#include<stdint.h>

#define NIL				0

typedef int8_t		BYTE;
typedef int16_t		WORD;
typedef intptr_t	CELL;		// 16, 32 or 64 bits depending on system

typedef CELL			tCELL;	// tagged cell

#define LINK(t_cell)					((PAIR*)((t_cell) & -4))
#define TYPE(t_cell)					((CELL)((t_cell) & 3))
#define AS(type, link)				((tCELL)((CELL)(link) | type))

#define T_FREE								0
#define T_ATOM								1
#define T_PRIMITIVE						2
#define T_LIST								3

#define ST_BRANCH							0
#define ST_RECURSION					1
#define ST_WORD								2
#define ST_QUOTATION					3

typedef struct _PAIR {
	tCELL	next;
	union {
		CELL value;
		struct _PAIR* prev;
		tCELL ref;
	};
} PAIR;

#define IS(type, pair)				(TYPE(pair->next) == type)
#define NEXT(pair)						(LINK(pair->next))
#define REF(pair)							(LINK(pair->ref))

CELL depth(PAIR* p) {	CELL c = 0; while (p != NIL) { c++; p = NEXT(p); } return c; }

typedef struct {
	CELL err;
	BYTE* bottom, * here;
	PAIR* there, * top;
	PAIR* dstack, * free, * rstack;
	PAIR* dict;
	PAIR* ip;
} CTX;


typedef void (*FUNC)(CTX*);

#define ALIGN(addr, bound)	(((CELL)addr + ((CELL)bound-1)) & ~((CELL)bound-1))
#define RESERVED(ctx)				(((CELL)ctx->there) - ((CELL)ctx->here))

#define ERR_NOT_ENOUGH_MEMORY		-1

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;	
	ctx->bottom = ctx->here = ((BYTE*)ctx) + sizeof(CTX);
	ctx->there = (PAIR*)ALIGN(ctx->here, sizeof(PAIR));
	ctx->free = ctx->top = (PAIR*)ALIGN(block + size - sizeof(PAIR) - 1, sizeof(PAIR));

	for (PAIR* p = ctx->there; p <= ctx->top; p++) {
		p->prev = p == ctx->top ? NIL : p + 1;
		p->next = AS(T_FREE, p == ctx->there ? NIL : p - 1);
	}

	ctx->ip = ctx->dict = ctx->rstack = ctx->dstack = NIL;
	ctx->err = NIL;

	return ctx;
}

void allot(CTX* ctx, CELL bytes) {
	if (bytes == 0) { 
		return;
	} else if (bytes < 0) {
		if ((ctx->here + bytes) > ctx->bottom) ctx->here += bytes;
		else ctx->here = ctx->bottom;
		while ((ctx->there - 1) >= (PAIR*)ALIGN(ctx->here, sizeof(PAIR))) {
			ctx->free->prev = --ctx->there;
			ctx->there->next = AS(T_FREE, ctx->free);
			ctx->there->prev = NIL;
			ctx->free = ctx->there;
		}
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

PAIR* alloc(CTX* ctx, CELL type, CELL value, PAIR* next) {
	if (ctx->free == NIL) {
		ctx->err = ERR_NOT_ENOUGH_MEMORY;
		return NIL;
	}
	PAIR* p = ctx->free;
	ctx->free = NEXT(p);
	p->next = AS(type, next);
	p->value = value;
	return p;
}

PAIR* reclaim(CTX* ctx, PAIR* p) {
	if (p == NIL) return NIL;
	if (IS(T_LIST, p)) {
		PAIR* item = REF(p);
		while (item != NIL) { item = reclaim(ctx, item); }
	}
	PAIR* tail = NEXT(p);
	p->next = AS(T_FREE, ctx->free);
	p->value = NIL;
	if (p->next != NIL) { NEXT(p)->prev = p; }
	ctx->free = p;

	return tail;
}

void inner(CTX* ctx) {
	while(ctx->err == NIL) {
		if (ctx->ip == NIL) {
			if (ctx->rstack == NIL) return;
			else {
				ctx->ip = NEXT(REF(ctx->rstack));
				ctx->rstack = reclaim(ctx, ctx->rstack);
			}
		} else {
			switch (TYPE(ctx->ip->next)) {
				case T_ATOM: 
					ctx->dstack = alloc(ctx, T_ATOM, ctx->ip->value, ctx->dstack);
					ctx->ip = NEXT(ctx->ip);
					break;
				case T_PRIMITIVE:
					((FUNC)ctx->ip->value)(ctx);
					ctx->ip = NEXT(ctx->ip);
					break;
				case T_LIST:
					switch (TYPE(ctx->ip->ref)) {
						case ST_BRANCH:
							if (ctx->dstack->value != 0) {
								ctx->ip = NEXT(ctx->ip);
							} else {
								ctx->ip = REF(ctx->ip);
							}
							ctx->dstack = reclaim(ctx, ctx->dstack);
							break;
						case ST_RECURSION: {
							// TODO: Check if NEXT is NIL to add tail recursion optimization
							PAIR* p = ctx->rstack;
							while (IS(T_LIST, p) && TYPE(p->ref) != ST_WORD && p != NIL) {
								p = NEXT(p);
							}
							ctx->rstack = alloc(ctx, T_LIST, (CELL)AS(ST_WORD, REF(p)), ctx->rstack);
							ctx->ip = REF(p);
							break; }
						case ST_WORD:
							ctx->rstack = alloc(ctx, T_LIST, (CELL)AS(ST_WORD, ctx->ip), ctx->rstack);
							ctx->ip = REF(ctx->ip);
							break;
						case ST_QUOTATION:
							// TODO
							break;
					}
			}
		}
	}
}

#define TOS(ctx)			(ctx->dstack->value)
#define SOS(ctx)			(NEXT(ctx->dstack)->value)
#define POP(ctx)			(ctx->dstack = reclaim(ctx, ctx->dstack))

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

//#define UNTYPED(c)				(((CELL)c) & -4)
//
//#define CAR(p)						((PAIR*)UNTYPED((CELL)p->value))
//#define CDR(p)						(p->next)

//#define CAAR(p)						CAR(CAR(p))
//#define CADR(p)						CAR(UNTYPED(CDR(p)))
//#define CDAR(p)						CDR(CAR(p))
//#define CDDR(p)						CDR(UNTYPED(CDR(p)))
//
//#define TYPE(p)						((CELL)CAR(p) & 3)
//#define SUBTYPE(p)				(CDR(p) & 3)
//
//#define FREE(p, next, prev)	{ p->value = (PAIR*)(UNTYPED((next))); p->next = (CELL)(prev); }
//#define ATOM(p, atom, next)	{ p->value = (PAIR*)(UNTYPED((next)) | 1); p->next = atom; }
//#define LIST(p, ptr, next)	{ p->value = (PAIR*)(UNTYPED((next)) | 3); p->next = (CELL)(ptr); }
//
//#define IS_FREE(p)				(TYPE(p) == 0)
//
//#define IS_ATOM(p)				(TYPE(p) == 1)
//#define IS_PRIMITIVE(p)		(TYPE(p) == 2)
//#define IS_LIST(p)				(TYPE(p) == 3)
//
//#define IS_BRANCH(p)			(IS_PRIMITIVE(p) && CDR(p) == 0)
//#define IS_RECURSION(p)		(IS_PRIMITIVE(p) && CDR(p) == 1)
//#define IS_WORD(p)				(IS_LIST(p) && SUBTYPE(p) == 1)
//#define IS_QUOTATION(p)		(IS_LIST(p) && SUBTYPE(p) == 3)

//#define PAIR_ALIGN(addr)		(PAIR*)(ALIGN(addr, sizeof(PAIR)))



// ----------------------------------------------------------------------------

//#include<stdint.h>
//#include<string.h>
//#include<stdio.h>
//
//#ifdef _WIN32
//  #include <conio.h>
//#else
//	#include <unistd.h>
//	#include <termios.h>
//#endif
//
//typedef int8_t		BYTE;
//typedef intptr_t	CELL;
//
//#define NIL									0						// Absence of value and truth
//	
//typedef struct _PAIR {
//	struct _PAIR* next;
//	CELL value;	
//} PAIR;
//
//#define UNTYPED(ptr)				((PAIR*)(((CELL)ptr) & -8))
//#define TYPED(ptr, type)		((PAIR*)((CELL)ptr | type))
//
//#define TYPE(pair)					(pair == NIL ? NIL : ((CELL)pair->next) & 7)
//#define NEXT(pair)					(UNTYPED(pair->next))
//
//#define T_FREE							0
//#define T_ATOM							1
//#define T_PRIM							2
//#define T_PRIM_IMM					3
//#define T_WORD							4
//#define T_WORD_IMM					5
//#define T_BRANCH						6
//#define T_LIST							7
//
//typedef struct {
//	CELL err;
//	BYTE* bottom, * here;
//	PAIR* there, * top;
//	PAIR* dstack, * free, * rstack;
//	PAIR* dict;
//	PAIR* ip;
//} CTX;
//
//typedef void (*FUNC)(CTX*);
//
//#define ALIGN(addr, bound)	(((CELL)addr + ((CELL)bound-1)) & ~((CELL)bound-1))
//#define RESERVED(ctx)				(((CELL)ctx->there) - ((CELL)ctx->here))
//
//#define ERR_NOT_ENOUGH_MEMORY		-1
//
//// Context initialization
//
//CTX* init(BYTE* block, CELL size) {
//	CTX* ctx = (CTX*)block;	
//	ctx->bottom = ctx->here = ((BYTE*)ctx) + sizeof(CTX);
//	ctx->there = (PAIR*)(ALIGN(ctx->here, sizeof(PAIR)));
//	ctx->free = ctx->top = (PAIR*)(ALIGN(block + size - sizeof(PAIR) - 1, sizeof(PAIR)));
//
//	for (PAIR* p = ctx->there; p <= ctx->top; p++) {
//		p->next = p == ctx->there ? NIL : p - 1;
//		p->value = (CELL)(p == ctx->top ? NIL : p + 1);
//	}
//
//	ctx->ip = ctx->dict = ctx->rstack = ctx->dstack = NIL;
//	ctx->err = NIL;
//
//	return ctx;
//}
//
//// Memory management
//
//void allot(CTX* ctx, CELL bytes) {
//	if (bytes == 0) { 
//		return;
//	} else if (bytes < 0) {
//		if ((ctx->here + bytes) > ctx->bottom) ctx->here += bytes;
//		else ctx->here = ctx->bottom;
//		while ((ctx->there - 1) >= (PAIR*)ALIGN(ctx->here, sizeof(PAIR))) {
//			ctx->free->value = (CELL)--ctx->there;
//			ctx->there->next = ctx->free;
//			ctx->there->value = NIL;
//			ctx->free = ctx->there;
//		}
//	} else /* bytes > 0 */ {
//		while (RESERVED(ctx) < bytes && ctx->there < ctx->top) {
//			if (TYPE(ctx->there) == T_FREE) {
//				if (ctx->there->value != NIL) {
//					((PAIR*)ctx->there->value)->next = ctx->there->next;
//				}
//				ctx->there++;
//			} else {
//				ctx->err = ERR_NOT_ENOUGH_MEMORY;
//				return;
//			}
//		}
//		if (RESERVED(ctx) >= bytes)	ctx->here += bytes;
//		else ctx->err = ERR_NOT_ENOUGH_MEMORY;
//	}
//}
//
//void align(CTX* ctx) { allot(ctx, ALIGN(ctx->here, sizeof(CELL)) - ((CELL)ctx->here)); }
//
//PAIR* alloc(CTX* ctx, CELL value, CELL type, PAIR* next) {
//	if (ctx->free == NIL) return NIL;
//	PAIR* p = ctx->free;
//	ctx->free = p->next;
//	p->next = (PAIR*)((CELL)next | type);
//	p->value = value;
//	return p;
//}
//
//PAIR* reclaim(CTX* ctx, PAIR* p) {
//	if (p == NIL) return NIL;
//	if (TYPE(p) == T_LIST) {
//		PAIR* item = (PAIR*)p->value;
//		while (item != NIL) { item = reclaim(ctx, item); }
//	}
//	PAIR* tail = NEXT(p);
//	p->next = ctx->free;
//	p->value = NIL;
//	if (p->next != NIL) { p->next->value = (CELL)p; }
//	ctx->free = p;
//
//	return tail;
//}
//
//// Inner interpreter
//
//void _rec(CTX* ctx);
//
//CELL depth(PAIR* p);
//
//void dump_stack(CTX* ctx) {
//	PAIR* s = ctx->dstack;
//	printf("<%ld> ", depth(ctx->dstack));
//	while (s != NIL) {
//		printf("%ld ", s->value);
//		s = NEXT(s);
//	}
//	printf("\n");
//}
//
//void dump_word(CTX* ctx, PAIR* cfa) {
//	PAIR* w = ctx->dict;
//	while (w != NIL) {
//		if (NEXT(NEXT((PAIR*)(w->value))) == cfa) {
//			printf("%s\n", ((BYTE*)w->NFA) + sizeof(CELL));
//			return;
//		}
//	}
//}
//
//// REST ------------------------------------------------------------------
//
//#define TOS(ctx)			(ctx->dstack->value)
//#define SOS(ctx)			(NEXT(ctx->dstack)->value)
//#define POP(ctx)			(ctx->dstack = reclaim(ctx, ctx->dstack))
//
//#define nalloc(ctx, value, next)		alloc(ctx, value, T_ATOM, next)
//#define palloc(ctx, value, next)		alloc(ctx, (CELL)value, T_PRIM, next)
//#define walloc(ctx, value, next)		alloc(ctx, (CELL)value, T_WORD, next)
//#define lalloc(ctx, value, next)		alloc(ctx, (CELL)value, T_LIST, next)
//
//PAIR* append(PAIR* list1, PAIR* list2) {
//	if (list1 == NIL) return list2;
//	PAIR* last = list1;
//	while (NEXT(last) != NIL) {
//		last = NEXT(last);
//	}
//	last->next = (PAIR*)((CELL)list2 | TYPE(last));
//
//	return list1;
//}
//
//#define balloc(ctx, tbranch, fbranch, next)	\
//	alloc(ctx, (CELL)append(fbranch, next), T_BRANCH, append(tbranch, next))
//
//PAIR* copy(CTX* ctx, PAIR* list) {
//	if (list == NIL) {
//		return NIL;
//	} else if (TYPE(list) == T_LIST) {
//		return alloc(ctx, (CELL)copy(ctx, (PAIR*)list->value), T_LIST, copy(ctx, NEXT(list)));
//	} else {
//		return alloc(ctx, list->value, TYPE(list), copy(ctx, NEXT(list)));
//	}
//}
//
//// Primitives
//
//void _add(CTX* ctx) { SOS(ctx) = SOS(ctx) + TOS(ctx); POP(ctx); }
//void _sub(CTX* ctx) { SOS(ctx) = SOS(ctx) - TOS(ctx); POP(ctx); }
//void _mul(CTX* ctx) { SOS(ctx) = SOS(ctx) * TOS(ctx); POP(ctx); }
//void _div(CTX* ctx) { SOS(ctx) = SOS(ctx) / TOS(ctx); POP(ctx); }
//void _mod(CTX* ctx) { SOS(ctx) = SOS(ctx) % TOS(ctx); POP(ctx); }
//
//void _gt(CTX* ctx) { SOS(ctx) = SOS(ctx) > TOS(ctx); POP(ctx); }
//void _lt(CTX* ctx) { SOS(ctx) = SOS(ctx) < TOS(ctx); POP(ctx); }
//void _eq(CTX* ctx) { SOS(ctx) = SOS(ctx) == TOS(ctx); POP(ctx); }
//void _neq(CTX* ctx) { SOS(ctx) = SOS(ctx) != TOS(ctx); POP(ctx); }
//
//void _and(CTX* ctx) { SOS(ctx) = SOS(ctx) && TOS(ctx); POP(ctx); }
//void _or(CTX* ctx) { SOS(ctx) = SOS(ctx) || TOS(ctx); POP(ctx); }
//void _not(CTX* ctx) { TOS(ctx) = !TOS(ctx); }
//
//void _dup(CTX* ctx) { 
//	if (TYPE(ctx->dstack) == T_LIST) {
//		ctx->dstack = alloc(ctx, (CELL)copy(ctx, (PAIR*)TOS(ctx)), T_LIST, ctx->dstack);
//	} else {
//		ctx->dstack = alloc(ctx, TOS(ctx), TYPE(ctx->dstack), ctx->dstack);
//	}
//}
//
//void _swap(CTX* ctx) {
//	PAIR* sos = NEXT(ctx->dstack);
//	ctx->dstack->next = (PAIR*)(((CELL)NEXT(sos)) | TYPE(ctx->dstack));
//	sos->next = (PAIR*)(((CELL)ctx->dstack) | TYPE(sos));
//	ctx->dstack = sos;
//}
//
//void _rec(CTX* ctx) {
//	PAIR* tor = ctx->rstack;
//	while (TYPE(tor) != T_WORD && tor != NIL) {
//		tor = NEXT(tor);
//	}
//
//	ctx->rstack = walloc(ctx, (CELL)tor->value, ctx->rstack);
//	ctx->ip = (PAIR*)tor->value;
//}
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
