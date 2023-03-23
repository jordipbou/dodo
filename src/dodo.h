#ifndef __DODO__
#define __DODO__

#include<inttypes.h>
#include<string.h>
#include<ctype.h>
#include<stdio.h>

typedef int8_t		B;
typedef intptr_t	C;

#define sC								(sizeof(C))
#define sP								(2*sC)

#define A(p)							(*((C*)p))
#define D(p)							(*(((C*)p) + 1))
#define N(p)							(D(p) & -4)
#define T(p)							(D(p) & 3)

enum Types { ATM, LST, PRM, WRD };

#define AS(t, n)					((n & -4) | t)
#define LK(p, n)					(D(p) = AS(T(p), n))

typedef struct {
	B *tb, *hr;
	C th, sz, fr, ds, err, rs, st, lt, in, free;
} X;

#define S(x)							(A(x->ds))
#define R(x)							(A(x->rs))

#define ALIGN(a, b)				((((C)a) + (b - 1)) & ~(b - 1))
#define BOTTOM(x)					(((B*)x) + sizeof(X))
#define TOP(x)						(ALIGN(((B*)x) + x->sz - sP - 1, sP))

X* init(B* bl, C sz) {
	X* x = (X*)bl;	
	x->sz = sz;
	x->hr = BOTTOM(x);
	x->th = ALIGN(BOTTOM(x), sP);
	x->ds = TOP(x); A(x->ds) = 0; D(x->ds) = AS(LST, 0);
	x->rs = x->ds - sP; A(x->rs) = 0; D(x->rs) = AS(LST, 0);
	x->fr = x->rs - sP;

	C p; for (x->free = -1, p = x->th; p <= x->fr; x->free++, p += sP) {
		A(p) = p == x->fr ? 0 : p + sP;
		D(p) = p == x->th ? 0 : p - sP;
	}

	x->tb = 0;
	x->in = x->st = x->err = x->lt = 0;

	return x;
}

// LIST/MANAGED MEMORY FUNCTIONS

C length(C l)	{ C c = 0; while (l) { c++; l = N(l); } return c; }
C cons(X* x, C a, C d) { x->free--; C p = x->fr; x->fr = D(x->fr); A(p) = a; D(p) = d; return p; }
#define arecl(x, p) ({ x->free++; C t = N(p); D(p) = x->fr; A(p) = 0; x->fr = p; t; })
C recl(X* x, C p) { if (T(p) == LST) while (A(p)) A(p) = recl(x, A(p)); return arecl(x, p); }
C clone(X* x, C p) { return p ? cons(x, T(p) == LST ? clone(x, A(p)) : A(p), AS(T(p), clone(x, N(p)))) : 0; }
C reverse(C p, C l) { return p ? ({ C t = N(p); D(p) = AS(T(p), l); reverse(t, p); }) : l; }

// ERRORS

enum Errors { E_EIB = -11, E_ZLN, E_EA, E_EL, E_EAA, E_UW, E_NER, E_NEM, E_DBZ, E_UF, E_OF, E_IP = 1 };

#define ERR(x, c, e)	if (c) { x->err = e; return; }

#define OF(x, n)			ERR(x, x->free < n, E_OF)
#define UF1(x)				ERR(x, S(x) == 0, E_UF)
#define UF2(x)				ERR(x, S(x) == 0 || N(S(x)) == 0, E_UF)
#define UF3(x)				ERR(x, !S(x) || !N(S(x)) || !N(N(S(x))), E_UF)

#define EA(x)					UF1(x); ERR(x, T(S(x)) != ATM, E_EA)
#define EL(x)					UF1(x); ERR(x, T(S(x)) != LST, E_EL)
#define EAA(x)				UF2(x); ERR(x, T(S(x)) != ATM || T(N(S(x))) != ATM, E_EAA)

#define EIB(x)				ERR(x, !x->tb, E_EIB)

// STACK PRIMITIVES

void duplicate(X* x) { UF1(x); OF(x, 1); S(x) = cons(x, T(S(x)) == LST ? clone(x, A(S(x))) : A(S(x)), AS(T(S(x)), S(x))); }
void swap(X* x) { UF2(x); C t = N(S(x)); LK(S(x), N(N(S(x)))); LK(t, S(x)); S(x) = t; }
void drop(X* x) { S(x) = recl(x, S(x)); }
void over(X* x) { UF2(x); OF(x, 1); S(x) = cons(x, T(N(S(x))) == LST ? clone(x, A(N(S(x)))) : A(N(S(x))), AS(T(N(S(x))), S(x))); }
void rot(X* x) { UF3(x); C t = N(N(S(x))); LK(N(S(x)), N(N(N(S(x))))); LK(t, S(x)); S(x) = t; }

// COMPARISON PRIMITIVES

#define BINOP(o) EAA(x); A(N(S(x))) = A(N(S(x))) o A(S(x)); S(x) = recl(x, S(x));

void gt(X* x) { BINOP(>) }
void lt(X* x) { BINOP(<) }
void eq(X* x) { BINOP(==) }
void neq(X* x) { BINOP(!=) }

// ARITHMETIC PRIMITIVES

void add(X* x) { BINOP(+) }
void sub(X* x) { BINOP(-) }
void mul(X* x) { BINOP(*) }
void division(X* x) { BINOP(/) }
void mod(X* x) { BINOP(%) }

// BIT PRIMITIVES

void and(X* x) { BINOP(&) }
void or(X* x) { BINOP(|) }
void invert(X* x) { UF1(x); A(S(x)) = ~A(S(x)); } 

// LIST PRIMITIVES (TODO: TEST!)

void empty(X* x) { OF(x, 1); S(x) = cons(x, 0, AS(LST, S(x))); }
void list_to_stack(X* x) { EL(x); C t = S(x); S(x) = N(S(x)); LK(t, x->ds); x->ds = t; }
void stack_to_list(X* x) {
	S(x) = reverse(S(x), 0);
	if (!N(x->ds)) { x->ds = cons(x, x->ds, AS(LST, 0)); }
	else { C t = x->ds; x->ds = N(x->ds); LK(t, A(x->ds)); A(x->ds) = t; }
}

// INNER INTERPRETER

typedef void (*FUNC)(X*);

void inner(X* x) {
	C t;
	do {
		while (!R(x) && N(x->rs)) { x->rs = recl(x, x->rs); }
		if (!R(x)) return;
		switch (T(R(x))) {
			case ATM: t = R(x); R(x) = N(R(x)); LK(t, S(x)); S(x) = t; break;
			case LST: t = R(x); R(x) = N(R(x)); LK(t, S(x)); S(x) = t; break;
			case PRM: ((FUNC)A(R(x)))(x); if (!x->err) R(x) = recl(x, R(x)); else x->err = 0; break;
			case WRD: /* Push word list into return stack */ break;
		}
	} while(1);
}

// EXECUTION PRIMITIVES

void exec_x(X* x) { UF1(x); // TODO: Check overflow for clone
	C c = T(S(x)) == LST ? clone(x, A(S(x))) : cons(x, A(S(x)), AS(T(S(x)), 0));
	if (R(x)) R(x) = recl(x, R(x)); x->rs = cons(x, c, AS(LST, x->rs)); x->err = 1;
}
void exec_i(X* x) { UF1(x); // TODO: Check overflow
	if (A(S(x)) == 0) { S(x) = recl(x, S(x)); return; }
	C t = S(x); S(x) = N(S(x)); R(x) = recl(x, R(x));
	if (T(t) == LST) { LK(t, x->rs); x->rs = t; x->err = 1; }
	else { LK(t, 0); x->rs = cons(x, t, AS(LST, 0)); x->err = 1; }
}
void branch(X* x) { UF3(x); if (!A(N(N(S(x))))) { swap(x); } drop(x); swap(x); drop(x); exec_i(x); }

void lbracket(X* x) { x->st = 0; }
void rbracket(X* x) { x->st = 1; }

// PARSING

