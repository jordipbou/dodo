#include<stdint.h>
#include<stddef.h>
#include<string.h>
#include<unistd.h>
#include<sys/mman.h>
#include<errno.h>

typedef uint8_t B;							// BYTE
typedef int64_t C;							// CELL

#define IS_ALIGNED(a)		!(a & (sizeof(C) - 1))		
#define AS_CELLS(b)			(b <= 0 ? 0 : ((b - 1) / sizeof(C)) + 1)
#define AS_BYTES(c)			(c*sizeof(C))

typedef struct _P {
	struct _P* cdr;							
	C car;
} P;														// PAIR - or list item

C length(P* l) {
	for (C a = 0;; a++) { if (!l) { return a; } else { l = l->cdr; } }
}

typedef struct {
	C len;												// length
	B str[8];											// at least one cell of string data
} S;														// COUNTED BYTE ARRAY (String)

typedef struct {
	C len;
	B* code;
} D;

typedef struct {
	P* cdr;
	C flags;
	S* name;											
	S* source;										
	D* code;											
} W;														// DICTIONARY WORD

W* find(W* word, B* name) {
	while (word != NULL && strcmp(word->name->str, name)) {
		word = (W*)word->cdr;
	}

	return word;
}

typedef struct {
	C err, st;										// status and error codes
	C dsize, csize;								// size of block, status and error code
	B* ip, * here, * chere;				// instruction ptr (RIP), here ptr
	P* sp, * rp;									// stack ptr, return stack ptr
	W* dp;												// dictionary ptr
	P* ftail, * lowest, * fhead;	// head of free list, lowest assigned, tail
	B* code;											// pointer to code space
	B data[];											// start of data space
} H;														// BLOCK HEADER

#define FREE(bl)		((B*)(bl->ftail) - bl->here)
#define PAGESIZE	sysconf(_SC_PAGESIZE)

H* init(C dsz /* Data size in bytes */, C csz /* Code size in bytes*/) {
	dsz = (dsz + (sizeof(P)-1)) & ~(sizeof(P)-1);
	csz = (csz + (PAGESIZE-1)) & ~(PAGESIZE-1);	

	// Allocate memory for data
	H* h = malloc(dsz);
	if (h == NULL) {
		return (H*)-1;
	}

	// Header data
	h->dsize = dsz;
	h->csize = csz;
	h->st = h->err = 0;
	P* p = h->ftail = (P*)(h->ip = h->here = &(h->data[0]));
	h->sp = h->rp = NULL;
	h->dp = NULL;

	// Initializes doubly linked list of free pairs
	while (p < (P*)(((C*)h) + dsz)) { p->cdr = p - 1; p->car = (C)(p + 1); p++;	}
	h->lowest = h->fhead = p - 1;
	h->ftail->cdr = NULL;
	h->fhead->car = (C)NULL;

	// Allocate executable memory for code
	h->code = mmap(NULL, csz, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS,	-1, 0);
	if (h->code == (void *)-1) {
		free(h);
		return (H*)-2;
	}
	h->chere = h->code;

	return h;
}

void deinit(H* h) {
	munmap(h->code, h->csize);
	free(h);
}

// Allocates bytes in groups of pairs of cells, as memory allocated is taken
// from the doubly linked list.

