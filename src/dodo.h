#include<inttypes.h>
#include<string.h>
#include<ctype.h>

typedef int8_t			BYTE;
typedef intptr_t		CELL;
typedef struct {
	BYTE *tib, *here;
	CELL there, size, free, stack, ip, rstack, compiled, defs, latest, compiling, token, in;
} CTX;
typedef CELL (*FUNC)(CTX*);

#define CAR(pair)								(*((CELL*)pair))
#define CDR(pair)								(*(((CELL*)pair) + 1))
#define NEXT(pair)							(CDR(pair) & -4)
#define TYPE(pair)							(CDR(pair) & 3)

#define AS(type, cell)					((cell & -4) | type)

#define ATOM										0
#define LIST										1
#define PRIMITIVE								2
//#define XT											3

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_UNDEFINED_WORD			-3
#define ERR_NOT_ENOUGH_MEMORY		-4

#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))
#define RESERVED(ctx)				((ctx->there) - ((CELL)ctx->here))

#define BOTTOM(ctx)			(((BYTE*)ctx) + sizeof(CTX))
#define TOP(ctx)				(ALIGN(((BYTE*)ctx) + ctx->size - (2*sizeof(CELL)) - 1, (2*sizeof(CELL))))

CTX* init(BYTE* block, CELL size) {
	if (size < sizeof(CELL) + 2*(2*sizeof(CELL))) return 0;
	CTX* ctx = (CTX*)block;	
	ctx->size = size;
	ctx->here = BOTTOM(ctx);
	ctx->there = ALIGN(BOTTOM(ctx), 2*sizeof(CELL));
	ctx->free = TOP(ctx);

	for (CELL pair = ctx->there; pair <= ctx->free; pair += 2*sizeof(CELL)) {
		CAR(pair) = pair == ctx->free ? 0 : pair + 2*sizeof(CELL);
		CDR(pair) = pair == ctx->there ? 0 : pair - 2*sizeof(CELL);
	}

	ctx->stack = ctx->rstack = ctx->compiled = ctx->defs = ctx->latest = ctx->compiling = 0;
	ctx->token = ctx->in = 0;
	ctx->tib = 0;

	return ctx;
}

CELL cons(CTX* ctx, CELL car, CELL cdr) {
	if (ctx->free == ctx->there) return 0;
	CELL pair = ctx->free;
	ctx->free = CDR(ctx->free);
	CAR(pair) = car;
	CDR(pair) = cdr;
	return pair;
}

CELL clone(CTX* ctx, CELL pair) {
	if (!pair) return 0;
	if (TYPE(pair) == LIST) {
		return cons(ctx, clone(ctx, CAR(pair)), AS(TYPE(pair), clone(ctx, NEXT(pair))));
	} else {
		return cons(ctx, CAR(pair), AS(TYPE(pair), clone(ctx, NEXT(pair))));
	}
}

CELL reclaim(CTX* ctx, CELL pair) {
	if (!pair) return 0;
	if (TYPE(pair) == LIST) { 
		while (CAR(pair) != 0) { 
			CAR(pair) = reclaim(ctx, CAR(pair)); 
		} 
	}
	CELL tail = NEXT(pair);
	CDR(pair) = ctx->free;
	CAR(pair) = 0;
	ctx->free = pair;
	return tail;
}

CELL length(CELL pair) { 
	CELL c = 0; 
	while (pair) { 
		c++; 
		pair = NEXT(pair); 
	} 
	return c; 
}

CELL depth(CELL pair) { 
	CELL c = 0; 
	while (pair) { 
		if (TYPE(pair) == LIST) {
			c += depth(CAR(pair)) + 1;
		} else {
			c++;
		}
		pair = NEXT(pair);
	}
	return c;
}

