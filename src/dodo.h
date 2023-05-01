#ifndef __DODO__
#define __DODO__

#include<stddef.h>
#include<stdint.h>
#include<string.h>

typedef char BYTE;
typedef intptr_t CELL;

typedef struct NODE_T { CELL next, val; } NODE;

typedef enum { FREE = 0, ATOM = 0, LIST, PRIM, WORD, MACRO, REF, CODE, IP } TYPE;
typedef enum { ARRAY, BARRAY, STRING } REF_TYPE;

CELL AS(TYPE t, NODE* n) { return (((CELL)(n)) | (t)); }
NODE* N(NODE* n) { return ((NODE*)(n->next & -8)); }
TYPE T(NODE* n) { return ((TYPE)(n->next & 7)); }
NODE* L(NODE* n, NODE* m) { n->next = AS(T(n), m); return n; }
NODE* V(NODE* n, CELL v) { n->val = v; return n; }
NODE* X(NODE* n, CELL x) { n->next = x; return n; }

CELL length(NODE* n) { CELL a = 0; while (n) { a++; n = N(n); } return a; }
NODE* reverse(NODE* n, NODE* acc) { NODE* t; return n ? (t = N(n), reverse(t, L(n, acc))) : acc; }

typedef struct { CELL type, size, length, data[1]; } DATA;

#define REF_DATA(ref)							((DATA*)((CELL)(ref) - sizeof(DATA) + sizeof(CELL)))

typedef struct {
	CELL size, err, compiling;
	BYTE* here;			/* Pointer to free area of contiguous memory */
	NODE* there;		/* End of free doubly linked list and start of contiguous memory */
	NODE* f;				/* Free doubly linked list */
	NODE* m;				/* Main stack */
	NODE* r;				/* Return stack */
	NODE* n;				/* Name stack */
	NODE** d;				/* Data stack */
	NODE* ip;				/* Instruction pointer */
	BYTE* ibuf;			/* Input buffer */
	CELL ipos;			/* Parsing position on input buffer */
	CELL maxS, maxR;
	BYTE bottom[1];	/* Start of memory block for this context */
} CTX;

#define S(x)											(*x->d)

typedef void (*FUNC)(CTX*);

#define ALIGN(addr, bound)				((((CELL)addr) + (bound - 1)) & ~(bound - 1))
#define FREE(x)										(length(x->f) - 1)

NODE* cons(CTX* x, CELL v, CELL n) { /* Not checking if it's full !!!! */ NODE* a = x->f; x->f = V(N(x->f), 0); return X(V(a, v), n); }

DATA* array(CTX* x, CELL s) {
	NODE** p = &x->f;
	NODE* n = x->f;
	CELL sz, t;
	DATA* a;

	s += sizeof(DATA) - sizeof(CELL);
	sz = s / sizeof(NODE) + ((s % sizeof(NODE)) == 0 ? 0 : 1);

	while (n && n != x->there) {
		t = sz;
		while (--t && n && n != x->there) {
			if ((((NODE*)n->val) == (n + 1) || n->val == 0)) {
				n = N(n);
			} else {
				break;
			}
		}

		if (t == 0) {
			*p = N(n);
			N(n)->val = (CELL)((*p == x->f) ? 0 : *p);

			a = (DATA*)n;	
			a->type = ARRAY;
			a->size = sz * sizeof(NODE);

			return a;
		}

		p = (NODE**)(&(n->next));
		n = N(n);
	}

	return 0;
}

BYTE* string(CTX* x, BYTE* s, CELL l) {
	DATA* d = array(x, l + 1);
	BYTE* t = (BYTE*)d->data;
	strncpy(t, s, l);
	t[l] = 0;
	d->length = l;
	d->type = STRING;
	return (BYTE*)d->data;
}

void recl_data(CTX* x, DATA* d) {
	NODE* n;
	CELL ns;
	if (!d) return;
	ns = d->size / sizeof(NODE);
	while (ns) {
		n = ((NODE*)d) + --ns;	
		n->next = (CELL)x->f;
		n->val = 0;
		x->f->val = (CELL)n;
		x->f = n;
	}
}

NODE* reclaim(CTX* x, NODE* n) {
	NODE* tail = N(n);

	if (T(n) == LIST) {
		while (n->val) {
			n->val = (CELL)reclaim(x, (NODE*)n->val);
		}
	}

	if (T(n) == REF || T(n) == CODE) {
		recl_data(x, REF_DATA(n->val));
	}

	n->next = (CELL)x->f;
	n->val = 0;
	x->f->val = (CELL)n;
	x->f = n;

	return tail;
}

