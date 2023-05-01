#ifndef DODO_CORE
#define DODO_CORE

#include<stddef.h>
#include<stdint.h>
#include<string.h>

typedef char BYTE;
typedef intptr_t CELL;

typedef struct NODE_T { 
	CELL next; 
	CELL value; 
} NODE;

typedef enum {
	ATOM,
	LIST,
	PRIM,
	IP
} TYPE;

CELL as(TYPE type, NODE* node) { 
	return (((CELL)(node)) | (type)); 
}

NODE* next(NODE* node) { 
	return ((NODE*)(node->next & -8)); 
}

TYPE type(NODE* node) { 
	return ((TYPE)(node->next & 7)); 
}

NODE* link(NODE* node, NODE* next) { 
	node->next = as(type(node), next); 
	return node; 
}

NODE* set_value(NODE* node, CELL value) { 
	node->value = value; 
	return node; 
}

NODE* set_next(NODE* node, CELL next) { 
	node->next = next; 
	return node; 
}

typedef struct {
	CELL size;
	CELL bottom;
	CELL here;
	CELL top;
	NODE* rstack;
	NODE* mstack;
	NODE** dstack;
	NODE* ip;
	BYTE block[1];
} CTX;

#define S(x)			(*x->dstack)

typedef void (*FUNC)(CTX*);

NODE* cons(CTX* x, CELL value, CELL next) {
	NODE* node = (NODE*)x->here;

	if ((x->here + sizeof(NODE)) > x->top) {
		printf("OUT OF MEMORY!!\n");
		return 0;
	}

	node->value = value;
	node->next = next;

	x->here += sizeof(NODE);

	return node;
}

#define atom(x, v, n)	(cons(x, v, as(ATOM, n)))
#define list(x, r, n)	(cons(x, (CELL)r, as(LIST, n)))
#define prim(x, p, n)	(cons(x, (CELL)p, as(PRIM, n)))

#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))

CTX* init(BYTE* block, CELL size) {
	CTX* x = (CTX*)block;

	x->size = size;

	x->bottom = (CELL)ALIGN(x + sizeof(CTX), sizeof(NODE));
	x->here = x->bottom;
	x->top = (CELL)(x + size);

	x->ip = 0;
	x->rstack = 0;
	x->mstack = list(x, 0, 0);
	x->dstack = (NODE**)&x->mstack->value;

	return x;
}

NODE* reclaim(CTX* x, NODE* node) {
	return next(node);
}

NODE* clone(CTX* x, NODE* node, CELL follow) {
	return cons(x, node->value, as(type(node), follow ? clone(x, next(node), 1) : 0));
}

void duplicate(CTX* x) { S(x) = link(clone(x, S(x), 0), S(x)); }
void swap(CTX* x) {	NODE* t = next(S(x)); link(S(x), next(next(S(x)))); link(t, S(x)); S(x) = t; }
void drop(CTX* x) { S(x) = reclaim(x, S(x)); }
void over(CTX* x) { S(x) = link(clone(x, next(S(x)), 0), S(x)); }
void rot(CTX* x) { NODE* t = next(next(S(x))); link(next(S(x)), next(next(next(S(x))))); link(t, S(x)); S(x) = t; }

void add(CTX* x) { next(S(x))->value = next(S(x))->value + S(x)->value; S(x) = reclaim(x, S(x)); }
void incr(CTX* x) {	S(x)->value++; }
void sub(CTX* x) { next(S(x))->value = next(S(x))->value - S(x)->value; S(x) = reclaim(x, S(x)); }
void decr(CTX* x) { S(x)->value--; }
void mul(CTX* x) { next(S(x))->value = next(S(x))->value * S(x)->value; S(x) = reclaim(x, S(x)); }
void division(CTX* x) { next(S(x))->value = next(S(x))->value / S(x)->value; S(x) = reclaim(x, S(x)); }
void mod(CTX* x) { next(S(x))->value = next(S(x))->value % S(x)->value; S(x) = reclaim(x, S(x)); }

void gt(CTX* x) { next(S(x))->value = next(S(x))->value > S(x)->value; S(x) = reclaim(x, S(x)); }
void lt(CTX* x) { next(S(x))->value = next(S(x))->value < S(x)->value; S(x) = reclaim(x, S(x)); }
void eq(CTX* x) { next(S(x))->value = next(S(x))->value == S(x)->value; S(x) = reclaim(x, S(x)); }
void neq(CTX* x) { next(S(x))->value = next(S(x))->value != S(x)->value; S(x) = reclaim(x, S(x)); }

void and(CTX* x) { next(S(x))->value = next(S(x))->value & S(x)->value; S(x) = reclaim(x, S(x)); }
void or(CTX* x) { next(S(x))->value = next(S(x))->value | S(x)->value; S(x) = reclaim(x, S(x)); }
void invert(CTX* x) { S(x)->value = ~(S(x)->value); }

void incrIP(CTX* x) {
	if (x->ip) { 
		x->ip = next(x->ip); 
	}
	while (!x->ip && x->rstack) {
		if (type(x->rstack) == IP) { 
			x->ip = (NODE*)x->rstack->value; 
		}
		x->rstack = reclaim(x, x->rstack);
	}
}

void call(CTX* x, NODE* dst, NODE* ret) { 
	if (ret) {
		x->rstack = cons(x, (CELL)ret, as(IP, x->rstack)); 
	}
	x->ip = dst; 
}

NODE* step(CTX* x) {
	NODE* r;
	if (x->ip) {
		switch (type(x->ip)) {
		case ATOM: 
			S(x) = atom(x, x->ip->value, S(x)); 
			incrIP(x); 
			break;
		case LIST: 
			S(x) = link(clone(x, x->ip, 0), S(x)); 
			incrIP(x); break;
		case PRIM: 
			r = x->ip; 
			((FUNC)(x->ip->value))(x); 
			if (r == x->ip || x->ip == 0) incrIP(x); 
			break;
			/*
			case WORD: case MACRO: call(x, XT((NODE*)x->ip->val), N(x->ip)); break;
			*/
		}
	}
	return x->ip;
}

void exec_i(CTX* x) {
	CELL p;
	NODE* node, * temp;
	switch (type(S(x))) {
	/*
	case ATOM: 
		p = S(x)->value; 
		S(x) = reclaim(x, S(x)); 
		((FUNC)p)(x); 
		break;
	*/
	case LIST: 
		if (S(x)->value) {
			temp = S(x);
			S(x) = next(S(x));
			call(x, (NODE*)temp->value, next(x->ip));
			link(temp, x->rstack);
			x->rstack = temp;
			step(x);
		} else {
			S(x) = reclaim(x, S(x));
		}
		break;
	case PRIM: 
		p = S(x)->value; 
		S(x) = reclaim(x, S(x)); 
		((FUNC)p)(x); 
		break;
	/*
		case WORD: case MACRO:
			n = x->ip == 0 ? 0 : N(x->ip);
			call(x, XT((NODE*)S(x)->val), n);
			S(x) = reclaim(x, S(x));
			while (step(x) != n);
			break;
		*/
	}
}

void exec_x(CTX* x) { 
	duplicate(x); 
	exec_i(x); 
}

void branch(CTX* x) {
	if (next(next(S(x)))->value == 0) { 
		swap(x); 
	} 
	drop(x); 
	swap(x); 
	drop(x); 
	exec_i(x); 
}

#endif