CELL execute(CTX* ctx, CELL xlist) {
	CELL result;
	ctx->ip = xlist;
	do {
		if (ctx->ip == 0) {
			if (ctx->rstack != 0) {
				ctx->ip = NEXT(CAR(ctx->rstack));
				ctx->rstack = reclaim(ctx, ctx->rstack);
			} else {
				return 0;
			}	
		}
		switch (TYPE(ctx->ip)) {
			case ATOM: 
				if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
				ctx->stack = cons(ctx, CAR(ctx->ip), AS(ATOM, ctx->stack));
				ctx->ip = NEXT(ctx->ip); 
				break;
			case LIST:
				if (length(ctx->free) < (depth(CAR(ctx->ip)) + 1)) { return ERR_STACK_OVERFLOW; }
				ctx->stack = cons(ctx, clone(ctx, CAR(ctx->ip)), AS(LIST, ctx->stack));
				ctx->ip = NEXT(ctx->ip);
				break;
			case PRIMITIVE:
				// Some cases of primitives (like jump or call) could be implemented here without
				// needing to call another function and without needing to use another type (XT)
				result = ((FUNC)CAR(ctx->ip))(ctx);
				if (result != 0) { return result; }
				break;
			//case XT:
			//	if (NEXT(ctx->ip) != 0) {
			//		if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
			//		ctx->rstack = cons(ctx, ctx->ip, AS(ATOM, ctx->rstack));
			//	}
			//	ctx->ip = CAR(CAR(ctx->ip));
			//	break;
		}
	} while (1);
}

// END OF CORE - Everything above this line is the bare minimum

