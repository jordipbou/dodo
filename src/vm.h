#include<stdint.h>
#include<stddef.h>

// Scalars

typedef int64_t cell_t;

// Lists

#define CAR(addr)		*(addr + 1)
#define CDR(addr)		*addr

cell_t length(cell_t *l) {
	for (cell_t a = 0;; a++) { if (!l) { return a; } else { l = (cell_t *)CDR(l); } }
}

// Memory

#define HEADER_SIZE			4

#define SIZE(b)					*b
#define FREE_HEAD(b)		*(b + 1)
#define FREE_TAIL(b)		*(b + 2)

cell_t *init_block(cell_t *b, cell_t s) {
	if ((SIZE(b) = s) % 2 != 0) return NULL;

	for (
		cell_t *p = (cell_t *)(FREE_TAIL(b) = (cell_t)(b + HEADER_SIZE));
		p <= (cell_t *)(FREE_HEAD(b) = (cell_t)(b + s - 2));
		p += 2
	) {
			CDR(p) = (cell_t)(p - 2);
			CAR(p) = (cell_t)(p + 2);
	}

	CDR((cell_t *)FREE_TAIL(b)) = (cell_t)NULL;
	CAR((cell_t *)FREE_HEAD(b)) = (cell_t)NULL;
	
	return b;
}

cell_t *take(cell_t *b) {
	if ((cell_t *)FREE_HEAD(b) == NULL) return NULL;

	cell_t *h = (cell_t *)FREE_HEAD(b);
	FREE_HEAD(b) = (cell_t)CDR(h);
	CAR((cell_t *)FREE_HEAD(b)) = (cell_t)NULL;

	CAR(h) = 0;
	CDR(h) = (cell_t)NULL;

	return h;
}

void put(cell_t *b, cell_t *i) {
	cell_t *h = (cell_t *)FREE_HEAD(b);
	FREE_HEAD(b) = (cell_t)i;
	CAR(h) = (cell_t)i;
	CAR(i) = (cell_t)NULL;
	CDR(i) = (cell_t)h;
}

// Scopes

typedef struct scp_s {
	cell_t *block;
	cell_t *dstack, *rstack;
} scp_t;

void push(scp_t *s, cell_t v) {
	cell_t *i = take(s->block);
	CAR(i) = v;
	CDR(i) = (cell_t)s->dstack;
	s->dstack = i;
}

cell_t pop(scp_t *s) {
	cell_t *i = s->dstack;
	cell_t v = CAR(i);
	s->dstack = (cell_t *)CDR(i);
	put(s->block, i);
	return v;
}

// Runtime context

typedef struct ctx_s {
	cell_t T, S;
	scp_t *scope;
} ctx_t;

#define DUP(c)		push(c->scope, c->S); c->S = c->T
#define LIT1(c)		push(c->scope, c->S); c->S = c->T; c->T = 1
#define GT(c)			c->T = c->S > c->T; c->S = pop(c->scope)
#define DEC(c)		c->T = c->T - 1
#define SUB(c)		c->T = c->S - c->T; c->S = pop(c->scope)
#define SWAP(c)		cell_t x = c->T; c->T = c->S; c->S = x
#define ADD(c)		c->T = c->S + c->T; c->S = pop(c->scope)