NODE* clone(CTX* x, NODE* n, CELL f) {
	DATA* d, * e;
	if (!n) return 0;
	if (T(n) == LIST) {
		return cons(x, (CELL)clone(x, (NODE*)n->val, 1), AS(LIST, f ? clone(x, N(n), 1) : 0));
	} else if (T(n) == REF) {
		d = REF_DATA(n->val);
		e = array(x, d->size - sizeof(DATA) + sizeof(CELL));
		e->type = d->type;
		e->length = d->length;
		memcpy(e->data, d->data, d->size - sizeof(DATA) + sizeof(CELL));
		return cons(x, (CELL)e->data, AS(REF, f ? clone(x, N(n), 1) : 0));
	} else {
		return cons(x, n->val, AS(T(n), f ? clone(x, N(n), 1) : 0));
	}
}

CTX* init(BYTE* b, CELL sz) {
	NODE* node;
	CTX* x = (CTX*)b;

	x->size = sz;
	x->here = ((BYTE*)x) + offsetof(CTX, bottom);
	x->there = (NODE*)ALIGN(x->here, sizeof(NODE));
	x->f = (NODE*)ALIGN(x + sz - sizeof(NODE) - 1, sizeof(NODE));

	for (node = x->there; node <= x->f; node++) {
		node->next = AS(FREE, node == x->there ? 0 : node - 1);
		node->val = (CELL)(node == x->f ? 0 : node + 1);
	}

	x->m = cons(x, 0, AS(LIST, 0));
	x->d = (NODE**)&x->m->val;
	x->r = x->n = 0;

	x->err = x->compiling = x->ipos = 0;
	x->ibuf = 0;
	x->ip = 0;

	x->maxR = 0;
	x->maxS = 0;

	return x;
}

void duplicate(CTX* x) { S(x) = L(clone((CTX*)x, (NODE*)S(x), 0), (NODE*)S(x)); }
void swap(CTX* x) {	NODE* t = N(S(x)); L(S(x), N(N(S(x)))); L(t, S(x)); S(x) = t; }
void drop(CTX* x) { S(x) = reclaim(x, S(x)); }
void over(CTX* x) { S(x) = L(clone(x, N(S(x)), 0), S(x)); }
void rot(CTX* x) { NODE* t = N(N(S(x))); L(N(S(x)), N(N(N(S(x))))); L(t, S(x)); S(x) = t; }

void add(CTX* x) { N(S(x))->val = N(S(x))->val + S(x)->val; S(x) = reclaim(x, S(x)); }
void incr(CTX* x) {	S(x)->val++; }
void sub(CTX* x) { N(S(x))->val = N(S(x))->val - S(x)->val; S(x) = reclaim(x, S(x)); }
void decr(CTX* x) { S(x)->val--; }
void mul(CTX* x) { N(S(x))->val = N(S(x))->val * S(x)->val; S(x) = reclaim(x, S(x)); }
void division(CTX* x) { N(S(x))->val = N(S(x))->val / S(x)->val; S(x) = reclaim(x, S(x)); }
void mod(CTX* x) { N(S(x))->val = N(S(x))->val % S(x)->val; S(x) = reclaim(x, S(x)); }

void gt(CTX* x) { N(S(x))->val = N(S(x))->val > S(x)->val; S(x) = reclaim(x, S(x)); }
void lt(CTX* x) { N(S(x))->val = N(S(x))->val < S(x)->val; S(x) = reclaim(x, S(x)); }
void eq(CTX* x) { N(S(x))->val = N(S(x))->val == S(x)->val; S(x) = reclaim(x, S(x)); }
void neq(CTX* x) { N(S(x))->val = N(S(x))->val != S(x)->val; S(x) = reclaim(x, S(x)); }

void and(CTX* x) { N(S(x))->val = N(S(x))->val & S(x)->val; S(x) = reclaim(x, S(x)); }
void or(CTX* x) { N(S(x))->val = N(S(x))->val | S(x)->val; S(x) = reclaim(x, S(x)); }
void invert(CTX* x) { S(x)->val = ~(S(x)->val); }

void empty(CTX* x) { S(x) = cons(x, 0, AS(LIST, S(x))); }
void list_to_stack(CTX* x) { NODE* a = S(x); S(x) = N(S(x)); x->d = (NODE**)&(x->m = L(a, x->m))->val; }
void stack_to_list(CTX* x) {
	if (N(x->m)) {
		NODE* t = x->m;
		NODE* s = N(x->m);
		L(t, (NODE*)s->val);
		s->val = (CELL)t;
		x->m = s;
		x->d = (NODE**)&x->m->val;
	} else {
		S(x) = cons(x, (CELL)S(x), AS(LIST, 0));
	}
}
void reverse_stack(CTX* x) { S(x) = reverse(S(x), 0); }

void rbracket(CTX* x) { x->compiling = 1; }
void lbracket(CTX* x) { x->compiling = 0; }

