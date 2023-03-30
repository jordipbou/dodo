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
	B *ib, *hr;
	C th, sz, fr, ds, err, rs, st, lt, in, il, free, total;
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

	x->total = x->free;
	x->ib = 0;
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

#define ERR(x, c, ...)	if (c) { x->err = __VA_ARGS__; return; }

#define OF(x, n)			ERR(x, x->free < n, E_OF)
#define UF1(x)				ERR(x, S(x) == 0, E_UF)
#define UF2(x)				ERR(x, S(x) == 0 || N(S(x)) == 0, E_UF)
#define UF3(x)				ERR(x, !S(x) || !N(S(x)) || !N(N(S(x))), E_UF)

#define EA(x)					UF1(x); ERR(x, T(S(x)) != ATM, E_EA)
#define EL(x)					UF1(x); ERR(x, T(S(x)) != LST, E_EL)
#define EW(x)					UF1(x); ERR(x, T(S(x)) != WRD, E_EW)
#define EAA(x)				UF2(x); ERR(x, T(S(x)) != ATM || T(N(S(x))) != ATM, E_EAA)

#define EIB(x)				ERR(x, !x->ib, E_EIB)

#define E(x, p)				({ p(x); if(x->err) { return; } })

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
	dump_context(x); \
	R(x); })

#define JUMP(x, l) R(x) = clone(x, l)

// EXECUTION PRIMITIVES

void exec_i(X* x) { UF1(x);
	switch (T(S(x))) {
		case ATM: ({ C p = A(S(x)); S(x) = recl(x, S(x)); ((FUNC)p)(x); }); break;
		case LST: CALL(x, A(S(x))); S(x) = recl(x, S(x)); STEP(x); break;
		case PRM: ({ C p = A(S(x)); S(x) = recl(x, S(x)); ((FUNC)p)(x); }); break;
		case WRD: 
			({ C r = R(x); CALL(x, XT(A(S(x)))); S(x) = recl(x, S(x)); STEP(x); }); break;
	}
}
void exec_x(X* x) { UF1(x); duplicate(x); exec_i(x); }
void branch(X* x) { UF3(x); if (!A(N(N(S(x))))) { swap(x); } drop(x); swap(x); drop(x); exec_i(x); }

void lbracket(X* x) { x->st = 0; }
void rbracket(X* x) { x->st = 1; }

// PARSING

#define TC(x)							(*(x->ib + x->in))