//#define NFA(word)								(CAR(word))
//#define DFA(word)								(0)	// TODO: Address after zero ended word's name string
//#define CFA(word)								(NEXT(NFA(word)))
//#define BODY(word)							(CAR(CFA(word)))
//#define IMMEDIATE(word)					(IS(XT, word))
//
//#define PUSH(ctx, value, type, next) \
//	if ((result = cons(ctx, value, AS(type, next))) != 0) { \
//		return ERR_STACK_OVERFLOW; \
//	} else { \
//		next = result; \
//	}
//
//CELL find_token(CTX* ctx) {
//	CELL word = ctx->latest;
//	while (word && strncmp((BYTE*)CAR(NFA(word)), ctx->tib + ctx->token, ctx->in - ctx->token)) {
//		word = NEXT(word);
//	}
//	return word;
//}
//
//CELL parse_token(CTX* ctx) {
//	while (*(ctx->tib + ctx->in) != 0 && isspace(*(ctx->tib + ctx->in))) {
//		ctx->in++;
//	}
//	ctx->token = ctx->in;
//	while (*(ctx->tib + ctx->in) != 0 && !isspace(*(ctx->tib + ctx->in))) {
//		ctx->in++;
//	}
//	return ctx->in - ctx->token;
//}
//
//CELL evaluate(CTX* ctx, BYTE* str) {
//	CELL word, result;
//	char *endptr;
//	ctx->tib = str;
//	ctx->token = 0;
//	ctx->in = 0;
//	do {
//		if (parse_token(ctx) == 0) { 
//			return 0; 
//		} else if ((word = find_token(ctx)) != 0) {
//			if (!ctx->compiling || IMMEDIATE(word)) {
//				if ((result = execute(ctx, BODY(word))) != 0) {
//					return result;
//				}
//			} else {
//				PUSH(ctx, CFA(word), XT, CAR(ctx->compiled));
//			}
//		} else {
//			intmax_t number = strtoimax(ctx->tib + ctx->token, &endptr, 10);
//			if (number == 0 && endptr == (char*)(ctx->tib + ctx->token)) {
//				return ERR_UNDEFINED_WORD;
//			} else {
//				if (ctx->compiling) {
//					PUSH(ctx, number, ATOM, CAR(ctx->compiled));
//				} else {
//					PUSH(ctx, number, ATOM, ctx->stack);
//				}
//			}
//		}
//	} while (1);
//}
//
//// TODO: Doing this!!!!
//CELL allot(CTX* ctx, CELL bytes) {
// 	B* here = x->here;
// 	if (!b) { return 0;
// 	} else if (b < 0) { 
// 		x->here = (x->here + b) > BOTTOM(x) ? x->here + b : BOTTOM(x);
// 		while (x->there - (2*sizeof(C)) >= ALIGN(x->here, (2*sizeof(C)))) { 
// 			C t = x->there;
// 			x->there -= (2*sizeof(C));
// 			A(x->there) = t;
// 			D(x->there) = 0;
// 			D(t) = x->there;
// 		}
// 	} else {
// 		C p = x->there;
// 		while(A(p) == (p + (2*sizeof(C))) && p < (C)(x->here + b) && p < TOP(x)) { p = A(p);	}
// 		if (p >= (C)(here + b)) {
// 			x->there = p;
// 			D(x->there) = 0;
// 			x->here += b;
// 		} else {
// 			x->err = ERR_NOT_ENOUGH_MEMORY;
// 			return 0;
// 		}
// 	}
// 	return here;
//}
//
//CELL header(CTX* ctx, BYTE* name, CELL length) {
//	// TODO: Error management, required free cells: 3, required memory: length + sizeof(CELL) + 1
//	BYTE* str;
//	CELL word, xt, nfa;
//	if ((word = cons(ctx, 0, AS(LIST, 0))) == 0) { return ERR_STACK_OVERFLOW; }
//	if ((str = allot(ctx, length + sizeof(CELL) + 1)) == 0) { return ERR_NOT_ENOUGH_MEMORY; }
//	*((CELL*)str) = length;
//	str += sizeof(CELL);
//	strcpy(str, name);
//	if ((xt = cons(ctx, 0, AS(XT, word))) == 0) { return ERR_STACK_OVERFLOW; }
//	if ((nfa = cons(ctx, (CELL)str, AS(ATOM, xt))) == 0) { return ERR_STACK_OVERFLOW; }
//	CAR(word) = nfa;
//	return word;
//}
//
////C header(X* x, B* n, C l) {
////	*allot(x, 1) = 0;
////	// TODO: Alignment of cell size of string is not checked
////	C h = cns(x, 0, T(LST, 0));
////	B* s = strcpy(allot_str(x, l), n);
////	A(h) = cns(x, (C)s, T(ATM, cns(x, 0, T(LST, h))));
////	return h;
////}
//
////#include<inttypes.h>
////#include<stdio.h>
////#include<string.h>
////#include<errno.h>
////#include<ctype.h>
////
////typedef int8_t		B;
////typedef intptr_t	C;				// 16, 32 or 64 bits depending on system
////typedef struct {
////	B* here, * ibuf;
////	C there, size, free, stack, rstack, cpile, dict, err, comp, tkst, tkln;
////} X;
////typedef void (*FUNC)(X*);
////
////#define A(p)										(*((C*)p))
////#define D(p)										(*(((C*)p) + 1))
////#define D_(p)										(D(p) & -4)	
////#define _D(p)										(D(p) & 3)
////#define T(t, c)									((c & -4) | t)
////#define R(p, a)									(D(p) = T(_D(p), a))
////
////#define ATM											0
////#define LST											1
////#define PRM											2
////#define JMP											3
////
////#define REF(p)									(_D(p) & 1)
////
////C length(C p) { C c = 0; while (p) { c++; p = D_(p); } return c; }
////C depth(C p) { C c = 0; while (p) { c += REF(p) ? depth(A(p)) + 1 : 1; p = D_(p); } return c; }
////C mlength(C p, C n) { C c = 0; while (p && c < n) { c++; p = D_(p); } return c == n; }
////C last(C p) { if (!p) return 0; while (D_(p)) { p = D_(p); } return p; }
////
////#define F(x)										x->free
////#define S(x)										x->stack
////#define P(x)										x->cpile
////#define TK(x)										(x->ibuf + x->tkst)
////
////#define NFA(w)									((B*)(A(A(w))))
////#define DFA(w)									(NFA(w) + count(NFA(w)) + 1)
////#define XT(w)										(D_(A(w)))	
////#define BODY(w)									(A(XT(w)))
////#define IMMEDIATE(w)						(*(NFA(w) - sizeof(C) - 1))
////
////#define ERR_NOT_ENOUGH_MEMORY		-1
////#define ERR_OVERFLOW						-2
////#define ERR_UNDERFLOW						-3
////#define ERR_UNDEFINED_WORD			-4
////#define ERR_BYE									-5
////
////C cns(X* x, C a, C d) { C p; return F(x) ? (p = F(x), F(x) = D(F(x)), A(p) = a, D(p) = d, p) : 0; }
////C rcl(X* x, C l) { C r; return l ? (r = D_(l), D(l) = F(x), A(l) = 0, F(x) = l, r) : 0; }
////C cln(X* x, C l) { return l ? cns(x, REF(l) ? cln(x, A(l)) : A(l), T(_D(l), cln(x, D_(l)))) : 0; }
////void rmv(X* x, C l) { while (l) { if (REF(l)) {	rmv(x, A(l)); } l = rcl(x, l); } }
////
////void push(X* x, C t, C v) { C p = cns(x, v, T(t, S(x))); if (p) S(x) = p; }
////C pop(X* x) { C v = A(S(x)); S(x) = rcl(x, S(x)); return v; }
////
////void cppush(X* x) { C s = cns(x, 0, T(LST, P(x))); if (s) P(x) = s; }
////void cspush(X* x, C p) { R(p, A(P(x))); A(P(x)) = p; }
////#define STACK_TO_LIST	C t = A(D_(P(x)));	A(D_(P(x))) = P(x);	P(x) = D_(P(x)); R(A(P(x)), t);
////#define TO_DATA_STACK	R(P(x), S(x)); S(x) = P(x); P(x) = 0;
////void cppop(X* x) { if ( D_(P(x)) ) { STACK_TO_LIST } else { TO_DATA_STACK } }
////
////void inner(X* x, C xlist) {
////		C ip = xlist;
////		while(!x->err && ip) {
////			switch(_D(ip)) {
////				case ATM: 
////					push(x, ATM, A(ip)); 
////					ip = D_(ip); 
////					break;
////				case LST: 
////					push(x, LST, cln(x, A(ip))); 
////					ip = D_(ip); 
////					break;
////				case JMP:
////					if (_D(A(ip)) == ATM) { 
////						ip = A(A(ip)); /* JUMP */
////					} else if (D_(A(ip))) { 
////						ip = pop(x) ? A(A(ip)) : A(D_(A(ip))); /* BRANCH */
////					} else { 
////						ip = D_(ip) ? (inner(x, A(A(A(ip)))), D_(ip)) : A(A(A(ip))); /* CALL */
////					}
////					break;
////				case PRM: 
////					if (A(ip)) { 
////						((FUNC)A(ip))(x); ip = D_(ip); /* PRIMITIVE */
////					} else { 
////						ip = D_(ip) ? (inner(x, xlist), D_(ip)) : xlist; /* RECURSION */
////					}
////					break;
////			}
////		}
////	}
////
////void compile(X* x, C xt) { 
////	if (IMMEDIATE(D_(xt))) {
////		inner(x, A(xt));
////	} else {
////		cspush(x, cns(x, cns(x, xt, T(LST, 0)), T(JMP, 0))); 
////	}
////}
////
////C number(X* x) { 
////	char* endptr;
////	intmax_t n = strtoimax(TK(x), &endptr, 10);
////	if (n == 0 && endptr == (char*)TK(x)) { x->err = ERR_UNDEFINED_WORD; return 0; } 
////	else { x->comp ? cspush(x, cns(x, n, T(ATM, 0))) : push(x, ATM, n); return 1; }
////}
////
////C word(X* x, C w) { x->comp ? compile(x, XT(w)) : inner(x, BODY(w)); return 1; }
////
////#define NOT_WORDS_NAME strncmp(NFA(w), n, l)
////C find(X* x, B* n, C l) { C w = x->dict; while (w && NOT_WORDS_NAME) { w = D_(w); } return w; }
////
////#define SPACES(x) while (*TK(x) != 0 && isspace(*TK(x))) { x->tkst++; }
////#define TOKEN(x) while (*(TK(x) + x->tkln) != 0 && !isspace(*(TK(x) + x->tkln))) { x->tkln++; }
////#define RST_TKN(x) x->tkst += x->tkln; x->tkln = 0
////C parse(X* x) { RST_TKN(x); SPACES(x); TOKEN(x); return x->tkln; }
////
////#define RST	x->ibuf = s; x->tkst = 0; x->tkln = 0;
////#define FIND(x) find(x, TK(x), x->tkln)
////void outer(X* x, B* s) { C w; RST; while (parse(x) ? (w = FIND(x)) ? word(x, w) : number(x) : 0); }
////
////#define ALIGN(addr, bound)	((((C)addr) + (bound - 1)) & ~(bound - 1))
////#define RESERVED(x)					((x->there) - ((C)x->here))
////
////#define BOTTOM(x)						(((B*)x) + sizeof(X))
////#define TOP(x)							(ALIGN(((B*)x) + x->size - (2*sizeof(C)) - 1, (2*sizeof(C))))
////
////#define count(s)				(*((C*)(s - sizeof(C))))
////
////B* allot_str(X* x, C l) { *(C*)allot(x, sizeof(C)) = l; return allot(x, l + 1); }
////
////C body(X* x, C h, C l) { BODY(h) = l; return h; }
////
////C reveal(X* x, C h) {	R(h, x->dict); x->dict = h;	return h; }
////
////X* init(B* block, C size) {
////		if (size < sizeof(C) + 2*(2*sizeof(C))) return 0;
////		X* x = (X*)block;	
////		x->size = size;
////		x->here = BOTTOM(x);
////		x->there = ALIGN(BOTTOM(x), 2*sizeof(C));
////		x->free = TOP(x);
////	
////		for (C p = x->there; p <= x->free; p += 2*sizeof(C)) {
////			A(p) = p == F(x) ? 0 : p + 2*sizeof(C);
////			D(p) = p == x->there ? 0 : p - 2*sizeof(C);
////		}
////	
////		x->err = x->comp = 0;
////		x->dict = 0;
////		x->stack = x->rstack = P(x) = 0;
////		x->tkst = x->tkln = 0;
////		x->ibuf = 0;
////	
////		return x;
////	}
////
////#define O(x)				if (!F(x)) { x->err = ERR_OVERFLOW; return; }
////#define On(x, l)		if (!mlength(F(x), depth(l))) { x->err = ERR_OVERFLOW; return; }
////#define U(x)				if (!S(x)) { x->err = ERR_UNDERFLOW; return; }
////#define U2(x)				if (!S(x) || !D_(S(x))) { x->err = ERR_UNDERFLOW; return; }
////#define U3(x)				if (!S(x) || !D_(S(x)) || !D_(D_(S(x)))) { x->err = ERR_UNDERFLOW; return; }
////#define OU2(x)			O(x); U2(x);
////
////#define W(n)				void n(X* x)
////
////W(_lbrace) { x->comp = 1; cppush(x); }
////W(_rbrace) { cppop(x); x->comp = P(x) != 0; }
////
////W(_empty) { O(x); push(x, LST, 0); }
////
////W(jAA) { OU2(x); C l = S(x); S(x) = D_(D_(S(x))); R(D_(l), 0); push(x, LST, l); }
////W(jAL) { OU2(x); C l = D_(S(x)); D(S(x)) = T(ATM, A(D_(S(x)))); A(l) = S(x); S(x) = l; }
////W(jLA) { OU2(x); C t = D_(D_(S(x))); C l = last(A(S(x))); R(l, D_(S(x))); R(D_(l), 0); R(S(x), t); }
////W(jLL) { OU2(x); C t = D_(S(x)); C l = last(A(S(x))); R(l, A(D_(S(x)))); R(S(x), rcl(x, t)); }
////W(_join) { REF(D_(S(x))) ? (REF(S(x)) ? jLL(x) : jAL(x)) : (REF(S(x)) ? jLA(x) : jAA(x)); }
////W(_quote) { U(x); C t = S(x); S(x) = D_(S(x)); R(t, 0); push(x, LST, t); }
////
////W(dupA) { U(x); O(x); push(x, _D(S(x)), A(S(x))); }
////W(dupL) { U(x); On(x, A(S(x))); push(x, _D(S(x)), cln(x, A(S(x)))); }
////W(_dup) { REF(S(x)) ? dupL(x) : dupA(x); }
////W(_swap) { U2(x); C t = D_(S(x)); R(S(x), D_(D_(S(x)))); R(t, S(x)); S(x) = t; }
////W(_drop) { U(x); if (REF(S(x))) rmv(x, A(S(x))); pop(x); }
////W(overV) { O(x); push(x, _D(D_(S(x))), A(D_(S(x)))); }
////W(overR) { On(x, A(D_(S(x)))); push(x, _D(D_(S(x))), cln(x, A(D_(S(x))))); } 
////W(_over) { U2(x); REF(D_(S(x))) ? overR(x) : overV(x); }
////W(_rot) { C t = D_(D_(S(x))); R(D_(S(x)), D_(D_(D_(S(x))))); R(t, S(x)); S(x) = t; }
////
////W(_add) { U2(x); A(D_(S(x))) = A(D_(S(x))) + A(S(x)); pop(x); }
////W(_sub) { U2(x); A(D_(S(x))) = A(D_(S(x))) - A(S(x)); pop(x); }
////W(_mul) { U2(x); A(D_(S(x))) = A(D_(S(x))) * A(S(x)); pop(x); }
////W(_div) { U2(x); A(D_(S(x))) = A(D_(S(x))) / A(S(x)); pop(x); }
////W(_mod) { U2(x); A(D_(S(x))) = A(D_(S(x))) % A(S(x)); pop(x); }
////
////W(_gt) { U2(x); A(D_(S(x))) = A(D_(S(x))) > A(S(x)); pop(x); }
////W(_lt) { U2(x); A(D_(S(x))) = A(D_(S(x))) < A(S(x)); pop(x); }
////W(_eq) { U2(x); A(D_(S(x))) = A(D_(S(x))) == A(S(x)); pop(x); }
////W(_neq) { U2(x); A(D_(S(x))) = A(D_(S(x))) != A(S(x)); pop(x); }
////
////W(_and) { U2(x); A(D_(S(x))) = A(D_(S(x))) & A(S(x)); pop(x); }
////W(_or) { U2(x); A(D_(S(x))) = A(D_(S(x))) | A(S(x)); pop(x); }
////W(_invert) { U(x); A(S(x)) = ~A(S(x)); }
////W(_not) { U(x); A(S(x)) = !A(S(x)); }
////
////W(_allot) { allot(x, pop(x)); }
////W(_align) { push(x, ATM, ALIGN(x->here, sizeof(C)) - ((C)x->here)); _allot(x); }
////
////void dump_list(C p) {
////	if (p) {
////		dump_list(D_(p));
////		switch (_D(p)) {
////			case ATM: printf("%ld ", A(p)); break;
////			case LST: printf("{ "); dump_list(A(p)); printf("} "); break;
////		}
////	}
////}
////
////W(_bye) { x->err = ERR_BYE; }
////
////W(_dump_stack) {
////	printf("<%ld> ", length(S(x)));
////	dump_list(S(x));
////	printf("\n");
////}
////
////#define WORD(n, f)	(reveal(x, body(x, header(x, n, strlen(n)), cns(x, (C)f, T(PRM, 0)))))
////
////X* bootstrap(X* x) {
////		WORD("{}", &_empty);
////		WORD("join", &_join);
////		WORD("quote", &_quote);
////		WORD("dup", &_dup);
////		WORD("swap", &_swap);
////		WORD("drop", &_drop);
////		WORD("over", &_over);
////		WORD("rot", &_rot);
////		WORD("+", &_add);
////		WORD("-", &_sub);
////		WORD("*", &_mul);
////		WORD("/", &_div);
////		WORD("%", &_mod);
////		WORD(">", &_gt);
////		WORD("<", &_lt);
////		WORD("=", &_eq);
////		WORD("and", &_and);
////		WORD("or", &_or);
////		WORD("invert", &_invert);
////		WORD("allot", &_allot);
////		WORD("align", &_align);
////		WORD("bye", &_bye);
////		WORD(".s", &_dump_stack);
////		// TODO: These two should be immediate words ???
////		IMMEDIATE(WORD("{", &_lbrace)) = 1;
////		IMMEDIATE(WORD("}", &_rbrace)) = 1;
////		return x;
////	}
////
////////// Source code for getch is taken from:
////////// Crossline readline (https://github.com/jcwangxp/Crossline).
////////// It's a fantastic readline cross-platform replacement, but only getch was
////////// needed and there's no need to include everything else.
////////#ifdef _WIN32	// Windows
////////int dodo_getch (void) {	fflush (stdout); return _getch(); }
////////#else
////////int dodo_getch ()
////////{
////////	char ch = 0;
////////	struct termios old_term, cur_term;
////////	fflush (stdout);
////////	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
////////	cur_term = old_term;
////////	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
////////	cur_term.c_cc[VMIN] = 1;
////////	cur_term.c_cc[VTIME] = 0;
////////	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
////////	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
////////	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
////////	return ch;
////////}
////////#endif
////////
////////void _key(X* x) { push(x, dodo_getch()); }
////////void _emit(X* x) { C K = T(x); pop(x); K == 127 ? printf ("\b \b") : printf ("%c", (char)K); }
////
//////////#define IS_IMMEDIATE(w)		(TYPE(w->ref) & 1)
//////////
//////////void _immediate(X* x) {
//////////	x->dict->ref = AS(1, REF(x->dict));
//////////}
////
