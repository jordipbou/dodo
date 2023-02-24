#include<inttypes.h>
#include<string.h>
#include<ctype.h>

#include<stdio.h>

typedef int8_t			B;
typedef intptr_t		C;
typedef struct { 
	B* tib, * here;
	C size, t, ip, state, token, in, base;
	C f, p, s, r, d, o;
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

#define TK(x)								(x->tib + x->token)
#define TL(x)								(x->in - x->token)

#define FREE(x)							(length(x->f) - 1)
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
#define OF1(x)							OF(x, x->f == x->t)
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
	x->t = ALIGN(BOTTOM(x), 2*sizeof(C));
	x->o = x->s = x->p = TOP(x);
	D(x->p) = A(x->p) = 0;
	x->f = TOP(x) - 2*sizeof(C);

	for (C pair = x->t; pair <= x->f; pair += 2*sizeof(C)) {
		A(pair) = pair == x->f ? 0 : pair + 2*sizeof(C);
		D(pair) = pair == x->t ? 0 : pair - 2*sizeof(C);
	}

	x->r = x->d = x->state = x->token = x->in = 0;
	x->base = 10;
	x->tib = 0;

	return x;
}

C cons(X* x, C car, C cdr) {
	if (x->f == x->t) return 0;
	C pair = x->f;
	x->f = D(x->f);
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
	D(pair) = x->f;
	A(pair) = 0;
	x->f = pair;
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

#define CALL(x, i, n) \
	if (n != 0) { \
		if (x->f == x->t) { return ERR_STACK_OVERFLOW; } \
		x->r = cons(x, n, AS(ATOM, x->r)); \
	} \
	x->ip = i;

C execute(X* x, C xlist) {
	C result;
	x->ip = xlist;
	do {
		if (x->ip == 0) {
			if (x->r) {	x->ip = A(x->r); x->r = reclaim(x, x->r);	} 
			else { return 0; }	
		}
		switch (T(x->ip)) {
			case ATOM: 
				OF(x, (S(x) = cons(x, A(x->ip), AS(ATOM, S(x)))) == 0);
				x->ip = N(x->ip); 
				break;
			case LIST:
				OF(x, (S(x) = cons(x, clone(x, A(x->ip)), AS(LIST, S(x)))) == 0);
				x->ip = N(x->ip);
				break;
			case PRIM:
				ERR(x, (result = ((FUNC)A(x->ip))(x)) < 0, result);
				if (result != 1) { x->ip = N(x->ip); }
				break;
			case WORD:
				CALL(x, XT(A(x->ip)), N(x->ip));
				break;
		}
	} while (1);
}

C parse_token(X* x) {
	while (*(x->tib + x->in) != 0 && isspace(*(x->tib + x->in))) { x->in++;	}
	x->token = x->in;
	while (*(x->tib + x->in) != 0 && !isspace(*(x->tib + x->in))) { x->in++; }
	return x->in - x->token;
}

C find_token(X* x) {
	C w = x->d;
	while (w && !(strlen(NFA(w)) == TL(x) && strncmp(NFA(w), TK(x), TL(x)) == 0)) {	w = N(w); }
	return w;
}

// OUTER INTERPRETER

C evaluate(X* x, B* str) {
	C word, result;
	char *endptr;
	x->tib = str;
	x->token = x->in = 0;
	do {
		if (parse_token(x) == 0) { return 0; }
		if ((word = find_token(x)) != 0) {
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
			intmax_t number = strtoimax(x->tib + x->token, &endptr, x->base);
			if (number == 0 && endptr == (char*)(x->tib + x->token)) {
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

C lbrace(X* x) {
	if (!x->state) {
		x->o = x->s;
	}
	x->p = cons(x, 0, AS(LIST, x->p));
	x->s = x->p;
	x->state = 1;
	return 0;
}

C rbrace(X* x) {
	// TODO: It will fail if called on x->s
	C t = A(x->p);
	A(x->p) = 0;
	x->p = reclaim(x, x->p);
	x->s = x->p;
	S(x) = cons(x, reverse(t, 0), AS(LIST, S(x)));
	if (x->s == x->o) x->state = 0;
	return 0;
}

C lbracket(X* x) {
	x->s = x->o;
	x->state = 0;
	return 0;
}

C rbracket(X* x) {
	x->o = x->s;
	x->state = 1;
	return 0;
}

C eval(X* x) {
	UF1(x);
	C t = A(S(x));
	A(S(x)) = 0;
	S(x) = reclaim(x, S(x));
	// TODO: t should be reclaimed later !!!! Here we have a leak !!!
	return 1;
}

C exec(X* x) {
	UF1(x);
	CALL(x, A(S(x)), N(x->ip));
	return 1;
}

C branch(X* x) {
	UF1(x);
	AE1(x);
	C b = A(S(x));
	S(x) = reclaim(x, S(x));
	CALL(x, (b ? A(N(x->ip)) : A(N(N(x->ip)))), N(N(N(x->ip))));
	return 1;
}

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

	ADD_PRIMITIVE(x, "i", &eval, 0);
	ADD_PRIMITIVE(x, "x", &exec, 0);

	ADD_PRIMITIVE(x, "branch", &branch, 0);

	return x;
}
