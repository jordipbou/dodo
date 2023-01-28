#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<inttypes.h>

#ifdef _WIN32
  #include <conio.h>
#else
	#include <unistd.h>
	#include <termios.h>
#endif

typedef int8_t		B;
typedef intptr_t	C;		// 16, 32 or 64 bits depending on system

#define A(pair)					(*((C*)pair))
#define D(pair)					(*(((C*)pair) + 1))

#define T_ATOM				0
#define T_PRIMITIVE		1
#define T_BRANCH			2
#define T_WORD				3
#define T_RECURSION		4

typedef struct {
	B* bottom, * here;
	C there, top, nodes, dict, ibuf, err;
} X;

#define N(x)					x->nodes
#define K(x)					D(N(x))						// Data stacK
#define F(x)					A(N(x))						// Doubly linked list of Free nodes
#define T(x)					A(K(x))						// Top of data stack
#define S(x)					A(D(K(x)))				// Second of data stack
#define P(x)					D(x->top)					// Pile of stacks
#define R(x)					A(x->top)					// Return stack

typedef void (*FUNC)(X*);

#define ALIGN(addr, bound)	((((C)addr) + (bound - 1)) & ~(bound - 1))

#define ERR_NOT_ENOUGH_MEMORY		-1
#define ERR_OVERFLOW						-2
#define ERR_UNDERFLOW						-3

C height(C p) { C c = 0; while (p != 0) { c++; p = A(p); } return c; }
C depth(C p) { C c = 0; while (p != 0) { c++; p = D(p); } return c; }


X* init(B* block, C size) {
	X* x = (X*)block;	
	x->bottom = x->here = ((B*)x) + sizeof(X);
	x->there = ALIGN((C)x->bottom, 2*sizeof(C));
	x->top = ALIGN((block + size - 2*sizeof(C) - 1), 2*sizeof(C));
	x->nodes = x->top - 4*sizeof(C);
	P(x) = R(x) = x->top - 2*sizeof(C);

	for (C p = x->there; p <= x->nodes; p += 2*sizeof(C)) {
		A(p) = p == x->there ? 0 : p - 2*sizeof(C);
		D(p) = p == x->nodes ? 0 : p + 2*sizeof(C);
	}

	D(R(x)) = A(R(x)) = x->err = x->ibuf = x->dict = 0;

	return x;
}

#define OF(x)		if (F(x) == 0) { x->err = ERR_OVERFLOW; return; }
#define UF(x)		if (K(x) == 0) { x->err = ERR_UNDERFLOW; return 0; }

void push(X* x, C v) { OF(x); N(x) = F(x); T(x) = v; }
C pop(X* x) { UF(x); C t = T(x); T(x) = N(x); N(x) = K(x); return t; }
C cons(X* x, C a, C d) { C p = N(x); push(x, a); K(x) = K(x) ? D(K(x)) : 0; D(p) = d; return p; }
C re(X* x, C p) {C t; return p ? (t = D(p), D(p) = K(x), A(p) = N(x), K(x) = p, N(x) = p, t) : 0; }

void _sclear(X* x) { while (K(x)) pop(x); }
void _spush(X* x) { P(x) = cons(x, K(x), P(x)); K(x) = 0; }
void _sdrop(X* x) { _sclear(x); if (P(x) != R(x)) { K(x) = A(P(x)); P(x) = re(x, P(x)); } }

void _dup(X* x) { push(x, T(x)); }
void _swap(X* x) { C t = T(x); T(x) = S(x); S(x) = t; }
void _over(X* x) { push(x, S(x)); }
void _rot(X* x) { C t = A(D(D(K(x)))); A(D(D(K(x)))) = S(x); S(x) = T(x); T(x) = t; }
void _drop(X* x) { pop(x); }
void _rev(X* x) {	C s = K(x);	K(x) = 0;	while (s) { C t = D(s); D(s) = K(x); K(x) = s; s = t; } }

void _add(X* x) { C t = pop(x); T(x) += t; }
void _sub(X* x) { C t = pop(x); T(x) -= t; }
void _mul(X* x) { C t = pop(x); T(x) *= t; }
void _div(X* x) { C t = pop(x); T(x) /= t; }
void _mod(X* x) { C t = pop(x); T(x) %= t; }

void _gt(X* x) { C t = pop(x); T(x) = T(x) > t; }
void _lt(X* x) { C t = pop(x); T(x) = T(x) < t; }
void _eq(X* x) { C t = pop(x); T(x) = T(x) == t; }
void _neq(X* x) { C t = pop(x); T(x) = T(x) != t; }

