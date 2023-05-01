#ifndef DODO_ARR
#define DODO_ARR

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>

typedef int8_t BYTE;
typedef int32_t HALF;
typedef intptr_t CELL;

enum { LIST, INT, INT_ARRAY, FLOAT, FLOAT_ARRAY, CHAR, STRING } TYPE;

typedef struct { /* CELL rc */ CELL type, size, length; CELL data[1]; } VECTOR;

typedef struct {
	VECTOR** dstack;
	VECTOR* mstack;
	VECTOR* rstack;
	BYTE* ip;
} CONTEXT;

#define S(x)				(*x->dstack)
#define TOS(x)			(S(x)->data[S(x)->length - 1])
#define SOS(x)			(S(x)->data[S(x)->length - 2])

#define PUSH(x, i)	(S(x)->data[S(x)->length++] = i)
#define POP(x)			(S(x)->data[--S(x)->length])

#define PUSHR(x, i)	(x->rstack->data[x->rstack->length++] = (CELL)i)
#define POPR(x)			(x->rstack->data[--x->rstack->length])

VECTOR* newV(CELL type, CELL size) {
	CELL array_size;
	VECTOR* vector;

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
			vector = calloc(size + 3*sizeof(VECTOR), sizeof(BYTE));
			break;
	}

	vector->type = type;
	vector->size = size;
	vector->length = 0;

	return vector;
}

CONTEXT* init() {
	CONTEXT* context = malloc(sizeof(CONTEXT));

	context->mstack = newV(INT, 256);
	context->rstack = newV(INT, 256);
	context->dstack = &context->mstack;

	return context;
}

void add(CONTEXT* x) { CELL a = POP(x); CELL b = POP(x); PUSH(x, b + a); }
void sub(CONTEXT* x) { CELL a = POP(x); CELL b = POP(x); PUSH(x, b - a); }

void gt(CONTEXT* x) { CELL a = POP(x); CELL b = POP(x); PUSH(x, b > a); }

void swap(CONTEXT* x) { CELL a = POP(x); CELL b = POP(x); PUSH(x, a); PUSH(x, b); }
void dup(CONTEXT* x) { CELL a = POP(x); PUSH(x, a); PUSH(x, a); }
BYTE* stepI(CONTEXT* x) {
	CELL a, b;
	/*
	printf("x->ip %p *x->ip %c\n", x->ip, *x->ip);
	*/
	switch (*x->ip) {
		case '1': PUSH(x, 1); break;
		case 's': a = POP(x); b = POP(x); PUSH(x, a); PUSH(x, b); break;
		case 'd': a = POP(x); PUSH(x, a); PUSH(x, a); break;
		case '+': a = POP(x); b = POP(x); PUSH(x, b + a); break;
		case '-': a = POP(x); b = POP(x); PUSH(x, b - a); break;
		case '>': a = POP(x); b = POP(x); PUSH(x, b > a); break;
		case '?': 
			if (!POP(x)) { while (*x->ip != ')') { x->ip++; } }
			break;
		case 'z':
			if (!POP(x)) x->ip += *(x->ip+1);
			else x->ip++;
			break;
		case 'c':
			PUSHR(x, x->ip + 1);
			x->ip += *(x->ip + 1);
			break;
		case '`':
			PUSHR(x, x->ip);
			while (*x->ip != ':') { x->ip--; }
			break;
		case ';':
			if (x->rstack->length == 0) { x->ip = 0; return x->ip; }
			x->ip = (BYTE*)POPR(x);
			break;
		default: break;
	}
	/*
	dump_stack(x);
	dump_return_stack(x);
	*/
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

void dump_return_stack(CONTEXT* x) {
	CELL i;

	printf("\n= Return Stack ");
	for (i = 0; i < x->rstack->length; i++) {
		printf("%p ", x->rstack->data[i]);
	}
	printf("\n");
}

#endif