#define PARSE(x, cond)		while(TC(x) && cond && x->in < x->il) { x->in++; }
void parse(X* x) { EA(x); OF(x, 1); EIB(x);
	PARSE(x, TC(x) != A(S(x))); A(S(x)) = (C)x->ib; S(x) = cons(x, x->in, AS(ATM, S(x))); x->in++;
}
void parse_name(X* x) { OF(x, 2); EIB(x);
	PARSE(x, isspace(TC(x))); S(x) = cons(x, (C)(x->ib + x->in), AS(ATM, S(x)));
	PARSE(x, !isspace(TC(x))); S(x) = cons(x, (C)((x->ib + x->in) - A(S(x))) , AS(ATM, S(x)));
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

void header(X* x) {
	parse_name(x); ERR(x, A(S(x)) == 0, E_ZLN);
	C u = A(S(x)); B* addr = (B*)A(N(S(x)));
	A(S(x))++;
	B* hr = x->hr;
	allot(x);
	C i; for (i = 0; i < u; i++) { hr[i] = addr[i]; } hr[u] = 0;
	A(S(x)) = (C)hr;
	S(x) = cons(x, 0, AS(WRD, S(x)));
	swap(x);
}

void type(X* x) {
	C u = A(S(x)); B* addr = (B*)A(N(S(x))); S(x) = recl(x, recl(x, S(x)));
	printf("%.*s", (int)u, addr);
}

void stack_to_word(X* x) {
	C l = S(x);
	S(x) = 0;
	x->ds = recl(x, x->ds);
	l = reverse(l, 0);
	A(l) = N(l);
	D(l) = AS(DEF, x->lt);
	x->lt = l;

}

void words(X* x) {
	printf("\n");
	C w = x->lt;
	while (w) {
		printf("%s ", NFA(w));
		w = N(w);
	}
	printf("\n");
}

void see(X* x) {
	C w = A(N(S(x)));
	printf(": %s ", NFA(w)); dump_list(x, XT(w), 0);
}

void colon(X* x) {
	E(x, empty);
	E(x, list_to_stack);
	E(x, header);
	E(x, rbracket);
}

void semicolon(X* x) {
	E(x, lbracket);
	E(x, stack_to_word);
}

void immediate(X* x) {
	if (T(x->lt) == DEF) {
		D(x->lt) = AS(NDCS_DEF, N(x->lt));
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

void compile(X* x) { UF1(x); EW(x); 
	C w = A(S(x)); S(x) = recl(x, S(x));
	if (PRIMITIVE(w)) S(x) = cons(x, A(XT(w)), AS(PRM, S(x)));
	else S(x) = cons(x, w, AS(WRD, S(x)));
}

void postpone(X* x) {
	parse_name(x);
	find_name(x);
	compile(x);
}

void outer(X* x) {
	char *endptr;
	do {
		parse_name(x); ERR(x, A(S(x)) == 0, E_EIB; drop(x); drop(x));
		find_name(x);
		if (A(S(x)) == 0) {
			C addr = A(N(N(S(x)))); S(x) = recl(x, recl(x, recl(x, S(x))));
			intmax_t n = strtoimax((B*)addr, &endptr, 10);
			ERR(x, n == 0 && endptr == (char*)addr, E_UW);
			S(x) = cons(x, n, AS(ATM, S(x)));
			/*dump_context(x);*/
		} else {
			if (x->st == 0) {
				C t = S(x); S(x) = N(S(x));
				LK(t, R(x)); R(x) = t;
				/*dump_context(x);*/
				while(STEP(x));
			} else if (NDCS(A(S(x))) && CS(A(S(x))) == 0) { exec_i(x); }
			/* } else if (NDCS(w)) { Execute custom NDCS xt } */
			/* } else if ( has code generator ) { execute code generator } */
			else { compile(x); }
		}
	} while(1);
}

void evaluate(X* x) { UF2(x); 
	x->in = 0; x->il = A(S(x)); x->ib = (B*)A(N(S(x))); S(x) = recl(x, recl(x, S(x))); 
	outer(x);
}

// ----------------------------------------------------------------------------- END OF CORE

void dump_cell(X* x, C c, C d);

void dump_list(X* x, C l, C d) {
	if (!l) return;
	if (!d) { while (l) { dump_cell(x, l, d); l = N(l); } }
	else {
		if (N(l)) { dump_list(x, N(l), d); }
		dump_cell(x, l, d);
	}
}

void dump_cell(X* x, C c, C d) {
	if (!c) return;
	switch(T(c)) {
		case ATM: printf("#%ld ", A(c)); break;
		case LST: printf("{ "); dump_list(x, A(c), d); printf("} "); break;
		case PRM:  // This should use find_primitive
			if (A(c) == (C)&swap) printf("P:swap ");
			else if (A(c) == (C)&duplicate) printf("P:dup ");
			else if (A(c) == (C)&gt) printf("P:> ");
			else if (A(c) == (C)&sub) printf("P:- ");
			else if (A(c) == (C)&add) printf("P:+ ");
			else if (A(c) == (C)&rot) printf("P:rot ");
			else if (A(c) == (C)&exec_x) printf("P:x ");
			else if (A(c) == (C)&exec_i) printf("P:i ");
			else if (A(c) == (C)&branch) printf("P:branch ");
			else if (A(c) == (C)&drop) printf("P:drop ");
			else if (A(c) == (C)&list_to_stack) printf("P:l>s ");
			else if (A(c) == (C)&stack_to_list) printf("P:s>l ");
			else if (A(c) == (C)&empty) printf("P:{} ");
			else if (A(c) == (C)&mul) printf("P:* ");
			else if (A(c) == (C)&div) printf("P:/ ");
			else if (A(c) == (C)&lbracket) printf("P:[ ");
			else if (A(c) == (C)&rbracket) printf("P:] ");
			else printf("P:%ld ", A(c));
			break;
		case WRD: 
			if (c != 0) {
				if (A(c) != 0) {
					printf("W:%s ", (B*)NFA(A(c)));
				} else {
					printf("HEADER ");
				}
			}
			break;
	}
}

void dump_context(X* x) {
	printf("%c/%ld/[%ld:%ld]", x->st ? 'C' : 'I', x->err, x->total, x->free);
	if (x->ib) {
		if ((x->ib + x->in)[x->il - x->in] == 0) {
			printf("'%.*s^%.*s'", (int)x->in, x->ib, (int)(x->il - x->in - 1), x->ib + x->in);
		} else {
			printf("'%.*s^%.*s'", (int)x->in, x->ib, (int)(x->il - x->in), x->ib + x->in);
		}
	}
	dump_list(x, x->ds, 1);
	printf("‖ ");
	dump_list(x, x->rs, 0);
	printf("\n");
}

// BOOTSTRAPPING

#define ADD_PRIMITIVE(x, n, f) \
	x->lt = cons(x, cons(x, (C)n, AS(ATM, cons(x, (C)f, AS(PRM, 0)))), AS(PRIM, x->lt));

#define ADD_NDCS_PRIM(x, n, f) \
	x->lt = cons(x, cons(x, (C)n, AS(ATM, cons(x, 0, AS(LST, cons(x, (C)f, AS(PRM, 0)))))), AS(NDCS_PRIM, x->lt));

X* bootstrap(X* x) {
	ADD_PRIMITIVE(x, "branch", &branch);
	ADD_PRIMITIVE(x, "i", &exec_i);
	ADD_PRIMITIVE(x, "x", &exec_x);

	ADD_NDCS_PRIM(x, "[", &lbracket);
	ADD_PRIMITIVE(x, "]", &rbracket);

	ADD_PRIMITIVE(x, "+", &add);
	ADD_PRIMITIVE(x, "-", &sub);
	ADD_PRIMITIVE(x, "*", &mul);
	ADD_PRIMITIVE(x, "/", &division);
	ADD_PRIMITIVE(x, "mod", &mod);

	ADD_PRIMITIVE(x, ">", &gt);
	ADD_PRIMITIVE(x, "<", &lt);
	ADD_PRIMITIVE(x, "=", &eq);
	ADD_PRIMITIVE(x, "<>", &neq);

	ADD_PRIMITIVE(x, "and", &and);
	ADD_PRIMITIVE(x, "or", &or);
	ADD_PRIMITIVE(x, "invert", &invert);

	ADD_PRIMITIVE(x, "dup", &duplicate);
	ADD_PRIMITIVE(x, "swap", &swap);
	ADD_PRIMITIVE(x, "drop", &drop);
	ADD_PRIMITIVE(x, "over", &over);
	ADD_PRIMITIVE(x, "rot", &rot);

	ADD_PRIMITIVE(x, "{}", &empty);
	ADD_PRIMITIVE(x, "s>l", &stack_to_list);
	ADD_PRIMITIVE(x, "l>s", &list_to_stack);

	ADD_PRIMITIVE(x, "grow", &grow);
	ADD_PRIMITIVE(x, "shrink", &shrink);
	ADD_PRIMITIVE(x, "allot", &allot);

	ADD_PRIMITIVE(x, "header", &header);
	ADD_PRIMITIVE(x, "type", &type);
	ADD_PRIMITIVE(x, "s>w", &stack_to_word);
	ADD_PRIMITIVE(x, "words", &words);
	ADD_PRIMITIVE(x, "see", &see);

	ADD_PRIMITIVE(x, ":", &colon);
	ADD_NDCS_PRIM(x, ";", &semicolon);

	ADD_PRIMITIVE(x, "immediate", &immediate);
	ADD_PRIMITIVE(x, "postpone", &postpone);

	//ADD_PRIMITIVE(x, "latest", &latest, 0);
	//ADD_PRIMITIVE(x, "here", &here, 0);

	//ADD_PRIMITIVE(x, "!", &store, 0);
	//ADD_PRIMITIVE(x, "@", &fetch, 0);
	//ADD_PRIMITIVE(x, "b!", &bstore, 0);
	//ADD_PRIMITIVE(x, "b@", &bfetch, 0);

	//ADD_PRIMITIVE(x, "key", &key, 0);
	//ADD_PRIMITIVE(x, "emit", &emit, 0);

	x->total = x->free;

	return x;
}

#endif

//C depth(C p) { return p ? (T(p) == LST ? 1 + depth(A(p)) + depth(N(p)) : 1 + depth(N(p))) : 0; }