#define TC(x)							(*(x->tb + x->in))

#define PARSE(x, cond)		while(TC(x) && cond) { x->in++; }
void parse(X* x) { EA(x); OF(x, 1); EIB(x);
	PARSE(x, TC(x) != A(S(x))); A(S(x)) = (C)x->tb; S(x) = cons(x, x->in, AS(ATM, S(x))); x->in++;
}
void parse_name(X* x) { OF(x, 2); EIB(x);
	PARSE(x, isspace(TC(x))); S(x) = cons(x, (C)(x->tb + x->in), AS(ATM, S(x)));
	PARSE(x, !isspace(TC(x))); S(x) = cons(x, (C)((x->tb + x->in) - A(S(x))) , AS(ATM, S(x)));
}

// CONTINUOUS MEMORY PRIMITIVES

#define RESERVED(x)				((x->th) - ((C)x->hr))

void grow(X* x) { ERR(x, A(x->th) != (x->th + sP), E_NEM); x->th += sP; D(x->th) = 0; x->free--; }
void shrink(X* x) { ERR(x, RESERVED(x) < sP, E_NER); 
	D(x->th) = x->th - sP; x->th -= sP; A(x->th) = x->th + sP; D(x->th) = 0; x->free++;
}
void allot(X* x) { UF1(x);
	C b = A(S(x)); S(x) = recl(x, S(x));
	if (b > 0) { ERR(x, b >= (TOP(x) - ((C)x->hr)), E_NEM);
		while (RESERVED(x) < b) { grow(x); if (x->err) return; }
		x->hr += b;
	} else if (b < 0) {
		x->hr = (b < (BOTTOM(x) - x->hr)) ? BOTTOM(x) : x->hr + b;
		while (RESERVED(x) >= sP) { shrink(x); if (x->err) return; }
	}
}

// ----------------------------------------------------------------------------- END OF CORE

void dump_cell(X*, C);

void dump_list(X* x, C l) { while (l) { dump_cell(x, l); l = N(l); } }

void dump_cell(X* x, C c) {
	switch(T(c)) {
		case ATM: printf("%ld ", A(c)); break;
		case LST: printf("{ "); dump_list(x, A(c)); printf("} "); break;
		case PRM: 
			if (A(c) == (C)&swap) printf("swap ");
			else if (A(c) == (C)&duplicate) printf("dup ");
			else if (A(c) == (C)&gt) printf("> ");
			else if (A(c) == (C)&sub) printf("- ");
			else if (A(c) == (C)&add) printf("+ ");
			else if (A(c) == (C)&rot) printf("rot ");
			else if (A(c) == (C)&exec_x) printf("x ");
			else if (A(c) == (C)&exec_i) printf("i ");
			else if (A(c) == (C)&branch) printf("branch ");
			else if (A(c) == (C)&drop) printf("drop ");
			else printf("P:%ld ", A(c));
			break;
		case WRD: break;
	}
}

void dump_stack(X* x, C l) {
	if (!l) return;
	if (N(l)) { dump_stack(x, N(l)); dump_cell(x, l); }
	else dump_cell(x, l);
}

void innerdbg(X* x) {
	C t;
	do {
		while (!R(x) && N(x->rs)) { x->rs = recl(x, x->rs); }
		if (!R(x)) return;
		printf("<%ld> ", length(x->ds));
		dump_stack(x, S(x));
		printf("█ ");
		dump_list(x, R(x));
		printf("<%ld>\n", length(x->rs));
		switch (T(R(x))) {
			case ATM: t = R(x); R(x) = N(R(x)); LK(t, S(x)); S(x) = t; break;
			case LST: t = R(x); R(x) = N(R(x)); LK(t, S(x)); S(x) = t; break;
			case PRM: ((FUNC)A(R(x)))(x); if (!x->err) R(x) = recl(x, R(x)); else x->err = 0; break;
			case WRD: /* Push word list into return stack */ break;
		}
		getchar();
	} while(1);
}

#endif

//C depth(C p) { return p ? (T(p) == LST ? 1 + depth(A(p)) + depth(N(p)) : 1 + depth(N(p))) : 0; }

