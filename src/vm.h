#include<stdint.h>
#include<stddef.h>

// As in every Forth out there, cell is the basic datatype and its platform
// dependent. This implementation has been thought to be used in 64 and 32
// bit platforms.

typedef unsigned char B;		// byte
typedef int64_t C;					// cell

// To reuse linked list code in dictionary, and as oposed to LISP 
// implementations, CDR is the first cell and CAR is the second cell, allowing
// extending CAR to sizes other than cell in memory.

typedef struct _P {
	struct _P* cdr;							
	C car;
} P;														// pair

typedef struct {
	C size, err;									// size of block, status and error code
	B* ip, * here;								// instruction ptr, here ptr
	P* sp, * rp, * dp;						// stack ptr, return stack ptr, dictionary ptr
	P* ftail, * lowest, * fhead;	// head of free list, lowest assigned, tail
} H;														// block header

#define szPAIR			sizeof(P) / sizeof(C)	
#define szHEADER		sizeof(H) / sizeof(C)

// Initializes the doubly linked list of free pairs of cells.

H* init_bl(C* bl, C sz /* Size in cells of this block */) {
	if (sz % 2 != 0) return NULL;

	H* h = (H*)bl;
	h->size = sz;
	h->err = 0;
	P* p = h->ftail = (P*)(h->ip = h->here = (B*)(bl + szHEADER));
	h->sp = h->rp = h->dp = NULL;

	while (p < (P*)(bl + sz)) {	p->cdr = p - 1;	p->car = (C)(p + 1); p++;	}
	h->lowest = h->fhead = p - 1;

	h->ftail->cdr = NULL;
	h->fhead->car = (C)NULL;

	return h;
}

// To allow use of arrays and contiguous memory, memory has to be reserved
// in the block (removing free pairs from the doubly linked list).

C reserve(H* bl, C np /* number of pairs of cells to reserve */) {
	if ((C*)bl->lowest - (C*)bl->ftail <= 2*np) return -1;

	bl->ftail = bl->ftail + np;
	bl->ftail->cdr = NULL;

	return 0;
}

// Allocates bytes in groups of pairs of cells, as memory allocated is taken
// from the doubly linked list.

C allot(H* bl, C nb /* number of bytes to allocate */) {
	if ((B*)bl->ftail - bl->here <= nb) {
		C req = nb - (C)((B*)bl->ftail - bl->here);
		C npairs = (req / szPAIR) * szPAIR < req ? req / szPAIR + 1 : req / szPAIR;
		if (reserve(bl, npairs) == -1) {
			return -1;
		}
	}

	bl->here += nb;

	return 0;
}

// Construct a pair by taking one free pair from the doubly linked list.

P* cons(H* bl, C car, P* cdr) {
	if (bl->fhead == NULL) return NULL;

	P* p = bl->fhead;
	bl->fhead = p->cdr;
	bl->fhead->car = (C)NULL;

	p->car = car;
	p->cdr = cdr;

	if (p < bl->lowest) bl->lowest = p;

	return p;
}

// Return a pair to the free doubly linked list when its not in use anymore.

void reclaim(H* bl, P* p /* pair that its returned */) {
	p->car = (C)NULL;
	p->cdr = bl->fhead;
	bl->fhead->car = (C)p;
	bl->fhead = p;
}

C length(P *l) {
	for (C a = 0;; a++) { if (!l) { return a; } else { l = l->cdr; } }
}

// Stack operations

void push(H* bl, C v) {
	bl->sp = cons(bl, v, bl->sp);
}

C pop(H* bl) {
	P* p = bl->sp;
	C v = p->car;
	bl->sp = p->cdr;
	reclaim(bl, p);
	return v;
}

void rpush(H* bl, C v) {
	bl->rp = cons(bl, v, bl->rp);
}

C rpop(H* bl) {
	P* p = bl->rp;
	C v = p->car;
	bl->rp = p->cdr;
	reclaim(bl, p);
	return v;
}

void dump_stack(H* bl) {
	printf("<%ld> ", length(bl->sp));
	for (P* p = bl->sp; p != NULL; p = p->cdr) { printf("%ld ", p->car); }
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

#define NEXT(bl)		bl->ip++
#define PREV(bl)		bl->ip--

void eval(H* bl) {
	while(1) {
		switch(*(bl->ip)) {
			case 'd': DUP(bl); NEXT(bl); break;
			case '1': LIT(bl, 1); NEXT(bl); break;
			case '>': GT(bl); NEXT(bl); break;
			case '?': 
				if (pop(bl) == 0) {
						while (*(bl->ip) != '(') { NEXT(bl); }
				} else {
						NEXT(bl);
				}
				break;
			case '_': DEC(bl); NEXT(bl); break;
			case '`': 
				rpush(bl, (C)(bl->ip + 1));
				while (*(bl->ip) != ':') { PREV(bl); }
				break;
			case 's': SWAP(bl); NEXT(bl); break;
			case '+': ADD(bl); NEXT(bl); break;
			case ';': 
				if (bl->rp != NULL) {
					bl->ip = (B*)rpop(bl);
				} else {
					return;
				}
				break;
			default: NEXT(bl); break;
		}
	}
}
