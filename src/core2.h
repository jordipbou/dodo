#ifndef __CORE__
#define __CORE__

#include<inttypes.h>
#include<string.h>
#include<ctype.h>
#include<stdio.h>

typedef int8_t		B;
typedef intptr_t	C;

#define sC								(sizeof(C))
#define sP 							(2*sC)

#define A(p)							(*((C*)p))
#define D(p)							(*(((C*)p) + 1))
#define N(p)							(D(p) & -4)
#define T(p)							(D(p) & 3)

enum Types { ATM, LST, PRM, WRD };

#define AS(t, n)					((n & -4) | t)
#define LK(p, n)					(D(p) = AS(T(p), n))

typedef struct {
	B *tb, *hr;
	C th, sz, fr, ps, err, ip, rs, st, lt, tk, in, free;
} X;

#define S(x)							(A(x->ps))

#define ALIGN(a, b)			((((C)a) + (b - 1)) & ~(b - 1))
#define BOTTOM(x)				(((B*)x) + sizeof(X))
#define TOP(x)					(ALIGN(((B*)x) + x->sz - sP - 1, sP))

X* init(B* bl, C sz) {
	X* x = (X*)bl;	
	x->sz = sz;
	x->hr = BOTTOM(x);
	x->th = ALIGN(BOTTOM(x), sP);
	x->ps = TOP(x);
	A(x->ps) = 0;
	D(x->ps) = AS(LST, 0);
	x->fr = x->ps - sP;

	x->free = -1;
	C p; for (p = x->th; p <= x->fr; p += sP) {
		x->free++;
		A(p) = p == x->fr ? 0 : p + sP;
		D(p) = p == x->th ? 0 : p - sP;
	}

	x->tb = 0;
	x->tk = x->in = x->st = x->rs = x->ip = x->err = x->lt = 0;

	return x;
}

// LIST/MANAGED MEMORY FUNCTIONS

C length(C l)	{ C c = 0; while (l) { c++; l = N(l); } return c; }
C cons(X* x, C a, C d) { x->free--; C p = x->fr; x->fr = D(x->fr); A(p) = a; D(p) = d; return p; }
C arecl(X* x, C p) { x->free++; C t = N(p); D(p) = x->fr; A(p) = 0; x->fr = p; return t; }
C recl(X* x, C p) { if (T(p) == LST) while (A(p)) A(p) = recl(x, A(p)); return arecl(x, p); }
C reverse(C p, C l) { return p ? ({ C t = N(p); D(p) = AS(T(p), l); reverse(t, p); }) : l; }
C clone(X* x, C p) { return p ? cons(x, T(p) == LST ? clone(x, A(p)) : A(p), AS(T(p), clone(x, N(p)))) : 0; }
C depth(C p) { return p ? (T(p) == LST ? 1 + depth(A(p)) + depth(N(p)) : 1 + depth(N(p))) : 0; }

typedef void (*FUNC)(X*);

void dump_stack(X*, C);
void dump_list(X*, C);

void inner(X* x) {
	C t;
	// Use return stack as stack of instructions to be executed from top to bottom
	while (x->rs) {
		//dump_stack(x, S(x));
		//printf("| ");
		//dump_list(x, x->rs);
		//printf("\n");
		switch (T(x->rs)) {
			case ATM: t = x->rs; x->rs = N(x->rs); LK(t, S(x)); S(x) = t; break;
			case LST: t = x->rs; x->rs = N(x->rs); LK(t, S(x)); S(x) = t; break;
			case PRM: ((FUNC)A(x->rs))(x); if (!x->err) x->rs = recl(x, x->rs); else x->err = 0; break;
			case WRD: /* Push word list into return stack */ break;
		}
		//getchar();
	}
}

// STACK PRIMITIVES

void duplicate(X* x) { 
	if (T(S(x)) == LST) { S(x) = cons(x, clone(x, A(S(x))), AS(LST, S(x))); }
	else { S(x) = cons(x, A(S(x)), AS(T(S(x)), S(x))); }
}
void swap(X* x) { C t = N(S(x)); LK(S(x), N(N(S(x)))); LK(t, S(x)); S(x) = t; }
void drop(X* x) { S(x) = recl(x, S(x)); }
void over(X* x) {
	if (T(N(S(x))) == LST) { S(x) = cons(x, clone(x, A(N(S(x)))), AS(LST, S(x))); }
	else { S(x) = cons(x, A(N(S(x))), AS(T(N(S(x))), S(x))); }
}
void rot(X* x) { C t = N(N(S(x))); LK(N(S(x)), N(N(N(S(x))))); LK(t, S(x)); S(x) = t; }

// COMPARISON PRIMITIVES

#define BINOP(o) A(N(S(x))) = A(N(S(x))) o A(S(x)); S(x) = recl(x, S(x));

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
void invert(X* x) { A(S(x)) = ~A(S(x)); } 

// LIST PRIMITIVES

void empty(X* x) { S(x) = cons(x, 0, AS(LST, S(x))); }
// TODO: If executing l>s and stack is empty or top of stack is not a list, crashes
void list_to_stack(X* x) { C t = S(x); S(x) = N(S(x)); LK(t, x->ps); x->ps = t; }
void stack_to_list(X* x) {
	S(x) = reverse(S(x), 0);
	if (!N(x->ps)) { x->ps = cons(x, x->ps, AS(LST, 0)); }
	else { C t = x->ps; x->ps = N(x->ps); LK(t, A(x->ps)); A(x->ps) = t; }
}

// EXECUTION PRIMITIVES

void exec_x(X* x) { //x->rs = cons(x, clone(x, A(S(x))), AS(LST, x->rs)); x->err = 1; }
	C c = clone(x, A(S(x)));
	C p = c;
	while (N(p)) {
		p = N(p);	
	}
	LK(p, recl(x, x->rs));
	x->rs = c;
	x->err = 1;
}
void exec_i(X* x) { //C t = S(x); S(x) = N(S(x)); LK(t, x->rs); x->rs = t; x->err = 1; }
	// Could be put as a stack on top of return stack directly
	if (A(S(x)) == 0) { S(x) = recl(x, S(x)); return; }
	C p = A(S(x));
	while (N(p)) {
		p = N(p);
	}
	LK(p, recl(x, x->rs));
	x->rs = A(S(x));
	A(S(x)) = 0;
	S(x) = recl(x, S(x));
	x->err = 1;
}

void branch(X* x) { if (!A(N(N(S(x))))) { swap(x); } drop(x); swap(x); drop(x); exec_i(x); }
void lbracket(X* x) { x->st = 0; }
void rbracket(X* x) { x->st = 1; }

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

#endif
