#include<stdint.h>

typedef int8_t		B;
typedef intptr_t	C;		// 16, 32 or 64 bits depending on system

#define A(pair)					(*((C*)pair))
#define D(pair)					(*(((C*)pair) + 1))
#define t(type, addr)		((addr & -4) | type)
#define T(pair)					((D(pair)) & 3)
#define ST(pair)				((A(pair)) & 3)
#define N(pair)					((D(pair)) & -4)
#define R(pair)					((A(pair)) & -4)

#define T_FREE				0
#define T_ATOM				1
#define T_LIST				2
#define T_PRIMITIVE		3

typedef struct {
	B* here;
	C there, size, horizon, rstack, dict, err;
} X;

#define BOTTOM(x)			(((B*)x) + sizeof(X))
#define TOP(x)				(ALIGN(((B*)x) + x->size - 2*sizeof(C) - 1, 2*sizeof(C)))

#define Z(x)					(x->horizon)
#define K(x)					D(Z(x))
#define F(x)					A(Z(x))
#define H(x)					A(K(x))
#define HR(x)					A(D(K(x)))
#define G(x)					(x->dict)
#define E(x)					(x->err)

typedef void (*FUNC)(X*);

#define ALIGN(addr, bound)	((((C)addr) + (bound - 1)) & ~(bound - 1))

#define ERR_NOT_ENOUGH_MEMORY		-1
#define ERR_OVERFLOW						-2
#define ERR_UNDERFLOW						-3

C height(C p) { C c = 0; while (p != 0) { c++; p = A(p); } return c; }
C depth(C p) { C c = 0; while (p != 0) { c++; p = D(p); } return c; }

X* init(B* block, C size) {
	X* x = (X*)block;	
	x->size = size;
	x->here = BOTTOM(x);
	x->there = ALIGN(BOTTOM(x), 2*sizeof(C));
	x->horizon = TOP(x);

	for (C p = x->there; p <= Z(x); p += 2*sizeof(C)) {
		A(p) = p == x->there ? 0 : p - 2*sizeof(C);
		D(p) = p == Z(x) ? 0 : p + 2*sizeof(C);
	}

	x->err = x->dict = x->rstack = 0;

	return x;
}

#define OF(x)		if (F(x) == 0) { x->err = ERR_OVERFLOW; return; }
#define UF(x)		if (K(x) == 0) { x->err = ERR_UNDERFLOW; return 0; }

// TODO: Pair creation and stack manipulation are not accounting for types !!
void push(X* x, C v) { OF(x); Z(x) = F(x); H(x) = v; }
C pop(X* x) { UF(x); C t = H(x); H(x) = Z(x); Z(x) = K(x); return t; }
C cons(X* x, C a, C d) { C p = Z(x); push(x, a); K(x) = K(x) ? D(K(x)) : 0; D(p) = d; return p; }

void _dup(X* x) { push(x, H(x)); }
void _swap(X* x) { C t = H(x); H(x) = HR(x); HR(x) = t; }
void _over(X* x) { push(x, HR(x)); }
void _rot(X* x) { C t = A(D(D(K(x))));; A(D(D(K(x)))) = HR(x); HR(x) = H(x); H(x) = t; }
void _drop(X* x) { pop(x); }

void _add(X* x) { C t = pop(x); H(x) += t; }
void _sub(X* x) { C t = pop(x); H(x) -= t; }
void _mul(X* x) { C t = pop(x); H(x) *= t; }
void _div(X* x) { C t = pop(x); H(x) /= t; }
void _mod(X* x) { C t = pop(x); H(x) %= t; }

void _gt(X* x) { C t = pop(x); H(x) = H(x) > t; }
void _lt(X* x) { C t = pop(x); H(x) = H(x) < t; }
void _eq(X* x) { C t = pop(x); H(x) = H(x) == t; }
void _neq(X* x) { C t = pop(x); H(x) = H(x) != t; }

void _and(X* x) { C t = pop(x); H(x) = H(x) && t; }
void _or(X* x) { C t = pop(x); H(x) = H(x) || t; }
void _not(X* x) { H(x) = !H(x); }