//#ifndef __DODO__
//#define __DODO__
//
//#include<inttypes.h>
//#include<string.h>
//#include<stdio.h>
//#include<ctype.h>
//
//#ifdef _WIN32
//	#include<conio.h>
//#else
//	#include<unistd.h>
//	#include<termios.h>
//#endif
//
//typedef int8_t			BYTE;
//// The basic type (as in every FORTH) is the CELL. It will be 64, 32 or 16 bits depending
//// on target platform.
//// 16 bits its the minimum required as pair addresses will be aligned to 2*sizeof(CELL) and
//// 2 bits are needed for typing information.
//typedef intptr_t		CELL;				
//
//// In the following MACROS, pair represents a 2*sizeof(CELL) aligned address.
//#define CAR(pair)								(*((CELL*)pair))
//#define CDR(pair)								(*(((CELL*)pair) + 1))
//#define NEXT(pair)							(CDR(pair) & -4)
//#define TYPE(pair)							(CDR(pair) & 3)
//
//// Basic types used for the inner interpreter:
//// ATOM -> [ NEXT | ATOM , CELL_VALUE ]
//// LIST -> [ NEXT | LIST , FIRST_LIST_ITEM_REFERENCE ]
//// PRIM -> [ NEXT | PRIM , (non aligned) PRIMITIVE_ADDRESS_VALUE ]
//// WORD -> [ NEXT | WORD , (2*sizeof(CELL) aligned) WORD_ADDRESS_VALUE ]
//enum Types { ATOM, LIST, PRIM, WORD };
//
//// AS tags a pointer to another pair with a type
//#define AS(type, pair)					((pair & -4) | type)
//// LINK modifies the NEXT of a pair without changing its type
//#define LINK(pair, next_pair)		(CDR(pair) = AS(TYPE(pair), next_pair))
//
//// Words are the types of pairs found on the dictionary
//enum Words { NON_IMM_PRIM, NON_IMM_COLON, IMM_PRIM, IMM_COLON };
//
//#define PRIMITIVE(word)					((TYPE(word) & 1) == 0)
//#define IMMEDIATE(word)					((TYPE(word) & 2) == 2)
//
//// Prims represent basic primitives ZJUMP and JUMP that are implemented directly
//// on inner interpret and also markers -orig, dest, orig_dest and recurse- used to
//// implement jumps, conditionals and recursion on compilation.
//enum Prims { ZJUMP, JUMP, ORIG, DEST, ORIG_DEST, RECURSE };
//
//// Words have the following structure:
//// WORD --> NFA --> XT (list of executable pairs)
//#define NFA(word)								((BYTE*)CAR(CAR(word)))
//#define XT(word)								(NEXT(CAR(word)))
//
//typedef struct {
//	BYTE *tib, *here;
//	CELL there, size; 
//	CELL free, pile, cpile, xstack, latest;
//	CELL ip, state, token, in, base;
//} CTX;
//
//#define S(ctx)									CAR(ctx->pile)
//#define C(ctx)									CAR(ctx->cpile)
//
//#define FREE(ctx)								(length(ctx->free) - 1)		// Don't count ctx->there
//#define ALIGN(addr, bound)			((((CELL)addr) + (bound - 1)) & ~(bound - 1))
//
//#define BOTTOM(ctx)			(((BYTE*)ctx) + sizeof(CTX))
//#define TOP(ctx)				(ALIGN(((BYTE*)ctx) + ctx->size - (2*sizeof(CELL)) - 1, (2*sizeof(CELL))))
//
//CTX* init(BYTE* block, CELL size) {
//	if (size < (sizeof(CTX) + 2*2*sizeof(CELL))) return 0;
//	CTX* ctx = (CTX*)block;	
//	ctx->size = size;
//	ctx->here = BOTTOM(ctx);
//	ctx->there = ALIGN(BOTTOM(ctx), 2*sizeof(CELL));
//	ctx->pile = TOP(ctx);
//	CAR(ctx->pile) = 0;
//	CDR(ctx->pile) = AS(LIST, 0);
//	ctx->free = ctx->pile - 2*sizeof(CELL);
//
//	CELL pair; for (pair = ctx->there; pair <= ctx->free; pair += 2*sizeof(CELL)) {
//		CAR(pair) = pair == ctx->free ? 0 : pair + 2*sizeof(CELL);
//		CDR(pair) = pair == ctx->there ? 0 : pair - 2*sizeof(CELL);
//	}
//
//	ctx->xstack = ctx->cpile = ctx->latest = 0;
//	ctx->state = ctx->token = ctx->in = 0;
//	ctx->tib = 0;
//	ctx->base = 10;
//
//	return ctx;
//}
//
//// CTX ctx{(f = free) > 0}, CELL car, CELL cdr
//// CTX ctx{f - 1}, pair(car, cdr)
//CELL cons(CTX* ctx, CELL car, CELL cdr) {
//	if (ctx->free == ctx->there) return 0;
//	CELL pair = ctx->free;
//	ctx->free = CDR(ctx->free);
//	CAR(pair) = car;
//	CDR(pair) = cdr;
//	return pair;
//}
//
//// f indicates if depth of pair and its nexts items are required or only of pair
//// When calculating depth of item on top of the stack, we don't want depth of all the stack,
//// for example.
//CELL depth(CELL pair, CELL f) {
//	if (pair == 0) return 0;
//	if (TYPE(pair) == LIST) return 1 + depth(CAR(pair), 1) + (f ? depth(NEXT(pair), 1) : 0);
//	else return 1 + (f ? depth(NEXT(pair), 1) : 0);
//}
//
//// As in depth, f indicates if p1 next items must be followed.
//CELL disjoint(CELL p1, CELL p2, CELL f) {
//	if (p1 == 0) return 1;
//	if (p1 == p2) return 0;
//	if (TYPE(p1) != TYPE(p2)) return 0;
//	if (TYPE(p1) == LIST) 
//		return disjoint(CAR(p1), CAR(p2), 1) & (f ? disjoint(NEXT(p1), NEXT(p2), 1) : 1);
//	else return CAR(p1) == CAR(p2) & (f ? disjoint(NEXT(p1), NEXT(p2), 1) : 1);
//}
//
//// CTX ctx{(f = free) >= depth(pair)}, CELL pair
//// CTX ctx{f - depth(pair, 0)}, clone(pair){deep equal with disjoint pairs}
//CELL clone(CTX* ctx, CELL pair) {
//	if (!pair) return 0;
//	if (TYPE(pair) == LIST) {
//		return cons(ctx, clone(ctx, CAR(pair)), AS(TYPE(pair), clone(ctx, NEXT(pair))));
//	} else {
//		return cons(ctx, CAR(pair), AS(TYPE(pair), clone(ctx, NEXT(pair))));
//	}
//}
//
//// CTX ctx{(f = free) >= 0}, CELL pair
//// CTX ctx{pair == 0 -> f , pair != 0 -> f + depth(pair, 0)}, NEXT(pair)
//// TODO: Ensure testing with depth !!
//CELL reclaim(CTX* ctx, CELL pair) {
//	if (!pair) return 0;
//	if (TYPE(pair) == LIST) { 
//		while (CAR(pair) != 0) { 
//			CAR(pair) = reclaim(ctx, CAR(pair)); 
//		} 
//	}
//	CELL tail = NEXT(pair);
//	CDR(pair) = ctx->free;
//	CAR(pair) = 0;
//	ctx->free = pair;
//	return tail;
//}
//
//CELL reverse(CELL pair, CELL list) {
//	if (pair != 0) {
//		CELL t = NEXT(pair);
//		CDR(pair) = AS(TYPE(pair), list);
//		return reverse(t, pair);
//	} else {
//		return list;
//	}
//}
//
//CELL length(CELL pair) { 
//	CELL c = 0; 
//	while (pair) { 
//		c++; 
//		pair = NEXT(pair); 
//	} 
//	return c; 
//}
//
//#define ERR_STACK_OVERFLOW			-1
//#define ERR_RSTACK_OVERFLOW			-2
//#define ERR_STACK_UNDERFLOW			-3
//#define ERR_RSTACK_UNDERFLOW		-4
//#define ERR_UNDEFINED_WORD			-5
//#define ERR_NOT_ENOUGH_MEMORY		-6
//#define ERR_NOT_ENOUGH_RESERVED	-7
//#define ERR_ZERO_LENGTH_NAME		-8
//#define ERR_ATOM_EXPECTED				-9
//#define ERR_LIST_EXPECTED				-10
//#define ERR_END_OF_XLIST				-11
//#define ERR_EXIT								-12
//
//CELL error(CTX* ctx, CELL err) {
//	// TODO
//	/* Lookup on exception stack for a correct handler for current error */
//	/* The debugger must be installed on the exception stack for it to work */
//	/* If a handler its found then execute it and return its return value */
//	/* If none its found, just return the error */
//	// TODO
//	/* The error function should have the ability to not just modify the context */
//	/* but of returning a compleletely new context to work on. For example, in the case */
//	/* of a memory error, a new context with a bigger buffer could be created, cloning */
//	/* the current context on it and returning it as the new context. */
//	return err;
//}
//
//#define INFO(msg) \
//    fprintf(stderr, "info: %s:%d: ", __FILE__, __LINE__); \
//    fprintf(stderr, "%s", msg);
//
//#define ERR(ctx, err)		{ INFO(""); CELL __err__ = error(ctx, err); if (__err__) { return __err__; } }
//
//// TODO: Add error management directly to PUSH? Or remove PUSH.
//#define ERR_PUSH(ctx, v) \
//	if ((S(ctx) = cons(ctx, (CELL)v, AS(ATOM, S(ctx)))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//#define ERR_PUSHL(ctx, v) \
//	if ((S(ctx) = cons(ctx, (CELL)v, AS(LIST, S(ctx)))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//
//CELL pop(CTX* ctx) {
//	if (S(ctx) == 0) { return 0; /* ERROR */ }
//	CELL v = CAR(S(ctx));
//	S(ctx) = reclaim(ctx, S(ctx));
//	return v;
//}
//
//#define ERR_POP(ctx, v) \
//	if (S(ctx) == 0) { \
//		ERR(ctx, ERR_STACK_UNDERFLOW); \
//	} else { \
//		v = pop(ctx); \
//	}
//
//typedef CELL (*FUNC)(CTX*);
//
//#define EXECUTE_PRIMITIVE(ctx, primitive) { \
//	CELL result = ((FUNC)primitive)(ctx); \
//	if (result < 0) { ERR(ctx, result); } \
//}
//
//CELL execute(CTX* ctx, CELL xlist) {
//	CELL result;
//	CELL p = xlist;
//	while (p) {
//		switch (TYPE(p)) {
//			case ATOM:
//				ERR_PUSH(ctx, CAR(p));
//				p = NEXT(p);
//				break;
//			case LIST:
//				ERR_PUSHL(ctx, clone(ctx, CAR(p)));
//				p = NEXT(p);
//				break;
//			case PRIM:
//				switch (CAR(p)) {
//					case ZJUMP: /* ZJUMP */
//						if (pop(ctx) == 0) {
//							p = CAR(NEXT(p));
//						} else {
//							p = NEXT(NEXT(p));
//						}
//						break;
//					case JUMP: /* JUMP */
//						p = CAR(NEXT(p));
//						break;
//					default:
//						EXECUTE_PRIMITIVE(ctx, CAR(p));
//						p = NEXT(p);
//						break;
//				}
//				break;
//			case WORD:
//				if (NEXT(p)) { 
//					execute(ctx, XT(CAR(p))); p = NEXT(p); 
//				} else { 
//					p = XT(CAR(p));
//				}
//				break;
//		}
//	}
//}
//
//#define TC(ctx)									(*(ctx->tib + ctx->in))
//#define TK(ctx)									(ctx->tib + ctx->token)
//#define TL(ctx)									(ctx->in - ctx->token)
//
//CELL parse_token(CTX* ctx) {
//	ctx->token = ctx->in;	while (TC(ctx) != 0 && isspace(TC(ctx))) { ctx->in++;	}
//	ctx->token = ctx->in;	while (TC(ctx) != 0 && !isspace(TC(ctx))) { ctx->in++; }
//	return ctx->in - ctx->token;
//}
//
//CELL find_token(CTX* ctx) {
//	CELL w = ctx->latest;
//	while (w && !(strlen(NFA(w)) == TL(ctx) && strncmp(NFA(w), TK(ctx), TL(ctx)) == 0)) {
//		w = NEXT(w);
//	}
//	return w;
//}
//
//CELL cpush(CTX* ctx) {
//	if ((ctx->cpile = cons(ctx, 0, AS(LIST, ctx->cpile))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//	return 0;
//}
//
//CELL compile_word(CTX* ctx, CELL word) {
//	if (ctx->cpile == 0) {
//		if (cpush(ctx) != 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//	}
//	if (TYPE(word) == ATOM || TYPE(word) == PRIM) {
//		return (C(ctx) = cons(ctx, CAR(XT(word)), AS(PRIM, C(ctx))));
//	} else {
//		return (C(ctx) = cons(ctx, word, AS(WORD, C(ctx))));
//	}
//}
//
//CELL compile_number(CTX* ctx, CELL number) {
//	if (ctx->cpile == 0) {
//		if (cpush(ctx) != 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//	}
//	return C(ctx) = cons(ctx, number, AS(ATOM, C(ctx)));
//}
//
//
//CELL evaluate(CTX* ctx, BYTE* str) { 
//	CELL word, result; 
//	char *endptr;
//	ctx->tib = str; 
//	ctx->token = ctx->in = 0;
//	do {
//		if (parse_token(ctx) == 0) { return 0; }
//		if ((word = find_token(ctx)) != 0) {
//			if (!ctx->state || IMMEDIATE(word)) {
//				if ((result = execute(ctx, XT(word))) != 0) { ERR(ctx, result); }
//			} else {
//				if (compile_word(ctx, word) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//			}
//		} else {
//			intmax_t number = strtoimax(TK(ctx), &endptr, 10);
//			if (number == 0 && endptr == (char*)(TK(ctx))) {
//				ERR(ctx, ERR_UNDEFINED_WORD);
//			} else if (ctx->state) {
//				if (compile_number(ctx, number) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//			} else {
//				ERR_PUSH(ctx, number);
//			}
//		}
//	} while (1);
//}
//
//// TODO: Up to here + some primitives is enough to create complex programs?
//
//CELL grow(CTX* ctx) { 
//	if (CAR(ctx->there) == (ctx->there + 2*sizeof(CELL))) { 
//		ctx->there += 2*sizeof(CELL);
//		CDR(ctx->there) = 0;
//		return 0;
//	} else {
//		return ERR_NOT_ENOUGH_MEMORY;
//	}
//}
//
//#define RESERVED(ctx)				((ctx->there) - ((CELL)ctx->here))
//
//CELL shrink(CTX* ctx) {
//	if (RESERVED(ctx) >= 2*sizeof(CELL)) {
//		CDR(ctx->there) = ctx->there - 2*sizeof(CELL);
//		ctx->there -= 2*sizeof(CELL);
//		CAR(ctx->there) = ctx->there + 2*sizeof(CELL);
//		CDR(ctx->there) = 0;
//		return 0;
//	} else {
//		return ERR_NOT_ENOUGH_RESERVED;
//	}
//}
//
//CELL allot(CTX* ctx) {
//	CELL bytes; ERR_POP(ctx, bytes);
//	if (bytes == 0) {
//		return 0;
//	} else if (bytes > 0) { 
//		if (bytes < (TOP(ctx) - ((CELL)ctx->here))) {
//			while (RESERVED(ctx) < bytes) { 
//				if (grow(ctx) != 0) { ERR(ctx, ERR_NOT_ENOUGH_MEMORY); } 
//			}
//			ctx->here += bytes;
//		} else {
//			ERR(ctx, ERR_NOT_ENOUGH_MEMORY);
//		}
//	} else if (bytes < 0) {
//		if (bytes < (BOTTOM(ctx) - ctx->here)) {
//			ctx->here = BOTTOM(ctx);
//		} else {
//			ctx->here += bytes;
//		}
//		while (RESERVED(ctx) >= 2*sizeof(CELL)) { 
//			if (shrink(ctx) != 0) { ERR(ctx, ERR_NOT_ENOUGH_RESERVED); } 
//		}
//	}
//	return 0;
//}
//
//CELL compile_str(CTX* ctx) {
//	ctx->token = ++ctx->in; while (TC(ctx) != 0 && TC(ctx) != '"') { ctx->in++; }
//	while (RESERVED(ctx) < TL(ctx)) { 
//		if (grow(ctx) != 0) { ERR(ctx, ERR_NOT_ENOUGH_MEMORY); } 
//	}
//	strncpy(ctx->here, TK(ctx), TL(ctx));
//	ctx->here[TL(ctx)] = 0;
//	ERR_PUSH(ctx, ctx->here);
//	ERR_PUSH(ctx, TL(ctx));
//	ctx->here += TL(ctx) + 1;
//	ctx->in++;
//	return 0;
//}
//
//CELL type(CTX* ctx) {
//	CELL addr, len;
//	ERR_POP(ctx, len);
//	ERR_POP(ctx, addr);
//	printf("%.*s", (int)len, (BYTE*)addr);
//	return 0;
//}
//
//CELL postpone(CTX* ctx) {
//	CELL word;
//	if (parse_token(ctx) == 0) { ERR(ctx, ERR_ZERO_LENGTH_NAME); }
//	if ((word = find_token(ctx)) == 0) { ERR(ctx, ERR_UNDEFINED_WORD); }
//	if (compile_word(ctx, word) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//	return 0;
//}
//
//CELL parse(CTX* ctx) { 
//	CELL c; ERR_POP(ctx, c);
//	ctx->token = ++ctx->in;	
//	ERR_PUSH(ctx, TK(ctx));
//	while (TC(ctx) != 0 && TC(ctx) != c) { ctx->in++;	}
//	ERR_PUSH(ctx, TL(ctx));
//	if (TC(ctx) != 0) ctx->in++;
//	return 0;
//}
//
//CELL parse_name(CTX* ctx) { 
//	parse_token(ctx);
//	ERR_PUSH(ctx, TK(ctx));
//	ERR_PUSH(ctx, TL(ctx));
//	return 0; 
//}
//
//// TODO: Start working here for piles/stacks
//
//CELL spush(CTX* ctx) {
//	if ((ctx->pile = cons(ctx, 0, AS(LIST, ctx->pile))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//	return 0;
//}
//
//CELL clear(CTX* ctx) {
//	while (S(ctx)) S(ctx) = reclaim(ctx, S(ctx));
//}
//
//CELL sdrop(CTX* ctx) {
//	if (NEXT(ctx->pile) == 0) { return clear(ctx); }
//	ctx->pile = reclaim(ctx, ctx->pile);
//	return 0;
//}
//
//CELL stack_to_list(CTX* ctx) {
//	if (NEXT(ctx->pile) == 0) {
//		ctx->pile = cons(ctx, ctx->pile, AS(LIST, 0));
//	} else {
//		CELL t = ctx->pile;
//		ctx->pile = NEXT(ctx->pile);
//		LINK(t, CAR(ctx->pile));
//		CAR(ctx->pile) = t;
//	}
//	return 0;
//}
//
//CELL list_to_stack(CTX* ctx) {
//	if (S(ctx) == 0) {
//		spush(ctx);
//	} else {
//		CELL t = S(ctx);
//		S(ctx) = NEXT(S(ctx));
//		LINK(t, ctx->pile);
//		ctx->pile = t;
//	}
//}
//
//CELL lbracket(CTX* ctx) {
//	ctx->state = 0;
//	return 0;
//}
//
//CELL rbracket(CTX* ctx) {
//	if (ctx->cpile == 0) {
//		if ((ctx->cpile = cons(ctx, 0, AS(LIST, 0))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//	}
//	ctx->state = 1;
//	return 0;
//}
//
//CELL reverse_stack(CTX* ctx) {
//	S(ctx) = reverse(S(ctx), 0);
//	return 0;
//}
//
//CELL reverse_cstack(CTX* ctx) {
//	C(ctx) = reverse(C(ctx), 0);
//	return 0;
//}
//
//CELL cstack_to_list(CTX* ctx) {
//	if (NEXT(ctx->cpile) == 0) {
//		LINK(ctx->cpile, S(ctx));
//		S(ctx) = ctx->cpile;
//		ctx->cpile = 0;
//		lbracket(ctx);
//	} else {
//		if (TYPE(NEXT(ctx->cpile)) == LIST) {
//			CELL t = ctx->cpile;
//			ctx->cpile = NEXT(ctx->cpile);
//			LINK(t, CAR(ctx->cpile));
//			CAR(ctx->cpile) = t;
//		} else if (TYPE(NEXT(ctx->cpile)) == WORD) {
//			// Append it as if >BODY
//			CELL t = ctx->cpile;
//			ctx->cpile = NEXT(ctx->cpile);
//			LINK(CAR(ctx->cpile), CAR(t));
//			CAR(t) = 0;
//			reclaim(ctx, t);
//		}
//	}
//	return 0;
//}
//
//CELL lbrace(CTX* ctx) {
//	cpush(ctx);
//	rbracket(ctx);
//	return 0;
//}
//
//CELL resolve(CTX* ctx) {
//	CELL prev = 0, dest;
//	CELL i = C(ctx);
//	while (i) {
//		if (TYPE(i) == PRIM && CAR(i) == DEST) {
//			S(ctx) = cons(ctx, prev, AS(ATOM, S(ctx)));
//			if (prev == 0) {
//				C(ctx) = NEXT(i);
//				prev = 0;
//			} else {
//				LINK(prev, NEXT(i));
//			}
//			CELL t = i;
//			i = NEXT(i);
//			reclaim(ctx, t);
//		} else if (TYPE(i) == PRIM && CAR(i) == ORIG_DEST) {
//			// prev can not be 0 as at least there must be one dest after
//			ERR_POP(ctx, dest);
//			S(ctx) = cons(ctx, prev, AS(ATOM, S(ctx)));
//			CDR(i) = AS(ATOM, NEXT(i));
//			CAR(i) = dest;
//			i = NEXT(i);
//		} else if (TYPE(i) == PRIM && CAR(i) == ORIG) {
//			ERR_POP(ctx, dest);
//			CDR(i) = AS(ATOM, NEXT(i));
//			CAR(i) = dest;
//			i = NEXT(i);
//		} else if (TYPE(i) == PRIM && CAR(i) == RECURSE) {
//			// Find last header in compilation pile
//			CELL h = ctx->cpile;
//			while (h && TYPE(h) != WORD) {
//				h = NEXT(h);
//			}
//			if (h) { 
//				CDR(i) = AS(WORD, NEXT(i));
//				CAR(i) = h;
//			}
//			i = NEXT(i);
//		} else {
//			prev = i;
//			i = NEXT(i);
//		}
//	}
//}
//
//CELL rbrace(CTX* ctx) {
//	if (ctx->cpile == 0 || TYPE(ctx->cpile) == WORD) return 0;
//	resolve(ctx);
//	reverse_cstack(ctx);
//	cstack_to_list(ctx);
//	return 0;
//}
//
//CELL duplicate(CTX* ctx) {
//	if (S(ctx) == 0) { ERR(ctx, ERR_STACK_UNDERFLOW); }
//	switch(TYPE(S(ctx))) {
//		case ATOM: ERR_PUSH(ctx, CAR(S(ctx))); break;
//		case LIST: ERR_PUSHL(ctx, clone(ctx, CAR(S(ctx)))); break;
//		case PRIM:
//			if ((S(ctx) = cons(ctx, CAR(S(ctx)), AS(PRIM, S(ctx)))) == 0) {
//				ERR(ctx, ERR_STACK_OVERFLOW);
//			}
//			break;
//		case WORD:
//			if ((S(ctx) = cons(ctx, CAR(S(ctx)), AS(WORD, S(ctx)))) == 0) {
//				ERR(ctx, ERR_STACK_OVERFLOW);
//			}
//			break;
//	}
//
//	return 0;
//}
//
//CELL swap(CTX* ctx) {
//	CELL t = S(ctx);
//	S(ctx) = NEXT(S(ctx));
//	LINK(t, NEXT(S(ctx)));
//	LINK(S(ctx), t);
//	return 0;
//}
//
//CELL drop(CTX* ctx) {
//	S(ctx) = reclaim(ctx, S(ctx));
//	return 0;
//}
//
//CELL rot(CTX* ctx) {
//	CELL t = NEXT(NEXT(S(ctx)));
//	LINK(NEXT(S(ctx)), NEXT(t));
//	LINK(t, S(ctx));
//	S(ctx) = t;
//	return 0;
//}
//
//CELL over(CTX* ctx) {
//	if (S(ctx) == 0 || NEXT(S(ctx)) == 0) { ERR(ctx, ERR_STACK_UNDERFLOW); }
//	switch(TYPE(NEXT(S(ctx)))) {
//		case ATOM: ERR_PUSH(ctx, CAR(NEXT(S(ctx)))); break;
//		case LIST: ERR_PUSHL(ctx, clone(ctx, CAR(NEXT(S(ctx))))); break;
//		case PRIM: 
//			if ((S(ctx) = cons(ctx, CAR(S(ctx)), AS(PRIM, S(ctx)))) == 0) {
//				ERR(ctx, ERR_STACK_OVERFLOW);
//			}
//			break;
//		case WORD:
//			if ((S(ctx) = cons(ctx, CAR(S(ctx)), AS(WORD, S(ctx)))) == 0) {
//				ERR(ctx, ERR_STACK_OVERFLOW);
//			}
//			break;
//	}
//	return 0;
//}
//
//CELL exec(CTX* ctx) {
//	CELL result = 0;
//	CELL t = S(ctx);
//	S(ctx) = NEXT(S(ctx));
//	LINK(t, 0);
//
//	switch (TYPE(t)) {
//		// TODO: ATOM should be executed as a primitive
//		case ATOM: ERR_PUSH(ctx, CAR(t)); break; 
//		case LIST: result = execute(ctx, CAR(t)); break;
//		case PRIM: result = ((FUNC)CAR(t))(ctx); break;
//		case WORD: result = execute(ctx, XT(CAR(t))); break;
//	}
//
//	reclaim(ctx, t);
//	if (result < 0) { ERR(ctx, result); } else { return result; }
//}
//
//CELL exec_x(CTX* ctx) {
//	CELL result = 0;
//
//	switch (TYPE(S(ctx))) {
//		// TODO: ATOM should be executed as a primitive
//		case ATOM: ERR_PUSH(ctx, CAR(S(ctx))); break;
//		case LIST: result = execute(ctx, CAR(S(ctx))); break;
//		case PRIM: result = ((FUNC)CAR(S(ctx)))(ctx); break;
//		case WORD: result = execute(ctx, XT(CAR(S(ctx)))); break;
//	}
//
//	if (result < 0) { ERR(ctx, result); } else { return result; }
//}
//
//CELL branch(CTX* ctx) {
//	rot(ctx);
//	CELL b = pop(ctx);
//	if (!b) {
//		swap(ctx);
//	}
//	drop(ctx);
//	return exec(ctx);
//}
//
//// ARITHMETIC PRIMITIVES
//
//CELL add(CTX* ctx) { 
//	CELL a, b;	
//	ERR_POP(ctx, a); 
//	ERR_POP(ctx, b);
//	ERR_PUSH(ctx, b + a);
//	return 0;
//}
//
//CELL sub(CTX* ctx) { 
//	CELL a, b;	
//	ERR_POP(ctx, a); 
//	ERR_POP(ctx, b);	
//	ERR_PUSH(ctx, b - a);
//	return 0;
//}
//
//CELL mul(CTX* ctx) { 
//	CELL a, b;	
//	ERR_POP(ctx, a); 
//	ERR_POP(ctx, b);	
//	ERR_PUSH(ctx, b * a);
//	return 0;
//}
//
//CELL division(CTX* ctx) { 
//	CELL a, b;	
//	ERR_POP(ctx, a); 
//	ERR_POP(ctx, b);	
//	ERR_PUSH(ctx, b / a);
//	return 0;
//}
//
//CELL mod(CTX* ctx) { 
//	CELL a, b;	
//	ERR_POP(ctx, a); 
//	ERR_POP(ctx, b);	
//	ERR_PUSH(ctx, b % a);
//	return 0;
//}
//
//CELL increment(CTX* ctx) { 
//	CAR(S(ctx)) = CAR(S(ctx)) + 1; 
//	return 0; 
//}
//
//// --------------------------------------------------------------- Throughly tested until here
//
//// COMPARISON PRIMITIVES
//
//CELL gt(CTX* ctx) {
//	CELL a = pop(ctx);
//	CELL b = pop(ctx);
//	ERR_PUSH(ctx, b > a);
//	return 0;
//}
//
//CELL lt(CTX* ctx) {
//	CELL a = pop(ctx);
//	CELL b = pop(ctx);
//	ERR_PUSH(ctx, b < a);
//	return 0;
//}
//
//CELL eq(CTX* ctx) {
//	CELL a = pop(ctx);
//	CELL b = pop(ctx);
//	ERR_PUSH(ctx, b == a);
//	return 0;
//}
//
//// BIT PRIMITIVES
//
//CELL and(CTX* ctx) {
//	CELL a = pop(ctx);
//	CELL b = pop(ctx);
//	ERR_PUSH(ctx, b & a);
//	return 0;
//}
//
//CELL or(CTX* ctx) {
//	CELL a = pop(ctx);
//	CELL b = pop(ctx);
//	ERR_PUSH(ctx, b | a);
//	return 0;
//}
//
//CELL invert(CTX* ctx) {
//	CAR(S(ctx)) = ~CAR(S(ctx));
//	return 0;
//}
//
//CELL literal(CTX* ctx) {
//	// TODO: Different types!!
//	CELL t = pop(ctx);
//	C(ctx)  = cons(ctx, t, AS(ATOM, C(ctx)));
//	return 0;
//}
//
//CELL fetch(CTX* ctx) {
//	CELL addr;
//	ERR_POP(ctx, addr);
//	return S(ctx) = cons(ctx, *((CELL*)addr), AS(ATOM, S(ctx)));
//}
//
//CELL store(CTX* ctx) {
//	CELL addr, x;
//	ERR_POP(ctx, addr);
//	ERR_POP(ctx, x);
//	*((CELL*)addr) = x;
//
//	return 0;
//}
//
//CELL here(CTX* ctx) {
//	ERR_PUSH(ctx, ctx->here);
//	return 0;
//}
//
//CELL latest(CTX* ctx) {
//	ERR_PUSH(ctx, &ctx->latest);
//	return 0;
//}
//
//CELL append(CTX* ctx) {
//	if (TYPE(NEXT(S(ctx))) == LIST) {
//		CELL t = NEXT(S(ctx));
//		LINK(S(ctx), CAR(NEXT(S(ctx))));
//		CAR(t) = S(ctx);
//		S(ctx) = t;
//	} else if (TYPE(NEXT(S(ctx))) == ATOM) {
//		CELL* dest = (CELL*)CAR(NEXT(S(ctx)));	
//		CELL t = NEXT(S(ctx));
//		LINK(S(ctx), *dest);
//		*dest = S(ctx);
//		S(ctx) = reclaim(ctx, t);
//	}
//
//	return 0;
//}
//
//CELL orig(CTX* ctx) {
//	// Compile a primitive with ORIG value
//	C(ctx) = cons(ctx, ORIG, AS(PRIM, C(ctx)));
//	return 0;
//}
//
//CELL dest(CTX* ctx) {
//	// Compile a primitive with DEST value
//	C(ctx) = cons(ctx, DEST, AS(PRIM, C(ctx)));
//	return 0;
//}
//
//CELL orig_dest(CTX* ctx) {
//	// Compile a double primitive with ORIG_DEST value
//	C(ctx) = cons(ctx, ORIG_DEST, AS(PRIM, C(ctx)));
//	//C(ctx) = cons(ctx, ORIG_DEST, AS(PRIM, C(ctx)));
//	return 0;
//}
//
//CELL recurse(CTX* ctx) {
//	C(ctx) = cons(ctx, RECURSE, AS(PRIM, C(ctx)));
//	return 0;
//}
//
//CELL sliteral(CTX* ctx) {
//	CELL result, addr, len;
//	if ((result = duplicate(ctx)) != 0) { ERR(ctx, result); }
//	if ((result = increment(ctx)) != 0) { ERR(ctx, result); }
//	BYTE* here = ctx->here;
//	if ((result = allot(ctx)) != 0) { ERR(ctx, result); }
//	ERR_POP(ctx, len);
//	ERR_POP(ctx, addr);
//	// TODO: If strncpy was a function sliteral can be defined in DODO
//	strncpy(here, (BYTE*)addr, len);
//	here[len] = 0;
//	ERR_PUSH(ctx, here);
//	ERR_PUSH(ctx, len);
//	return 0;
//}
//
//CELL header(CTX* ctx) {
//	// Push into Compilation Pile a WORD that represents a new header, with name
//	CELL result, addr, len;
//	if ((result = parse_name(ctx)) != 0) { ERR(ctx, result); }
//	if ((result = sliteral(ctx)) != 0) { ERR(ctx, result); }
//	ERR_POP(ctx, len);
//	ERR_POP(ctx, addr);
//	// TODO: If cons is defined as a word header can be defined in DODO
//	if ((ctx->cpile = cons(ctx, cons(ctx, addr, AS(ATOM, 0)), AS(WORD, ctx->cpile))) == 0) { ERR(ctx, result); }
//	return 0;
//}
//
//CELL colon(CTX* ctx) {
//	CELL result;
//	if ((result = header(ctx)) != 0) { ERR(ctx, result); }
//	if ((result = lbrace(ctx)) != 0) { ERR(ctx, result); }
//	return 0;
//}
//
//CELL semicolon(CTX* ctx) {
//	CELL result;
//	rbrace(ctx);
//	if ((result = cstack_to_list(ctx)) != 0) { ERR(ctx, result); }
//	latest(ctx);
//	swap(ctx);
//	// Modify to be a non immediate word by default
//	CDR(S(ctx)) = AS(NON_IMM_COLON, NEXT(S(ctx)));
//	append(ctx);
//}
//
//CELL _if_(CTX* ctx) {
//	if ((C(ctx) = cons(ctx, ZJUMP, AS(PRIM, C(ctx)))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//	return orig(ctx);
//}
//
//CELL _else_(CTX* ctx) {
//	if ((C(ctx) = cons(ctx, JUMP, AS(PRIM, C(ctx)))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//	return orig_dest(ctx);
//}
//
//CELL _then_(CTX* ctx) {
//	return dest(ctx);
//}
//
//CELL tick(CTX* ctx) {
//	CELL word;
//	if (parse_token(ctx) == 0) { ERR(ctx, ERR_ZERO_LENGTH_NAME); }
//	if ((word = find_token(ctx)) == 0) { ERR(ctx, ERR_UNDEFINED_WORD); }
//	if (PRIMITIVE(word)) {
//		return S(ctx) = cons(ctx, CAR(XT(word)), AS(PRIM, S(ctx)));
//	} else {
//		return S(ctx) = cons(ctx, word, AS(WORD, S(ctx)));
//	}
//}
//
//CELL immediate(CTX* ctx) {
//	switch(TYPE(ctx->latest)) {
//		case NON_IMM_PRIM: CDR(ctx->latest) = AS(IMM_PRIM, NEXT(ctx->latest)); break;
//		case NON_IMM_COLON: CDR(ctx->latest) = AS(IMM_COLON, NEXT(ctx->latest)); break;
//	}
//	return 0;
//}
//
//CELL cell(CTX* ctx) {
//	ERR_PUSH(ctx, sizeof(CELL));
//	return 0;
//}
//
//CELL emit(CTX* ctx) {
//	CELL k;
//	ERR_POP(ctx, k);
//	printf("%c", (BYTE)k);
//	return 0;
//}
//
//#define ADD_PRIMITIVE(ctx, name, func, immediate) \
//	ctx->latest = cons(ctx, \
//		cons(ctx, (CELL)name, AS(ATOM, \
//		cons(ctx, (CELL)func, AS(PRIM, 0)))), \
//	AS(immediate, ctx->latest));
//
//CTX* bootstrap(CTX* ctx) {
//	ADD_PRIMITIVE(ctx, "+", &add, 0);
//	ADD_PRIMITIVE(ctx, "-", &sub, 0);
//	ADD_PRIMITIVE(ctx, "*", &mul, 0);
//	ADD_PRIMITIVE(ctx, "/", &division, 0);
//	ADD_PRIMITIVE(ctx, "mod", &mod, 0);
//
//	ADD_PRIMITIVE(ctx, ">", &gt, 0);
//	ADD_PRIMITIVE(ctx, "<", &lt, 0);
//	ADD_PRIMITIVE(ctx, "=", &eq, 0);
//
//	ADD_PRIMITIVE(ctx, "and", &and, 0);
//	ADD_PRIMITIVE(ctx, "or", &or, 0);
//	ADD_PRIMITIVE(ctx, "invert", &invert, 0);
//
//	ADD_PRIMITIVE(ctx, "dup", &duplicate, 0);
//	ADD_PRIMITIVE(ctx, "swap", &swap, 0);
//	ADD_PRIMITIVE(ctx, "drop", &drop, 0);
//	ADD_PRIMITIVE(ctx, "over", &over, 0);
//	ADD_PRIMITIVE(ctx, "rot", &rot, 0);
//
//	ADD_PRIMITIVE(ctx, "{", &lbrace, 2);
//	ADD_PRIMITIVE(ctx, "}", &rbrace, 2);
//
//	ADD_PRIMITIVE(ctx, "[", &lbracket, 2);
//	ADD_PRIMITIVE(ctx, "]", &rbracket, 2);
//
//	ADD_PRIMITIVE(ctx, "spush", &spush, 2);
//	ADD_PRIMITIVE(ctx, "s>l", &stack_to_list, 2);
//
//	ADD_PRIMITIVE(ctx, "i", &exec, 0);
//	ADD_PRIMITIVE(ctx, "x", &exec_x, 0);
//
//	ADD_PRIMITIVE(ctx, "branch", &branch, 0);
//
//	ADD_PRIMITIVE(ctx, "grow", &grow, 0);
//
//	ADD_PRIMITIVE(ctx, "s\"", &compile_str, 2);
//	ADD_PRIMITIVE(ctx, "type", &type, 0);
//
//	ADD_PRIMITIVE(ctx, "latest", &latest, 0);
//	ADD_PRIMITIVE(ctx, "append", &append, 0);
//
//	ADD_PRIMITIVE(ctx, "parse", &parse, 0);
//	ADD_PRIMITIVE(ctx, "parse-name", &parse_name, 0);
//
//	ADD_PRIMITIVE(ctx, ":", &colon, 2);
//	ADD_PRIMITIVE(ctx, ";", &semicolon, 2);
//	ADD_PRIMITIVE(ctx, "immediate", &immediate, 0);
//
//	ADD_PRIMITIVE(ctx, "here", &here, 0);
//	ADD_PRIMITIVE(ctx, "allot", &allot, 0);
//
//	ADD_PRIMITIVE(ctx, "!", &store, 0);
//	ADD_PRIMITIVE(ctx, "@", &fetch, 0);
//	//ADD_PRIMITIVE(ctx, "b!", &bstore, 0);
//	//ADD_PRIMITIVE(ctx, "b@", &bfetch, 0);
//
//	ADD_PRIMITIVE(ctx, "cell", &cell, 0);
//
//	ADD_PRIMITIVE(ctx, "postpone", &postpone, 2);
//	//ADD_PRIMITIVE(ctx, "p", &postpone, 2);
//
//	ADD_PRIMITIVE(ctx, "literal", &literal, 2);
//
//	ADD_PRIMITIVE(ctx, "header", &header, 0);
//	ADD_PRIMITIVE(ctx, "sliteral", &sliteral, 0);
//
//	ADD_PRIMITIVE(ctx, "zjump", ZJUMP, 0);
//	ADD_PRIMITIVE(ctx, "jump", JUMP, 0);
//	ADD_PRIMITIVE(ctx, "orig", ORIG, 0);
//	ADD_PRIMITIVE(ctx, "dest", DEST, 0);
//	ADD_PRIMITIVE(ctx, "orig-dest", ORIG_DEST, 0);
//	ADD_PRIMITIVE(ctx, "recurse", RECURSE, 0);
//
//	ADD_PRIMITIVE(ctx, "if", &_if_, 2);
//	ADD_PRIMITIVE(ctx, "else", &_else_, 2);
//	ADD_PRIMITIVE(ctx, "then", &_then_, 2);
//
//	ADD_PRIMITIVE(ctx, "'", &tick, 0);
//
//	ADD_PRIMITIVE(ctx, "emit", &emit, 0);
//
//	return ctx;
//}
//
////// PRIMITIVES
////
////// LIST OPERATIONS
////
////CELL find_prim(CTX* ctx, CELL xt) {
////	CELL word = ctx->latest;
////	while (word && CAR(XT(word)) != xt) { 
////		word = NEXT(word); }
////	return word;
////}
////
////void dump_list(CTX* ctx, CELL pair, CELL order) {
////	CELL word;
////	if (pair) {
////		if (order) dump_list(ctx, NEXT(pair), order);
////		switch (TYPE(pair)) {
////			case ATOM: printf("%ld ", CAR(pair)); break;
////			case LIST: printf("{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
////			case PRIM: 
////				word = find_prim(ctx, CAR(pair));
////				if (word) {
////					printf("%s ", (BYTE*)(NFA(word)));
////				} else {
////					printf("PRIM_NOT_FOUND ");
////				}
////				break;
////			case WORD: printf("X{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
////		}
////		if (!order) dump_list(ctx, NEXT(pair), order);
////	}
////}
////
////CELL dump_stack(CTX* ctx) {
////	printf("<%ld> ", length(ctx->stack));
////	dump_list(ctx, ctx->stack, 1);
////	printf("\n");
////
////	return 0;
////}
////
////CELL dump_cpile(CTX* ctx) {
////	printf("COMPILED: <%ld> ", length(ctx->cpile));
////	dump_list(ctx, ctx->cpile, 1);
////	printf("\n");
////}
////
////CELL words(CTX* ctx) {
////	printf("WORDS: ");
////	CELL p = ctx->latest;
////	while (p) { printf("%s ", (BYTE*)NFA(p)); p = NEXT(p); }
////	printf("\n");
////}
////
////// PRIMITIVES
////
////CELL see(CTX* ctx) {
////	// TODO
////	parse_token(ctx);
////	CELL word = find_token(ctx);
////	if (word) {
////		printf(": %s ", (BYTE*)NFA(word));
////		if (PRIMITIVE(word)) {
////			printf("; PRIMITIVE");
////		} else {
////			dump_list(ctx, XT(word), 1);
////			printf("; ");
////		}
////		printf("\n");
////	}
////	return 0;
////}
////
////CELL type(CTX* ctx) {
////	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
////	CELL l = CAR(ctx->stack);
////	BYTE* s = (BYTE*)CAR(NEXT(ctx->stack));
////	ctx->stack = reclaim(ctx, reclaim(ctx, ctx->stack));
////	printf("%.*s", (int)l, s);
////	return 0;
////}
////
////CELL here(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, (CELL)ctx->here, AS(ATOM, ctx->stack));
////	return 0;
////}
////
////CELL carcdr(CTX* ctx) {
////	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
////	if (TYPE(ctx->stack) != LIST) { return ERR_LIST_EXPECTED; }
////	if (CAR(ctx->stack) == 0) { return 0; }
////	CELL h = CAR(ctx->stack);
////	CAR(ctx->stack) = NEXT(CAR(ctx->stack));
////	CDR(h) = AS(TYPE(h), ctx->stack);
////	ctx->stack = h;
////	return 0;
////}
////
////CELL comma(CTX* ctx) {
////	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
////	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
////	CELL v = CAR(ctx->stack);
////	ctx->stack = reclaim(ctx, ctx->stack);
////	CELL* here = (CELL*)ctx->here;
////	ctx->stack = cons(ctx, sizeof(CELL), AS(ATOM, ctx->stack));
////	if (allot(ctx) != 0) { return ERR_NOT_ENOUGH_MEMORY; }
////	*here = v;
////	return 0;
////}
////
////CELL align(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, ALIGN(ctx->here, sizeof(CELL)) - (CELL)ctx->here, AS(ATOM, ctx->stack));
////	return allot(ctx);
////}
////
////CELL cells(CTX* ctx) {
////	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
////	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
////	CAR(ctx->stack) = CAR(ctx->stack) * sizeof(CELL);
////	return 0;
////}
////
////CELL immediate(CTX* ctx) {
////	if (ctx->latest == 0) return 0;
////	CDR(ctx->latest) = CDR(ctx->latest) | 2;
////	return 0;
////}
////
////#define WORD(n, f) \
////	ctx->latest = \
////		cons(ctx, \
////			cons(ctx, (CELL)n, AS(ATOM, \
////			cons(ctx, (CELL)f, AS(PRIM, 0)))), \
////		AS(ATOM, ctx->latest));
////
////#define IMM(n, f) \
////	ctx->latest = \
////		cons(ctx, \
////			cons(ctx, (CELL)n, AS(ATOM, \
////			cons(ctx, (CELL)f, AS(PRIM, 0)))), \
////		AS(PRIM, ctx->latest));
////
////CTX* bootstrap(CTX* ctx) {
////	// STACK PRIMITIVES
////	WORD("dup", &duplicate);
////	WORD("swap", &swap);
////	WORD("over", &over);
////	WORD("rot", &rot);
////	WORD("drop", &drop);
////
////	// IP PRIMITIVES
////	WORD("branch", &branch);
////	WORD("jump", &jump);
////	WORD("zjump", &zjump);
////
////	// ARITHMETIC PRIMITIVES
////	WORD("+", &add);
////	WORD("-", &sub);
////	WORD("*", &mul);
////	WORD("/", &division);
////	WORD("%", &mod);
////
////	// COMPARISON PRIMITIVES
////	WORD(">", &gt);
////	WORD("<", &lt);
////	WORD("=", &eq);
////
////	// BIT PRIMITIVES
////	WORD("and", &and);
////	WORD("or", &or);
////	WORD("invert", &invert);
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"immediate", AS(ATOM, 
////			cons(ctx, (CELL)&immediate, AS(PRIM, 0)))), 
////		AS(PRIM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"postpone", AS(ATOM, 
////			cons(ctx, (CELL)&postpone, AS(PRIM, 0)))), 
////		AS(PRIM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"cells", AS(ATOM, 
////			cons(ctx, (CELL)&cells, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)",", AS(ATOM, 
////			cons(ctx, (CELL)&comma, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"align", AS(ATOM, 
////			cons(ctx, (CELL)&align, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"carcdr", AS(ATOM, 
////			cons(ctx, (CELL)&carcdr, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"sliteral", AS(ATOM, 
////			cons(ctx, (CELL)&sliteral, AS(PRIM, 0)))), 
////		AS(PRIM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"type", AS(ATOM, 
////			cons(ctx, (CELL)&type, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"parse-name", AS(ATOM, 
////			cons(ctx, (CELL)&parse_name, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"x", AS(ATOM, 
////			cons(ctx, (CELL)&exec_x, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"execute", AS(ATOM, 
////			cons(ctx, (CELL)&exec, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"see", AS(ATOM, 
////			cons(ctx, (CELL)&see, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"parse", AS(ATOM, 
////			cons(ctx, (CELL)&parse, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"literal", AS(ATOM, 
////			cons(ctx, (CELL)&literal, AS(PRIM, 0)))), 
////		AS(PRIM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"[", AS(ATOM, 
////			cons(ctx, (CELL)&lbracket, AS(PRIM, 0)))), 
////		AS(PRIM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"]", AS(ATOM, 
////			cons(ctx, (CELL)&rbracket, AS(PRIM, 0)))), 
////		AS(PRIM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"@", AS(ATOM, 
////			cons(ctx, (CELL)&fetch, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"!", AS(ATOM, 
////			cons(ctx, (CELL)&store, AS(PRIM, 0)))), 
////		AS(ATOM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx, 
////			cons(ctx, (CELL)"s\"", AS(ATOM, 
////			cons(ctx, (CELL)&compile_str, AS(PRIM, 0)))), 
////		AS(PRIM, ctx->latest));	
////
////	ctx->latest = 
////		cons(ctx,
////			cons(ctx, (CELL)"allot", AS(ATOM,
////			cons(ctx, (CELL)&allot, AS(PRIM, 0)))),
////		AS(ATOM, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"append", AS(ATOM,
////			cons(ctx, (CELL)&append, AS(PRIM, 0)))),
////		AS(ATOM, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)".s", AS(ATOM,
////			cons(ctx, (CELL)&dump_stack, AS(PRIM, 0)))),
////		AS(ATOM, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)".c", AS(ATOM,
////			cons(ctx, (CELL)&dump_cpile, AS(PRIM, 0)))),
////		AS(ATOM, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"{", AS(ATOM,
////			cons(ctx, (CELL)&lbrace, AS(PRIM, 0)))),
////		AS(PRIM, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"}", AS(ATOM,
////			cons(ctx, (CELL)&rbrace, AS(PRIM, 0)))),
////		AS(PRIM, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"words", AS(ATOM,
////			cons(ctx, (CELL)&words, AS(PRIM, 0)))),
////		AS(ATOM, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"here", AS(ATOM,
////			cons(ctx, (CELL)&here, AS(PRIM, 0)))),
////		AS(ATOM, ctx->latest));
////
////	// VARIABLES
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"there", AS(ATOM,
////			cons(ctx, (CELL)&ctx->there, AS(ATOM, 0)))),
////		AS(LIST, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"state", AS(ATOM,
////			cons(ctx, (CELL)&ctx->state, AS(ATOM, 0)))),
////		AS(LIST, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"latest", AS(ATOM,
////			cons(ctx, (CELL)&ctx->latest, AS(ATOM, 0)))),
////		AS(LIST, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"hstack", AS(ATOM,
////			cons(ctx, (CELL)&ctx->hstack, AS(ATOM, 0)))),
////		AS(LIST, ctx->latest));
////
////	ctx->latest =
////		cons(ctx,
////			cons(ctx, (CELL)"base", AS(ATOM,
////			cons(ctx, (CELL)&ctx->base, AS(ATOM, 0)))),
////		AS(LIST, ctx->latest));
////
////	return ctx;
////}
//
//#endif
