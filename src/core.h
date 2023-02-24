#include<inttypes.h>
#include<string.h>
#include<ctype.h>

#include<stdio.h>

typedef int8_t			B;
typedef intptr_t		C;
typedef struct { 
	B* here;
	C there, free, p, s, d, o;
	C size, state, base;
} X;
typedef C (*FUNC)(X*);

#define A(pair)							(*((C*)pair))
#define D(pair)							(*(((C*)pair) + 1))
#define N(pair)							(D(pair) & -4)
#define T(pair)							(D(pair) & 3)

enum Types { ATOM, LIST, PRIM, WORD };

#define AS(type, cell)			((cell & -4) | type)

#define NFA(word)						((B*)A(A(word)))
#define XT(word)						(N(A(word)))
#define PRIMITIVE(word)			(T(word) == ATOM || T(word) == PRIM)
#define IMMEDIATE(word)			(T(word) == PRIM || T(word) == WORD)

#define S(x)								(A(x->s))
#define O(x)								(A(x->o))

#define TK(x)								(x->tib + x->token)
#define TL(x)								(x->in - x->token)

#define FREE(x)							(length(x->free) - 1)
#define ALIGN(addr, bound)	((((C)addr) + (bound - 1)) & ~(bound - 1))

#define BOTTOM(x)						(((B*)x) + sizeof(X))
#define TOP(x)							(ALIGN(((B*)x) + x->size - (2*sizeof(C)) - 1, (2*sizeof(C))))

#define ERR_STACK_OVERFLOW	-1
#define ERR_STACK_UNDERFLOW	-2
#define ERR_UNDEFINED_WORD	-3
#define ERR_ATOM_EXPECTED		-4

#define ERR(x, cond, err)		if (cond) { return err; }
#define OF(x, cond)					ERR(x, cond, ERR_STACK_OVERFLOW)
#define UF(x, cond)					ERR(x, cond, ERR_STACK_UNDERFLOW)
#define OF1(x)							OF(x, x->free == x->there)
#define UF1(x)							UF(x, S(x) == 0)
#define UF2(x)							UF(x, S(x) == 0 || N(S(x)) == 0)
#define UF3(x)							UF(x, S(x) == 0 || N(S(x)) == 0 || N(N(S(x))) == 0)
#define AE1(x)							ERR(x, T(S(x)) != ATOM, ERR_ATOM_EXPECTED)
#define AE2(x)							ERR(x, T(S(x)) != ATOM || T(N(S(x))) != ATOM, ERR_ATOM_EXPECTED)

X* init(B* block, C size) {
	if (size < sizeof(C) + 3*(2*sizeof(C))) return 0;
	X* x = (X*)block;	
	x->size = size;
	x->here = BOTTOM(x);
	x->there = ALIGN(BOTTOM(x), 2*sizeof(C));
	x->o = x->s = x->p = TOP(x);
	D(x->p) = A(x->p) = 0;
	x->free = TOP(x) - 2*sizeof(C);

	for (C pair = x->there; pair <= x->free; pair += 2*sizeof(C)) {
		A(pair) = pair == x->free ? 0 : pair + 2*sizeof(C);
		D(pair) = pair == x->there ? 0 : pair - 2*sizeof(C);
	}

	x->d = x->state = 0;
	x->base = 10;

	return x;
}

C cons(X* x, C car, C cdr) {
	if (x->free == x->there) return 0;
	C pair = x->free;
	x->free = D(x->free);
	A(pair) = car;
	D(pair) = cdr;
	return pair;
}

C clone(X* x, C pair) {
	if (!pair) return 0;
	return cons(x, T(pair) == LIST ? clone(x, A(pair)) : A(pair), AS(T(pair), clone(x, N(pair))));
}

C reclaim(X* x, C pair) {
	if (!pair) return 0;
	if (T(pair) == LIST) {
		while (A(pair) != 0) { 
			A(pair) = reclaim(x, A(pair)); 
		} 
	} 
	C tail = N(pair);
	D(pair) = x->free;
	A(pair) = 0;
	x->free = pair;
	return tail;
}

C reverse(C pair, C list) {
	if (pair != 0) {
		C t = N(pair);
		D(pair) = AS(T(pair), list);
		return reverse(t, pair);
	} else {
		return list;
	}
}

C length(C pair) { 
	C c = 0; 
	while (pair) { 
		c++; 
		pair = N(pair); 
	} 
	return c; 
}

#define CALL(x, i, n)		((n) ? (execute(x, i), (n)) : (i))