#define ATOM(x, n, cdr)				cons(x, n, t(T_ATOM, cdr))
#define PRIMITIVE(x, p, cdr)	cons(x, (C)p, t(T_PRIMITIVE, cdr))
#define RECURSION(x, cdr)			cons(x, 0, t(T_PRIMITIVE, cdr))
C BRANCH(X* x, C tb, C fb, C cdr) {
	if (tb) { C lt = tb; while(N(lt)) lt = N(lt); D(lt) = t(T(lt), cdr); } else { tb = cdr; }
	if (fb) { C lf = fb; while(N(lf)) lf = N(lf); D(lf) = t(T(lf), cdr); } else { fb = cdr; }
	return cons(x, t(T_ATOM, cons(x, tb, fb)), t(T_LIST, cdr));
}
#define WORD(x, xt, cdr)			cons(x, t(T_LIST, xt), t(T_LIST, cdr))

void inner(X* x, C xlist) {
	C ip = xlist;
	while(x->err == 0 && ip != 0) {
		switch(T(ip)) {
			case T_ATOM: push(x, A(ip)); ip = N(ip); break;
			case T_PRIMITIVE:	
/* RECURSION */	if (A(ip) == 0) { ip = N(ip) ? (inner(x, xlist), N(ip)) : xlist; }
/* PRIMITIVE */	else { ((FUNC)A(ip))(x); ip = N(ip); }
				break;
			case T_LIST:
				switch(ST(ip)) {
/* BRANCH */	case T_ATOM: ip = pop(x) ? A(R(ip)) : D(R(ip)); break;
/* WORD   */	case T_LIST: ip = N(ip) ? (inner(x, R(ip)), N(ip)) : R(ip); break;
				}
		}
	}
}