C allot(H* bl, C nb /* number of bytes to allocate */) {
	// TODO: Allow negative numbers too
	// TODO: Extreme cases are not correctly tested
	while (FREE(bl) < nb) {
		if ((P*)(bl->ftail->car) - bl->ftail != 1 || (P*)(bl->ftail->car) == bl->lowest) {
			// Not enough contiguous free pairs
			return -1;
		} else {
			// TODO: This should be done in a temporary variable to keep things like
			// they were if not enough memory to allocate
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

S* compile_str(H* bl, B* str) {
	C len = strlen(str);
	// TODO: Check alignment?
	S* s = (S*)(bl->here);
	if (allot(bl, sizeof(C) + AS_CELLS(len))) {
		return NULL;
	} else {
		s->len = len;
		memset(s->str, 0, AS_CELLS(len));
		memcpy(s->str, str, len);
	}

	return s;
}

D* compile_code(H* bl, B* code, C len) {
	D* d = (D*)(bl->here);
	B* chere = bl->chere;
	if (allot(bl, 2*sizeof(C))) {
		return NULL;
	} else {
		if (chere + len >= bl->code + bl->csize) return NULL;
		if (mprotect(bl->code, bl->csize, PROT_WRITE)) return NULL;
		memcpy(chere, code, len);
		if (mprotect(bl->code, bl->csize, PROT_READ|PROT_EXEC)) return NULL;
		d->len = len;
		d->code = chere;
		bl->chere += len;
	}
	return d;
}

int append_ripret(H* bl, D* d) {
	B* chere = bl->chere;
	if (mprotect(bl->code, bl->csize, PROT_WRITE)) return -1;
	chere[0] = 0x48; chere[1] = 0xb8;
	// Save chere + 11 
	C* c = (C*)(chere + 2);
	*c = (C)(chere + 11);
	chere[10] = 0xC3;
	if (mprotect(bl->code, bl->csize, PROT_READ|PROT_EXEC)) return -1;
	d->len += 11;
	bl->chere += 11;
	return 0;
}

// TODO: Append code to compilation...maybe start compilation/end compilation

W* compile_word(H* bl, B* name, B* source, B* code, C code_len, C flags) {
	W* w = (W*)(bl->here);
	if (allot(bl, sizeof(W))) {
		return NULL;
	}
	w->flags = flags;
	w->name = compile_str(bl, name);
	w->source = compile_str(bl, source);
	w->code = compile_code(bl, code, code_len);

	return w;
}

#define PUSH(h, st, v)		st = cons(h, v, st)
#define POP(h, st)				P* p = st; C v = p->car; st = p->cdr; reclaim(h, p); return v;

void push(H* bl, C v) { PUSH(bl, bl->sp, v); }
C pop(H* bl) { POP(bl, bl->sp); }
void rpush(H* bl, C v) { PUSH(bl, bl->rp, v); }
C rpop(H* bl) { POP(bl, bl->rp); }

typedef int (*F)(H*);						// C FUNCTION

void exec(H* bl, B* word) {
	W* w = find(bl->dp, word);
	while(1) {
		// TODO: Calling convention on Linux and Windows x86_64 are different
		// and use different registers for arguments
		// One option is to change assembler to use different registers,
		// the other option is change how assembler is called (parameters passed)
		// to use the same assembler everywhere (Linux and Windows).
		int res = ((int (*)(H*))(w->code->code))(bl);
		if (res == 0) {
			// TODO: Error code, check block for error type (or exit)
		} else {
			// TODO: Call C function
		}
	}
}



#define PRIMITIVE					1
#define CFUNC							2

#define IS(w, f)					(w->flags & f)

//// By defining the primitives as macros we can use them both in the switch
//// of eval and on tests directly without needing to evaluate.
//
//#define DUP(bl)			{ C x = pop(bl); push(bl, x); push(bl, x); }
//#define LIT(bl, v)	push(bl, v)
//#define GT(bl)			{ C x = pop(bl); C y = pop(bl); push(bl, y > x); }
//#define DEC(bl)			push(bl, pop(bl) - 1)
//#define SUB(bl)			{ C x = pop(bl); C y = pop(bl); push(bl, y - x); }
//#define SWAP(bl)		{ C x = pop(bl); C y = pop(bl); push(bl, x); push(bl, y); }
//#define ADD(bl)			{ C x = pop(bl); C y = pop(bl); push(bl, y + x); }
//
//// Current implementation is that of a bytecode interpreter. Only ASCII 
//// graphical characters are used, and if possible, the character represents
//// the primitive.
//
//#define NEXT(bl)		bl->ip++
//#define PREV(bl)		bl->ip--
//
//void eval(H* bl, F* t) {
//	while(1) {
//		switch(*(bl->ip)) {
//			case 'c': 
//				NEXT(bl);
//				t[*(bl->ip)](bl);
//				NEXT(bl);
//				break;
//			case 'd': DUP(bl); NEXT(bl); break;
//			case '1': LIT(bl, 1); NEXT(bl); break;
//			case '>': GT(bl); NEXT(bl); break;
//			case '?': 
//				if (pop(bl) == 0) {
//						while (*(bl->ip) != '(') { NEXT(bl); }
//				} else {
//						NEXT(bl);
//				}
//				break;
//			case '_': DEC(bl); NEXT(bl); break;
//			case '`': 
//				rpush(bl, (C)(bl->ip + 1));
//				while (*(bl->ip) != ':') { PREV(bl); }
//				break;
//			case 's': SWAP(bl); NEXT(bl); break;
//			case '+': ADD(bl); NEXT(bl); break;
//			case ';': 
//				if (bl->rp != NULL) {
//					bl->ip = (B*)rpop(bl);
//				} else {
//					return;
//				}
//				break;
//			default: NEXT(bl); break;
//		}
//	}
//}

//int compile_str(H* bl, B* str, B len) {
//	S* s = (S*)(bl->here);
//	if (allot(bl, AS_BYTES(AS_CELLS(2 + len + 1))) != 0) return -1;
//	s->len = len;
//	for (C i = 0; i < AS_BYTES(AS_CELLS(2 + len + 1)) - 2; i++) {
//		if (i < len) { s->str[i] = str[i]; }
//		else { s->str[i] = 0; }
//	}
//
//	return 0;
//}
//
//int compile_code(H* bl, B* code, B len) {
//	B* dest = (B*)(bl->here);
//	if (allot(bl, AS_BYTES(AS_CELLS(len))) != 0) return -1;
//	for (C i = 0; i < AS_BYTES(AS_CELLS(len)); i++, dest++) {
//		if (i < len) { *dest = code[i]; }
//		else { *dest = 0; }
//	}
//
//	return 0;
//}
//
//int add_word(H* bl, B* name, B nlen, B* code, B clen) {
//	W* w = (W*)(bl->here);
//	if (allot(bl, sizeof(W)) != 0) { bl->here = (B*)w; return -1; }
//	w->flags = PRIMITIVE | CFUNC;
//
//	w->name = (B*)(bl->here);
//	if (compile_str(bl, name, nlen) != 0) { bl->here = (B*)w; return -2; }
//
//	w->source = NULL;
//
//	w->code = (B*)(bl->here);
//	if (compile_code(bl, code, clen) != 0) { bl->here = (B*)w; return -3; }
//
//	w->cdr = bl->dp;
//	bl->dp = (P*)w;
//
//	return 0;
//}
//
//int add_cfunc(H* bl, F* t, B idx, F func, B* name, B nlen) {
//	B code[] = { 
//		0xb8, 0x00, 0x00, 0x00,			// mov eax,0
//		0xc3												// ret
//	};
//
//	code[1] = idx;
//	t[idx] = func;
//
//	return add_word(bl, name, nlen, code, 5);
//}
//
//void call(H* bl, F* t, S* name) {
//	W* w = bl->dp;
//	while (!equals(w->name, name)) {
//		w = (W*)(w->cdr);
//	}
//
//	if (equals(w->name, name)) {
//		B* c = w->code;
//		//int res = ((int (*)(void))c)();
//		//printf("Res: %d\n", res);
//		//t[res](bl);
//	}
//}
