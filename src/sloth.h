#ifndef SLOTH
#define SLOTH

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>


/*

SLOTH is the VM that powers the DODO programming language. 

Bytecode VM with human readable opcodes (as seen on STABLE forth https://w3group.de/stable.html).

*/

/*
CODE 
DATA
MAIN stack / DATA stack
RETURN stack
*/

typedef int8_t BYTE;
typedef intptr_t CELL;

enum { LIST, INT, INT_ARRAY, BYTE_ARRAY, FLOAT, FLOAT_ARRAY, CHAR, STRING } TYPE;

/* TODO: Cells for type, size and length seems too much wasted space! */
typedef struct { /* CELL rc */ CELL type, size, length; CELL data[1]; } OBJECT;

/* TODO: Cells for name_length and flags seems too much wasted space! */
typedef struct { CELL name_length, flags; BYTE* code; BYTE* data; BYTE name[1]; } WORD;

struct CONTEXT;
typedef void (*FUNC)(struct CONTEXT*);

typedef struct {
	OBJECT** dstack;
	OBJECT* mstack;
	OBJECT* rstack;
	OBJECT* code;
	OBJECT* data;
	OBJECT* latest;
	FUNC* ip;
} CONTEXT;

#define S(x)				(*x->dstack)
#define TOS(x)			(S(x)->data[S(x)->length - 1])
#define NOS(x)			(S(x)->data[S(x)->length - 2])

#define PUSH(x, i)	(S(x)->data[S(x)->length++] = i)
#define POP(x)			(S(x)->data[--S(x)->length])

#define PUSHR(x, i)	(x->rstack->data[x->rstack->length++] = (CELL)i)
#define POPR(x)			(x->rstack->data[--x->rstack->length])

OBJECT* newV(CELL type, CELL size) {
	CELL array_size;
	OBJECT* vector;

	switch (type) {
		case LIST:
		case INT:
		case INT_ARRAY:
		case FLOAT:
		case FLOAT_ARRAY:
			vector = calloc(size + 3, sizeof(CELL));
			break;
		case CHAR:
		case STRING:
			vector = calloc(size + 3*sizeof(OBJECT), sizeof(BYTE));
			break;
	}

	vector->type = type;
	vector->size = size;
	vector->length = 0;

	return vector;
}

CONTEXT* init() {
	CONTEXT* context = malloc(sizeof(CONTEXT));

	context->mstack = newV(INT_ARRAY, 256);
	context->rstack = newV(INT_ARRAY, 256);
	context->dstack = &context->mstack;

	context->code = newV(INT, 65536);
	context->data = newV(BYTE_ARRAY, 65536);

	context->ip = 0;
	context->latest = 0;

	return context;
}

void one(CONTEXT* x) { PUSH(x, 1); }
void add(CONTEXT* x) { CELL a = POP(x); CELL b = POP(x); PUSH(x, b + a); }
void sub(CONTEXT* x) { CELL a = POP(x); CELL b = POP(x); PUSH(x, b - a); }
void gt(CONTEXT* x) { CELL a = POP(x); CELL b = POP(x); PUSH(x, b > a); }
void swap(CONTEXT* x) { CELL a = POP(x); CELL b = POP(x); PUSH(x, a); PUSH(x, b); }
void dup(CONTEXT* x) { CELL a = POP(x); PUSH(x, a); PUSH(x, a); }
void recurse(CONTEXT* x) { PUSHR(x, x->ip); x->ip = (FUNC)x->code->data; x->ip--; }
void ret(CONTEXT* x) { x->ip = POPR(x); }
void when(CONTEXT* x) { if (!POP(x)) { ret(x); } }

CELL* stepI(CONTEXT* x) {
	while (x->ip != 0) {
		/* This has the problem of functions that manipulate ip */
		/* Also colon definitions are not the same as C primitives !! */
		(*x->ip)(x);	
	}
	x->ip++;
	return x->ip;
}

void dump_stack(CONTEXT* x) {
	CELL i;

	printf("\n--- Data Stack ");
	for (i = 0; i < S(x)->length; i++) {
		printf("%ld ", S(x)->data[i]);
	}
	printf("\n");
}

#endif
