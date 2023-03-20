#ifndef __CORE__
#define __CORE__

#include<inttypes.h>

typedef int8_t		B;
typedef intptr_t	C;

#define sC								(sizeof(C))
#define sP  							(2*sC)

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
	for (C p = x->th; p <= x->fr; p += sP) {
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
C recl(X* x, C p) { x->free++; C t = N(p); D(p) = x->fr; A(p) = 0; x->fr = p; return t; }
C lrecl(X* x, C p) { if (T(p) == LST) while (A(p)) A(p) = lrecl(x, A(p)); return recl(x, p); }
C reverse(C p, C l) { return p ? ({ C t = N(p); D(p) = AS(T(p), l); reverse(t, p); }) : l; }
C clone(X* x, C p) { return p ? cons(x, T(p) == LST ? clone(x, A(p)) : A(p), AS(T(p), clone(x, N(p)))) : 0; }
C depth(C p) { return p ? (T(p) == LST ? 1 + depth(A(p)) + depth(N(p)) : 1 + depth(N(p))) : 0; }

// ERRORS

#define IP_MODIFIED								1
#define ERR_STACK_OVERFLOW				-1
#define ERR_STACK_UNDERFLOW				-2
#define ERR_DIVISION_BY_ZERO			-3
#define ERR_NOT_ENOUGH_MEMORY			-4
#define ERR_NOT_ENOUGH_RESERVED		-5
#define ERR_UNDEFINED_WORD				-6
#define ERR_EXPECTED_LIST					-7
#define ERR_ZERO_LENGTH_NAME			-8
#define ERR_EMPTY_TIB							-9

#define OFn(x, n, b)	if (x->free < n) { x->err = ERR_STACK_OVERFLOW; } else { b; }
#define UF1(x)				if (S(x) == 0) { x->err = ERR_STACK_UNDERFLOW; return; }
#define UF2(x)				if (S(x) == 0 || N(S(x)) == 0) { x->err = ERR_STACK_UNDERFLOW; return; }
#define UF3(x)				if (!S(x) || !N(S(x)) || !N(N(S(x)))) { x->err = ERR_STACK_UNDERFLOW; return; }

#define PUSH(x, s, n, t)		OFn(x, 1, s = cons(x, n, AS(t, s)))
#define PUSHL(x, s, l)			OFn(x, depth(l), s = cons(x, clone(x, l), AS(LST, s)))
#define POP(x, s)						({ C v = A(s); s = recl(x, s); v; })

// INNER INTERPRETER

typedef void (*FUNC)(X*);

enum Words { PRIM, DEF, NDCS_PRIM, NDCS_DEF };

#define NDCS(w)		(T(w) == NDCS_PRIM || T(w) == NDCS_DEF)

#define NFA(w)		((B*)A(A(w)))
#define CS(w)			(A(N(A(w))))
#define XT(w)			(NDCS(w) ? N(N(A(w))) : N(A(w)))

#define ERR(x)	if (x->err < 0) { /* Do something */ return; }

#define RETURN(x) \
		if (!x->ip) { \
			while (x->rs && T(x->rs) != PRM) { x->rs = lrecl(x, x->rs); } \
			if (x->rs) { x->ip = A(x->rs); x->rs = recl(x, x->rs); } \
			if (!x->ip) return; \
		}

void inner(X* x, C l) {
	x->ip = l;
	do {
		RETURN(x);
		switch(T(x->ip)) {
			case ATM: PUSH(x, S(x), A(x->ip), ATM); ERR(x); x->ip = N(x->ip); break;
			case LST: PUSHL(x, S(x), A(x->ip)); ERR(x); x->ip = N(x->ip); break;
			case PRM: ((FUNC)A(x->ip))(x); ERR(x); if (!x->err) x->ip = N(x->ip); else x->err = 0; break;
			case WRD: 
				if (N(x->ip)) {
					PUSH(x, x->rs, N(x->ip), PRM); ERR(x);
					PUSH(x, x->rs, A(x->ip), WRD); ERR(x);
				}
				x->ip = XT(A(x->ip));
		}
	} while(1);
}

// EXECUTION PRIMITIVES

void branch(X* x) { 
	UF1(x); 
	if (N(N(N(x->ip)))) { PUSH(x, x->rs, N(N(N(x->ip))), PRM); }
	x->ip = POP(x, S(x)) ? A(N(x->ip)) : A(N(N(x->ip)));
	x->err = IP_MODIFIED;
}
void zjump(X* x) { UF1(x); x->ip = POP(x, S(x)) ? N(N(x->ip)) : A(N(x->ip)); x->err = IP_MODIFIED; }
void jump(X* x) {	x->ip = A(N(x->ip)); x->err = IP_MODIFIED; }
//void exec_x(X* x) { 
//	UF1(x);
//	if (N(x->ip)) { R(x) = cons(x, N(x->ip), AS(PRM, R(x))); }
//	// TODO: Type of item on top of data stack should be checked, if not list, execute primitive
//	//printf("exec_x::setting ip to %ld\n", A(S(x)));
//	x->ip = A(S(x)); 
//	x->err = 1;
//}
//void exec_i(X* x) { 
//	UF1(x); 
//	C t = S(x); S(x) = N(S(x)); 
//	if (N(x->ip)) R(x) = cons(x, N(x->ip), AS(PRM, R(x)));
//	R(x) = cons(x, t, AS(LST, R(x)));
//	x->ip = A(t);
//	x->err = 1;
//}
//void lbracket(X* x) { x->state = 0; }
//void rbracket(X* x) { x->state = 1; }
//
//#include<string.h>
//#include<ctype.h>
//#include<stdio.h>

//#ifdef _WIN32
//  #include <conio.h>
//#else
//	#include <unistd.h>
//	#include <termios.h>
//#endif
//
//#define NFA(w)						((B*)(A(A(w))))
//#define XT(w)							(A(N(A(w))))
//#define NDCS(w)						(N(N(A(w))))
//
//enum Words { NIP, NIC, IMP, IMC };
//
//#define PRIMITIVE(w)			((T(w) & 1) == 0)
//#define IMMEDIATE(w)			((T(w) & 2) == 2)
//
//#define TK(x)							(x->tb + x->tk)
//#define TC(x)							(*(x->tb + x->in))
//#define TL(x)							(x->in - x->tk)
//
//#define PUSH(x, n, t)		S(x) = cons(x, (C)n, AS(t, S(x)))
//C pop(X* x) { C v = A(S(x)); S(x) = recl(x, S(x)); return v; }
//
///* TEMPORAL: INSPECTION */
//void dump_list(X* x, C l) {
//	while (l) {
//		switch (T(l)) {
//			case ATM: printf("%ld ", A(l)); break;
//			case LST: printf("{ "); dump_list(x, A(l)); printf("} "); break;
//			case PRM: printf("#%ld ", A(l)); break;
//			case WRD: printf("@%ld ", A(l)); break;
//		}
//		l = N(l);
//	}
//}
//
//void dump_stack(X* x) { printf("<%ld> ", length(S(x))); dump_list(x, S(x)); printf("\n"); }
//
//C dump_return_stack(X* x) {
//	C p = x->rstack;
//	while (p) { printf("%ld\n", A(p)); p = N(p); }
//	return 0;
//}
//
//C dump_context(X* x) {
//	printf("ERROR %ld STATE %ld\n", x->err, x->state);
//	printf("DATA STACK ---------------\n"); dump_stack(x);
//	printf("RETURN STACK -------------\n"); dump_return_stack(x);
//}
///* -TEMPORAL */
//
//#define UF1(x)	if (!S(x)) { x->err = ERR_STACK_UNDERFLOW; return; }
//#define UF2(x)	if (!(S(x) && N(S(x)))) { x->err = ERR_STACK_UNDERFLOW; return; }
//#define UF3(x)	if (!(S(x) && N(S(x)) && N(N(S(x))))) { x->err = ERR_STACK_UNDERFLOW; return; }
//
//#define EL(x)		if (T(S(x)) != LST) { x->err = ERR_EXPECTED_LIST; return; }
//
//// STACK PRIMITIVES
//
//void duplicate(X* x) { 
//	UF1(x); 
//	if (T(S(x)) == LST) { S(x) = cons(x, clone(x, A(S(x))), AS(LST, S(x))); }
//	else { S(x) = cons(x, A(S(x)), AS(T(S(x)), S(x))); }
//}
//void swap(X* x) { UF2(x); C t = N(S(x)); LK(S(x), N(N(S(x)))); LK(t, S(x)); S(x) = t; }
//void drop(X* x) { S(x) = recl(x, S(x)); }
//void over(X* x) { 
//	UF2(x);
//	if (T(N(S(x))) == LST) { S(x) = cons(x, clone(x, A(N(S(x)))), AS(LST, S(x))); }
//	else { S(x) = cons(x, A(N(S(x))), AS(T(N(S(x))), S(x))); }
//}
//void rot(X* x) { UF3(x); C t = N(N(S(x))); LK(N(S(x)), N(N(N(S(x))))); LK(t, S(x)); S(x) = t; }
//
//// COMPARISON PRIMITIVES
//
//#define BINOP(o) A(N(S(x))) = A(N(S(x))) o A(S(x)); S(x) = recl(x, S(x));
//
//void gt(X* x) { UF2(x); BINOP(>) }
//void lt(X* x) { UF2(x); BINOP(<) }
//void eq(X* x) { UF2(x); BINOP(==) }
//void neq(X* x) { UF2(x); BINOP(!=) }
//
//// ARITHMETIC PRIMITIVES
//
//void add(X* x) { UF2(x); BINOP(+) }
//void sub(X* x) { UF2(x); BINOP(-) }
//void mul(X* x) { UF2(x); BINOP(*) }
//void division(X* x) { UF2(x); if (!A(S(x))) { x->err = ERR_DIVISION_BY_ZERO; return; } BINOP(/) }
//void mod(X* x) { UF2(x); BINOP(%) }
//
//// BIT PRIMITIVES
//
//void and(X* x) { UF2(x); BINOP(&) }
//void or(X* x) { UF2(x); BINOP(|) }
//void invert(X* x) { UF1(x); A(S(x)) = ~A(S(x)); } 
//
//// MEMORY ACCESS PRIMITIVES
//
//void fetch(X* x) { UF1(x); A(S(x)) = *((C*)A(S(x))); }
//void store(X* x) { UF2(x); C a = pop(x); C v = pop(x); *((C*)a) = v; }
//void bfetch(X* x) { UF1(x); A(S(x)) = (C)*((B*)A(S(x))); }
//void bstore(X* x) { UF2(x); C a = pop(x); C v = pop(x); *((B*)a) = (B)v; }
//
//// CONTINUOUS MEMORY PRIMITIVES
//
//#define RESERVED(x)				((TH(x)) - ((C)H(x)))
//
//void grow(X* x) { 
//	if (A(TH(x)) != (TH(x) + 2*sizeof(C))) { x->err = ERR_NOT_ENOUGH_MEMORY; return; }
//	TH(x) += 2*sizeof(C);
//	D(TH(x)) = 0;
//}
//void shrink(X* x) {
//	if (RESERVED(x) < 2*sizeof(C)) { x->err = ERR_NOT_ENOUGH_RESERVED; return; }
//	D(TH(x)) = TH(x) - 2*sizeof(C);
//	TH(x) -= 2*sizeof(C);
//	A(TH(x)) = TH(x) + 2*sizeof(C);
//	D(TH(x)) = 0;
//}
//void allot(X* x) {
//	C b = pop(x);
//	if (b > 0) { 
//		if (b >= (TOP(x) - ((C)H(x)))) { x->err = ERR_NOT_ENOUGH_MEMORY; return; }
//		while (RESERVED(x) < b) { grow(x); if (x->err) return; }
//		H(x) += b;
//	} else if (b < 0) {
//		H(x) = (b < (BOTTOM(x) - H(x))) ? BOTTOM(x) : H(x) + b;
//		while (RESERVED(x) >= 2*sizeof(C)) { shrink(x); if (x->err) return; }
//	}
//}
//
//// CONTEXT PRIMITIVES
//
//void context(X* x) { S(x) = cons(x, (C)x, AS(ATM, S(x))); }
//void here(X* x) { S(x) = cons(x, (C)H(x), AS(ATM, S(x))); }
//void reserved(X* x) { S(x) = cons(x, RESERVED(x), AS(ATM, S(x))); }
//void latest(X* x) { S(x) = cons(x, (C)&x->latest, AS(ATM, S(x))); }
//
//// LIST PRIMITIVES
//
//void empty(X* x) { S(x) = cons(x, 0, AS(LST, S(x))); }
//// TODO: If executing l>s and stack is empty or top of stack is not a list, crashes
//void list_to_stack(X* x) { UF1(x); EL(x); C t = S(x); S(x) = N(S(x)); LK(t, P(x)); P(x) = t; }
//void stack_to_list(X* x) {
//	S(x) = reverse(S(x), 0);
//	if (!N(P(x))) { P(x) = cons(x, P(x), AS(LST, 0)); }
//	else { C t = P(x); P(x) = N(P(x)); LK(t, A(P(x))); A(P(x)) = t; }
//}
//
//// INPUT/OUTPUT PRIMITIVES (this should go in platform dependent headers)
//
//// Source code for getch is taken from:
//// Crossline readline (https://github.com/jcwangxp/Crossline).
//// It's a fantastic readline cross-platform replacement, but only getch was
//// needed and there's no need to include everything else.
//#ifdef _WIN32	// Windows
//int __getch__(void) { fflush(stdout); return _getch(); }
//#else
//int __getch__()
//{
//	char ch = 0;
//	struct termios old_term, cur_term;
//	fflush (stdout);
//	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
//	cur_term = old_term;
//	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
//	cur_term.c_cc[VMIN] = 1;
//	cur_term.c_cc[VTIME] = 0;
//	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
//	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
//	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
//	return ch;
//}
//#endif
//
//void key(X* x) { S(x) = cons(x, __getch__(), AS(ATM, S(x))); }
//void emit(X* x) { UF1(x); C k = A(S(x)); S(x) = recl(x, S(x)); printf("%c", (B)k); }
//
//// PARSING AND EVALUATION
//
//#define PRS_SPC(x)		while (TC(x) != 0 && isspace(TC(x))) { x->in++; } x->token = x->in;
//#define PRS_N_SPC(x)	while (TC(x) != 0 && !isspace(TC(x))) { x->in++; }
//void parse_token(X* x) { PRS_SPC(x); PUSH(x, TK(x), ATM); PRS_N_SPC(x); PUSH(x, TL(x), ATM); }
//#define FOUND(w)			((strlen(NFA(w)) == TL(x)) && (strncmp(NFA(w), TK(x), TL(x)) == 0))
//void find_token(X* x) { 
//	UF2(x);
//	if (TL(x) == 0) { x->err = ERR_ZERO_LENGTH_NAME; return; }
//	C w = L(x); while(w && !FOUND(w)) { w = N(w); }
//	if (w) { S(x) = recl(x, recl(x, S(x))); PUSH(x, w, WRD); PUSH(x, IMMEDIATE(w) ? 1 : -1, ATM); }
//	else { PUSH(x, 0, ATM); }
//}
//
//void outer(X* x) {
//	C w, i;
//	char *endptr;
//	do {
//		parse_token(x); 
//		if (TL(x) == 0) { drop(x); drop(x); x->err = ERR_EMPTY_TIB; return; }
//		printf("TOKEN: %.*s\n", (int)TL(x), TK(x));
//		find_token(x); i = pop(x); w = pop(x);
//		if (i) {
//			if (!x->state || IMMEDIATE(w)) { inner(x, XT(w)); if (x->err) return; }
//			else { if (PRIMITIVE(w)) { PUSH(x, A(XT(w)), PRM); } else { PUSH(x, w, WRD); } }
//		} else {
//			pop(x); // Remove address
//			intmax_t n = strtoimax(TK(x), &endptr, 10);
//			if (n == 0 && endptr == (char*)(TK(x))) { x->err = ERR_UNDEFINED_WORD; return; }
//			else { PUSH(x, n, ATM); }
//		}
//	} while(1);
//}
//
//void evaluate(X* x, B* buf) {
//	x->tib = buf;
//	x->token = x->in = 0;
//	C c = cons(x, (C)outer, AS(PRM, 0));
//	inner(x, c);
//}
//
//// BOOTSTRAPPING
//
//#define ADD_PRIMITIVE(x, n, f, i) \
//	L(x) = cons(x, cons(x, (C)n, AS(ATM, cons(x, (C)f, AS(PRM, 0)))), AS(i, L(x)));
//
//X* bootstrap(X* x) {
//	ADD_PRIMITIVE(x, "branch", &branch, 0);
//	ADD_PRIMITIVE(x, "zjump", &zjump, 0);
//	ADD_PRIMITIVE(x, "jump", &jump, 0);
//
//	ADD_PRIMITIVE(x, "i", &exec_i, 0);
//	ADD_PRIMITIVE(x, "x", &exec_x, 0);
//
//	ADD_PRIMITIVE(x, "[", &lbracket, 2);
//	ADD_PRIMITIVE(x, "]", &rbracket, 0);
//
//	ADD_PRIMITIVE(x, "+", &add, 0);
//	ADD_PRIMITIVE(x, "-", &sub, 0);
//	ADD_PRIMITIVE(x, "*", &mul, 0);
//	ADD_PRIMITIVE(x, "/", &division, 0);
//	ADD_PRIMITIVE(x, "mod", &mod, 0);
//
//	ADD_PRIMITIVE(x, ">", &gt, 0);
//	ADD_PRIMITIVE(x, "<", &lt, 0);
//	ADD_PRIMITIVE(x, "=", &eq, 0);
//	ADD_PRIMITIVE(x, "<>", &neq, 0);
//
//	ADD_PRIMITIVE(x, "and", &and, 0);
//	ADD_PRIMITIVE(x, "or", &or, 0);
//	ADD_PRIMITIVE(x, "invert", &invert, 0);
//
//	ADD_PRIMITIVE(x, "dup", &duplicate, 0);
//	ADD_PRIMITIVE(x, "swap", &swap, 0);
//	ADD_PRIMITIVE(x, "drop", &drop, 0);
//	ADD_PRIMITIVE(x, "over", &over, 0);
//	ADD_PRIMITIVE(x, "rot", &rot, 0);
//
//	ADD_PRIMITIVE(x, "{}", &empty, 0);
//	ADD_PRIMITIVE(x, "s>l", &stack_to_list, 0);
//	ADD_PRIMITIVE(x, "l>s", &list_to_stack, 0);
//
//	ADD_PRIMITIVE(x, "grow", &grow, 0);
//	ADD_PRIMITIVE(x, "shrink", &shrink, 0);
//
//	ADD_PRIMITIVE(x, "latest", &latest, 0);
//	ADD_PRIMITIVE(x, "here", &here, 0);
//
//	ADD_PRIMITIVE(x, "!", &store, 0);
//	ADD_PRIMITIVE(x, "@", &fetch, 0);
//	ADD_PRIMITIVE(x, "b!", &bstore, 0);
//	ADD_PRIMITIVE(x, "b@", &bfetch, 0);
//
//	ADD_PRIMITIVE(x, "key", &key, 0);
//	ADD_PRIMITIVE(x, "emit", &emit, 0);
//
//	return x;
//}
//
#endif
