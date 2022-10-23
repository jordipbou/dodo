#include<stdint.h>
#include<stddef.h>

// As in every Forth out there, cell is the basic datatype and its platform
// dependent. 

typedef int64_t C;					// cell
typedef unsigned char B;		// byte

typedef struct {
	C* bl, * ds, * rs;				// block, data stack, return stack
	C dd, rd;									// stacks depth
} S;												// sc

typedef struct {
	C err, T, S, R;
	B* PC;
	S* sc;
} X;												// context

// As oposed to LISP implementations, CDR is the first cell and CAR is the
// second cell. This way, list items can have a value with a size longer
// than one cell (as arrays).
// This has been done to reuse linked list code in dictionary code.

#define CAR(addr)		*(addr + 1)
#define CDR(addr)		*addr

// The block header stores all the info needed to manage memory on that block.

#define SIZE(b)							*b
#define FREE_HEAD(b)				*(b + 1)
#define LOWEST_ASSIGNED(b)	*(b + 2)
#define FREE_TAIL(b)				*(b + 3)
#define HERE(b)							*(b + 4)
#define HEADER_SIZE					6

// Block initialization. The doubly linked list that holds all free cells
// is initialized here.
// For each free pair, CAR represents previous item and CDR next item.

C* init_bl(C* bl, C sz) {
	if ((SIZE(bl) = sz) % 2 != 0) return NULL;

	for (
		C* p = (C*)(HERE(bl) = FREE_TAIL(bl) = (C)(bl + HEADER_SIZE));
		p <= (C*)(LOWEST_ASSIGNED(bl) = FREE_HEAD(bl) = (C)(bl + sz - 2));
		p += 2
	) {
			CDR(p) = (C)(p - 2);
			CAR(p) = (C)(p + 2);
	}

	CDR((C*)FREE_TAIL(bl)) = (C)NULL;
	CAR((C*)FREE_HEAD(bl)) = (C)NULL;

	return bl;
}

// To allow use of arrays and contiguous memory, memory has to be reserved
// in the block (removing free pairs from the doubly linked list).

C reserve(C* b, C npairs) {
	if (((C*)LOWEST_ASSIGNED(b)) - ((C*)FREE_TAIL(b)) <= 2*npairs) return -1;

	FREE_TAIL(b) = (C)((C*)FREE_TAIL(b) + 2*npairs);
	CDR((C*)FREE_TAIL(b)) = (C)NULL;

	return 0;
}

// Allocates bytes in groups of pairs of cells, as memory allocated is taken
// from the doubly linked list.

C allot(C* b, C nbytes) {
	if (FREE_TAIL(b) - HERE(b) <= nbytes) {
		C psize = 2*sizeof(C);
		C req = nbytes - (FREE_TAIL(b) - HERE(b));
		C npairs = (req / psize) * psize < req ? req / psize + 1 : req / psize;
		if (reserve(b, npairs) == -1) {
			return -1;
		}
	}

	HERE(b) += nbytes;

	return 0;
}

// Construct a pair by taking one free pair from the doubly linked list.

C *cons(C *b, C car, C cdr) {
	if ((C *)FREE_HEAD(b) == NULL) return NULL;

	C *h = (C *)FREE_HEAD(b);
	FREE_HEAD(b) = (C)CDR(h);
	CAR((C *)FREE_HEAD(b)) = (C)NULL;

	CAR(h) = car;
	CDR(h) = cdr;

	if (h < (C *)LOWEST_ASSIGNED(b)) LOWEST_ASSIGNED(b) = (C)h;

	return h;
}

// Return a pair to the free doubly linked list when its not in use anymore.

void reclaim(C *b, C *i) {
	C *h = (C *)FREE_HEAD(b);
	FREE_HEAD(b) = (C)i;
	CAR(h) = (C)i;
	CAR(i) = (C)NULL;
	CDR(i) = (C)h;
}

C length(C *l) {
	for (C a = 0;; a++) { if (!l) { return a; } else { l = (C *)CDR(l); } }
}

// Stack operations

void push(S *s, C v) {
	C *i = cons(s->bl, v, (C)s->ds);
	s->ds = i;
	s->dd++;
}

C pop(S *s) {
	C *i = s->ds;
	C v = CAR(i);
	s->ds = (C *)CDR(i);
	reclaim(s->bl, i);
	s->dd--;
	return v;
}

void rpush(S *s, C v) {
	C *i = cons(s->bl, v, (C)s->rs);
	s->rs = i;
	s->rd++;
}

C rpop(S *s) {
	C *i = s->rs;
	C v = CAR(i);
	s->rs = (C *)CDR(i);
	reclaim(s->bl, i);
	s->rd--;
	return v;
}

void dump_stack(X *c) {
	printf("<%ld> ", c->sc->dd);
	if (c->sc->dd >= 1) { printf("T:%ld ", c->T); }
	if (c->sc->dd >= 2) { printf("S:%ld ", c->S); }
	C *i = c->sc->ds;
	while (i != NULL) {
		printf("[%p] %ld ", i, CAR(i));
		i = (C *)CDR(i);
	}
	printf("\n");
}

// By defining the primitives as macros we can use them both in the switch
// of eval and on tests directly without needing to evaluate.

#define DUP(c)			push(c->sc, c->S); c->S = c->T
#define LIT(c, v)		push(c->sc, c->S); c->S = c->T; c->T = v
#define GT(c)				c->T = c->S > c->T; c->S = pop(c->sc)
#define DEC(c)			c->T = c->T - 1
#define SUB(c)			c->T = c->S - c->T; c->S = pop(c->sc)
#define SWAP(c)			{ C x = c->T; c->T = c->S; c->S = x; }
#define ADD(c)			c->T = c->S + c->T; c->S = pop(c->sc)

// Current implementation is that of a bytecode interpreter. Only ASCII 
// graphical characters are used, and if possible, the character represents
// the primitive.

void eval(X *c) {
	while(1) {
		switch(*c->PC) {
			case 'd': DUP(c); c->PC++; break;
			case '1': LIT(c, 1); c->PC++; break;
			case '>': GT(c); c->PC++; break;
			case '?': 
				if (c->T == 0) {
						c->T = c->S; c->S = pop(c->sc);
						while (*c->PC != '(') { c->PC++; }
				} else {
						c->T = c->S; c->S = pop(c->sc);
						c->PC++;
				}
				break;
			case '_': DEC(c); c->PC++; break;
			case '`': 
				rpush(c->sc, (C)(c->PC + 1));
				while (*c->PC != ':') { c->PC--; }
				break;
			case 's': SWAP(c); c->PC++; break;
			case '+': ADD(c); c->PC++; break;
			case ';': 
				if (c->sc->rd > 0) {
					c->PC = ((char *)rpop(c->sc));
				} else {
					return;
				}
				break;
			default: c->PC++; break;
		}
	}
}