C execute(X* x, C xlist) {
	C r, p = xlist;
	while (p) {
		switch (T(p)) {
			case ATOM: OF(x, (S(x) = cons(x, A(p), AS(ATOM, S(x)))) == 0); p = N(p); break;
			case LIST: OF(x, (S(x) = cons(x, clone(x, A(p)), AS(LIST, S(x)))) == 0); p = N(p); break;
			case PRIM:
				switch (A(p)) {
					case 0: 
						r = A(S(x)); S(x) = reclaim(x, S(x));
						p = CALL(x, r ? A(N(p)) : A(N(N(p))), N(N(N(p))));
						break;
					case 1: break; // JUMP
					case 2: break; // ZJUMP
					default: ERR(x, (r = ((FUNC)A(p))(x)) < 0, r); p = N(p); break;
				};
				break;
			case WORD: p = CALL(x, XT(A(p)), N(p)); break;
		}
	} 
}

C parse_token(X* x, B* tib, C* token, C* in) {
	while (*(tib + *in) != 0 && isspace(*(tib + *in))) { *in += 1;	}
	*token = *in;
	while (*(tib + *in) != 0 && !isspace(*(tib + *in))) { *in += 1; }
	return *in - *token;
}

C find_token(X* x, B* tib, C token, C in) {
	C w = x->d;
	while (w && !(strlen(NFA(w)) == (in - token) && strncmp(NFA(w), tib + token, in - token) == 0)) {
		w = N(w); 
	}
	return w;
}

// OUTER INTERPRETER

C evaluate(X* x, B* str) {
	C word, result;
	char *endptr;
	C token, in = 0;
	do {
		if (parse_token(x, str, &token, &in) == 0) { return 0; }
		if ((word = find_token(x, str, token, in)) != 0) {
			if (!x->state || IMMEDIATE(word)) {
				ERR(x, (result = execute(x, XT(word))) != 0, result);
			} else {
				OF1(x);
				if (PRIMITIVE(word)) {
					S(x) = cons(x, A(XT(word)), AS(PRIM, S(x)));
				} else {
					S(x) = cons(x, word, AS(WORD, S(x)));
				}
			}
		} else {
			intmax_t number = strtoimax(str + token, &endptr, x->base);
			if (number == 0 && endptr == (char*)(str + token)) {
				return ERR_UNDEFINED_WORD;
			} else {
				OF1(x); S(x) = cons(x, number, AS(ATOM, S(x)));
			}
		}
	} while (1);
}

#define BINOP(name, op) \
C name(X* x) { \
	UF2(x); AE2(x); \
	A(N(S(x))) = A(N(S(x))) op A(S(x)); \
	S(x) = reclaim(x, S(x)); \
	return 0; \
}

BINOP(add, +)
BINOP(sub, -)
BINOP(mul, *)
BINOP(division, /)
BINOP(mod, %)

BINOP(gt, >)
BINOP(lt, <)
BINOP(eq, ==)

BINOP(and, &)
BINOP(or, |)

C invert(X* x) {
	UF1(x); AE1(x);
	A(S(x)) = ~A(S(x));
	return 0;
}

C duplicate(X* x) {
	UF1(x);
	OF(x, (S(x) = cons(x, T(S(x)) == LIST ? clone(x, A(S(x))) : A(S(x)), AS(T(S(x)), S(x)))) == 0);
	return 0;
}

C swap(X* x) {
	UF2(x);
	C t = N(S(x)); D(S(x)) = AS(T(S(x)), N(N(S(x)))); D(t) = AS(T(t), S(x)); S(x) = t; 
	return 0;
}

C drop(X* x) {
	S(x) = reclaim(x, S(x));
	return 0;
}

C over(X* x) {
	UF2(x); OF1(x);
	S(x) = cons(x, T(N(S(x))) == LIST ? clone(x, A(N(S(x)))) : A(N(S(x))), AS(LIST, S(x)));
	return 0;
}

C rot(X* x) { 
	UF3(x); 
	C t = S(x); S(x) = N(N(S(x))); D(N(t)) = AS(T(N(t)), N(S(x))); D(S(x)) = AS(T(S(x)), t); return 0;
}

C spush(X* x) { x->p = cons(x, 0, AS(LIST, x->p)); x->s = x->p; return 0; }
C stack_to_list(X* x) {
	if (!N(x->p)) { return 0; } // Current stack can be pushed to itself as list ?
	A(x->p) = reverse(A(x->p), 0);
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
C literal(X* x) { UF1(x); C t = O(x); O(x) = N(O(x)); D(t) = AS(T(t), S(x)); S(x) = t; return 0; }

C interpret(X* x) { 
	UF1(x); 
	C t = A(S(x)); A(S(x)) = 0; S(x) = reclaim(x, S(x)); 
	C r = execute(x, t); 
	reclaim(x, t); 
	return r; 
}
C exec(X* x) { UF1(x); return execute(x, A(S(x))); }

#define ADD_PRIMITIVE(x, name, func, immediate) \
	x->d = cons(x, \
		cons(x, (C)name, AS(ATOM, \
		cons(x, (C)func, AS(PRIM, 0)))), \
	AS(immediate, x->d));

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

	return x;
}
