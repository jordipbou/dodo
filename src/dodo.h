#include<stdint.h>

typedef int8_t		B;
typedef intptr_t	C;		// 16, 32 or 64 bits depending on system

#define A(p)					(*((C*)p))				// cAr
#define A_(p)					(A(p) & -4)				// value part of cAr
#define _A(p)					(A(p) & 3)				// type tag part of cAr
#define D(p)					(*((C*)p + 1))		// cDr
#define D_(p)					(D(p) & -4)				// value part of cDr
#define _D(p)					(D(p) & 3)				// type tag part of cDr
#define T(t, v)				((v & -4) | t)		// Tag a value with a type

#define T_STACK				0
#define T_ATOM				1
#define T_LIST				2
#define T_PRIMITIVE		3

#define IS_ATOM(p)		(_D(p) == T_ATOM)
#define IS_LIST(p)		(_D(p) == T_LIST)

typedef struct {
	B* here;
	C there, size, horizon, rstack, dict, err;
} X;

#define BOTTOM(x)			(((B*)x) + sizeof(X))
#define TOP(x)				(ALIGN(((B*)x) + x->size - 2*sizeof(C) - 1, 2*sizeof(C)))

#define F(x)					A(Z(x))
#define Z(x)					x->horizon
#define S(x)					D(Z(x))

typedef void (*FUNC)(X*);

#define ALIGN(addr, bound)	((((C)addr) + (bound - 1)) & ~(bound - 1))

#define ERR_NOT_ENOUGH_MEMORY		-1
#define ERR_OVERFLOW						-2
#define ERR_UNDERFLOW						-3
#define ERR_NOT_A_LIST					-4
#define ERR_EMPTY_LIST					-5

C height(C p) { C c = 0; while (p != 0) { c++; p = A_(p); } return c; }
C depth(C p) { C c = 0; while (p != 0) { c++; p = D_(p); } return c; }

X* init(B* block, C size) {
	X* x = (X*)block;	
	x->size = size;
	x->here = BOTTOM(x);
	x->there = ALIGN(BOTTOM(x), 2*sizeof(C));
	x->horizon = TOP(x);

	for (C p = x->there; p <= x->horizon; p += 2*sizeof(C)) {
		A(p) = p == x->there ? 0 : p - 2*sizeof(C);
		D(p) = p == x->horizon ? 0 : p + 2*sizeof(C);
	}

	x->err = x->dict = x->rstack = 0;

	return x;
}

#define OF(x)				if (F(x) == 0) { x->err = ERR_OVERFLOW; return; }
#define UF(x, r)		if (S(x) == 0) { x->err = ERR_UNDERFLOW; return r; }
#define UF2(x)			if (S(x) == 0 || D_(S(x)) == 0) { x->err = ERR_UNDERFLOW; return; }

void push(X* x, C t, C v) { OF(x); Z(x) = F(x);	A(S(x)) = v; D(S(x)) = T(t, D_(S(x))); }
void lit(X* x, C v) { push(x, T_ATOM, v); }
C pop(X* x) { UF(x, 0); C v = A(S(x)); A(S(x)) = Z(x); D(S(x)) = D_(S(x)); Z(x) = S(x); return v; }

void _cons(X* x) { 
	UF2(x); 
	if (IS_ATOM(D_(S(x)))) { 
		C t = S(x); 
		S(x) = D_(D_(S(x))); 
		D(D_(t)) = T(T_ATOM, 0); 
		push(x, T_LIST, t);
	} else if (IS_LIST(D_(S(x)))) {
		C t = S(x);
		S(x) = D_(S(x));
		D(t) = T(_D(t), A(S(x)));
		A(S(x)) = t;
	}
}

void _carcdr(X* x) {
	UF(x,);
	if (!IS_LIST(S(x))) { x->err = ERR_NOT_A_LIST; return; }
	if (A(S(x)) == 0) { x->err = ERR_EMPTY_LIST; return; }
	C head = A(S(x));
	A(S(x)) = D_(A(S(x)));
	D(head) = T(_D(head), S(x));
	S(x) = head;
}

