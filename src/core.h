#ifndef __CORE__
#define __CORE__

#include<inttypes.h>
#include<string.h>
#include<ctype.h>
#include<stdio.h>

#ifdef _WIN32
  #include <conio.h>
#else
	#include <unistd.h>
	#include <termios.h>
#endif

typedef int8_t		B;
typedef intptr_t	C;

#define A(p)							(*((C*)p))
#define D(p)							(*(((C*)p) + 1))
#define N(p)							(D(p) & -4)
#define T(p)							(D(p) & 3)

enum Types { ATM, LST, PRM, WRD };

#define AS(t, n)					((n & -4) | t)
#define LK(p, n)					(D(p) = AS(T(p), n))

#define NFA(w)						((B*)(A(A(w))))
#define XT(w)							(N(A(w)))

enum Words { NIP, NIC, IMP, IMC };

#define PRIMITIVE(w)			((T(w) & 1) == 0)
#define IMMEDIATE(w)			((T(w) & 2) == 2)

typedef struct {
	B *here;
	C there, size, free, pile;
	C err, ip, rstack;
	C state, latest;
} X;

#define F(x)							(x->free)
#define P(x)							(x->pile)
#define S(x)							(A(P(x)))
#define R(x)							(x->rstack)
#define L(x)							(x->latest)

typedef void (*FUNC)(X*);

#define ALIGN(a, b)			((((C)a) + (b - 1)) & ~(b - 1))
#define BOTTOM(x)				(((B*)x) + sizeof(X))
#define TOP(x)					(ALIGN(((B*)x) + x->size - (2*sizeof(C)) - 1, (2*sizeof(C))))

X* init(B* bl, C sz) {
	X* x = (X*)bl;	
	x->size = sz;
	x->here = BOTTOM(x);
	x->there = ALIGN(BOTTOM(x), 2*sizeof(C));
	x->pile = TOP(x);
	A(x->pile) = 0;
	D(x->pile) = AS(LST, 0);
	x->free = x->pile - 2*sizeof(C);

	for (C p = x->there; p <= x->free; p += 2*sizeof(C)) {
		A(p) = p == x->free ? 0 : p + 2*sizeof(C);
		D(p) = p == x->there ? 0 : p - 2*sizeof(C);
	}

	x->state = x->rstack = x->ip = x->err = x->latest = 0;

	return x;
}

#define ERR_STACK_OVERFLOW				-1
#define ERR_STACK_UNDERFLOW				-2
#define ERR_DIVISION_BY_ZERO			-3
#define ERR_NOT_ENOUGH_MEMORY			-4
#define ERR_NOT_ENOUGH_RESERVED		-5
#define ERR_UNDEFINED_WORD				-6
#define ERR_EXPECTED_LIST					-7

// LIST FUNCTIONS

C cons(X* x, C a, C d) { 
	if (x->free == x->there) { x->err = ERR_STACK_OVERFLOW; return 0; }
	else { C p = F(x); F(x) = D(F(x)); A(p) = a; D(p) = d; return p; }
}
C clone(X* x, C p) { 
	if (!p) return 0;
	else { return cons(x, T(p) == LST ? clone(x, A(p)) : A(p), AS(T(p), clone(x, N(p)))); }
}
C recl(X* x, C p) { 
	if (!p) return 0;
	else {
		if (T(p) == LST) while (A(p)) A(p) = recl(x, A(p));
		C t = N(p); D(p) = F(x); A(p) = 0; F(x) = p;
		return t;
	}
}
C pop(X* x) { C v = A(S(x)); S(x) = recl(x, S(x)); return v; }
C reverse(C p, C l) { 
	if (p) { C t = N(p); D(p) = AS(T(p), l); return reverse(t, p); } 
	else { return l; }
}
C length(C p) { C c = 0; while (p) { c++; p = N(p); } return c; }

/* TEMPORAL: INSPECTION */
void dump_list(X* x, C l) {
	while (l) {
		switch (T(l)) {
			case ATM: printf("%ld ", A(l)); break;
			case LST: printf("{ "); dump_list(x, A(l)); printf("} "); break;
			case PRM: printf("#%ld ", A(l)); break;
			case WRD: printf("@%ld ", A(l)); break;
		}
		l = N(l);
	}
}

void dump_stack(X* x) { printf("<%ld> ", length(S(x))); dump_list(x, S(x)); printf("\n"); }

C dump_return_stack(X* x) {
	C p = x->rstack;
	while (p) { printf("%ld\n", A(p)); p = N(p); }
	return 0;
}

C dump_context(X* x) {
	printf("DATA STACK ---------------\n"); dump_stack(x);
	printf("RETURN STACK -------------\n"); dump_return_stack(x);
}
/* -TEMPORAL */

