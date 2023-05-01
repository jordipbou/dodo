#ifndef DODO_INNER
#define DODO_INNER

#include "types.h"

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

CTX* init(BYTE* block, CELL size) {

}

void incrIP(CTX* x) {
	if (x->ip) { 
		x->ip = next(x->ip); 
	}
	while (!x->ip && x->rstack) {
		if (type(x->r) == IP) { 
			x->ip = (NODE*)x->rstack->value; 
		}
		x->rstack = reclaim(x, x->rstack);
	}
}

/*
void call(CTX* x, NODE* n, NODE* r) { if (r) x->r = cons(x, (CELL)r, AS(IP, x->r)); x->ip = n; }
*/

NODE* step(CTX* x) {
	NODE* r;
	if (x->ip) {
		switch (T(x->ip)) {
			case ATOM: S(x) = cons(x, x->ip->val, AS(ATOM, S(x))); incrIP(x); break;
			case LIST: S(x) = L(clone(x, x->ip, 0), S(x)); incrIP(x); break;
			case PRIM: r = x->ip; ((FUNC)(x->ip->val))(x); if (r == x->ip || x->ip == 0) incrIP(x); break;
			/*
			case WORD: case MACRO: call(x, XT((NODE*)x->ip->val), N(x->ip)); break;
			*/
		}
	}
	return x->ip;
}

#endif
