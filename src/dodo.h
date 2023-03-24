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
	C th, sz, fr, ds, err, rs, st, lt, in, il, free;
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
	x->in = x->il = x->st = x->err = x->lt = 0;

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

enum Errors { E_EIB = -12, E_ZLN, E_EW, E_EA, E_EL, E_EAA, E_UW, E_NER, E_NEM, E_DBZ, E_UF, E_OF, E_IP = 1 };

#define ERR(x, c, e)	if (c) { x->err = e; return; }

#define OF(x, n)			ERR(x, x->free < n, E_OF)
#define UF1(x)				ERR(x, S(x) == 0, E_UF)
#define UF2(x)				ERR(x, S(x) == 0 || N(S(x)) == 0, E_UF)
#define UF3(x)				ERR(x, !S(x) || !N(S(x)) || !N(N(S(x))), E_UF)

#define EA(x)					UF1(x); ERR(x, T(S(x)) != ATM, E_EA)
#define EL(x)					UF1(x); ERR(x, T(S(x)) != LST, E_EL)
#define EW(x)					UF1(x); ERR(x, T(S(x)) != WRD, E_EW)
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

enum Words { PRIM, DEF, NDCS_PRIM, NDCS_DEF };

#define PRIMITIVE(w)	(T(w) == PRIM || T(w) == NDCS_PRIM)
#define NDCS(w)				(T(w) == NDCS_PRIM || T(w) == NDCS_DEF)

#define NFA(w)				((B*)A(A(w)))
#define CS(w)					(A(N(A(w))))
#define XT(w)					(NDCS(w) ? N(N(A(w))) : N(A(w)))

void dump_cell(X* x, C c);
void dump_list(X* x, C l);
void dump_stack(X* x, C s);
void dump_context(X* x);

typedef void (*FUNC)(X*);

#define NEXT(x) \
	if (R(x)) { R(x) = recl(x, R(x)); } \
	while (!R(x) && N(x->rs)) { x->rs = recl(x, x->rs); if(R(x)){ R(x) = recl(x, R(x)); } }

#define CALL(x, l)			(x->rs = cons(x, clone(x, l), AS(LST, x->rs)))

#define STEP(x) ( {\
	if (R(x)) { \
		switch(T(R(x))) { \
			case ATM: S(x) = cons(x, A(R(x)), AS(ATM, S(x))); NEXT(x); break; \
			case LST: S(x) = cons(x, clone(x, A(R(x))), AS(LST, S(x))); NEXT(x); break; \
			case PRM: ({ C r = R(x); ((FUNC)A(R(x)))(x); if (r == R(x)) NEXT(x); }); break; \
			case WRD: CALL(x, XT(A(R(x)))); break; \
		} \
	}; \
	R(x) != 0; })

#define JUMP(x, l) R(x) = clone(x, l)

// EXECUTION PRIMITIVES

void exec_i(X* x) { UF1(x);
	switch (T(S(x))) {
		case ATM: ({ C p = A(S(x)); S(x) = recl(x, S(x)); ((FUNC)p)(x); }); break;
		case LST: CALL(x, A(S(x))); S(x) = recl(x, S(x)); STEP(x); break;
		case PRM: ({ C p = A(S(x)); S(x) = recl(x, S(x)); ((FUNC)p)(x); }); break;
		case WRD: CALL(x, XT(A(S(x)))); STEP(x); break;
	}
}
void exec_x(X* x) { UF1(x); duplicate(x); exec_i(x); }
void branch(X* x) { UF3(x); if (!A(N(N(S(x))))) { swap(x); } drop(x); swap(x); drop(x); exec_i(x); }

void lbracket(X* x) { x->st = 0; }
void rbracket(X* x) { x->st = 1; }

// PARSING

#define TC(x)							(*(x->tb + x->in))

#define PARSE(x, cond)		while(TC(x) && cond && x->in < x->il) { x->in++; }
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

// OUTER INTERPRETER

#define FOUND(w, a, u)	(strlen(NFA(w)) == u && !strncmp(NFA(w), (B*)a, u))
void find_name(X* x) { UF2(x); ERR(x, A(S(x)) == 0, E_ZLN);
	C w = x->lt; 
	while(w && !FOUND(w, A(N(S(x))), A(S(x)))) { w = N(w); }
	if (w) { S(x) = recl(x, S(x)); A(S(x)) = w; D(S(x)) = AS(WRD, N(S(x))); }
	else { OF(x, 1); S(x) = cons(x, 0, AS(ATM, S(x))); }
}

void compile(X* x) { UF1(x); EW(x); A(S(x)) = A(XT(A(S(x)))); D(S(x)) = AS(PRM, N(S(x))); }