// INNER INTERPRETER

#define RETURN(x) \
	if (!x->ip) { \
		while (R(x) && T(R(x)) != PRM) { R(x) = recl(x, R(x)); } \
		if (R(x)) x->ip = A(R(x)); R(x) = recl(x, R(x)); \
		if (!x->ip) return; \
	}

#define ERROR(x) \
	if (x->err == 1) { /*printf("ip modified by primitive, resetting x->err to 0\n");*/ x->err = 0; } \
	else if (x->err) { /*printf("ERROR:%ld\n", x->err);*/ /*dump_context(x);*/ return; }

void inner(X* x, C l) {
	//printf("************ INNER l = %ld\n", l);
	x->ip = l;
	//C old_ip = 0;
	do {
		//if (old_ip == x->ip) { printf("SAME IP!!!!\n"); return; } else { old_ip = x->ip; }
		//printf("[IP:%ld|ERR:%ld] ", x->ip, x->err);
		RETURN(x);
		switch (T(x->ip)) {
			case ATM: /*printf("pushing %ld ", A(x->ip));*/ S(x) = cons(x, A(x->ip), AS(ATM, S(x))); x->ip = N(x->ip); break;
			case LST: /*printf("cloning %ld ", A(x->ip));*/ S(x) = cons(x, clone(x, A(x->ip)), AS(LST, S(x))); x->ip = N(x->ip); break;
			case PRM: /*printf("executing %ld ", A(x->ip));*/ ((FUNC)A(x->ip))(x); /*printf("x->err %ld ", x->err);*/ if (x->err == 0) { /*printf("auto-advancing ip ");*/ x->ip = N(x->ip); } break;
			case WRD:
				//printf("calling %ld ", A(x->ip));
				if (N(x->ip)) { 
					R(x) = cons(x, N(x->ip), AS(PRM, R(x)));
					R(x) = cons(x, A(x->ip), AS(WRD, R(x)));
				}
				x->ip = XT(A(x->ip));
				break;
		}
		//printf("\n");
		ERROR(x);
	} while(1);
}

#define UF1(x)	if (!S(x)) { x->err = ERR_STACK_UNDERFLOW; return; }
#define UF2(x)	if (!(S(x) && N(S(x)))) { x->err = ERR_STACK_UNDERFLOW; return; }
#define UF3(x)	if (!(S(x) && N(S(x)) && N(N(S(x))))) { x->err = ERR_STACK_UNDERFLOW; return; }

#define EL(x)		if (T(S(x)) != LST) { x->err = ERR_EXPECTED_LIST; return; }

// EXECUTION PRIMITIVES

void branch(X* x) {
	UF1(x);
	C b = A(S(x)); S(x) = recl(x, S(x));
	if (N(N(N(x->ip)))) { R(x) = cons(x, N(N(N(x->ip))), AS(PRM, R(x))); }
	if (b) { x->ip = A(N(x->ip)); }
	else { x->ip = A(N(N(x->ip))); }
	x->err = 1;
}
void zjump(X* x) {
	UF1(x);
	C b = A(S(x)); S(x) = recl(x, S(x));
	if (!b) { x->ip = A(N(x->ip)); }
	else { x->ip = N(N(x->ip)); }
	x->err = 1;
}
void jump(X* x) {
	x->ip = A(N(x->ip));
	x->err = 1;
}
void exec_x(X* x) { 
	UF1(x);
	if (N(x->ip)) { R(x) = cons(x, N(x->ip), AS(PRM, R(x))); }
	// TODO: Type of item on top of data stack should be checked, if not list, execute primitive
	//printf("exec_x::setting ip to %ld\n", A(S(x)));
	x->ip = A(S(x)); 
	x->err = 1;
}
void exec_i(X* x) { 
	UF1(x); 
	C t = S(x); S(x) = N(S(x)); 
	if (N(x->ip)) R(x) = cons(x, N(x->ip), AS(PRM, R(x)));
	R(x) = cons(x, t, AS(LST, R(x)));
	x->ip = A(t);
	x->err = 1;
}
void lbracket(X* x) { x->state = 0; }
void rbracket(X* x) { x->state = 1; }

// STACK PRIMITIVES