void _and(X* x) { C t = pop(x); T(x) = T(x) && t; }
void _or(X* x) { C t = pop(x); T(x) = T(x) || t; }
void _not(X* x) { T(x) = !T(x); }

void _fetch(X* x) { push(x, *((C*)pop(x))); }
void _store(X* x) { C v = pop(x); *((C*)pop(x)) = v; }

void _sfetch(X* x) { C a = pop(x); _spush(x); K(x) = *((C*)a); *((C*)a) = 0; }
void _sstore(X* x) { C a = pop(x); *((C*)a) = K(x); K(x) = 0; }

#define ATOM(x, n, d)					cons(x, cons(x, T_ATOM, n), d)
#define PRIMITIVE(x, p, d)		cons(x, cons(x, T_PRIMITIVE, (C)p), d)
#define RECURSION(x, d)				cons(x, cons(x, T_RECURSION, 0), d)
C BRANCH(X* x, C t, C f, C d) {
	if (t) { C lt = t; while(D(lt)) lt = D(lt); D(lt) = d; } else { t = d; }
	if (f) { C lf = f; while(D(lf)) lf = D(lf); D(lf) = d; } else { f = d; }
	return cons(x, cons(x, T_BRANCH, cons(x, t, cons(x, f, 0))), d);
}
#define WORD(x, c, d)					cons(x, cons(x, T_WORD, c), d)

void inner(X* x, C xlist) {
	C ip = xlist;
	while(x->err == 0 && ip != 0) {
		switch(A(A(ip))) {
			case T_ATOM: push(x, D(A(ip))); ip = D(ip); break;
			case T_PRIMITIVE:	((FUNC)D(A(ip)))(x); ip = D(ip); break;
			case T_RECURSION: ip = D(ip) ? (inner(x, xlist), D(ip)) : xlist; break;
			case T_BRANCH: ip = pop(x) ? A(D(A(ip))) : A(D(D(A(ip)))); break;
			case T_WORD: ip = D(ip) ? (inner(x, D(A(ip))), D(ip)) : D(A(ip)); break;
		}
	}
}

// Source code for getch is taken from:
// Crossline readline (https://github.com/jcwangxp/Crossline).
// It's a fantastic readline cross-platform replacement, but only getch was
// needed and there's no need to include everything else.
#ifdef _WIN32	// Windows
int dodo_getch (void) {	fflush (stdout); return _getch(); }
#else
int dodo_getch ()
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

void _key(X* x) { push(x, dodo_getch()); }
void _emit(X* x) { C K = T(x); pop(x); K == 127 ? printf ("\b \b") : printf ("%c", (char)K); }

void _dot_s(X* x) {	
	C p = K(x);	
	printf("<%ld> ", depth(K(x))); 
	while (p) { printf("%ld ", A(p)); p = D(p); }
	printf("\n");
}

void _interpret(X* x) {
	// skip leading spaces
	while (x->ibuf && (A(x->ibuf) == 32 || A(x->ibuf) == 10)) { x->ibuf = re(x, x->ibuf); }
	C c = 0; C p = 0
	while (x->ibuf && A(x->ibuf) != 32 && A(x->ibuf) != 10) { c++; 
	// find word, if found...
		// if interpreting...
		// if compiling...
	// ...if not found...
		// transform word to number, if possible...
			// ...if interpreting, push it to the stack
			// ...if compiling, compile atom
		// ...if can't be converted...
			// ambiguous condition, do whatever you want here!
}

void _quit(X* x) {
	//while (!x->err) {
		while (K(x) ? T(x) != 10 : 1) { _key(x); _dup(x); _emit(x); } _drop(x);
		_rev(x);
		push(x, (C)&x->ibuf);
		_sstore(x);
		while (x->ibuf) { _interpret(x); }
		
	//}
}

void _header(X* x) {
	// Parses next word from input buffer
	//
}

C find(X* x, B* t) {
	return 0;	
}

void _include(X* x, B* fn) {
	B buf[255];
	B* token;
	FILE* fptr = fopen(fn, "r");
	if (fptr) {
		while (fgets(buf, 255, fptr)) {
			printf("LINE: %s", buf);
			token = strtok(buf, " \n");
			while (token) {
				printf("TOKEN: %s\n", token);
				token = strtok(NULL, " \n");
				C xlist = find(x, token);
				if (xlist) { }
				else {
					// Try to convert to number
					char* endptr;
					errno = 0;
					C n = strtoimax(token, &endptr, 10);
					printf("errno: %ld\n", errno);
					if (!errno) { printf("ATOM: %ld\n", n); push(x, n); }	
					else { printf("Unknown word\n"); }
				}
			}
		}
	}
	_dot_s(x);
}

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
