#include<stdint.h>
#include<stddef.h>
#include<string.h>

typedef uint8_t B;							// BYTE
typedef uint8_t D;							// DOUBLE BYTE
typedef int64_t C;							// CELL - 64 bit or 32 bits

#define IS_ALIGNED(a)		!(a & (sizeof(C) - 1))		
#define AS_CELLS(b)			(b <= 0 ? 0 : ((b - 1) / sizeof(C)) + 1)
#define AS_BYTES(c)			(c*sizeof(C))

typedef struct _P {
	struct _P* cdr;							
	C car;
} P;														// PAIR - or list item

typedef struct {
	D len;												// 65536 chars maximum
	B str[sizeof(C) - 2];					// padding to one cell
} S;														// BYTE ARRAY (String)

C equals(S* a, S* b) {
	// TODO: Compare full cells, not just chars
	if (*((C*)a) != *((C*)b)) return 0;
	for (C i = 6; i < a->len; a++) {
		if (a->str[i] != b->str[i]) return 0;
	}
	return 1;
}

typedef struct {
	P* cdr;
	C flags;
	S* name;
	S* source;
	S* code;
} W;														// DICTIONARY WORD

typedef struct {
	C size, err;									// size of block, status and error code
	B* ip, * here;								// instruction ptr, here ptr
	P* sp, * rp;									// stack ptr, return stack ptr
	W* dp;												// dictionary ptr
	P* ftail, * lowest, * fhead;	// head of free list, lowest assigned, tail
	B data[];											// start of data space
} H;														// BLOCK HEADER

#define FREE(bl)		((B*)(bl->ftail) - bl->here)

typedef int (*F)(H*);						// C FUNCTION

// Initializes the doubly linked list of free pairs of cells.

H* init(C* bl, C sz /* Size in cells of this block */) {
	if (sz % 2 != 0) return NULL;

	H* h = (H*)bl;
	h->size = sz;
	h->err = 0;
	P* p = h->ftail = (P*)(h->ip = h->here = &(h->data[0]));
	h->sp = h->rp = NULL;
	h->dp = NULL;

	while (p < (P*)(bl + sz)) {	p->cdr = p - 1;	p->car = (C)(p + 1); p++;	}
	h->lowest = h->fhead = p - 1;

	h->ftail->cdr = NULL;
	h->fhead->car = (C)NULL;

	return h;
}

// Allocates bytes in groups of pairs of cells, as memory allocated is taken
// from the doubly linked list.

C allot(H* bl, C nb /* number of bytes to allocate */) {
	// TODO: Allow negative numbers too
	// TODO: Extreme cases are not correctly tested
	while (FREE(bl) < nb) {
		if ((P*)(bl->ftail->car) - bl->ftail != 1 || (P*)(bl->ftail->car) == bl->lowest) {
			// Free pairs are not contiguous, it's not possible to allocate memory
			return -1;
		} else {
			bl->ftail = (P*)bl->ftail->car;
		}
	}
	bl->ftail->cdr = NULL;

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

#define PUSH(h, st, v)		st = cons(h, v, st)
#define POP(h, st)				P* p = st; C v = p->car; st = p->cdr; reclaim(h, p); return v;

void push(H* bl, C v) { PUSH(bl, bl->sp, v); }
C pop(H* bl) { POP(bl, bl->sp); }
void rpush(H* bl, C v) { PUSH(bl, bl->rp, v); }
C rpop(H* bl) { POP(bl, bl->rp); }




#define PRIMITIVE					1
#define CFUNC							2

#define IS(w, f)					(w->flags & f)

//void dump_stack(H* bl) {
//	printf("<%ld> ", length(bl->sp));
//	for (P* p = bl->sp; p != NULL; p = p->cdr) { printf("%ld ", p->car); }
//	printf("\n");
//}

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

void eval(H* bl, F* t) {
	while(1) {
		switch(*(bl->ip)) {
			case 'c': 
				NEXT(bl);
				t[*(bl->ip)](bl);
				NEXT(bl);
				break;
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

//void dump_string(S* s) {
//	printf("<%d>%s\n", s->len, s->str);
//}
//
//void dump_dict(H* bl) {
//	printf("Dictionary length: %ld\n", length(bl->dp));
//	W* w = (W*)bl->dp;
//	while (w != NULL) {
//		printf("@%p[%p]%ld|%p|%p|%p\n", w, w->cdr, w->flags, w->name, w->source, w->code);
//		printf("Flags::%ld\n", w->flags);
//		printf("%p Name::%s (length: %d)\n", w->name->str, w->name->str, w->name->len);
//		printf("Code::%s\n", w->code->str);
//		//if (IS(w, PRIMITIVE)) {	printf("PRIMITIVE "); }
//		//if (IS(w, CFUNC)) {	printf("CFUNC %d ", *(w->name.str + bytes(cells(w->name.len)) + 3)); }
//		w = (W*)w->cdr;
//		//printf("\n");
//	}
//}

int add_cfunc(H* bl, F* t, B idx, F func, B* name_str, B name_len) {
	// TODO: If any allot fails, go back to init state (here backwards)
	W* w = (W*)(bl->here);
	if (allot(bl, sizeof(W)) != 0) return -1;
	printf("space for word header: %ld\n", ((char*)bl->here) - ((char*)w));
	w->name = (S*)(bl->here);
	w->source = NULL;
	w->cdr = bl->dp;
	bl->dp = (P*)w;
	w->flags = CFUNC;
	if (allot(bl, AS_BYTES(AS_CELLS(2 + name_len + 1))) != 0) return -2;
	printf("space for name: %ld\n", ((char*)bl->here) - ((char*)w->name));
	w->code = (S*)(bl->here);
	w->name->len = name_len;
	for (C i = 0; i < AS_BYTES(AS_CELLS(2 + name_len + 1)) - 2; i++) {
		if (i < name_len) { w->name->str[i] = name_str[i]; }
		else { w->name->str[i] = 0; }
		printf("[%x] %c ", i, w->name->str[i]);
	}
	printf("\n1 name stored as:%s @ %p with len: %d\n", w->name->str, w->name, w->name->len);
	if (allot(bl, 8) != 0) return -3;
	printf("\n2 name stored as:%s @ %p with len: %d\n", w->name->str, w->name, w->name->len);
	printf("space for code: %ld\n", ((char*)bl->here) - ((char*)w->code));
	printf("\n3 name stored as:%s @ %p with len: %d\n", w->name->str, w->name, w->name->len);
	w->code->len = 3;
	printf("\n4 name stored as:%s @ %p with len: %d\n", w->name->str, w->name, w->name->len);
	w->code->str[0] = 'c';
	printf("\n5 name stored as:%s @ %p with len: %d\n", w->name->str, w->name, w->name->len);
	w->code->str[1] = idx;
	printf("\n6 name stored as:%s @ %p with len: %d\n", w->name->str, w->name, w->name->len);
	w->code->str[2] = ';';
	printf("\n7 name stored as:%s @ %p with len: %d\n", w->name->str, w->name, w->name->len);
	w->code->str[3] = 0;

	printf("\n8 name stored as:%s @ %p with len: %d\n", w->name->str, w->name, w->name->len);

	return 0;
}