void duplicate(X* x) { 
	UF1(x); 
	if (T(S(x)) == LST) { S(x) = cons(x, clone(x, A(S(x))), AS(LST, S(x))); }
	else { S(x) = cons(x, A(S(x)), AS(T(S(x)), S(x))); }
}
void swap(X* x) { UF2(x); C t = N(S(x)); LK(S(x), N(N(S(x)))); LK(t, S(x)); S(x) = t; }
void drop(X* x) { S(x) = recl(x, S(x)); }
void over(X* x) { 
	UF2(x);
	if (T(N(S(x))) == LST) { S(x) = cons(x, clone(x, A(N(S(x)))), AS(LST, S(x))); }
	else { S(x) = cons(x, A(N(S(x))), AS(T(N(S(x))), S(x))); }
}
void rot(X* x) { UF3(x); C t = N(N(S(x))); LK(N(S(x)), N(N(N(S(x))))); LK(t, S(x)); S(x) = t; }

// COMPARISON PRIMITIVES

#define BINOP(o) A(N(S(x))) = A(N(S(x))) o A(S(x)); S(x) = recl(x, S(x));

void gt(X* x) { UF2(x); BINOP(>) }
void lt(X* x) { UF2(x); BINOP(<) }
void eq(X* x) { UF2(x); BINOP(==) }
void neq(X* x) { UF2(x); BINOP(!=) }

// ARITHMETIC PRIMITIVES

void add(X* x) { UF2(x); BINOP(+) }
void sub(X* x) { UF2(x); BINOP(-) }
void mul(X* x) { UF2(x); BINOP(*) }
void division(X* x) { UF2(x); if (!A(S(x))) { x->err = ERR_DIVISION_BY_ZERO; return; } BINOP(/) }
void mod(X* x) { UF2(x); BINOP(%) }

// BIT PRIMITIVES

void and(X* x) { UF2(x); BINOP(&) }
void or(X* x) { UF2(x); BINOP(|) }
void invert(X* x) { UF1(x); A(S(x)) = ~A(S(x)); } 

// MEMORY ACCESS PRIMITIVES

void fetch(X* x) { UF1(x); A(S(x)) = *((C*)A(S(x))); }
void store(X* x) { UF2(x); C a = pop(x); C v = pop(x); *((C*)a) = v; }
void bfetch(X* x) { UF1(x); A(S(x)) = (C)*((B*)A(S(x))); }
void bstore(X* x) { UF2(x); C a = pop(x); C v = pop(x); *((B*)a) = (B)v; }

// CONTINUOUS MEMORY PRIMITIVES

#define RESERVED(x)				((x->there) - ((C)x->here))

void grow(X* x) { 
	if (A(x->there) != (x->there + 2*sizeof(C))) { x->err = ERR_NOT_ENOUGH_MEMORY; return; }
	x->there += 2*sizeof(C);
	D(x->there) = 0;
}
void shrink(X* x) {
	if (RESERVED(x) < 2*sizeof(C)) { x->err = ERR_NOT_ENOUGH_RESERVED; return; }
	D(x->there) = x->there - 2*sizeof(C);
	x->there -= 2*sizeof(C);
	A(x->there) = x->there + 2*sizeof(C);
	D(x->there) = 0;
}
// TODO: allot

// CONTEXT PRIMITIVES

void context(X* x) { S(x) = cons(x, (C)x, AS(ATM, S(x))); }
void here(X* x) { S(x) = cons(x, (C)x->here, AS(ATM, S(x))); }
void reserved(X* x) { S(x) = cons(x, RESERVED(x), AS(ATM, S(x))); }
void latest(X* x) { S(x) = cons(x, (C)&x->latest, AS(ATM, S(x))); }

// LIST PRIMITIVES

void empty(X* x) { S(x) = cons(x, 0, AS(LST, S(x))); }
// TODO: If executing l>s and stack is empty or top of stack is not a list, crashes
void list_to_stack(X* x) { UF1(x); EL(x); C t = S(x); S(x) = N(S(x)); LK(t, P(x)); P(x) = t; }
void stack_to_list(X* x) {
	S(x) = reverse(S(x), 0);
	if (!N(P(x))) { P(x) = cons(x, P(x), AS(LST, 0)); }
	else { C t = P(x); P(x) = N(P(x)); LK(t, A(P(x))); A(P(x)) = t; }
}

// INPUT/OUTPUT PRIMITIVES (this should go in platform dependent headers)

// Source code for getch is taken from:
// Crossline readline (https://github.com/jcwangxp/Crossline).
// It's a fantastic readline cross-platform replacement, but only getch was
// needed and there's no need to include everything else.
#ifdef _WIN32	// Windows
int __getch__(void) { fflush(stdout); return _getch(); }
#else
int __getch__()
{
	char ch = 0;
	struct termios old_term, cur_term;
	fflush (stdout);
	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
	cur_term = old_term;
	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
	cur_term.c_cc[VMIN] = 1;
	cur_term.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
	return ch;
}
#endif

void key(X* x) { S(x) = cons(x, __getch__(), AS(ATM, S(x))); }
void emit(X* x) { UF1(x); C k = A(S(x)); S(x) = recl(x, S(x)); printf("%c", (B)k); }

