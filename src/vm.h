#include<stdint.h>
#include<stddef.h>

// As in every Forth out there, cell is the basic datatype and its platform
// dependent. This implementation has been thought to be used in 64 and 32
// bit platforms.

typedef int64_t C;					// cell
typedef unsigned char B;		// byte

// Lexical scope. There can be multiple lexical scopes and they can extend
// other scopes.
// The use of linked lists for the stacks and the dictionary allows the 
// creation of closures by extending a stack with closure's own stack and a
// dictionary with closure's own dictionary of words.

// TODO: Where are scopes stored? There should be another entry in the
// header for the list of scopes owned by that block.

typedef struct {
	C* ds, * rs;							// data stack, return stack
	// TODO: Here a pointer to scope's dictionary is needed
} S;												// scope

typedef struct {
	C* bl;										// memory block
	C err;
	B* PC;										// Program Counter
	S* sc;										// current scope
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
#define SCOPES(b)						*(b + 5)
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

C reserve(C* bl, C np /* number of pairs of cells to reserve */) {
	if (((C*)LOWEST_ASSIGNED(bl)) - ((C*)FREE_TAIL(bl)) <= 2*np) return -1;

	FREE_TAIL(bl) = (C)((C*)FREE_TAIL(bl) + 2*np);
	CDR((C*)FREE_TAIL(bl)) = (C)NULL;

	return 0;
}

// Allocates bytes in groups of pairs of cells, as memory allocated is taken
// from the doubly linked list.

C allot(C* bl, C nb /* number of bytes to allocate */) {
	if (FREE_TAIL(bl) - HERE(bl) <= nb) {
		C ps = 2*sizeof(C); // Size in bytes of a pair of cells
		C req = nb - (FREE_TAIL(bl) - HERE(bl));
		C npairs = (req / ps) * ps < req ? req / ps + 1 : req / ps;
		if (reserve(bl, npairs) == -1) {
			return -1;
		}
	}

	HERE(bl) += nb;

	return 0;
}

// Construct a pair by taking one free pair from the doubly linked list.

C* cons(C* bl, C car, C cdr) {
	if ((C*)FREE_HEAD(bl) == NULL) return NULL;

	C* h = (C*)FREE_HEAD(bl);
	FREE_HEAD(bl) = (C)CDR(h);
	CAR((C*)FREE_HEAD(bl)) = (C)NULL;

	CAR(h) = car;
	CDR(h) = cdr;

	if (h < (C*)LOWEST_ASSIGNED(bl)) LOWEST_ASSIGNED(bl) = (C)h;

	return h;
}

// Return a pair to the free doubly linked list when its not in use anymore.

void reclaim(C* bl, C* p /* pair that its returned */) {
	C* h = (C*)FREE_HEAD(bl);
	FREE_HEAD(bl) = (C)p;
	CAR(h) = (C)p;
	CAR(p) = (C)NULL;
	CDR(p) = (C)h;
}

C length(C *l /* list */) {
	for (C a = 0;; a++) { if (!l) { return a; } else { l = (C*)CDR(l); } }
}

// Stack operations

void push(C* bl, S* sc, C v) {
	C* i = cons(bl, v, (C)sc->ds);
	sc->ds = i;
}

C pop(C* bl, S* sc) {
	C* i = sc->ds;
	C v = CAR(i);
	sc->ds = (C*)CDR(i);
	reclaim(bl, i);
	return v;
}

void rpush(C* bl, S* sc, C v) {
	C* i = cons(bl, v, (C)sc->rs);
	sc->rs = i;
}

C rpop(C* bl, S* sc) {
	C* i = sc->rs;
	C v = CAR(i);
	sc->rs = (C*)CDR(i);
	reclaim(bl, i);
	return v;
}

void dump_stack(X *c) {
	//printf("<%ld> ", c->sc->dd);
	//if (c->sc->dd >= 1) { printf("T:%ld ", c->T); }
	//if (c->sc->dd >= 2) { printf("S:%ld ", c->S); }
	printf("<%ld> ", length(c->sc->ds));
	C *i = c->sc->ds;
	while (i != NULL) {
		printf("[%p] %ld ", i, CAR(i));
		i = (C *)CDR(i);
	}
	printf("\n");
}

// By defining the primitives as macros we can use them both in the switch
// of eval and on tests directly without needing to evaluate.

#define DUP(c)			{ C x = pop(c->bl, c->sc); push(c->bl, c->sc, x); push(c->bl, c->sc, x); }
#define LIT(c, v)		push(c->bl, c->sc, v)
#define GT(c)				{ C x = pop(c->bl, c->sc); C y = pop(c->bl, c->sc); push(c->bl, c->sc, y > x); }
#define DEC(c)			push(c->bl, c->sc, pop(c->bl, c->sc) - 1)
#define SUB(c)			{ C x = pop(c->bl, c->sc); C y = pop(c->bl, c->sc); push(c->bl, c->sc, y - x); }
#define SWAP(c)			{ C x = pop(c->bl, c->sc); C y = pop(c->bl, c->sc); push(c->bl, c->sc, x); push(c->bl, c->sc, y); }
#define ADD(c)			{ C x = pop(c->bl, c->sc); C y = pop(c->bl, c->sc); push(c->bl, c->sc, y + x); }


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
				if (pop(c->bl, c->sc) == 0) {
						while (*c->PC != '(') { c->PC++; }
				} else {
						c->PC++;
				}
				break;
			case '_': DEC(c); c->PC++; break;
			case '`': 
				rpush(c->bl, c->sc, (C)(c->PC + 1));
				while (*c->PC != ':') { c->PC--; }
				break;
			case 's': SWAP(c); c->PC++; break;
			case '+': ADD(c); c->PC++; break;
			case ';': 
				if (c->sc->rs != NULL) {
					c->PC = ((char *)rpop(c->bl, c->sc));
				} else {
					return;
				}
				break;
			default: c->PC++; break;
		}
	}
}