void outer(X* x) {
	char *endptr;
	do {
		printf("%.*s^%.*s\n", (int)x->in, x->tb, (int)(x->il - x->in), x->tb + x->in);
		parse_name(x); ERR(x, A(S(x)) == 0, E_EIB);
		printf("TOKEN: %.*s\n", (int)A(S(x)), (B*)A(N(S(x))));
		find_name(x);
		printf("<%ld> ", length(S(x))); dump_stack(x, S(x));
		if (A(S(x)) == 0) {
			C addr = A(N(N(S(x)))); S(x) = recl(x, recl(x, recl(x, S(x))));
			intmax_t n = strtoimax((B*)addr, &endptr, 10);
			ERR(x, n == 0 && endptr == (char*)addr, E_UW);
			S(x) = cons(x, n, AS(ATM, S(x)));
		} else {
			// TODO: exec_i can not be called from code !!!!
			if (x->st == 0) {	exec_i(x); }
			else if (NDCS(A(S(x))) && CS(A(S(x))) == 0) { exec_i(x); }
			/* } else if (NDCS(w)) { Execute custom NDCS xt } */
			/* } else if ( has code generator ) { execute code generator } */
			else { compile(x); }
		}
	} while(1);
}

void evaluate(X* x) { UF2(x); 
	x->in = 0; x->il = A(S(x)); x->tb = (B*)A(N(S(x))); S(x) = recl(x, recl(x, S(x))); 
	outer(x);
}

// ----------------------------------------------------------------------------- END OF CORE

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
		case WRD: printf("%s ", (B*)NFA(A(c))); break;
	}
}

void dump_stack(X* x, C l) {
	if (!l) return;
	if (N(l)) { dump_stack(x, N(l)); dump_cell(x, l); }
	else dump_cell(x, l);
}

void dump_context(X* x) {
	printf("<%ld> ", length(S(x)));
	dump_stack(x, S(x));
	printf("‖ ");
	C p = x->rs; while (p) { dump_list(x, A(p)); printf("| "); p = N(p); }
	printf("\n");
}

// BOOTSTRAPPING

#define ADD_PRIMITIVE(x, n, f, i) \
	x->lt = cons(x, cons(x, (C)n, AS(ATM, cons(x, (C)f, AS(PRM, 0)))), AS(i, x->lt));

X* bootstrap(X* x) {
	ADD_PRIMITIVE(x, "branch", &branch, PRIM);
	ADD_PRIMITIVE(x, "i", &exec_i, PRIM);
	ADD_PRIMITIVE(x, "x", &exec_x, PRIM);

	ADD_PRIMITIVE(x, "[", &lbracket, NDCS_PRIM);
	ADD_PRIMITIVE(x, "]", &rbracket, PRIM);

	ADD_PRIMITIVE(x, "+", &add, PRIM);
	ADD_PRIMITIVE(x, "-", &sub, PRIM);
	ADD_PRIMITIVE(x, "*", &mul, PRIM);
	ADD_PRIMITIVE(x, "/", &division, PRIM);
	ADD_PRIMITIVE(x, "mod", &mod, PRIM);

	ADD_PRIMITIVE(x, ">", &gt, PRIM);
	ADD_PRIMITIVE(x, "<", &lt, PRIM);
	ADD_PRIMITIVE(x, "=", &eq, PRIM);
	ADD_PRIMITIVE(x, "<>", &neq, PRIM);

	ADD_PRIMITIVE(x, "and", &and, PRIM);
	ADD_PRIMITIVE(x, "or", &or, PRIM);
	ADD_PRIMITIVE(x, "invert", &invert, PRIM);

	ADD_PRIMITIVE(x, "dup", &duplicate, PRIM);
	ADD_PRIMITIVE(x, "swap", &swap, PRIM);
	ADD_PRIMITIVE(x, "drop", &drop, PRIM);
	ADD_PRIMITIVE(x, "over", &over, PRIM);
	ADD_PRIMITIVE(x, "rot", &rot, PRIM);

	ADD_PRIMITIVE(x, "{}", &empty, PRIM);
	ADD_PRIMITIVE(x, "s>l", &stack_to_list, PRIM);
	ADD_PRIMITIVE(x, "l>s", &list_to_stack, PRIM);

	ADD_PRIMITIVE(x, "grow", &grow, PRIM);
	ADD_PRIMITIVE(x, "shrink", &shrink, PRIM);
	ADD_PRIMITIVE(x, "allot", &allot, PRIM);

	//ADD_PRIMITIVE(x, "latest", &latest, 0);
	//ADD_PRIMITIVE(x, "here", &here, 0);

	//ADD_PRIMITIVE(x, "!", &store, 0);
	//ADD_PRIMITIVE(x, "@", &fetch, 0);
	//ADD_PRIMITIVE(x, "b!", &bstore, 0);
	//ADD_PRIMITIVE(x, "b@", &bfetch, 0);

	//ADD_PRIMITIVE(x, "key", &key, 0);
	//ADD_PRIMITIVE(x, "emit", &emit, 0);

	return x;
}

#endif

//C depth(C p) { return p ? (T(p) == LST ? 1 + depth(A(p)) + depth(N(p)) : 1 + depth(N(p))) : 0; }
