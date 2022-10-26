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

#define CAR(addr)		*(((C*)addr) + 1)
#define CDR(addr)		*((C*)addr)

// The block header stores all the info needed to manage memory on that block.
// A block includes in itself all the info that represents a computation at
// any moment in time.

#define DS(b)				*(b + 0)			// data stack
#define DICT(b)			*(b + 1)			// dictionary
#define RS(b)				*(b + 2)			// retur stack (only one, scopes don't have)
#define SZ(b)				*(b	+ 3)			// size of block
#define FH(b)				*(b + 4)			// head of free doubly linked list
#define LA(b)				*(b + 5)			// lowest (in memory) assigned pair
#define FT(b)				*(b + 6)			// tail of free doubly linked list
#define HERE(b)			*(b + 7)			// end of allocated contiguous memory
#define ERR(b)			*(b + 8)			// context status or error code
#define PC(b)				*(b + 9)			// program counter / instruction pointer
#define Hsz					10	

// Block initialization. 
// Initializes the doubly linked list of free pairs of cells and the list of
// scopes.
// For each free pair, CAR represents previous item and CDR next item.

C* init_bl(C* bl, C sz) {
	if ((SZ(bl) = sz) % 2 != 0) return NULL;

	for (
		C* p = (C*)(HERE(bl) = FT(bl) = (C)(bl + Hsz));
		p <= (C*)(LA(bl) = FH(bl) = (C)(bl + sz - 2));
		p += 2
	) {
			CDR(p) = (C)(p - 2);
			CAR(p) = (C)(p + 2);
	}

	CDR(FT(bl)) = CAR(FH(bl)) = (C)NULL;
	DS(bl) = DICT(bl) = RS(bl) = (C)NULL;

	return bl;
}

// To allow use of arrays and contiguous memory, memory has to be reserved
// in the block (removing free pairs from the doubly linked list).

C reserve(C* bl, C np /* number of pairs of cells to reserve */) {
	if (((C*)LA(bl)) - ((C*)FT(bl)) <= 2*np) return -1;

	FT(bl) = (C)((C*)FT(bl) + 2*np);
	CDR(FT(bl)) = (C)NULL;

	return 0;
}

// Allocates bytes in groups of pairs of cells, as memory allocated is taken
// from the doubly linked list.

C allot(C* bl, C nb /* number of bytes to allocate */) {
	if (FT(bl) - HERE(bl) <= nb) {
		C ps = 2*sizeof(C); // Size in bytes of a pair of cells
		C req = nb - (FT(bl) - HERE(bl));
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
	if ((C*)FH(bl) == NULL) return NULL;

	C* h = (C*)FH(bl);
	FH(bl) = (C)CDR(h);
	CAR(FH(bl)) = (C)NULL;

	CAR(h) = car;
	CDR(h) = cdr;

	if (h < (C*)LA(bl)) LA(bl) = (C)h;

	return h;
}

// Return a pair to the free doubly linked list when its not in use anymore.

void reclaim(C* bl, C* p /* pair that its returned */) {
	C* h = (C*)FH(bl);
	FH(bl) = (C)p;
	CAR(h) = (C)p;
	CAR(p) = (C)NULL;
	CDR(p) = (C)h;
}

C length(C *l /* list */) {
	for (C a = 0;; a++) { if (!l) { return a; } else { l = (C*)CDR(l); } }
}

// Stack operations

void push(C* bl, C v) {
	C* i = cons(bl, v, (C)DS(bl));
	DS(bl) = (C)i;
}

C pop(C* bl) {
	C* i = (C*)DS(bl);
	C v = CAR(i);
	DS(bl) = CDR(i);
	reclaim(bl, i);
	return v;
}

void rpush(C* bl, C v) {
	C* i = cons(bl, v, (C)RS(bl));
	RS(bl) = (C)i;
}

C rpop(C* bl) {
	C* i = (C*)RS(bl);
	C v = CAR(i);
	RS(bl) = CDR(i);
	reclaim(bl, i);
	return v;
}

void dump_stack(C* bl) {
	printf("<%ld> ", length((C*)DS(bl)));
	C* i = (C*)DS(bl);
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
				if ((C*)RS(bl) != NULL) {
					PC(bl) = rpop(bl);
				} else {
					return;
				}
				break;
			default: NEXT(bl); break;
		}
	}
}
