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
	C there, size, free, pile, latest;
} X;

#define F(x)			(x->free)
#define S(x)			(A(x->pile))

typedef C (*FUNC)(X*);

enum Prims { ZJUMP, JUMP, BRANCH };

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

	x->latest = 0;

	return x;
}

C cons(X* x, C a, C d) { 
	if (x->free == x->there) return 0;
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

#define ERR_STACK_OVERFLOW		-1
#define ERR_STACK_UNDERFLOW		-2

C error(X* x, C err) { /* TODO */	return err; }

#define ERR(x, e)	{ C __err__ = error(x, e); if (__err__) { return __err__; } }

C inner(X* x, C l) {
	C p = l, b;
	while (p) {
		switch (T(p)) {
			case ATM: 
				if ((S(x) = cons(x, A(p), AS(ATM, S(x)))) == 0) { ERR(x, ERR_STACK_OVERFLOW); }
				p = N(p); 
				break;
			case LST: 
				if ((S(x) = cons(x, clone(x, A(p)), AS(LST, S(x)))) == 0) { ERR(x, ERR_STACK_OVERFLOW); }
				p = N(p); break;
			case PRM: 
				switch (A(p)) {
					case BRANCH:
						if (!S(x)) { ERR(x, ERR_STACK_UNDERFLOW); }
						b = A(S(x)); S(x) = recl(x, S(x));
						if (b) { p = (N(N(N(p)))) ? (inner(x, A(N(p))), N(N(N(p)))) : A(N(p)); } 
						else { p = (N(N(N(p)))) ? (inner(x, A(N(N(p)))), N(N(N(p)))) : A(N(N(p))); }
						break;
					case ZJUMP:
						if (!S(x)) { ERR(x, ERR_STACK_UNDERFLOW); }
						b = A(S(x)); S(x) = recl(x, S(x));
						if (b) { p = A(N(p)); }
						else { p = N(N(p)); }
						break;
					case JUMP: 
						p = A(N(p));
						break;
					default:
						b = ((FUNC)A(p))(x); 
						if (b != 0) { ERR(x, b); }
						p = N(p); 
						break;
				} 
				break;
			case WRD: 
				p = (N(p) ? (inner(x, XT(A(p))), N(p)) : XT(A(p))); break;
		}
	}
	return 0;
}

C duplicate(X* x) { 
	if (!S(x)) { ERR(x, ERR_STACK_UNDERFLOW); }
	else if ((S(x) = cons(x, A(S(x)), AS(ATM, S(x)))) == 0) { ERR(x, ERR_STACK_OVERFLOW); }
	else return 0; 
}
C swap(X* x) { 
	if (!(S(x) && N(S(x)))) { ERR(x, ERR_STACK_UNDERFLOW); }
	else { C t = A(S(x)); A(S(x)) = A(N(S(x))); A(N(S(x))) = t; return 0; }
}
C rot(X* x) { 
	if (!(S(x) && N(S(x)) && N(N(S(x))))) { ERR(x, ERR_STACK_UNDERFLOW); }
	else { C t = N(N(S(x))); LK(N(S(x)), N(N(N(S(x))))); LK(t, S(x)); S(x) = t; return 0;	}
}
C drop(X* x) { S(x) = recl(x, S(x)); return 0; }

#define BINOP(o) \
	if (!(S(x) && N(S(x)))) { ERR(x, ERR_STACK_UNDERFLOW); } \
	else { A(N(S(x))) = A(N(S(x))) o A(S(x)); S(x) = recl(x, S(x)); return 0; }

C gt(X* x) { BINOP(>) }
C lt(X* x) { BINOP(<) }
C eq(X* x) { BINOP(==) }
C neq(X* x) { BINOP(!=) }

C add(X* x) { BINOP(+) }
C sub(X* x) { BINOP(-) }
C mul(X* x) { BINOP(*) }
C division(X* x) { BINOP(/) }
C mod(X* x) { BINOP(%) }

C and(X* x) { BINOP(&) }
C or(X* x) { BINOP(|) }
C invert(X* x) { 
	if (!S(x)) { ERR(x, ERR_STACK_UNDERFLOW); } 
	else { A(S(x)) = ~A(S(x)); return 0; } 
}

C exec_x(X* x) { 
	if (!S(x)) { ERR(x, ERR_STACK_UNDERFLOW); }
	else { return inner(x, A(S(x))); }
}
C exec_i(X* x) { 
	if (!(S(x))) { ERR(x, ERR_STACK_UNDERFLOW); }
	else { C t = S(x); S(x) = N(S(x)); C res = inner(x, A(t)); recl(x, t); return res; }
}

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

C key(X* x) {
	if ((S(x) = cons(x, __getch__(), AS(ATM, S(x)))) == 0) { ERR(x, ERR_STACK_OVERFLOW); }
	else { return 0; }
}
C emit(X* x) {
	if (!S(x)) { ERR(x, ERR_STACK_UNDERFLOW); }
	else { C k = A(S(x)); S(x) = recl(x, S(x)); printf("%c", (B)k); return 0; }
}

// TODO: grow, shrink, store, fetch, bstore, bfetch

#endif