////#ifdef _WIN32
////  #include <conio.h>
////#else
////	#include <unistd.h>
////	#include <termios.h>
////#endif
//
////// Source code for getch is taken from:
////// Crossline readline (https://github.com/jcwangxp/Crossline).
////// It's a fantastic readline cross-platform replacement, but only getch was
////// needed and there's no need to include everything else.
////#ifdef _WIN32	// Windows
////int dodo_getch (void) {	fflush (stdout); return _getch(); }
////#else
////int dodo_getch ()
////{
////	char ch = 0;
////	struct termios old_term, cur_term;
////	fflush (stdout);
////	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
////	cur_term = old_term;
////	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
////	cur_term.c_cc[VMIN] = 1;
////	cur_term.c_cc[VTIME] = 0;
////	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
////	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
////	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
////	return ch;
////}
////#endif
////
////void _key(X* x) { PUSH(x, dodo_getch()); }
////void _emit(X* x) { 
////	C K = H(x); 
////	POP(x); 
////	if (K == 127) { printf ("\b \b"); } 
////	else { printf ("%c", (char)K); }
////}
//
//void push_stack(X* x) {
//	x->stacks.next = cons(x, T_LIST, (C)AS(T_LIST, REF((&x->stacks))), NEXT((&x->stacks)));
//	x->stacks.stack = 0;
//}
//
//void stack_to_list(X* x) {
//	PAIR* list = REF((&x->stacks));
//	x->stacks = *(NEXT((&x->stacks)));
//	x->stacks.ref = (C)cons(x, T_LIST, (C)list, REF((&x->stacks)));
//}
//
//#define RESERVED(x)				((x->there) - ((C)x->here))
//
//B* allot(X* x, C bytes) {
//	B* here = x->here;
//	if (bytes == 0) { 
//		return here;
//	} else if (bytes < 0) {
//		if ((x->here + bytes) > BOTTOM(x)) x->here += bytes;
//		else x->here = BOTTOM(x);
//		while ((x->there - 1) >= PALIGN(x->here)) { reclaim(x, --x->there); }
//	} else /* bytes > 0 */ {
//		while (RESERVED(x) < bytes && x->there < x->top) {
//			if (IS(T_F, x->there)) {
//				if (x->there->prev != 0) {
//					x->there->prev->next = AS(T_F, NEXT(x->there));
//				}
//				x->there++;
//			} else {
//				x->err = ERR_NOT_ENOUGH_MEMORY;
//				return here;
//			}
//		}
//		if (RESERVED(x) >= bytes)	x->here += bytes;
//		else x->err = ERR_NOT_ENOUGH_MEMORY;
//	}
//	return here;
//}
//
//void align(X* x) { allot(x, ALIGN(x->here, sizeof(C)) - ((C)x->here)); }
//
//#define NFA(w)		(REF(REF(w)))
//#define DFA(w)		(REF(NEXT(REF(w))))
//#define CFA(w)		(NEXT(NEXT(REF(w))))
//#define COUNT(s)	(*((C*)(((B*)s) - sizeof(C))))
//
//B* compile_str(X* x, B* str, C len) {
//	align(x);
//	B* cstr = allot(x, sizeof(C) + len + 1);
//	*((C*)cstr) = len;
//	for (C i = 0; i < len; i++) {
//		cstr[sizeof(C) + i] = str[i];
//	}
//	cstr[sizeof(C) + len] = 0;
//	return cstr + sizeof(C);
//}
//
//PAIR* header(X* x, B* name, C nlen) {
//	B* str = compile_str(x, name, nlen);
//
//	return 
//		cons(x, T_ATOM, 
//			(C)cons(x, T_ATOM, (C)str,
//						cons(x, T_ATOM, (C)x->here, 0)), 
//			0);
//}
//
//PAIR* body(X* x, PAIR* word, PAIR* cfa) {
//	PAIR* old_cfa = CFA(word);
//	NEXT(REF(word))->next = AS(T_ATOM, cfa);
//	while (old_cfa != 0) { old_cfa = reclaim(x, old_cfa); }
//	return word;
//}
//
//PAIR* reveal(X* x, PAIR* header) {
//	header->next = AS(T_ATOM, x->dict);
//	x->dict = header;
//	return header;
//}
//
//#define IS_IMMEDIATE(w)		(TYPE(w->ref) & 1)
//
//void _immediate(X* x) {
//	x->dict->ref = AS(1, REF(x->dict));
//}
//
////PAIR* find(X* x, B* name, C nlen) {
////	// TODO
////}
//
//X* dodo(X* x) {
//	reveal(x, body(x, header(x, "+", 1), cons(x, T_PRIMITIVE, (C)&_add, 0)));
//	reveal(x, body(x, header(x, "-", 1), cons(x, T_PRIMITIVE, (C)&_sub, 0)));
//	reveal(x, body(x, header(x, "*", 1), cons(x, T_PRIMITIVE, (C)&_mul, 0)));
//	reveal(x, body(x, header(x, "/", 1), cons(x, T_PRIMITIVE, (C)&_div, 0)));
//	reveal(x, body(x, header(x, "mod", 3), cons(x, T_PRIMITIVE, (C)&_mod, 0)));
//
//	reveal(x, body(x, header(x, ">", 1), cons(x, T_PRIMITIVE, (C)&_gt, 0)));
//	reveal(x, body(x, header(x, "<", 1), cons(x, T_PRIMITIVE, (C)&_lt, 0)));
//	reveal(x, body(x, header(x, "=", 1), cons(x, T_PRIMITIVE, (C)&_eq, 0)));
//	reveal(x, body(x, header(x, "<>", 2), cons(x, T_PRIMITIVE, (C)&_neq, 0)));
//
//	reveal(x, body(x, header(x, "and", 3), cons(x, T_PRIMITIVE, (C)&_and, 0)));
//	reveal(x, body(x, header(x, "or", 2), cons(x, T_PRIMITIVE, (C)&_or, 0)));
//	reveal(x, body(x, header(x, "invert", 3), cons(x, T_PRIMITIVE, (C)&_not, 0)));
//
//	reveal(x, body(x, header(x, "dup", 3), cons(x, T_PRIMITIVE, (C)&_dup, 0)));
//	reveal(x, body(x, header(x, "swap", 4), cons(x, T_PRIMITIVE, (C)&_swap, 0)));
//
//	//reveal(x, body(x, header(x, "key", 3), cons(x, T_PRIMITIVE, (C)&_key, 0)));
//	//reveal(x, body(x, header(x, "emit", 4), cons(x, T_PRIMITIVE, (C)&_emit, 0)));
//
//	return x;
//}
//
//// ----------------------------------------------------------------------------
//
//
