#include<inttypes.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

typedef int8_t		B;
typedef intptr_t	C;		// 16, 32 or 64 bits depending on system

#define sC									sizeof(C)
#define sP									(2*sC)

#define A(p)								(*((C*)p))
#define D(p)								(*(((C*)p) + 1))
#define D_(p)								(D(p) & -4)	
#define _D(p)								(D(p) & 3)
#define T(t, c)							((c & -4) | t)
#define R(p, a)							(D(p) = T(_D(p), a))

#define ATM									0
#define LST									1
#define PRM									2
#define JMP									3

#define IS(t, p)						((_D(p) & 3) == t)
#define ARE(x, t1, t2)			IS(t1, x->s) && IS(t2, D_(x->s))

#define REF(p)							(_D(p) & 1)

C lth(C p) { C c = 0; while (p) { c++; p = D_(p); } return c; }
C dth(C p) { C c = 0; while (p) { c += REF(p) ? dth(A(p)) + 1 : 1; p = D_(p); } return c; }
C mlth(C p, C n) { C c = 0; while (p && c < n) { c++; p = D_(p); } return c == n; }
C lst(C p) { if (!p) return 0; while (D_(p)) { p = D_(p); } return p; }
C rev(C p, C l) { C t; return p ? (t = D_(p), R(p, l), rev(t, p)) : l; }

typedef struct {
		B* here, * ibuf;
		C there, size, f, s, r, cf, cp, d, err, st, in;
	} X;

#define ALIGN(addr, bound)	((((C)addr) + (bound - 1)) & ~(bound - 1))
#define RESERVED(x)					((x->there) - ((C)x->here))

#define BOTTOM(x)						(((B*)x) + sizeof(X))
#define TOP(x)							(ALIGN(((B*)x) + x->size - sP - 1, sP))

typedef void (*FUNC)(X*);

#define ERR_NOT_ENOUGH_MEMORY		-1
#define ERR_OVERFLOW						-2
#define ERR_UNDERFLOW						-3
#define ERR_BYE									-5

#define ST_INTERPRETING					0
#define ST_COMPILING						1

X* init(B* block, C size) {
		if (size < sC + 2*sP) return 0;
		X* x = (X*)block;	
		x->size = size;
		x->here = BOTTOM(x);
		x->there = ALIGN(BOTTOM(x), 2*sC);
		x->f = TOP(x);
	
		for (C p = x->there; p <= x->f; p += 2*sC) {
			A(p) = p == x->f ? 0 : p + 2*sC;
			D(p) = p == x->there ? 0 : p - 2*sC;
		}
	
		x->err = x->d = x->s = x->r = x->cf = x->cp = x->st = x->in = 0;
		x->ibuf = 0;
	
		return x;
	}

C cns(X* x, C a, C d) { C p; return x->f ? (p = x->f, x->f = D(x->f), A(p) = a, D(p) = d, p) : 0; }
C rcl(X* x, C l) { C r; return l ? (r = D_(l), D(l) = x->f, A(l) = 0, x->f = l, r) : 0; }
C cln(X* x, C l) { return l ? cns(x, REF(l) ? cln(x, A(l)) : A(l), T(_D(l), cln(x, D_(l)))) : 0; }
void rmv(X* x, C l) { while (l) { if (REF(l)) {	rmv(x, A(l)); } l = rcl(x, l); } }

void push(X* x, C t, C v) { C p = cns(x, v, T(t, x->s)); if (p) x->s = p; }
C pop(X* x) { C v = A(x->s); x->s = rcl(x, x->s); return v; }

void cppush(X* x) { C s = cns(x, 0, T(LST, x->cp)); if (s) x->cp = s; }
void cspush(X* x, C p) { R(p, A(x->cp)); A(x->cp) = p; }
void cppop(X* x) {
		if (D_(x->cp)) { C t = A(D_(x->cp));	A(D_(x->cp)) = x->cp;	x->cp = D_(x->cp); R(A(x->cp), t);
		} else { R(x->cp, x->s); x->s = x->cp; x->cp = 0;	}
	}