void string_literal(CTX* x) {
	CELL l = 0;
	CELL p = ++x->ipos;
	while(*(x->ibuf + p) && *(x->ibuf + p) != '"') {
		l++;
		p++;
	}
	S(x) = cons(x, (CELL)string(x, x->ibuf + x->ipos, l), AS(REF, S(x)));
	x->ipos = p + 1;
}

void header(CTX* x) { S(x) = cons(x, 0, AS(WORD, S(x))); }
void to_word(CTX* x) { NODE* h = V(S(x), N(S(x))->val); S(x) = reclaim(x, V(N(S(x)), 0)); x->n = L(h, x->n); }

BYTE* NFA(NODE* w) { return (BYTE*)(((NODE*)w->val)->val); }
NODE* XT(NODE* w) { return N((NODE*)(w->val)); }

CELL IS_WORD(NODE* w) { return T(w) == WORD || T(w) == MACRO; }
CELL IS_PRIMITIVE(NODE* w) { return T(w) == PRIM || (IS_WORD(w) && length(XT(w)) == 1 && T(XT(w)) == PRIM); }

CELL IS_HEADER(NODE* w) { return IS_WORD(w) && w->val == 0; }

#define ADD_PRIMITIVE(x, name, func, t) \
	x->n = \
		cons(x, (CELL) \
			cons(x, (CELL)name, AS(ATOM, \
			cons(x, (CELL)func, AS(PRIM, 0)))), \
		AS(t, x->n));

void incrIP(CTX* x) {
	if (x->ip) { x->ip = N(x->ip); }
	while (!x->ip && x->r) {
		if (T(x->r) == IP) { x->ip = (NODE*)x->r->val; }
		x->r = reclaim(x, x->r);
	}
}

void call(CTX* x, NODE* n, NODE* r) { if (r) x->r = cons(x, (CELL)r, AS(IP, x->r)); x->ip = n; }

NODE* step(CTX* x) {
	NODE* r;
	CELL ls, lr;
	if (x->ip) {
		switch (T(x->ip)) {
			case ATOM: S(x) = cons(x, x->ip->val, AS(ATOM, S(x))); incrIP(x); break;
			case LIST: S(x) = L(clone(x, x->ip, 0), S(x)); incrIP(x); break;
			case PRIM: r = x->ip; ((FUNC)(x->ip->val))(x); if (r == x->ip || x->ip == 0) incrIP(x); break;
			case WORD: case MACRO: call(x, XT((NODE*)x->ip->val), N(x->ip)); break;
		}
	}
	ls = length(S(x)); if (ls > x->maxS) x->maxS = ls;
	lr = length(x->r); if (lr > x->maxR) x->maxR = lr;
	return x->ip;
}

void exec_i(CTX* x) {
	CELL p;
	NODE* n, * t;
	switch (T(S(x))) {
		case ATOM: p = S(x)->val; S(x) = reclaim(x, S(x)); ((FUNC)p)(x); break;
		case LIST: 
			if (S(x)->val) {
				t = S(x);
				S(x) = N(S(x));
				call(x, (NODE*)t->val, N(x->ip));
				L(t, x->r);
				x->r = t;
				step(x);
			} else {
				S(x) = reclaim(x, S(x));
			}
			break;
		case PRIM: p = S(x)->val; S(x) = reclaim(x, S(x)); ((FUNC)p)(x); break;
		case WORD: case MACRO:
			n = x->ip == 0 ? 0 : N(x->ip);
			call(x, XT((NODE*)S(x)->val), n);
			S(x) = reclaim(x, S(x));
			while (step(x) != n);
			break;
	}
}

void exec_x(CTX* x) { duplicate(x); exec_i(x); }
void branch(CTX* x) {
	if (N(N(S(x)))->val == 0) { 
		swap(x); 
	} 
	drop(x); 
	swap(x); 
	drop(x); 
	exec_i(x); 
}

void find_token(CTX* x) {
	NODE* w;
	BYTE* token;
	CELL length;
	char* endptr;
	intmax_t num;
	
	while (*(x->ibuf + x->ipos) && isspace(*(x->ibuf + x->ipos))) { x->ipos++; }
	token = x->ibuf + x->ipos;
	while (*(x->ibuf + x->ipos) && !isspace(*(x->ibuf + x->ipos))) { x->ipos++; }
	length = (x->ibuf + x->ipos) - token;

	if (length == 0) {
		x->err = -2;
		return;
	}

	w = x->n;
	while (w && !(strlen(NFA(w)) == length && !strncmp(NFA(w), token, length))) {
		w = N(w);
	}

	if (w) {
		S(x) = cons(x, (CELL)w, AS(T(w), S(x)));
	} else {
		num = strtoimax(token, &endptr, 0);
		if (num == 0 && endptr == token) {
			x->err = -1;
			return;
		} else {
			S(x) = cons(x, (CELL)num, AS(ATOM, S(x)));
		}
	}
}

