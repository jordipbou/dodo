#include<stdint.h>

#define NIL				0

typedef int8_t		BYTE;
typedef intptr_t	CELL;		// 16, 32 or 64 bits depending on system

#define CAR(pair)					(*((CELL*)pair))
#define CDR(pair)					(*(((CELL*)pair) + 1))
#define VALUE(cell)				((cell) >> 2)
#define TYPE(cell)				((cell) & 3)
#define AS(type, cell)		(((cell) << 2) | type)
#define IS(type, cell)		(TYPE(cell) == type)

typedef struct {
	CELL err;
	BYTE* bottom, * here;
	CELL there, top, dstack, free, rstack;
	CELL dict;
} CTX;

typedef void (*FUNC)(CTX*);

#define FREE								0
#define ATOM								1
#define PRIMITIVE						2
#define LIST								3

#define BRANCH							0
#define RECURSION						1
#define WORD								2
#define QUOTATION						3

#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))
#define RESERVED(ctx)				((ctx->there) - ((CELL)ctx->here))

#define ERR_NOT_ENOUGH_MEMORY		-1
#define ERR_STACK_OVERFLOW				-2
#define ERR_STACK_UNDERFLOW			-3

CELL depth(CELL p) { CELL c = 0; while (p != NIL) { c++; p = CDR(p); } return c; }

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;	
	ctx->err = NIL;
	ctx->bottom = ctx->here = ((BYTE*)ctx) + sizeof(CTX);
	ctx->there = ALIGN((CELL)ctx->bottom, 2*sizeof(CELL));
	ctx->free = ctx->top = ALIGN((CELL)(block + size - 2*sizeof(CELL) - 1), 2*sizeof(CELL));

	for (CELL p = ctx->there; p <= ctx->top; p += 2*sizeof(CELL)) {
		CDR(p) = p == ctx->there ? NIL : p - 2*sizeof(CELL);
		CAR(p) = p == ctx->top ? NIL : p + 2*sizeof(CELL);
	}

	ctx->dict = ctx->dstack = ctx->rstack = NIL;

	return ctx;
}

CELL cons(CTX* ctx, CELL car, CELL cdr) {
	if (ctx->free == NIL) { ctx->err = ERR_STACK_OVERFLOW; return NIL; }
	CELL c = ctx->free;
	ctx->free = CDR(ctx->free);
	CAR(c) = car;
	CDR(c) = cdr;
	return c;
}

CELL clone(CTX* ctx, CELL pair) {
	if (pair == NIL) { 
		return NIL;
	} else if (IS(ATOM, CAR(pair))) { 
		return cons(ctx, CAR(pair), clone(ctx, CDR(pair)));
	} else { 
		return cons(ctx, AS(LIST, clone(ctx, VALUE(CAR(pair)))), clone(ctx, CDR(pair))); 
	}
}

CELL eq(CTX* ctx, CELL p1, CELL p2) {
	if (TYPE(CAR(p1)) != TYPE(CAR(p2))) return 0;
	if (IS(ATOM, CAR(p1))) return VALUE(CAR(p1)) == VALUE(CAR(p2));
	else {
		CELL e1 = VALUE(CAR(p1));
		CELL e2 = VALUE(CAR(p2));
		while (e1 != NIL) {
			if (e2 == NIL || eq(ctx, e1, e2) != 1) return 0;
			e1 = CDR(e1);
			e2 = CDR(e2);
		}
		return 1;
	}
}

CELL reclaim(CTX* ctx, CELL pair) {
	if (pair == NIL) return NIL;
	if (IS(LIST, CAR(pair))) { 
		CELL p = VALUE(CAR(pair)); while ((p = reclaim(ctx, p)) != NIL) {}
	}
	CELL tail = CDR(pair);
	CAR(ctx->free) = pair;
	CDR(pair) = ctx->free;
	CAR(pair) = NIL;
	ctx->free = pair;
	return tail;
}

#define PUSH(ctx, v)		ctx->dstack = cons(ctx, v, ctx->dstack);
#define TOS(ctx)				CAR(ctx->dstack)
#define SOS(ctx)				CAR(CDR(ctx->dstack))
#define POP(ctx, v)			CELL v = TOS(ctx); ctx->dstack = reclaim(ctx, ctx->dstack);

void _dup(CTX* ctx) {
	if (ctx->dstack == NIL) { ctx->err = ERR_STACK_UNDERFLOW; return; }
	if (IS(ATOM, CAR(ctx->dstack))) { PUSH(ctx, CAR(ctx->dstack)); }
	else { PUSH(ctx, AS(LIST, clone(ctx, VALUE(CAR(ctx->dstack))))); }
}

void _swap(CTX* ctx) { POP(ctx, t); POP(ctx, s); PUSH(ctx, t); PUSH(ctx, s); }

#define BINOP(op)		POP(ctx, t); POP(ctx, s); PUSH(ctx, AS(ATOM, VALUE(s) op VALUE(t)));