#define O(x)				if (!x->f) { x->err = ERR_OVERFLOW; return; }
#define On(x, l)		if (!mlth(x->f, dth(l))) { x->err = ERR_OVERFLOW; return; }
#define U(x)				if (!x->s) { x->err = ERR_UNDERFLOW; return; }
#define U2(x)				if (!x->s || !D_(x->s)) { x->err = ERR_UNDERFLOW; return; }
#define U3(x)				if (!x->s || !D_(x->s) || !D_(D_(x->s))) { x->err = ERR_UNDERFLOW; return; }
#define OU2(x)			O(x); U2(x);

#define W(n)				void n(X* x)

W(_lbrace) { x->st = ST_COMPILING; cppush(x); }
W(_rbrace) { cppop(x); x->st = x->cp != 0; }

W(_empty) { O(x); push(x, LST, 0); }

W(jAA) { OU2(x); C l = x->s; x->s = D_(D_(x->s)); R(D_(l), 0); push(x, LST, l); }
W(jAL) { OU2(x); C l = D_(x->s); D(x->s) = T(ATM, A(D_(x->s))); A(l) = x->s; x->s = l; }
W(jLA) { OU2(x); C t = D_(D_(x->s)); C l = lst(A(x->s)); R(l, D_(x->s)); R(D_(l), 0); R(x->s, t); }
W(jLL) { OU2(x); C t = D_(x->s); C l = lst(A(x->s)); R(l, A(D_(x->s))); R(x->s, rcl(x, t)); }
W(_join) { REF(D_(x->s)) ? (REF(x->s) ? jLL(x) : jAL(x)) : (REF(x->s) ? jLA(x) : jAA(x)); }
W(_quote) { U(x); C t = x->s; x->s = D_(x->s); R(t, 0); push(x, LST, t); }

W(dupA) { U(x); O(x); push(x, _D(x->s), A(x->s)); }
W(dupL) { U(x); On(x, A(x->s)); push(x, _D(x->s), cln(x, A(x->s))); }
W(_dup) { REF(x->s) ? dupL(x) : dupA(x); }
W(_swap) { U2(x); C t = D_(x->s); R(x->s, D_(D_(x->s))); R(t, x->s); x->s = t; }
W(_drop) { U(x); if (REF(x->s)) rmv(x, A(x->s)); pop(x); }
W(overV) { O(x); push(x, _D(D_(x->s)), A(D_(x->s))); }
W(overR) { On(x, A(D_(x->s))); push(x, _D(D_(x->s)), cln(x, A(D_(x->s)))); } 
W(_over) { U2(x); REF(D_(x->s)) ? overR(x) : overV(x); }
W(_rot) { C t = D_(D_(x->s)); R(D_(x->s), D_(D_(D_(x->s)))); R(t, x->s); x->s = t; }

W(_add) { U2(x); A(D_(x->s)) = A(D_(x->s)) + A(x->s); pop(x); }
W(_sub) { U2(x); A(D_(x->s)) = A(D_(x->s)) - A(x->s); pop(x); }
W(_mul) { U2(x); A(D_(x->s)) = A(D_(x->s)) * A(x->s); pop(x); }
W(_div) { U2(x); A(D_(x->s)) = A(D_(x->s)) / A(x->s); pop(x); }
W(_mod) { U2(x); A(D_(x->s)) = A(D_(x->s)) % A(x->s); pop(x); }

W(_gt) { U2(x); A(D_(x->s)) = A(D_(x->s)) > A(x->s); pop(x); }
W(_lt) { U2(x); A(D_(x->s)) = A(D_(x->s)) < A(x->s); pop(x); }
W(_eq) { U2(x); A(D_(x->s)) = A(D_(x->s)) == A(x->s); pop(x); }
W(_neq) { U2(x); A(D_(x->s)) = A(D_(x->s)) != A(x->s); pop(x); }

