#include<stdint.h>
#include<stddef.h>

// As in every Forth out there, cell is the basic datatype and its platform
// dependent. This implementation has been thought to be used in 64 and 32
// bit platforms.

typedef int64_t C;					// cell
typedef unsigned char B;		// byte

// As oposed to LISP implementations, CDR is the first cell and CAR is the
// second cell. This way, list items can have a value with a size longer
// than one cell (as arrays).
// This has been done to reuse linked list code in dictionary code.

#define CAR(addr)		*(addr + 1)
#define CDR(addr)		*addr

// Lexical scope. There can be multiple lexical scopes and they can extend
// other scopes.
// The use of linked lists for the stacks and the dictionary allows the 
// creation of closures by extending a stack with closure's own stack and a
// dictionary with closure's own dictionary of words.

// An scope definition occupies 4 cells, the first one is the CDR that links
// to next available scope.

#define DS(addr)		*(addr + 1)		// data stack
#define RS(addr)		*(addr + 2)		// return stack
#define DT(addr)		*(addr + 3)		// dictionary
#define SCOPE_SIZE	4

// The block header stores all the info needed to manage memory on that block.
// A block includes in itself all the info that represents a computation at
// any moment in time.

#define SIZE(b)							*b
#define FREE_HEAD(b)				*(b + 1)
#define LOWEST_ASSIGNED(b)	*(b + 2)
#define FREE_TAIL(b)				*(b + 3)
#define HERE(b)							*(b + 4)
#define SCOPE(b)						*(b + 5)
#define ERR(b)							*(b + 6)
#define PC(b)								*(b + 7)
#define HEADER_SIZE					8

// Block initialization. 
// Initializes the doubly linked list of free pairs of cells and the list of
// scopes.
// For each free pair, CAR represents previous item and CDR next item.

C* init_bl(C* bl, C sz) {
	if ((SIZE(bl) = sz) % 2 != 0) return NULL;

	for (
		C* p = (C*)(HERE(bl) = FREE_TAIL(bl) = (C)(bl + HEADER_SIZE));
		p <= (C*)(LOWEST_ASSIGNED(bl) = FREE_HEAD(bl) = (C)(bl + sz - SCOPE_SIZE - 2));
		p += 2
	) {
			CDR(p) = (C)(p - 2);
			CAR(p) = (C)(p + 2);
	}

	CDR((C*)FREE_TAIL(bl)) = (C)NULL;
	CAR((C*)FREE_HEAD(bl)) = (C)NULL;

	SCOPE(bl) = (C)(bl + sz - SCOPE_SIZE);
	CDR((C*)SCOPE(bl)) = (C)NULL;
	DS((C*)SCOPE(bl)) = (C)NULL;
	RS((C*)SCOPE(bl)) = (C)NULL;
	DT((C*)SCOPE(bl)) = (C)NULL;


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

void push(C* bl, C v) {
	C* i = cons(bl, v, (C)DS((C*)SCOPE(bl)));
	DS((C*)SCOPE(bl)) = (C)i;
}

C pop(C* bl) {
	C* i = (C*)DS((C*)SCOPE(bl));
	C v = CAR(i);
	DS((C*)SCOPE(bl)) = CDR(i);
	reclaim(bl, i);
	return v;
}

void rpush(C* bl, C v) {
	C* i = cons(bl, v, (C)RS((C*)SCOPE(bl)));
	RS((C*)SCOPE(bl)) = (C)i;
}

C rpop(C* bl) {
	C* i = (C*)RS((C*)SCOPE(bl));
	C v = CAR(i);
	RS((C*)SCOPE(bl)) = CDR(i);
	reclaim(bl, i);
	return v;
}

void dump_stack(C* bl) {
	printf("<%ld> ", length((C*)DS((C*)SCOPE(bl))));
	C* i = (C*)DS((C*)SCOPE(bl));
	while (i != NULL) {
		printf("[%p] %ld ", i, CAR(i));
		i = (C*)CDR(i);
	}
	printf("\n");
}

// By defining the primitives as macros we can use them both in the switch
// of eval and on tests directly without needing to evaluate.

#define DUP(bl)			{ C x = pop(bl); push(bl, x); push(bl, x); }
#define LIT(bl, v)	push(bl, v)
#define GT(bl)			{ C x = pop(bl); C y = pop(bl); push(bl, y > x); }
#define DEC(bl)			push(bl, pop(bl) - 1)
#define SUB(bl)			{ C x = pop(bl); C y = pop(bl); push(bl, y - x); }
#define SWAP(bl)		{ C x = pop(bl); C y = pop(bl); push(bl, x); push(bl, y); }
#define ADD(bl)			{ C x = pop(bl); C y = pop(bl); push(bl, y + x); }

// Current implementation is that of a bytecode interpreter. Only ASCII 
// graphical characters are used, and if possible, the character represents
// the primitive.

#define NEXT(bl)		PC(bl) = PC(bl) + 1
#define PREV(bl)		PC(bl) = PC(bl) - 1

void eval(C* bl) {
	while(1) {
		switch(*((B*)PC(bl))) {
			case 'd': DUP(bl); NEXT(bl); break;
			case '1': LIT(bl, 1); NEXT(bl); break;
			case '>': GT(bl); NEXT(bl); break;
			case '?': 
				if (pop(bl) == 0) {
						while (*((B*)PC(bl)) != '(') { NEXT(bl); }
				} else {
						NEXT(bl);
				}
				break;
			case '_': DEC(bl); NEXT(bl); break;
			case '`': 
				rpush(bl, (C)(PC(bl) + 1));
				while (*((B*)PC(bl)) != ':') { PREV(bl); }
				break;
			case 's': SWAP(bl); NEXT(bl); break;
			case '+': ADD(bl); NEXT(bl); break;
			case ';': 
				if ((C*)RS((C*)SCOPE(bl)) != NULL) {
					PC(bl) = rpop(bl);
				} else {
					return;
				}
				break;
			default: NEXT(bl); break;
		}
	}
}