void _add(CTX* ctx) { BINOP(+); }
void _sub(CTX* ctx) { BINOP(-); }
void _mul(CTX* ctx) { BINOP(*); }
void _div(CTX* ctx) { BINOP(/); }
void _mod(CTX* ctx) { BINOP(%); }

void _gt(CTX* ctx) { BINOP(>); }
void _lt(CTX* ctx) { BINOP(<); }
void _eq(CTX* ctx) { POP(ctx, t); POP(ctx, s); PUSH(ctx, eq(ctx, s, t)); }
void _neq(CTX* ctx) { POP(ctx, t); POP(ctx, s); PUSH(ctx, !eq(ctx, s, t)); }

void _and(CTX* ctx) { BINOP(&&); }
void _or(CTX* ctx) { BINOP(||); }
void _not(CTX* ctx) { TOS(ctx) = AS(ATOM, !VALUE(TOS(ctx))); }

#define PRIM(ctx, prim, cdr)			cons(ctx, AS(PRIMITIVE, cdr), (CELL)prim)
#define COND(ctx, true, false)		cons(ctx, AS(LIST, true), AS(BRANCH, false))
#define REC(ctx, cdr)							cons(ctx, AS(LIST, NIL), AS(RECURSION, cdr))

void inner(CTX* ctx, CELL xlist) {
	CELL ip = xlist;
	while(ctx->err == NIL && ip != NIL) {
		switch(TYPE(CAR(ip))) {
			case ATOM: PUSH(ctx, CAR(ip)); ip = CDR(ip); break;
			case PRIMITIVE:	((FUNC)(CDR(ip)))(ctx);	ip = VALUE(CAR(ip)); break;
			case LIST:
				switch(TYPE(CDR(ip))) {
					case BRANCH: 
						ip = VALUE(TOS(ctx)) ? VALUE(CAR(ip)) : VALUE(CDR(ip));
						ctx->dstack = reclaim(ctx, ctx->dstack);
						break;
					case RECURSION: inner(ctx, xlist);	ip = VALUE(CDR(ip)); break;
					case WORD: inner(ctx, VALUE(CAR(ip))); ip = VALUE(CDR(ip));	break;
					case QUOTATION: /* TODO */	break;
			}
		}
	}
}

