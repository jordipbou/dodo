// TODO: Try to make everything smaller
// TODO: Add tests for basic primitives
// TODO: Add introspection facilities (from basic primitives)
// TODO: Add errors based on Common Lisp conditions/restarts system

#include<inttypes.h>
#include<string.h>
#include<ctype.h>

#include<stdio.h>

typedef int8_t			B;
typedef intptr_t		C;
typedef struct { 
	B* here, * ib;
	C t, f, p, s, o, r;
	C dict, size, state;
	C tk, in;
	C ip;
} X;
typedef C (*FUNC)(X*);

enum Types { ATOM, LIST, PRIM, WORD };
enum Prims { BRANCH, JUMP, ZJUMP };

#define A(pair)								(*((C*)pair))
#define D(pair)								(*(((C*)pair) + 1))
#define N(pair)								(D(pair) & -4)
#define T(pair)								(D(pair) & 3)
#define LST(p)								(T(p) == LIST)

#define AS(type, cell)				((cell & -4) | type)

#define NFA(word)							((B*)A(A(word)))
#define XT(word)							(N(A(word)))

#define S(x)									(A(x->s))
#define RSX										(S(x) = rcl(x, S(x)))
#define O(x)									(A(x->o))

#define TC(x)									(*(x->ib + x->in))
#define TK(x)									(x->ib + x->tk)
#define TL(x)									(x->in - x->tk)
#define PRS(x, cond)					while (TC(x) && cond) { x->in++; }

#define FREE(x)								(lth(x->f) - 1)
#define ALIGN(addr, bound)		((((C)addr) + (bound - 1)) & ~(bound - 1))
#define RESERVED(x)						((x->t) - ((C)x->here))

#define BOTTOM(x)							(((B*)x) + sizeof(X))
#define TOP(x)								(ALIGN(((B*)x) + x->size - (2*sizeof(C)) - 1, (2*sizeof(C))))

#define ERR_STACK_OVERFLOW		-1
#define ERR_STACK_UNDERFLOW		-2
#define ERR_UNDEFINED_WORD		-3
#define ERR_ATOM_EXPECTED			-4
#define ERR_NOT_ENOUGH_MEMORY	-5

#define ERR(x, cond, err)		if (cond) { return err; }
#define ERR_OF(x, cond)			ERR(x, cond, ERR_STACK_OVERFLOW)
#define ERR_UF(x, cond)			ERR(x, cond, ERR_STACK_UNDERFLOW)
#define OF(x)								(x->f == x->t)
#define OF1(x)							ERR_OF(x, OF(x))
#define UF1(x)							ERR_UF(x, S(x) == 0)
#define UF2(x)							ERR_UF(x, S(x) == 0 || N(S(x)) == 0)
#define UF3(x)							ERR_UF(x, S(x) == 0 || N(S(x)) == 0 || N(N(S(x))) == 0)
#define AE1(x)							ERR(x, T(S(x)) != ATOM, ERR_ATOM_EXPECTED)
#define AE2(x)							ERR(x, T(S(x)) != ATOM || T(N(S(x))) != ATOM, ERR_ATOM_EXPECTED)

X* init(B* block, C size) {
	if (size < sizeof(C) + 3*(2*sizeof(C))) return 0;
	X* x = (X*)block;	
	x->size = size;
	x->here = BOTTOM(x);
	x->t = ALIGN(BOTTOM(x), 2*sizeof(C));
	x->o = x->s = x->p = TOP(x);
	D(x->p) = A(x->p) = 0;
	x->f = TOP(x) - 2*sizeof(C);

	for (C pair = x->t; pair <= x->f; pair += 2*sizeof(C)) {
		A(pair) = pair == x->f ? 0 : pair + 2*sizeof(C);
		D(pair) = pair == x->t ? 0 : pair - 2*sizeof(C);
	}

	x->r = x->dict = x->state = 0;

	return x;
}

C cns(X* x, C a, C d) { C p; return OF(x) ? 0 : (p = x->f, x->f = D(x->f), A(p) = a, D(p) = d, p); }
C cln(X* x, C p) { return !p ? 0 : cns(x, LST(p) ? cln(x, A(p)) : A(p), AS(T(p), cln(x, N(p)))); }
#define RL(x, p)	if (LST(p)) { while (A(p) != 0) { A(p) = rcl(x, A(p)); } }
C rcl(X* x, C p) { C t; RL(x, p); return !p ? 0 : (t = N(p), D(p) = x->f, A(p) = 0, x->f = p, t); }
C rvs(C p, C l) { C t; return p ? t = N(p), D(p) = AS(T(p), l), rvs(t, p) : l; }
C lth(C p) { C c = 0; while (p) { c++; p = N(p); } return c; }

