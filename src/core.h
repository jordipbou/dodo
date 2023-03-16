#ifndef __CORE__
#define __CORE__

#include<inttypes.h>
#include<stdio.h>
#ifdef _WIN32
  #include <conio.h>
#else
	#include <unistd.h>
	#include <termios.h>
#endif

typedef int8_t		B;
typedef intptr_t	C;

#define A(p)			(*((C*)p))
#define D(p)			(*(((C*)p) + 1))
#define N(p)			(D(p) & -4)
#define T(p)			(D(p) & 3)

enum Types { ATM, LST, PRM, WRD };

#define AS(t, n)	((n & -4) | t)
#define LK(p, n)	(D(p) = AS(T(p), n))

#define XT(w)			(N(A(w)))

typedef struct {
	B *here;
	C there, size, free, pile;
	C err, ip, rstack;
	C latest;
} X;

#define F(x)			(x->free)
#define S(x)			(A(x->pile))
#define R(x)			(x->rstack)

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

	x->rstack = x->ip = x->err = x->latest = 0;

	return x;
}

#define ERR_STACK_OVERFLOW		-1
#define ERR_STACK_UNDERFLOW		-2
#define ERR_DIVISION_BY_ZERO	-3

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

/* TEMPORAL */
C dump_stack(X* x) {
	C p = S(x);
	while (p) { printf("%ld\n", A(p)); p = N(p); }
	return 0;
}

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

#define RETURN(x) \
	if (!x->ip) { \
		while (R(x) && T(R(x)) != ATM) { R(x) = recl(x, R(x)); } \
		if (R(x)) x->ip = A(R(x)); R(x) = recl(x, R(x)); \
		if (!x->ip) return; \
	}

#define ERROR(x) \
	if (x->err == 1) { /*printf("ip modified by primitive, resetting x->err to 0\n");*/ x->err = 0; } \
	else if (x->err) { /*printf("ERROR:%ld\n", x->err);*/ dump_context(x); return; }

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
					R(x) = cons(x, N(x->ip), AS(ATM, R(x)));
					R(x) = cons(x, A(x->ip), AS(WRD, R(x)));
				}
				x->ip = A(x->ip);
				break;
		}
		//printf("\n");
		ERROR(x);
	} while(1);
}

#define UF1(x)	if (!S(x)) { x->err = ERR_STACK_UNDERFLOW; return; }
#define UF2(x)	if (!(S(x) && N(S(x)))) { x->err = ERR_STACK_UNDERFLOW; return; }
#define UF3(x)	if (!(S(x) && N(S(x)) && N(N(S(x))))) { x->err = ERR_STACK_UNDERFLOW; return; }

void branch(X* x) {
	UF1(x);
	C b = A(S(x)); S(x) = recl(x, S(x));
	if (N(N(N(x->ip)))) { R(x) = cons(x, N(N(N(x->ip))), AS(ATM, R(x))); }
	if (b) { x->ip = A(N(x->ip)); }
	else { x->ip = A(N(N(x->ip))); }
	x->err = 1;
}

void exec_x(X* x) { 
	UF1(x); 
	if (N(x->ip)) { R(x) = cons(x, N(x->ip), AS(ATM, R(x))); }
	// TODO: Type of item on top of data stack should be checked
	//printf("exec_x::setting ip to %ld\n", A(S(x)));
	x->ip = A(S(x)); 
	x->err = 1;
}
void exec_i(X* x) { 
	UF1(x); 
	C t = S(x); S(x) = N(S(x)); 
	if (N(x->ip)) R(x) = cons(x, N(x->ip), AS(ATM, R(x)));
	R(x) = cons(x, t, AS(LST, R(x)));
	x->ip = A(t);
	x->err = 1;
}

void duplicate(X* x) { UF1(x); S(x) = cons(x, A(S(x)), AS(ATM, S(x))); }
void swap(X* x) { UF2(x); C t = A(S(x)); A(S(x)) = A(N(S(x))); A(N(S(x))) = t; }
void rot(X* x) { UF3(x); C t = N(N(S(x))); LK(N(S(x)), N(N(N(S(x))))); LK(t, S(x)); S(x) = t; }
void drop(X* x) { S(x) = recl(x, S(x)); }

#define BINOP(o) A(N(S(x))) = A(N(S(x))) o A(S(x)); S(x) = recl(x, S(x));

void gt(X* x) { UF2(x); BINOP(>) }
void lt(X* x) { UF2(x); BINOP(<) }
void eq(X* x) { UF2(x); BINOP(==) }
void neq(X* x) { UF2(x); BINOP(!=) }

void add(X* x) { UF2(x); BINOP(+) }
void sub(X* x) { UF2(x); BINOP(-) }
void mul(X* x) { UF2(x); BINOP(*) }
void division(X* x) { UF2(x); if (!A(S(x))) { x->err = ERR_DIVISION_BY_ZERO; return; } BINOP(/) }
void mod(X* x) { UF2(x); BINOP(%) }

void and(X* x) { UF2(x); BINOP(&) }
void or(X* x) { UF2(x); BINOP(|) }
void invert(X* x) { UF1(x); A(S(x)) = ~A(S(x)); } 

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

// TODO: grow, shrink, store, fetch, bstore, bfetch

#endif