////#ifdef _WIN32
////  #include <conio.h>
////#else
////	#include <unistd.h>
////	#include <termios.h>
////#endif
//
////// Source code for getch is taken from:
////// Crossline readline (https://github.com/jcwangxp/Crossline).
////// It's a fantastic readline cross-platform replacement, but only getch was
////// needed and there's no need to include everything else.
////#ifdef _WIN32	// Windows
////int dodo_getch (void) {	fflush (stdout); return _getch(); }
////#else
////int dodo_getch ()
////{
////	char ch = 0;
////	struct termios old_term, cur_term;
////	fflush (stdout);
////	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
////	cur_term = old_term;
////	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
////	cur_term.c_cc[VMIN] = 1;
////	cur_term.c_cc[VTIME] = 0;
////	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
////	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
////	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
////	return ch;
////}
////#endif
////
////void _key(CTX* ctx) { PUSH(ctx, dodo_getch()); }
////void _emit(CTX* ctx) { 
////	CELL K = TOS(ctx); 
////	POP(ctx); 
////	if (K == 127) { printf ("\b \b"); } 
////	else { printf ("%c", (char)K); }
////}
//
////void _loop(CTX* ctx) {
////}
////
////void _print(CTX* ctx) {
////}
////
////void _eval(CTX* ctx) {
////}
//
//void push_stack(CTX* ctx) {
//	ctx->stacks.next = cons(ctx, T_LIST, (CELL)AS(T_LIST, REF((&ctx->stacks))), NEXT((&ctx->stacks)));
//	ctx->stacks.stack = NIL;
//}
//
//void stack_to_list(CTX* ctx) {
//	PAIR* list = REF((&ctx->stacks));
//	ctx->stacks = *(NEXT((&ctx->stacks)));
//	ctx->stacks.ref = (CELL)cons(ctx, T_LIST, (CELL)list, REF((&ctx->stacks)));
//}
//
////void _read(CTX* ctx) {
////	BYTE K;
////	push_stack(ctx);
////	do {
////		K = dodo_getch();
////		if (K == 10 || K == 13) {
////			stack_to_list(ctx);
////		} else {
////			PUSH(ctx, K);
////		}
////	} while (1);
////}
//
//BYTE* allot(CTX* ctx, CELL bytes) {
//	BYTE* here = ctx->here;
//	if (bytes == 0) { 
//		return here;
//	} else if (bytes < 0) {
//		if ((ctx->here + bytes) > BOTTOM(ctx)) ctx->here += bytes;
//		else ctx->here = BOTTOM(ctx);
//		while ((ctx->there - 1) >= PALIGN(ctx->here)) { reclaim(ctx, --ctx->there); }
//	} else /* bytes > 0 */ {
//		while (RESERVED(ctx) < bytes && ctx->there < ctx->top) {
//			if (IS(T_FREE, ctx->there)) {
//				if (ctx->there->prev != NIL) {
//					ctx->there->prev->next = AS(T_FREE, NEXT(ctx->there));
//				}
//				ctx->there++;
//			} else {
//				ctx->err = ERR_NOT_ENOUGH_MEMORY;
//				return here;
//			}
//		}
//		if (RESERVED(ctx) >= bytes)	ctx->here += bytes;
//		else ctx->err = ERR_NOT_ENOUGH_MEMORY;
//	}
//	return here;
//}
//
//void align(CTX* ctx) { allot(ctx, ALIGN(ctx->here, sizeof(CELL)) - ((CELL)ctx->here)); }
//
//#define NFA(w)		(REF(REF(w)))
//#define DFA(w)		(REF(NEXT(REF(w))))
//#define CFA(w)		(NEXT(NEXT(REF(w))))
//#define COUNT(s)	(*((CELL*)(((BYTE*)s) - sizeof(CELL))))
//
//BYTE* compile_str(CTX* ctx, BYTE* str, CELL len) {
//	align(ctx);
//	BYTE* cstr = allot(ctx, sizeof(CELL) + len + 1);
//	*((CELL*)cstr) = len;
//	for (CELL i = 0; i < len; i++) {
//		cstr[sizeof(CELL) + i] = str[i];
//	}
//	cstr[sizeof(CELL) + len] = 0;
//	return cstr + sizeof(CELL);
//}
//
//PAIR* header(CTX* ctx, BYTE* name, CELL nlen) {
//	BYTE* str = compile_str(ctx, name, nlen);
//
//	return 
//		cons(ctx, T_ATOM, 
//			(CELL)cons(ctx, T_ATOM, (CELL)str,
//						cons(ctx, T_ATOM, (CELL)ctx->here, NIL)), 
//			NIL);
//}
//
//PAIR* body(CTX* ctx, PAIR* word, PAIR* cfa) {
//	PAIR* old_cfa = CFA(word);
//	NEXT(REF(word))->next = AS(T_ATOM, cfa);
//	while (old_cfa != NIL) { old_cfa = reclaim(ctx, old_cfa); }
//	return word;
//}
//
//PAIR* reveal(CTX* ctx, PAIR* header) {
//	header->next = AS(T_ATOM, ctx->dict);
//	ctx->dict = header;
//	return header;
//}
//
//#define IS_IMMEDIATE(w)		(TYPE(w->ref) & 1)
//
//void _immediate(CTX* ctx) {
//	ctx->dict->ref = AS(1, REF(ctx->dict));
//}
//
////PAIR* find(CTX* ctx, BYTE* name, CELL nlen) {
////	// TODO
////}
//
//CTX* dodo(CTX* x) {
//	reveal(x, body(x, header(x, "+", 1), cons(x, T_PRIMITIVE, (CELL)&_add, NIL)));
//	reveal(x, body(x, header(x, "-", 1), cons(x, T_PRIMITIVE, (CELL)&_sub, NIL)));
//	reveal(x, body(x, header(x, "*", 1), cons(x, T_PRIMITIVE, (CELL)&_mul, NIL)));
//	reveal(x, body(x, header(x, "/", 1), cons(x, T_PRIMITIVE, (CELL)&_div, NIL)));
//	reveal(x, body(x, header(x, "mod", 3), cons(x, T_PRIMITIVE, (CELL)&_mod, NIL)));
//
//	reveal(x, body(x, header(x, ">", 1), cons(x, T_PRIMITIVE, (CELL)&_gt, NIL)));
//	reveal(x, body(x, header(x, "<", 1), cons(x, T_PRIMITIVE, (CELL)&_lt, NIL)));
//	reveal(x, body(x, header(x, "=", 1), cons(x, T_PRIMITIVE, (CELL)&_eq, NIL)));
//	reveal(x, body(x, header(x, "<>", 2), cons(x, T_PRIMITIVE, (CELL)&_neq, NIL)));
//
//	reveal(x, body(x, header(x, "and", 3), cons(x, T_PRIMITIVE, (CELL)&_and, NIL)));
//	reveal(x, body(x, header(x, "or", 2), cons(x, T_PRIMITIVE, (CELL)&_or, NIL)));
//	reveal(x, body(x, header(x, "invert", 3), cons(x, T_PRIMITIVE, (CELL)&_not, NIL)));
//
//	reveal(x, body(x, header(x, "dup", 3), cons(x, T_PRIMITIVE, (CELL)&_dup, NIL)));
//	reveal(x, body(x, header(x, "swap", 4), cons(x, T_PRIMITIVE, (CELL)&_swap, NIL)));
//
//	//reveal(x, body(x, header(ctx, "key", 3), cons(x, T_PRIMITIVE, (CELL)&_key, NIL)));
//	//reveal(x, body(x, header(ctx, "emit", 4), cons(x, T_PRIMITIVE, (CELL)&_emit, NIL)));
//
//	return x;
//}
//
//// ----------------------------------------------------------------------------
//
//
