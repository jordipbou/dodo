#include<stdint.h>
#include<stdio.h>

typedef int8_t		B;
typedef intptr_t	C;		// 16, 32 or 64 bits depending on system

#define A(p)								(*((C*)p))
#define D(p)								(*(((C*)p) + 1))
#define D_(p)								(D(p) & -4)	
#define _D(p)								(D(p) & 3)
#define T(t, c)							((c & -4) | t)
#define R(p, a)							(D(p) = T(_D(p), a))

#define ATM									0
#define LST									1
#define PRM									2
#define BRN									3

#define IS(t, p)						((_D(p) & 3) == t)
#define ARE(x, t1, t2)			IS(t1, x->s) && IS(t2, D_(x->s))

#define REF(p)							(_D(p) & 1)

C lth(C p) { C c = 0; while (p) { c++; p = D_(p); } return c; }
C dth(C p) { C c = 0; while (p) { c += REF(p) ? dth(A(p)) + 1 : 1; p = D_(p); } return c; }
C mlth(C p, C n) { C c = 0; while (p && c < n) { c++; p = D_(p); } return c == n; }
C lst(C p) { if (!p) return 0; while (D_(p)) { p = D_(p); } return p; }

typedef struct {
	B* here;
	C there, size, f, s, rstack, dict, err;
} X;

#define ALIGN(addr, bound)	((((C)addr) + (bound - 1)) & ~(bound - 1))

#define BOTTOM(x)						(((B*)x) + sizeof(X))
#define TOP(x)							(ALIGN(((B*)x) + x->size - 2*sizeof(C) - 1, 2*sizeof(C)))

typedef void (*FUNC)(X*);

#define ERR_NOT_ENOUGH_MEMORY		-1
#define ERR_OVERFLOW						-2
#define ERR_UNDERFLOW						-3

X* init(B* block, C size) {
	X* x = (X*)block;	
	x->size = size;
	x->here = BOTTOM(x);
	x->there = ALIGN(BOTTOM(x), 2*sizeof(C));
	x->f = TOP(x);

	for (C p = x->there; p <= x->f; p += 2*sizeof(C)) {
		A(p) = p == x->f ? 0 : p + 2*sizeof(C);
		D(p) = p == x->there ? 0 : p - 2*sizeof(C);
	}

	x->err = x->dict = x->s = x->rstack = 0;

	return x;
}

C cns(X* x, C a, C d) { C p; return x->f ? (p = x->f, x->f = D(x->f), A(p) = a, D(p) = d, p) : 0; }
C rcl(X* x, C l) { C r; return l ? (r = D_(l), D(l) = x->f, A(l) = 0, x->f = l, r) : 0; }
C cln(X* x, C l) { return l ? cns(x, REF(l) ? cln(x, A(l)) : A(l), T(_D(l), cln(x, D_(l)))) : 0; }

void push(X* x, C t, C v) { C p = cns(x, v, T(t, x->s)); if (p) x->s = p; }
C pop(X* x) { C v = A(x->s); x->s = rcl(x, x->s); return v; }

#define O(x)				if (x->f == 0) { x->err = ERR_OVERFLOW; return; }
#define On(x, l)		if (!mlth(x->f, dth(l))) { x->err = ERR_OVERFLOW; return; }
#define U(x)				if (x->s == 0) { x->err = ERR_UNDERFLOW; return; }
#define U2(x)				if (x->s == 0 || D_(x->s) == 0) { x->err = ERR_UNDERFLOW; return; }
#define OU2(x)			O(x); U2(x);

#define W(n)				void n(X* x)

W(_empty) { O(x); push(x, LST, 0); }

W(jAA) { OU2(x); C l = x->s; x->s = D_(D_(x->s)); R(D_(l), 0); push(x, LST, l); }
W(jAL) { OU2(x); C l = D_(x->s); D(x->s) = T(ATM, A(D_(x->s))); A(l) = x->s; x->s = l; }
W(jLA) { OU2(x); C t = D_(D_(x->s)); C l = lst(A(x->s)); R(l, D_(x->s)); R(D_(l), 0); R(x->s, t); }
W(jLL) { OU2(x); C t = D_(x->s); C l = lst(A(x->s)); R(l, A(D_(x->s))); R(x->s, rcl(x, t)); }
W(_join) { REF(D_(x->s)) ? (REF(x->s) ? jLL(x) : jAL(x)) : (REF(x->s) ? jLA(x) : jAA(x)); }
W(_quote) { U(x); C t = x->s; x->s = D_(x->s); R(t, 0); push(x, LST, t); }

W(dA) { U(x); O(x); push(x, _D(x->s), A(x->s)); }
W(dL) { U(x); On(x, A(x->s)); push(x, _D(x->s), cln(x, A(x->s))); }
W(_dup) { REF(x->s) ? dL(x) : dA(x); }
W(_swap) { U2(x); C t = D_(x->s); R(x->s, D_(D_(x->s))); R(t, x->s); x->s = t; }

