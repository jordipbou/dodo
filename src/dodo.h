#include<stdint.h>

#define NIL				0

typedef int8_t		BYTE;
typedef intptr_t	CELL;		// 16, 32 or 64 bits depending on system

#define CAR(pair)					(*((CELL*)pair))
#define CDR(pair)					(*(((CELL*)pair) + 1))
#define TYPE(pair)				((CDR(pair)) & 3)
#define NEXT(pair)				((CDR(pair)) & -4)
#define TAG(type, cell)		((cell) | type)

#define ATOM				0
#define PRIMITIVE		1
#define BRANCH			2
#define WORD				3

#define RECURSION		0

typedef struct {
	CELL err;
	BYTE* bottom, * here;
	CELL there, top, nodes, rstack;
	CELL dict;
} CTX;

typedef void (*FUNC)(CTX*);

#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))
#define RESERVED(ctx)				((ctx->there) - ((CELL)ctx->here))

#define ERR_NOT_ENOUGH_MEMORY		-1
#define ERR_STACK_OVERFLOW			-2
#define ERR_STACK_UNDERFLOW			-3

CELL height(CELL p) { CELL c = 0; while (p != NIL) { c++; p = CAR(p); } return c; }
CELL depth(CELL p) { CELL c = 0; while (p != NIL) { c++; p = CDR(p); } return c; }

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;	
	ctx->err = NIL;
	ctx->bottom = ctx->here = ((BYTE*)ctx) + sizeof(CTX);
	ctx->there = ALIGN((CELL)ctx->bottom, 2*sizeof(CELL));
	ctx->nodes = ctx->top = ALIGN((block + size - 2*sizeof(CELL) - 1), 2*sizeof(CELL));

	for (CELL p = ctx->there; p <= ctx->top; p += 2*sizeof(CELL)) {
		CAR(p) = p == ctx->there ? NIL : p - 2*sizeof(CELL);
		CDR(p) = p == ctx->top ? NIL : p + 2*sizeof(CELL);
	}

	ctx->dict = ctx->rstack = NIL;

	return ctx;
}

void push(CTX* ctx, CELL value) {
	if (CAR(ctx->nodes) == NIL) { ctx->err = ERR_STACK_OVERFLOW; return; }
	ctx->nodes = CAR(ctx->nodes);
	CAR(CDR(ctx->nodes)) = value;
}

CELL pop(CTX* ctx) {
	if (CDR(ctx->nodes) == NIL) { ctx->err = ERR_STACK_UNDERFLOW; return NIL; }
	CELL value = CAR(CDR(ctx->nodes));
	CAR(CDR(ctx->nodes)) = ctx->nodes;
	ctx->nodes = CDR(ctx->nodes);
	return value;
}

CELL cons(CTX* ctx, CELL car, CELL cdr) {
	if (CAR(ctx->nodes) == NIL) { ctx->err = ERR_STACK_OVERFLOW; return NIL; }
	CELL p = CAR(ctx->nodes);
	CAR(ctx->nodes) = CAR(CAR(ctx->nodes));
	if (CAR(ctx->nodes) != NIL) CDR(CAR(ctx->nodes)) = ctx->nodes;
	CAR(p) = car;
	CDR(p) = cdr;
	return p;
}

#define DSTACK(ctx)			CDR(ctx->nodes)
#define TOS(ctx)				CAR(DSTACK(ctx))
#define SOS(ctx)				CAR(CDR(DSTACK(ctx)))

void _dup(CTX* ctx) { push(ctx, TOS(ctx)); }
void _swap(CTX* ctx) { CELL t = TOS(ctx); TOS(ctx) = SOS(ctx); SOS(ctx) = t; }

#define BINOP(op)		CELL t = pop(ctx); TOS(ctx) = TOS(ctx) op t;

void _add(CTX* ctx) { BINOP(+); }
void _sub(CTX* ctx) { BINOP(-); }
void _mul(CTX* ctx) { BINOP(*); }
void _div(CTX* ctx) { BINOP(/); }
void _mod(CTX* ctx) { BINOP(%); }

void _gt(CTX* ctx) { BINOP(>); }
void _lt(CTX* ctx) { BINOP(<); }
void _eq(CTX* ctx) { BINOP(==); }
void _neq(CTX* ctx) { BINOP(!=); }

void _and(CTX* ctx) { BINOP(&&); }
void _or(CTX* ctx) { BINOP(||); }
void _not(CTX* ctx) { TOS(ctx) = !TOS(ctx); }

#define PRIM(ctx, p, cdr)		cons(ctx, (CELL)p, TAG(PRIMITIVE, cdr))
#define COND(ctx, t, f)			cons(ctx, t, TAG(BRANCH, f))

void inner(CTX* ctx, CELL xlist) {
	CELL ip = xlist;
	while(ctx->err == NIL && ip != NIL) {
		switch(TYPE(ip)) {
			case ATOM: push(ctx, CAR(ip)); ip = CDR(ip); break;
			case PRIMITIVE:	
				if (CAR(ip) == RECURSION) {	inner(ctx, xlist); }
				else { ((FUNC)CAR(ip))(ctx); }
				ip = NEXT(ip);
				break;
			case BRANCH: ip = pop(ctx) ? CAR(ip) : NEXT(ip); break;
			case WORD: inner(ctx, CAR(ip)); break;
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