void _drop(X* x) { 
	UF(x,);
	if (IS_ATOM(S(x))) { pop(x); }
	else if (IS_LIST(S(x)) && A(S(x)) != 0) {
		while (IS_LIST(S(x)) && A(S(x)) != 0) {
			_carcdr(x);
			_drop(x);
		}
		_drop(x);
	} else if (IS_LIST(S(x)) && A(S(x)) == 0) { pop(x); }
}

//
////C cons(X* x, C a, C d) { C p = Z(x); push(x, a); K(x) = K(x) ? D(K(x)) : 0; D(p) = d; return p; }
////void push(X* x, C v) { OF(x); Z(x) = A(Z(x)); D(D(Z(x))) = T(T_ATOM, D_(D(Z(x)))); A(D(Z(x))) = v; }
////C pop(X* x) { UF(x); C v = A(D(Z(x))); A(D(Z(x))) = Z(x); Z(x) = D(Z(x)); return v; }
//#define S(x)	D(Z(x))
//#define F(x)	(A(Z(x)))
//C cons(X* x, C a, C d) { 
//	OF(x, 0); 
//	C t = Z(x); 
//	D(F(x)) = S(x);
//	Z(x) = F(x);
//	A(t) = a; D(t) = d; 
//	return t; 
//}
//
//void push(X* x, C v) { C t = cons(x, v, T(T_ATOM, S(x)));	S(x) = t; }
//C pop(X* x) { UF(x); C v = A(S(x)); A(S(x)) = Z(x); D(S(x)) = D_(S(x)); Z(x) = S(x); return v; }
//
//void _dup(X* x) { 
//	if (_D(D(Z(x))) == T_ATOM) { D(Z(x)) = cons(x, A(D(Z(x))), T(T_ATOM, D(Z(x)))); } else { } }
//void _swap(X* x) { C t = A(D(Z(x))); A(D(Z(x))) = A(_D(D(Z(x)))); A(_D(D(Z(x)))) = t; }
//void _over(X* x) { push(x, A(_D(D(Z(x))))); }
//void _rot(X* x) { C t = A(D(D(D(Z(x))))); A(D(D(D(Z(x))))) = A(_D(D(Z(x)))); A(_D(D(Z(x)))) = A(D(Z(x))); A(D(Z(x))) = t; }
//void _drop(X* x) { pop(x); }
//
//void _add(X* x) { C t = pop(x); A(D(Z(x))) += t; }
//void _sub(X* x) { C t = pop(x); A(D(Z(x))) -= t; }
//void _mul(X* x) { C t = pop(x); A(D(Z(x))) *= t; }
//void _div(X* x) { C t = pop(x); A(D(Z(x))) /= t; }
//void _mod(X* x) { C t = pop(x); A(D(Z(x))) %= t; }
//
//void _gt(X* x) { C t = pop(x); A(D(Z(x))) = A(D(Z(x))) > t; }
//void _lt(X* x) { C t = pop(x); A(D(Z(x))) = A(D(Z(x))) < t; }
//void _eq(X* x) { C t = pop(x); A(D(Z(x))) = A(D(Z(x))) == t; }
//void _neq(X* x) { C t = pop(x); A(D(Z(x))) = A(D(Z(x))) != t; }
//
//void _and(X* x) { C t = pop(x); A(D(Z(x))) = A(D(Z(x))) && t; }
//void _or(X* x) { C t = pop(x); A(D(Z(x))) = A(D(Z(x))) || t; }
//void _not(X* x) { A(D(Z(x))) = !A(D(Z(x))); }
//
//#define ATOM(x, n, d)				cons(x, n, T(T_ATOM, d))
//#define PRIMITIVE(x, p, d)	cons(x, (C)p, T(T_PRIMITIVE, d))
//#define RECURSION(x, d)			cons(x, 0, T(T_PRIMITIVE, d))
//C BRANCH(X* x, C t, C f, C d) {
//	if (t) { C lt = t; while(D_(lt)) lt = D_(lt); D(lt) = T(_D(lt), d); } else { t = d; }
//	if (f) { C lf = f; while(D_(lf)) lf = D_(lf); D(lf) = T(_D(lf), d); } else { f = d; }
//	return cons(x, T(T_ATOM, cons(x, t, f)), T(T_LIST, d));
//}
//#define WORD(x, xt, cdr)			cons(x, T(T_LIST, xt), T(T_LIST, cdr))
//
//void inner(X* x, C xlist) {
//	C ip = xlist;
//	while(x->err == 0 && ip != 0) {
//		switch(_D(ip)) {
//			case T_ATOM: push(x, A(ip)); ip = D_(ip); break;
//			case T_PRIMITIVE:	
///* RECURSION */	if (A(ip) == 0) { ip = D_(ip) ? (inner(x, xlist), D_(ip)) : xlist; }
///* PRIMITIVE */	else { ((FUNC)A(ip))(x); ip = D_(ip); }
//				break;
//			case T_LIST:
//				switch(_A(ip)) {
///* BRANCH */	case T_ATOM: ip = pop(x) ? A(A_(ip)) : D(A_(ip)); break;
///* WORD   */	case T_LIST: ip = D_(ip) ? (inner(x, A_(ip)), D_(ip)) : A_(ip); break;
//				}
//		}
//	}
//}
//
//////#ifdef _WIN32
//////  #include <conio.h>
//////#else
//////	#include <unistd.h>
//////	#include <termios.h>
//////#endif
////
//////// Source code for getch is taken from:
//////// Crossline readline (https://github.com/jcwangxp/Crossline).
//////// It's a fantastic readline cross-platform replacement, but only getch was
//////// needed and there's no need to include everything else.
//////#ifdef _WIN32	// Windows
//////int dodo_getch (void) {	fflush (stdout); return _getch(); }
//////#else
//////int dodo_getch ()
//////{
//////	char ch = 0;
//////	struct termios old_term, cur_term;
//////	fflush (stdout);
//////	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
//////	cur_term = old_term;
//////	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
//////	cur_term.c_cc[VMIN] = 1;
//////	cur_term.c_cc[VTIME] = 0;
//////	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
//////	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
//////	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
//////	return ch;
//////}
//////#endif
//////
//////void _key(X* x) { PUSH(x, dodo_getch()); }
//////void _emit(X* x) { 
//////	C K = A(D(Z(x))); 
//////	POP(x); 
//////	if (K == 127) { printf ("\b \b"); } 
//////	else { printf ("%c", (char)K); }
//////}
////
////void push_stack(X* x) {
////	x->stacks.next = cons(x, T_LIST, (C)AS(T_LIST, REF((&x->stacks))), NEXT((&x->stacks)));
////	x->stacks.stack = 0;
////}
////
////void stack_to_list(X* x) {
////	PAIR* list = REF((&x->stacks));
////	x->stacks = *(NEXT((&x->stacks)));
////	x->stacks.ref = (C)cons(x, T_LIST, (C)list, REF((&x->stacks)));
////}
////
////#define RESERVED(x)				((x->there) - ((C)x->here))
////
////B* allot(X* x, C bytes) {
////	B* here = x->here;
////	if (bytes == 0) { 
////		return here;
////	} else if (bytes < 0) {
////		if ((x->here + bytes) > BOTTOM(x)) x->here += bytes;
////		else x->here = BOTTOM(x);
////		while ((x->there - 1) >= PALIGN(x->here)) { reclaim(x, --x->there); }
////	} else /* bytes > 0 */ {
////		while (RESERVED(x) < bytes && x->there < x->top) {
////			if (IS(T_F, x->there)) {
////				if (x->there->prev != 0) {
////					x->there->prev->next = AS(T_F, NEXT(x->there));
////				}
////				x->there++;
////			} else {
////				x->err = ERR_NOT_ENOUGH_MEMORY;
////				return here;
////			}
////		}
////		if (RESERVED(x) >= bytes)	x->here += bytes;
////		else x->err = ERR_NOT_ENOUGH_MEMORY;
////	}
////	return here;
////}
////
////void align(X* x) { allot(x, ALIGN(x->here, sizeof(C)) - ((C)x->here)); }
////
////#define NFA(w)		(REF(REF(w)))
////#define DFA(w)		(REF(NEXT(REF(w))))
////#define CFA(w)		(NEXT(NEXT(REF(w))))
////#define COUNT(s)	(*((C*)(((B*)s) - sizeof(C))))
////
////B* compile_str(X* x, B* str, C len) {
////	align(x);
////	B* cstr = allot(x, sizeof(C) + len + 1);
////	*((C*)cstr) = len;
////	for (C i = 0; i < len; i++) {
////		cstr[sizeof(C) + i] = str[i];
////	}
////	cstr[sizeof(C) + len] = 0;
////	return cstr + sizeof(C);
////}
////
////PAIR* header(X* x, B* name, C nlen) {
////	B* str = compile_str(x, name, nlen);
////
////	return 
////		cons(x, T_ATOM, 
////			(C)cons(x, T_ATOM, (C)str,
////						cons(x, T_ATOM, (C)x->here, 0)), 
////			0);
////}
////
////PAIR* body(X* x, PAIR* word, PAIR* cfa) {
////	PAIR* old_cfa = CFA(word);
////	NEXT(REF(word))->next = AS(T_ATOM, cfa);
////	while (old_cfa != 0) { old_cfa = reclaim(x, old_cfa); }
////	return word;
////}
////
////PAIR* reveal(X* x, PAIR* header) {
////	header->next = AS(T_ATOM, x->dict);
////	x->dict = header;
////	return header;
////}
////
////#define IS_IMMEDIATE(w)		(TYPE(w->ref) & 1)
////
////void _immediate(X* x) {
////	x->dict->ref = AS(1, REF(x->dict));
////}
////
//////PAIR* find(X* x, B* name, C nlen) {
//////	// TODO
//////}
////
////X* dodo(X* x) {
////	reveal(x, body(x, header(x, "+", 1), cons(x, T_PRIMITIVE, (C)&_add, 0)));
////	reveal(x, body(x, header(x, "-", 1), cons(x, T_PRIMITIVE, (C)&_sub, 0)));
////	reveal(x, body(x, header(x, "*", 1), cons(x, T_PRIMITIVE, (C)&_mul, 0)));
////	reveal(x, body(x, header(x, "/", 1), cons(x, T_PRIMITIVE, (C)&_div, 0)));
////	reveal(x, body(x, header(x, "mod", 3), cons(x, T_PRIMITIVE, (C)&_mod, 0)));
////
////	reveal(x, body(x, header(x, ">", 1), cons(x, T_PRIMITIVE, (C)&_gt, 0)));
////	reveal(x, body(x, header(x, "<", 1), cons(x, T_PRIMITIVE, (C)&_lt, 0)));
////	reveal(x, body(x, header(x, "=", 1), cons(x, T_PRIMITIVE, (C)&_eq, 0)));
////	reveal(x, body(x, header(x, "<>", 2), cons(x, T_PRIMITIVE, (C)&_neq, 0)));
////
////	reveal(x, body(x, header(x, "and", 3), cons(x, T_PRIMITIVE, (C)&_and, 0)));
////	reveal(x, body(x, header(x, "or", 2), cons(x, T_PRIMITIVE, (C)&_or, 0)));
////	reveal(x, body(x, header(x, "invert", 3), cons(x, T_PRIMITIVE, (C)&_not, 0)));
////
////	reveal(x, body(x, header(x, "dup", 3), cons(x, T_PRIMITIVE, (C)&_dup, 0)));
////	reveal(x, body(x, header(x, "swap", 4), cons(x, T_PRIMITIVE, (C)&_swap, 0)));
////
////	//reveal(x, body(x, header(x, "key", 3), cons(x, T_PRIMITIVE, (C)&_key, 0)));
////	//reveal(x, body(x, header(x, "emit", 4), cons(x, T_PRIMITIVE, (C)&_emit, 0)));
////
////	return x;
////}
////
////// ----------------------------------------------------------------------------
////
////