W(_add) { U2(x); A(D_(x->s)) = A(D_(x->s)) + A(x->s); pop(x); }
W(_sub) { U2(x); A(D_(x->s)) = A(D_(x->s)) - A(x->s); pop(x); }
W(_mul) { U2(x); A(D_(x->s)) = A(D_(x->s)) * A(x->s); pop(x); }
W(_div) { U2(x); A(D_(x->s)) = A(D_(x->s)) / A(x->s); pop(x); }
W(_mod) { U2(x); A(D_(x->s)) = A(D_(x->s)) % A(x->s); pop(x); }

W(_gt) { U2(x); A(D_(x->s)) = A(D_(x->s)) > A(x->s); pop(x); }
W(_lt) { U2(x); A(D_(x->s)) = A(D_(x->s)) < A(x->s); pop(x); }
W(_eq) { U2(x); A(D_(x->s)) = A(D_(x->s)) == A(x->s); pop(x); }
W(_neq) { U2(x); A(D_(x->s)) = A(D_(x->s)) != A(x->s); pop(x); }



//#define OUx->f			Ox->f; U(x,);
//
//void push(X* x, C t, C v) { Ox->f; N(x) = x->f; D(x->s) = T(t, D_(x->s)); A(x->s) = v; }
//C pop(X* x) { U(x, 0); C v = A(x->s); A(x->s) = N(x); N(x) = x->s; return v; }
//
//void _quote(X* x) { OUx->f; C t = x->s; x->s = D_(x->s); D(t) = T(_D(t), 0); push(x, LST, t); }
//
//void _drop(X* x) { 
//	U(x,); 
//	if (IS(ATM, x->s)) {
//		A(x->s) = N(x); N(x) = x->s;
//	} else if (IS(LST, x->s)) {
//		C t = x->s;
//		x->s = A(x->s);
//		while (x->s) { printf("drop\n"); _drop(x); }
//		x->s = t;
//		D(x->s) = D_(x->s); A(x->s) = N(x); N(x) = x->s;
//
//		//C l = lth(A(x->s));
//		//if (l == 0) {
//		//	A(x->s) = N(x); N(x) = x->s;
//		//} else {
//		//	R(lst(A(x->s)), D_(x->s));
//		//	for (C i = 0; i < l; i++) { _drop(x); }
//		//	_drop(x);
//		//}
//	}
//}
//void _join(X* x) {
//	U2(x);
//	if (ARE(x, ATM, ATM)) { _quote(x); _swap(x); _quote(x); _swap(x); _join(x); return; }
//	if (ARE(x, ATM, LST)) { _quote(x); _join(x); return; }
//	if (ARE(x, LST, ATM)) { _swap(x); _quote(x); _swap(x); _join(x); return; }
//	if (ARE(x, LST, LST)) { //C t = lst(A(x->s)); R(t, A(D_(x->s))); A(D_(x->s)) = x->s; _drop(x); }
//		printf("Depth %ld\n", lth(x->s));
//		if (A(x->s)) {
//			C t = lst(A(x->s));
//			R(t, A(D_(x->s)));
//		} else {
//			A(x->s) = A(D_(x->s));
//		}
//		printf("Depth %ld\n", lth(x->s));
//		A(D_(x->s)) = A(x->s);
//		printf("Depth %ld\n", lth(x->s));
//		_drop(x);
//		printf("Depth %ld\n", lth(x->s));
//	}
//	////ARE(x, ATM, ATM) ? (t = x->s, x->s = D_(D_(x->s)), D(D_(t)) = T(ATM, 0), push(x, LST, t)) :
//	//ARE(x, ATM, ATM) ? _quote(x), _swap(x), _quote(x), _swap(x), _join(x) :
//	////ARE(x, ATM, LST) ? (t = D_(x->s), D(x->s) = T(ATM, A(D_(x->s))), A(t) = x->s, x->s = t) :
//	//ARE(x, ATM, LST) ? _quote(x), _join(x) :
//	//ARE(x, LST, LST) ? 
//	//0 ;
//}
////C pop(X* x) { Ux->f; C t = T(x); T(x) = N(x); N(x) = K(x); return t; }
////C cns(X* x, C a, C d) { C p = N(x); push(x, a); K(x) = K(x) ? D(K(x)) : 0; D(p) = d; return p; }
////
////void _sclear(X* x) { while (K(x)) pop(x); }
////void _spush(X* x) { C t = P(x);	C n = x->f;	P(x) = N(x); A(N(x)) = t;	N(x) = n;	K(x) = 0; }
////void _sdrop(X* x) { 
////	_sclear(x); 
////	if (P(x) != R(x)) { C t = A(P(x)); A(P(x)) = N(x); K(x) = D(P(x)); N(x) = P(x); P(x) = t;	} 
////}
////
////void _dup(X* x) { push(x, T(x)); }
////void _swap(X* x) { C t = T(x); T(x) = x->s; x->s = t; }
////void _over(X* x) { push(x, x->s); }
////void _rot(X* x) { C t = A(D(D(K(x)))); A(D(D(K(x)))) = x->s; x->s = T(x); T(x) = t; }
////void _drop(X* x) { pop(x); }
////void _rev(X* x) {	C s = K(x);	K(x) = 0;	while (s) { C t = D(s); D(s) = K(x); K(x) = s; s = t; } }
////
////void _add(X* x) { C t = pop(x); T(x) += t; }
////void _sub(X* x) { C t = pop(x); T(x) -= t; }
////void _mul(X* x) { C t = pop(x); T(x) *= t; }
////void _div(X* x) { C t = pop(x); T(x) /= t; }
////void _mod(X* x) { C t = pop(x); T(x) %= t; }
////
////void _gt(X* x) { C t = pop(x); T(x) = T(x) > t; }
////void _lt(X* x) { C t = pop(x); T(x) = T(x) < t; }
////void _eq(X* x) { C t = pop(x); T(x) = T(x) == t; }
////void _neq(X* x) { C t = pop(x); T(x) = T(x) != t; }
////
////void _and(X* x) { C t = pop(x); T(x) = T(x) && t; }
////void _or(X* x) { C t = pop(x); T(x) = T(x) || t; }
////void _not(X* x) { T(x) = !T(x); }
////
////#define ATM(x, n, d)					cns(x, cns(x, T_ATM, n), d)
////#define PRM(x, p, d)		cns(x, cns(x, T_PRM, (C)p), d)
////#define RECURSION(x, d)				cns(x, cns(x, T_RECURSION, 0), d)
////C BRN(X* x, C t, C f, C d) {
////	if (t) { C lt = t; while(D(lt)) lt = D(lt); D(lt) = d; } else { t = d; }
////	if (f) { C lf = f; while(D(lf)) lf = D(lf); D(lf) = d; } else { f = d; }
////	return cns(x, cns(x, T_BRN, cns(x, t, cns(x, f, 0))), d);
////}
////#define WORD(x, c, d)					cns(x, cns(x, T_WORD, c), d)
////
////void inner(X* x, C xlist) {
////	C ip = xlist;
////	while(x->err == 0 && ip != 0) {
////		switch(A(A(ip))) {
////			case T_ATM: push(x, D(A(ip))); ip = D(ip); break;
////			case T_PRM:	((FUNC)D(A(ip)))(x); ip = D(ip); break;
////			case T_RECURSION: ip = D(ip) ? (inner(x, xlist), D(ip)) : xlist; break;
////			case T_BRN: ip = pop(x) ? A(D(A(ip))) : A(D(D(A(ip)))); break;
////			case T_WORD: ip = D(ip) ? (inner(x, D(A(ip))), D(ip)) : D(A(ip)); break;
////		}
////	}
////}
////
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
////void _key(X* x) { push(x, dodo_getch()); }
////void _emit(X* x) { C K = T(x); pop(x); K == 127 ? printf ("\b \b") : printf ("%c", (char)K); }
////
////void _stack_to_ibuf(X* x) {
////	C s = K(x);
////	K(x) = x->ibuf;
////	_sdrop(x);
////}
////
////void _quit(X* x) {
////	//while (!x->err) {
////		while (K(x) ? T(x) != 10 : 1) { _key(x); _dup(x); _emit(x); }
////		_rev(x);
////		_stack_to_ibuf(x);
////	//}
////}
////
////void _header(X* x) {
////	// Parses next word from input buffer
////	//
////}
//////
//////void push_stack(X* x) {
//////	x->stacks.next = cns(x, T_LST, (C)AS(T_LST, REF((&x->stacks))), NEXT((&x->stacks)));
//////	x->stacks.stack = 0;
//////}
//////
//////void stack_to_list(X* x) {
//////	PAIR* list = REF((&x->stacks));
//////	x->stacks = *(NEXT((&x->stacks)));
//////	x->stacks.ref = (C)cns(x, T_LST, (C)list, REF((&x->stacks)));
//////}
//////
//////#define RESERVED(x)				((x->there) - ((C)x->here))
//////
//////B* allot(X* x, C bytes) {
//////	B* here = x->here;
//////	if (bytes == 0) { 
//////		return here;
//////	} else if (bytes < 0) {
//////		if ((x->here + bytes) > BOTTOM(x)) x->here += bytes;
//////		else x->here = BOTTOM(x);
//////		while ((x->there - 1) >= PALIGN(x->here)) { rcl(x, --x->there); }
//////	} else /* bytes > 0 */ {
//////		while (RESERVED(x) < bytes && x->there < x->top) {
//////			if (IS(T_F, x->there)) {
//////				if (x->there->prev != 0) {
//////					x->there->prev->next = AS(T_F, NEXT(x->there));
//////				}
//////				x->there++;
//////			} else {
//////				x->err = ERR_NOT_ENOUGH_MEMORY;
//////				return here;
//////			}
//////		}
//////		if (RESERVED(x) >= bytes)	x->here += bytes;
//////		else x->err = ERR_NOT_ENOUGH_MEMORY;
//////	}
//////	return here;
//////}
//////
//////void align(X* x) { allot(x, ALIGN(x->here, sizeof(C)) - ((C)x->here)); }
//////
//////#define NFA(w)		(REF(REF(w)))
//////#define DFA(w)		(REF(NEXT(REF(w))))
//////#define CFA(w)		(NEXT(NEXT(REF(w))))
//////#define COUNT(s)	(*((C*)(((B*)s) - sizeof(C))))
//////
//////B* compile_str(X* x, B* str, C len) {
//////	align(x);
//////	B* cstr = allot(x, sizeof(C) + len + 1);
//////	*((C*)cstr) = len;
//////	for (C i = 0; i < len; i++) {
//////		cstr[sizeof(C) + i] = str[i];
//////	}
//////	cstr[sizeof(C) + len] = 0;
//////	return cstr + sizeof(C);
//////}
//////
//////PAIR* header(X* x, B* name, C nlen) {
//////	B* str = compile_str(x, name, nlen);
//////
//////	return 
//////		cns(x, T_ATM, 
//////			(C)cns(x, T_ATM, (C)str,
//////						cns(x, T_ATM, (C)x->here, 0)), 
//////			0);
//////}
//////
//////PAIR* body(X* x, PAIR* word, PAIR* cfa) {
//////	PAIR* old_cfa = CFA(word);
//////	NEXT(REF(word))->next = AS(T_ATM, cfa);
//////	while (old_cfa != 0) { old_cfa = rcl(x, old_cfa); }
//////	return word;
//////}
//////
//////PAIR* reveal(X* x, PAIR* header) {
//////	header->next = AS(T_ATM, x->dict);
//////	x->dict = header;
//////	return header;
//////}
//////
//////#define IS_IMMEDIATE(w)		(TYPE(w->ref) & 1)
//////
//////void _immediate(X* x) {
//////	x->dict->ref = AS(1, REF(x->dict));
//////}
//////
////////PAIR* find(X* x, B* name, C nlen) {
////////	// TODO
////////}
//////
//////X* dodo(X* x) {
//////	reveal(x, body(x, header(x, "+", 1), cns(x, T_PRM, (C)&_add, 0)));
//////	reveal(x, body(x, header(x, "-", 1), cns(x, T_PRM, (C)&_sub, 0)));
//////	reveal(x, body(x, header(x, "*", 1), cns(x, T_PRM, (C)&_mul, 0)));
//////	reveal(x, body(x, header(x, "/", 1), cns(x, T_PRM, (C)&_div, 0)));
//////	reveal(x, body(x, header(x, "mod", 3), cns(x, T_PRM, (C)&_mod, 0)));
//////
//////	reveal(x, body(x, header(x, ">", 1), cns(x, T_PRM, (C)&_gt, 0)));
//////	reveal(x, body(x, header(x, "<", 1), cns(x, T_PRM, (C)&_lt, 0)));
//////	reveal(x, body(x, header(x, "=", 1), cns(x, T_PRM, (C)&_eq, 0)));
//////	reveal(x, body(x, header(x, "<>", 2), cns(x, T_PRM, (C)&_neq, 0)));
//////
//////	reveal(x, body(x, header(x, "and", 3), cns(x, T_PRM, (C)&_and, 0)));
//////	reveal(x, body(x, header(x, "or", 2), cns(x, T_PRM, (C)&_or, 0)));
//////	reveal(x, body(x, header(x, "invert", 3), cns(x, T_PRM, (C)&_not, 0)));
//////
//////	reveal(x, body(x, header(x, "dup", 3), cns(x, T_PRM, (C)&_dup, 0)));
//////	reveal(x, body(x, header(x, "swap", 4), cns(x, T_PRM, (C)&_swap, 0)));
//////
//////	//reveal(x, body(x, header(x, "key", 3), cns(x, T_PRM, (C)&_key, 0)));
//////	//reveal(x, body(x, header(x, "emit", 4), cns(x, T_PRM, (C)&_emit, 0)));
//////
//////	return x;
//////}
//////
//////// ----------------------------------------------------------------------------
//////
//////