W(_and) { U2(x); A(D_(x->s)) = A(D_(x->s)) & A(x->s); pop(x); }
W(_or) { U2(x); A(D_(x->s)) = A(D_(x->s)) | A(x->s); pop(x); }
W(_invert) { U(x); A(x->s) = ~A(x->s); }
W(_not) { U(x); A(x->s) = !A(x->s); }

#define ATOM(x, n, d)							cns(x, n, T(ATM, d))
#define LIST(x, l, d)							cns(x, l, T(LST, d))
#define PRIMITIVE(x, p, d)				cns(x, (C)p, T(PRM, d))
#define RECURSION(x, d)						PRIMITIVE(x, 0, d)
#define JUMP(x, j, d)							cns(x, ATOM(x, j, 0), T(JMP, d))
#define LAMBDA(x, w, d)						cns(x, cns(x, cns(x, w, T(LST, 0)), T(LST, 0)), T(JMP, d))
C BRANCH(X* x, C t, C f, C d) {
		if (t) R(lst(t), d); else t = d;
		if (f) R(lst(f), d); else f = d;
		return cns(x, cns(x, t, T(LST, cns(x, f, T(LST, 0)))), T(JMP, d));
	}

void inner(X* x, C xt) {
		C ip = xt;
		while(!x->err && ip) {
			switch(_D(ip)) {
				case ATM: 
					push(x, ATM, A(ip)); ip = D_(ip); break;
				case LST: 
					push(x, LST, cln(x, A(ip))); ip = D_(ip); break;
				case JMP:
					if (IS(ATM, A(ip))) { ip = A(A(ip)); } /* JUMP */
					else if (D_(A(ip))) { ip = pop(x) ? A(A(ip)) : A(D_(A(ip))); } /* BRANCH */
					else { ip = D_(ip) ? (inner(x, A(A(A(ip)))), D_(ip)) : A(A(A(ip))); } /* CALL */
					break;
				case PRM: 
					if (A(ip)) { ((FUNC)A(ip))(x); ip = D_(ip); }
					else { ip = D_(ip) ? (inner(x, xt), D_(ip)) : xt; } /* RECURSION */
					break;
			}
		}
	}

B* allot(X* x, C b) {
		B* here = x->here;
		if (!b) { return 0;
		} else if (b < 0) { 
			x->here = (x->here + b) > BOTTOM(x) ? x->here + b : BOTTOM(x);
			while (x->there - sP >= ALIGN(x->here, sP)) { 
				C t = x->there;
				x->there -= sP;
				A(x->there) = t;
				D(x->there) = 0;
				D(t) = x->there;
			}
		} else {
			C p = x->there;
			while(A(p) == (p + sP) && p < (C)(x->here + b) && p < TOP(x)) { p = A(p);	}
			if (p >= (C)(here + b)) {
				x->there = p;
				D(x->there) = 0;
				x->here += b;
			} else {
				x->err = ERR_NOT_ENOUGH_MEMORY;
				return 0;
			}
		}
		return here;
	}

W(_allot) { allot(x, pop(x)); }
W(_align) { push(x, ATM, ALIGN(x->here, sC) - ((C)x->here)); _allot(x); }

#define count(s)				(*((C*)(s - sC)))

B* allot_str(X* x, C l) { *(C*)allot(x, sC) = l; return allot(x, l + 1); }

#define NFA(w)					((B*)(A(A(w))))
#define DFA(w)					(NFA(w) + count(NFA(w)) + 1)
#define XT(w)						(D_(A(w)))	
#define BODY(w)					(A(XT(w)))
#define IMMEDIATE(w)		(*(NFA(w) - sC - 1))

C header(X* x, B* n, C l) {
		*allot(x, 1) = 0;
		// TODO: Alignment of cell size of string is not checked
		C h = cns(x, 0, T(LST, 0));
		B* s = strcpy(allot_str(x, l), n);
		A(h) = ATOM(x, (C)s, cns(x, 0, T(LST, h)));
		return h;
	}