// PARSING AND EVALUATION

#define PARSE_SPACE(b, t, i)			*t = *i; while (*(b + *i) != 0 && isspace(*(b + *i))) { (*i)++; }
#define PARSE_NON_SPACE(b, t, i)	*t = *i; while (*(b + *i) != 0 && !isspace(*(b + *i))) { (*i)++; }
C parse_token(B* b, C* t, C* i) { PARSE_SPACE(b, t, i); PARSE_NON_SPACE(b, t, i); return *i - *t; }
#define FOUND(w, n, l) ((strlen(NFA(w)) == l) && (strncmp(NFA(w), n, l) == 0))
C find_token(X* x, B* n, C l) { C w = L(x); while(w && !FOUND(w, n, l)) { w = N(w); } return w; }

void evaluate(X* x, B* buf) {
	C w, tk = 0, in = 0;
	char *endptr;
	do {
		if (parse_token(buf, &tk, &in) == 0) { return; }
		if ((w = find_token(x, buf + tk, in - tk)) != 0) {
			if (!x->state || IMMEDIATE(w)) {
				inner(x, XT(w)); if (x->err) return;
			} else {
				if (PRIMITIVE(w)) {
					S(x) = cons(x, A(XT(w)), AS(PRM, S(x)));
				} else {
					S(x) = cons(x, w, AS(WRD, S(x)));
				}
			}
		} else {
			intmax_t n = strtoimax(buf + tk, &endptr, 10);
			if (n == 0 && endptr == (char*)(buf + tk)) {
				x->err = ERR_UNDEFINED_WORD;
				return;
			} else {
				S(x) = cons(x, n, AS(ATM, S(x)));
			}
		}
	} while(1);
}

// BOOTSTRAPPING

#define ADD_PRIMITIVE(x, n, f, i) \
	L(x) = cons(x, cons(x, (C)n, AS(ATM, cons(x, (C)f, AS(PRM, 0)))), AS(i, L(x)));

X* bootstrap(X* x) {
	ADD_PRIMITIVE(x, "branch", &branch, 0);
	ADD_PRIMITIVE(x, "zjump", &zjump, 0);
	ADD_PRIMITIVE(x, "jump", &jump, 0);

	ADD_PRIMITIVE(x, "i", &exec_i, 0);
	ADD_PRIMITIVE(x, "x", &exec_x, 0);

	ADD_PRIMITIVE(x, "[", &lbracket, 2);
	ADD_PRIMITIVE(x, "]", &rbracket, 0);

	ADD_PRIMITIVE(x, "+", &add, 0);
	ADD_PRIMITIVE(x, "-", &sub, 0);
	ADD_PRIMITIVE(x, "*", &mul, 0);
	ADD_PRIMITIVE(x, "/", &division, 0);
	ADD_PRIMITIVE(x, "mod", &mod, 0);

	ADD_PRIMITIVE(x, ">", &gt, 0);
	ADD_PRIMITIVE(x, "<", &lt, 0);
	ADD_PRIMITIVE(x, "=", &eq, 0);
	ADD_PRIMITIVE(x, "<>", &neq, 0);

	ADD_PRIMITIVE(x, "and", &and, 0);
	ADD_PRIMITIVE(x, "or", &or, 0);
	ADD_PRIMITIVE(x, "invert", &invert, 0);

	ADD_PRIMITIVE(x, "dup", &duplicate, 0);
	ADD_PRIMITIVE(x, "swap", &swap, 0);
	ADD_PRIMITIVE(x, "drop", &drop, 0);
	ADD_PRIMITIVE(x, "over", &over, 0);
	ADD_PRIMITIVE(x, "rot", &rot, 0);

	ADD_PRIMITIVE(x, "{}", &empty, 0);
	ADD_PRIMITIVE(x, "s>l", &stack_to_list, 0);
	ADD_PRIMITIVE(x, "l>s", &list_to_stack, 0);

	ADD_PRIMITIVE(x, "grow", &grow, 0);
	ADD_PRIMITIVE(x, "shrink", &shrink, 0);

	ADD_PRIMITIVE(x, "latest", &latest, 0);
	ADD_PRIMITIVE(x, "here", &here, 0);

	ADD_PRIMITIVE(x, "!", &store, 0);
	ADD_PRIMITIVE(x, "@", &fetch, 0);
	ADD_PRIMITIVE(x, "b!", &bstore, 0);
	ADD_PRIMITIVE(x, "b@", &bfetch, 0);

	ADD_PRIMITIVE(x, "key", &key, 0);
	ADD_PRIMITIVE(x, "emit", &emit, 0);

	return x;
}

#endif