#define NEXT					x->ip = N(x->ip)
C execute(X* x, C xlist) {
	C r;
	x->ip = xlist;	
	do {
		if (x->ip == 0) {
			if (x->r) {	x->ip = A(x->r); x->r = rcl(x, rcl(x, x->r)); } 
			else { return 0; }
		}
		switch (T(x->ip)) {
			case ATOM: ERR_OF(x, (S(x) = cns(x, A(x->ip), AS(ATOM, S(x)))) == 0); NEXT; break;
			case LIST: ERR_OF(x, (S(x) = cns(x, cln(x, A(x->ip)), AS(LIST, S(x)))) == 0); NEXT; break;
			case PRIM: 
				switch (A(x->ip)) {
					case BRANCH:
						r = A(S(x)); RSX;
						if (N(N(N(x->ip)))) {
							x->r = cns(x, r, AS(ATOM, x->r));								// Branch called (0 / 1)
							x->r = cns(x, N(N(N(x->ip))), AS(ATOM, x->r));	// Return address
						}
						x->ip = r ? A(N(x->ip)) : A(N(N(x->ip)));
						break;
					case JUMP: break;
					case ZJUMP: break;
					default: ERR(x, (r = ((FUNC)A(x->ip))(x)) < 0, r); if (r != 1) NEXT; break;
				}
				break;
			case WORD: 
				if (N(x->ip)) {
					x->r = cns(x, A(x->ip), AS(ATOM, x->r));	// Word called
					x->r = cns(x, N(x->ip), AS(ATOM, x->r));	// Return address
				}
				x->ip = XT(A(x->ip));
				break;
		}
	} while(1);
}

C prs_tk(X* x) { PRS(x, isspace(TC(x))); x->tk = x->in; PRS(x, !isspace(TC(x))); return TL(x); }
#define NAME_AS_TOKEN(w)	(strlen(NFA(w)) == TL(x) && strncmp(NFA(w), TK(x), TL(x)) == 0)
C fnd_tk(X* x) { C w = x->dict; while (w && !(NAME_AS_TOKEN(w))) { w = N(w); } return w; }

// OUTER INTERPRETER

C evaluate(X* x, B* str) {
	C word, result;
	char *endptr;
	x->ib = str;
	x->tk = x->in = 0;
	do {
		if (prs_tk(x) == 0) { return 0; }
		if ((word = fnd_tk(x)) != 0) {
			if (!x->state || (T(word) == PRIM || T(word) == WORD)) {
				ERR(x, (result = execute(x, XT(word))) != 0, result);
			} else {
				OF1(x);
				if (T(word) == ATOM || T(word) == PRIM) {
					S(x) = cns(x, A(XT(word)), AS(PRIM, S(x)));
				} else {
					S(x) = cns(x, word, AS(WORD, S(x)));
				}
			}
		} else {
			intmax_t number = strtoimax(TK(x), &endptr, 10);
			if (number == 0 && endptr == (char*)(TK(x))) {
				return ERR_UNDEFINED_WORD;
			} else {
				OF1(x); S(x) = cns(x, number, AS(ATOM, S(x)));
			}
		}
	} while (1);
}

C add(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) + A(S(x)); RSX; return 0; }
C sub(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) - A(S(x)); RSX; return 0; }
C mul(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) * A(S(x)); RSX; return 0; }
C division(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) / A(S(x)); RSX; return 0; }
C mod(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) % A(S(x)); RSX; return 0; }

C gt(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) > A(S(x)); RSX; return 0; }
C lt(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) < A(S(x)); RSX; return 0; }
C eq(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) == A(S(x)); RSX; return 0; }

C and(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) & A(S(x)); RSX; return 0; }
C or(X* x) { UF2(x); AE2(x); A(N(S(x))) = A(N(S(x))) | A(S(x)); RSX; return 0; }
C invert(X* x) { UF1(x); AE1(x); A(S(x)) = ~A(S(x)); return 0; }

C duplicate(X* x) {
	UF1(x);
	ERR_OF(x, (S(x) = cns(x, T(S(x)) == LIST ? cln(x, A(S(x))) : A(S(x)), AS(T(S(x)), S(x)))) == 0);
	return 0;
}
C swap(X* x) {
	UF2(x);
	C t = N(S(x)); D(S(x)) = AS(T(S(x)), N(N(S(x)))); D(t) = AS(T(t), S(x)); S(x) = t; 
	return 0;
}
C drop(X* x) { RSX; return 0; }
C over(X* x) {
	UF2(x); OF1(x);
	S(x) = cns(x, T(N(S(x))) == LIST ? cln(x, A(N(S(x)))) : A(N(S(x))), AS(LIST, S(x)));
	return 0;
}
C rot(X* x) { 
	UF3(x); 
	C t = S(x); S(x) = N(N(S(x))); D(N(t)) = AS(T(N(t)), N(S(x))); D(S(x)) = AS(T(S(x)), t); return 0;
}

C spush(X* x) { x->p = cns(x, 0, AS(LIST, x->p)); x->s = x->p; return 0; }
C stack_to_list(X* x) {
	if (!N(x->p)) { return 0; } // Current stack can be pushed to itself as list ?
	A(x->p) = rvs(A(x->p), 0);
	C t = N(x->p);
	D(x->p) = AS(LIST, A(N(x->p)));
	A(t) = x->p;
	x->p = t;
	x->s = x->p;
	return 0;
}
C lbracket(X* x) { x->s = x->o; x->state = 0; return 0; }
C rbracket(X* x) { if (!x->state) { x->o = x->s; }; x->s = x->p; x->state = 1; return 0; }
C lbrace(X* x) { rbracket(x); spush(x); return 0; }
C rbrace(X* x) { lbracket(x); stack_to_list(x); return 0; }
C literal(X* x) { 
	// TODO: Must check underflow on O(x) !!!
	C t = O(x); O(x) = N(O(x)); D(t) = AS(T(t), S(x)); S(x) = t; return 0; 
}