void eval(CTX* x, BYTE* str) {
	x->ibuf = str;
	x->ipos = 0;
	while (1) {
		find_token(x);
		if (x->err != 0) return;
		if (IS_WORD(S(x))) {
			if (!x->compiling || T(S(x)) == MACRO) {
				exec_i(x);
			} else if (x->compiling && IS_PRIMITIVE((NODE*)(S(x)->val))) {
				S(x)->val = XT((NODE*)S(x)->val)->val;
				S(x)->next = AS(PRIM, N(S(x)));
			}
		}
	}
}

NODE* find_primitive(CTX* x, CELL p) {
	NODE* w = x->n;
	while (w) {
		if (IS_PRIMITIVE(w) && XT(w)->val == p) {
			return w;
		}
		w = N(w);
	}
	return 0;
}

BYTE* print(BYTE* str, NODE* n, CELL f, CTX* x) {
	NODE* w;

	if (!n) return str;

	switch (T(n)) {
	case ATOM: 
		sprintf(str, "%s#%ld ", str, n->val); 
		break;
	case REF:
		switch (REF_DATA(n->val)->type) {
		case ARRAY: break;
		case BARRAY: break;
		case STRING: 
			sprintf(str, "%s\"%.*s\" ", str, (int)REF_DATA(n->val)->length, (BYTE*)(n->val)); 
			break;
		}
		break;
	case LIST: 
		sprintf(str, "%s{ ", str); 
		print(str, (NODE*)n->val, 1, x); 
		sprintf(str, "%s} ", str); 
		break;
	case PRIM: 
		if (x != 0) {
			if ((w = find_primitive(x, n->val)) != 0) {
				sprintf(str, "%sP:%s ", str, NFA(w));
				break;
			}
		}
		sprintf(str, "%sP:%ld ", str, n->val); 
		break;
	case WORD: 
		if (n->val) {
			sprintf(str, "%sW:%s ", str, NFA((NODE*)n->val)); 
		} else {
			sprintf(str, "%sHEADER ", str);
		}
		break;
	case MACRO: 
		sprintf(str, "%sM:%s ", str, NFA((NODE*)n->val)); 
		break;
	}

	if (f && N(n)) print(str, N(n), 1, x);

	return str;
}

CTX* bootstrap(CTX* x) {
	ADD_PRIMITIVE(x, "dup", &duplicate, WORD);
	ADD_PRIMITIVE(x, "swap", &swap, WORD);
	ADD_PRIMITIVE(x, "drop", &drop, WORD);
	ADD_PRIMITIVE(x, "over", &over, WORD);
	ADD_PRIMITIVE(x, "rot", &rot, WORD);

	ADD_PRIMITIVE(x, "+", &add, WORD);
	ADD_PRIMITIVE(x, "1+", &incr, WORD);
	ADD_PRIMITIVE(x, "-", &sub, WORD);
	ADD_PRIMITIVE(x, "1-", &decr, WORD);
	ADD_PRIMITIVE(x, "*", &mul, WORD);
	ADD_PRIMITIVE(x, "/", &division, WORD);
	ADD_PRIMITIVE(x, "mod", &mod, WORD);

	ADD_PRIMITIVE(x, ">", &gt, WORD);
	ADD_PRIMITIVE(x, "<", &lt, WORD);
	ADD_PRIMITIVE(x, "=", &eq, WORD);
	ADD_PRIMITIVE(x, "<>", &neq, WORD);

	ADD_PRIMITIVE(x, "and", &and, WORD);
	ADD_PRIMITIVE(x, "or", &or, WORD);
	ADD_PRIMITIVE(x, "invert", &invert, WORD);

	ADD_PRIMITIVE(x, "{}", &empty, WORD);
	ADD_PRIMITIVE(x, "l>s", &list_to_stack, WORD);
	ADD_PRIMITIVE(x, "s>l", &stack_to_list, WORD);
	ADD_PRIMITIVE(x, "reverse", &reverse_stack, WORD);
	ADD_PRIMITIVE(x, "header", &header, WORD);
	ADD_PRIMITIVE(x, ">word", &to_word, WORD);

	ADD_PRIMITIVE(x, "]", &rbracket, WORD);
	ADD_PRIMITIVE(x, "[", &lbracket, MACRO);

	ADD_PRIMITIVE(x, "\"", &string_literal, WORD);

	ADD_PRIMITIVE(x, "i", &exec_i, WORD);
	ADD_PRIMITIVE(x, "x", &exec_x, WORD);
	ADD_PRIMITIVE(x, "branch", &branch, WORD);


	return x;
}

#endif