C body(X* x, C h, C l) { BODY(h) = l; return h; }
C reveal(X* x, C h) {	R(h, x->d); x->d = h;	return h; }

W(_bye) { x->err = ERR_BYE; }

void dump_list(C p) {
	if (p) {
		dump_list(D_(p));
		switch (_D(p)) {
			case ATM: printf("%ld ", A(p)); break;
			case LST: printf("{ "); dump_list(A(p)); printf("} "); break;
		}
	}
}

W(_dump_stack) {
	printf("<%ld> ", lth(x->s));
	dump_list(x->s);
	printf("\n");
}

#define WORD(n, f)	reveal(x, body(x, header(x, n, strlen(n)), PRIMITIVE(x, f, 0)));

X* bootstrap(X* x) {
		WORD("{}", &_empty);
		WORD("join", &_join);
		WORD("quote", &_quote);
		WORD("dup", &_dup);
		WORD("swap", &_swap);
		WORD("drop", &_drop);
		WORD("over", &_over);
		WORD("rot", &_rot);
		WORD("+", &_add);
		WORD("-", &_sub);
		WORD("*", &_mul);
		WORD("/", &_div);
		WORD("%", &_mod);
		WORD(">", &_gt);
		WORD("<", &_lt);
		WORD("=", &_eq);
		WORD("and", &_and);
		WORD("or", &_or);
		WORD("invert", &_invert);
		WORD("allot", &_allot);
		WORD("align", &_align);
		WORD("bye", &_bye);
		WORD(".s", &_dump_stack);
		WORD("{", &_lbrace);
		WORD("}", &_rbrace);
		return x;
	}

W(_parse_name) { 
	C l = 0;
	while (*(x->ibuf + x->in) != 0 && *(x->ibuf + x->in) == ' ') { x->in++; }
	push(x, ATM, (C)(x->ibuf + x->in));
	while (*(x->ibuf + x->in) != 0 && *(x->ibuf + x->in) != ' ') { x->in++; l++; }
	push(x, ATM, l);
}

C find(X* x, B* w, C l) { 
	for (C h = x->d; h; h = D_(h)) if (!strncmp(w, NFA(h), l)) return XT(h);	
	return 0;	
}

W(_find_name) { 
	C l = pop(x); 
	C a = pop(x); 
	C w = find(x, (B*)a, l);
	if (w && IMMEDIATE(D_(w))) {
		push(x, ATM, XT(w));
		push(x, ATM, 1);
	} else if (w && !IMMEDIATE(D_(w))) {
		push(x, ATM, XT(w));
		push(x, ATM, -1);
	} else {
		push(x, ATM, a);
		push(x, ATM, l);
		push(x, ATM, 0);
	}
}

void outer(X* x, B* s) {
		x->ibuf = s;
		while (1) {
			_parse_name(x);
			_find_name(x);
			C f = pop(x);
			if (f) {
				
			} else {
				// WORD NOT FOUND
			}
		}
		//B* tok = strtok(s, " ");
		//while (tok)
		//	C w = find(x, tok, strlen(tok));
		//	if (w) {
		//		if (x->st == ST_INTERPRETING || IMMEDIATE(w)) {
		//			inner(x, LAMBDA(x, A(w), 0));
		//		} else {
		//			cspush(x, LAMBDA(x, A(w), 0));
		//		}
		//	} else {
		//		intmax_t num = strtoimax(tok, NULL, 10);
		//		if (num == INTMAX_MAX && errno == ERANGE) {
		//			x->err = -1000;
		//		} else {
		//			if (x->st == ST_INTERPRETING) {
		//				inner(x, ATOM(x, num, 0));
		//			} else {
		//				cspush(x, ATOM(x, num, 0));
		//			}
		//		}
		//	}
		//	tok = strtok(NULL, " ");
		//}
	}


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

//////#define IS_IMMEDIATE(w)		(TYPE(w->ref) & 1)
//////
//////void _immediate(X* x) {
//////	x->d->ref = AS(1, REF(x->d));
//////}