C interpret(X* x) { 
	UF1(x); 
	// TODO: Expects a list?
	C t = S(x); S(x) = N(S(x)); D(t) = AS(LIST, 0);
	C r = execute(x, A(t)); 
	rcl(x, t); 
	return r; 
}
//C exec(X* x) { UF1(x); /* TODO: Expects a list? */ return execute(x, A(S(x))); }
C exec(X* x) {
	UF1(x);
	if (N(x->ip)) {
		x->r = cns(x, -1, AS(ATOM, x->r));
		x->r = cns(x, N(x->ip), AS(ATOM, x->r));
	}
	x->ip = A(S(x));
	return 1;
}

C grow(X* x) { 
	if (A(x->t) == (x->t + 2*sizeof(C))) { 
		x->t += 2*sizeof(C);
		D(x->t) = 0;
		return 0;
	} else {
		return ERR_NOT_ENOUGH_MEMORY;
	}
}
C shrink(X* x) {
	// TODO
}

C compile_str(X* x) {
	x->tk = ++x->in;
	while (*(x->ib + x->in) != 0 && *(x->ib + x->in) != '"') { x->in++; }
	B* here = x->here;
	while (RESERVED(x) < (TL(x) + 1)) { ERR(x, grow(x) != 0, ERR_NOT_ENOUGH_MEMORY); }
	for (C i; i < TL(x); i++) { here[i] = *(x->ib + x->tk + i); }
	here[TL(x)] = 0;
	S(x) = cns(x, (C)here, AS(ATOM, S(x)));
	S(x) = cns(x, TL(x), AS(ATOM, S(x)));
	x->here += TL(x) + 1;
	x->in++;
	return 0;
}

C latest(X* x) { S(x) = cns(x, (C)&x->dict, AS(ATOM, S(x))); return 0; }

C append(X* x) {
	UF2(x);
	if (T(N(S(x))) == LIST) {
		C t = N(S(x));
		D(S(x)) = AS(T(S(x)), A(N(S(x))));
		A(t) = S(x);
		S(x) = t;
	} else if (T(N(S(x))) == ATOM) {
		C* d = (C*)A(N(S(x)));
		C t = N(S(x));
		D(S(x)) = AS(T(S(x)), *d);
		*d = S(x);
		S(x) = rcl(x, t);
	}
	return 0;
}

#define ADD_PRIMITIVE(x, name, func, immediate) \
	x->dict = cns(x, \
		cns(x, (C)name, AS(ATOM, \
		cns(x, (C)func, AS(PRIM, 0)))), \
	AS(immediate, x->dict));

X* bootstrap(X* x) {
	ADD_PRIMITIVE(x, "+", &add, 0);
	ADD_PRIMITIVE(x, "-", &sub, 0);
	ADD_PRIMITIVE(x, "*", &mul, 0);
	ADD_PRIMITIVE(x, "/", &division, 0);
	ADD_PRIMITIVE(x, "mod", &mod, 0);

	ADD_PRIMITIVE(x, ">", &gt, 0);
	ADD_PRIMITIVE(x, "<", &lt, 0);
	ADD_PRIMITIVE(x, "=", &eq, 0);

	ADD_PRIMITIVE(x, "and", &and, 0);
	ADD_PRIMITIVE(x, "or", &or, 0);
	ADD_PRIMITIVE(x, "invert", &invert, 0);

	ADD_PRIMITIVE(x, "dup", &duplicate, 0);
	ADD_PRIMITIVE(x, "swap", &swap, 0);
	ADD_PRIMITIVE(x, "drop", &drop, 0);
	ADD_PRIMITIVE(x, "over", &over, 0);
	ADD_PRIMITIVE(x, "rot", &rot, 0);

	ADD_PRIMITIVE(x, "{", &lbrace, 2);
	ADD_PRIMITIVE(x, "}", &rbrace, 2);

	ADD_PRIMITIVE(x, "[", &lbracket, 2);
	ADD_PRIMITIVE(x, "]", &rbracket, 2);

	ADD_PRIMITIVE(x, "spush", &spush, 2);
	ADD_PRIMITIVE(x, "s>l", &stack_to_list, 2);

	ADD_PRIMITIVE(x, "literal", &literal, 2);

	ADD_PRIMITIVE(x, "i", &interpret, 0);
	ADD_PRIMITIVE(x, "x", &exec, 0);

	ADD_PRIMITIVE(x, "branch", 0, 0);

	ADD_PRIMITIVE(x, "grow", &grow, 0);

	ADD_PRIMITIVE(x, "s\"", &compile_str, 2);

	ADD_PRIMITIVE(x, "latest", &latest, 0);
	ADD_PRIMITIVE(x, "append", &append, 0);

	return x;
}
