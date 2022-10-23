#include <stdio.h>
#include "vm.h"

// Lists

cell_t *cons(cell_t *b, cell_t car, cell_t cdr) {
	if ((cell_t *)FREE_HEAD(b) == NULL) return NULL;

	cell_t *h = (cell_t *)FREE_HEAD(b);
	FREE_HEAD(b) = (cell_t)CDR(h);
	CAR((cell_t *)FREE_HEAD(b)) = (cell_t)NULL;

	CAR(h) = car;
	CDR(h) = cdr;

	if (h < (cell_t *)LOWEST_ASSIGNED(b)) LOWEST_ASSIGNED(b) = (cell_t)h;

	return h;
}

void reclaim(cell_t *b, cell_t *i) {
	cell_t *h = (cell_t *)FREE_HEAD(b);
	FREE_HEAD(b) = (cell_t)i;
	CAR(h) = (cell_t)i;
	CAR(i) = (cell_t)NULL;
	CDR(i) = (cell_t)h;
}

cell_t length(cell_t *l) {
	for (cell_t a = 0;; a++) { if (!l) { return a; } else { l = (cell_t *)CDR(l); } }
}

// Block

cell_t *init_block(cell_t *b, cell_t s) {
	if ((SIZE(b) = s) % 2 != 0) return NULL;

	for (
		cell_t *p = (cell_t *)(HERE(b) = FREE_TAIL(b) = (cell_t)(b + HEADER_SIZE));
		p <= (cell_t *)(LOWEST_ASSIGNED(b) = FREE_HEAD(b) = (cell_t)(b + s - 2));
		p += 2
	) {
			CDR(p) = (cell_t)(p - 2);
			CAR(p) = (cell_t)(p + 2);
	}

	CDR((cell_t *)FREE_TAIL(b)) = (cell_t)NULL;
	CAR((cell_t *)FREE_HEAD(b)) = (cell_t)NULL;

	return b;
}

cell_t reserve(cell_t *b, cell_t npairs) {
	if (((cell_t *)LOWEST_ASSIGNED(b)) - ((cell_t *)FREE_TAIL(b)) <= 2*npairs) return -1;

	FREE_TAIL(b) = (cell_t)((cell_t *)FREE_TAIL(b) + 2*npairs);
	CDR((cell_t *)FREE_TAIL(b)) = (cell_t)NULL;

	return 0;
}

cell_t allot(cell_t *b, cell_t nbytes) {
	if (FREE_TAIL(b) - HERE(b) <= nbytes) {
		cell_t psize = 2*sizeof(cell_t);
		cell_t required = nbytes - (FREE_TAIL(b) - HERE(b));
		cell_t npairs = (required / psize) * psize < required ? required / psize + 1 : required / psize;
		if (reserve(b, npairs) == -1) {
			return -1;
		}
	}

	HERE(b) += nbytes;

	return 0;
}

// Stacks


void push(scp_t *s, cell_t v) {
	cell_t *i = cons(s->block, v, (cell_t)s->dstack);
	s->dstack = i;
	s->ddepth++;
}

cell_t pop(scp_t *s) {
	cell_t *i = s->dstack;
	cell_t v = CAR(i);
	s->dstack = (cell_t *)CDR(i);
	reclaim(s->block, i);
	s->ddepth--;
	return v;
}

void rpush(scp_t *s, cell_t v) {
	cell_t *i = cons(s->block, v, (cell_t)s->rstack);
	s->rstack = i;
	s->rdepth++;
}

cell_t rpop(scp_t *s) {
	cell_t *i = s->rstack;
	cell_t v = CAR(i);
	s->rstack = (cell_t *)CDR(i);
	reclaim(s->block, i);
	s->rdepth--;
	return v;
}

void dump_stack(ctx_t *c) {
	printf("<%ld> ", c->scope->ddepth);
	if (c->scope->ddepth >= 1) { printf("T:%ld ", c->T); }
	if (c->scope->ddepth >= 2) { printf("S:%ld ", c->S); }
	cell_t *i = c->scope->dstack;
	while (i != NULL) {
		printf("[%p] %ld ", i, CAR(i));
		i = (cell_t *)CDR(i);
	}
	printf("\n");
}

void eval(ctx_t *c) {
	while(1) {
		switch(*c->PC) {
			case 'd': DUP(c); c->PC++; break;
			case '1': LIT(c, 1); c->PC++; break;
			case '>': GT(c); c->PC++; break;
			case '?': 
				if (c->T == 0) {
						c->T = c->S; c->S = pop(c->scope);
						while (*c->PC != '(') { c->PC++; }
				} else {
						c->T = c->S; c->S = pop(c->scope);
						c->PC++;
				}
				break;
			case '_': DEC(c); c->PC++; break;
			case '`': 
				rpush(c->scope, (cell_t)(c->PC + 1));
				while (*c->PC != ':') { c->PC--; }
				break;
			case 's': SWAP(c); c->PC++; break;
			case '+': ADD(c); c->PC++; break;
			case ';': 
				if (c->scope->rdepth > 0) {
					c->PC = ((char *)rpop(c->scope));
				} else {
					return;
				}
				break;
			default: c->PC++; break;
		}
	}
}
