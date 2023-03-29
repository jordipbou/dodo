#ifndef __DODO_TYPES__
#define __DODO_TYPES__

typedef int8_t BYTE;
typedef intptr_t CELL;

enum DTypes { ATOM, LIST, PRIM, WORD };
enum STypes { LINKED };
enum WTypes { PL, CC, IMM, NDCS };

typedef struct {
	BYTE *ibuf, *here;
	CELL there, size;
	CELL fstack, dstack, rstack, xstack;
	CELL err, status, latest;
	CELL ipos, ilen;
	CELL free, ftotal, fmax;
} CTX;

typedef void (*FUNC)(CTX*);

#endif
