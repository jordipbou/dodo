#include<stdio.h>

#include "capybara.h"

typedef struct _PAIR {
	struct _PAIR* cdr;
	CELL car;
} PAIR;

// Strings are a mixture of C zero terminated strings and Forth counted strings 
// but with 16 bits for the count for an easy work both in C and Dodo.

typedef struct {
	HALF len;
	BYTE str[4];
} STR;

typedef struct _ENTRY {
	struct _ENTRY* next;
	HALF flags;
	STR* name;
	STR* source;
	HALF codelen;
	BYTE* code;
	BYTE data[];
} ENTRY;

typedef struct {
	CTX ctx;
	HALF flags;
	ENTRY* dict;
	PAIR* dstack;			// Data stack
	PAIR* rstack;			// Return stack
	PAIR* lstack;			// Lists stack, used to manage lists memory more easily
	PAIR* ftail, * flowest, * fhead;
	// More stacks can be easily added (like control flow stack or exception
	// stack) without taxing the memory.
	PAIR* input;
} DODO;

#define COMPILATION_STATE		1			// 0 interpreting 1 compiling
#define BYE									2

// DODO context initialization and destruction

DODO* init_dodo() {
	DODO* dodo = (DODO*)init(10*pagesize(), 10*pagesize());
	if (!dodo) return NULL;

	// Reserve enough space for DODO header
	allot((CTX*)dodo, sizeof(DODO) - sizeof(CTX));
	align((CTX*)dodo);
	dodo->ctx.bottom = dodo->ctx.dhere;

	dodo->ftail = (PAIR*)(dodo->ctx.dhere);
	for (PAIR* p = dodo->ftail; p < (PAIR*)(top((CTX*)dodo) - sizeof(PAIR)); p++) {
		p->cdr = p - 1;
		p->car = (CELL)(p + 1);
		dodo->fhead = p;
	}
	dodo->ftail->cdr = NULL;
	dodo->fhead->car = (CELL)NULL;
	dodo->flowest = dodo->fhead;
}

void deinit_dodo(DODO* dodo) {
	deinit((CTX*)dodo);
}

PAIR* insert(DODO* dodo, PAIR* list, CELL car) {
	if (dodo->fhead == NULL) return NULL;

	// Remove one pair from unused list
	PAIR* p = dodo->fhead;
	dodo->fhead = p->cdr;

	// Add it to new list
	p->cdr = list->cdr;
	p->car = car;
	list->cdr = p;

	return p;
}

CELL remove(DODO* dodo, PAIR* list, PAIR* p) {
	CELL value = p->car;
	
	p->cdr = dodo->fhead;
	dodo->fhead = p;
	p->car = (CELL)NULL;
}

void PUSH(DODO* dodo, CELL value) {
	cons(dodo, dodo->dstack, value);
}

CELL POP(DODO* dodo) {
	if (dodo->dstack == NULL) { /* TODO: Stack underflow */ }
	PAIR* p = dodo->dstack;
	CELL value = p->car;
	dodo->dstack = p->cdr;
	reclaim(dodo, p);
	return value;
}

CELL length(PAIR* list) {
	for(CELL a = 0;; a++) { if (!list) { return a; } else { list = list->cdr; } }
}

void _dump_stack(DODO* dodo) {
	printf("<%ld> ", length(dodo->dstack));
	for (PAIR* item = dodo->dstack; item != NULL; item = item->cdr) {
		printf("%ld ", item->car);
	}
	printf("\n");
}

void _free_pairs(DODO* dodo) {
	PUSH(dodo, length(dodo->fhead));
}

void _bye(DODO* dodo) {
	dodo->flags |= BYE;
}

void _read(DODO* dodo) {
	if (dodo->input) {
		// Parse tokens
	} else {
		CELL k = 0;
		while (k != KEY_RETURN) {
			
		}
	}
}

void _repl(DODO* dodo) {
	while (!(dodo->flags & BYE)) {
		_read(dodo);
		_eval(dodo);
	}
}
