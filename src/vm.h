#include<stdint.h>
#include<stddef.h>

// Scalars

typedef int64_t cell_t;
typedef unsigned char byte_t;

// Lists

#define CAR(addr)		*(addr + 1)
#define CDR(addr)		*addr

cell_t *cons(cell_t *block, cell_t car, cell_t cdr);
void reclaim(cell_t *b, cell_t *i);
cell_t length(cell_t *list);

// Scope

typedef struct scp_s {
	cell_t *block;
	cell_t ddepth, rdepth;
	cell_t *dstack, *rstack;
} scp_t;

// Block

#define SIZE(b)							*b
#define FREE_HEAD(b)				*(b + 1)
#define LOWEST_ASSIGNED(b)	*(b + 2)
#define FREE_TAIL(b)				*(b + 3)
#define HERE(b)							*(b + 4)
#define HEADER_SIZE					6

cell_t *init_block(cell_t *block, cell_t size);
cell_t reserve(cell_t *block, cell_t npairs);
cell_t allot(cell_t *b, cell_t nbytes);

// Stacks

void push(scp_t *scope, cell_t value);
cell_t pop(scp_t *scope);
void rpush(scp_t *scope, cell_t value);
cell_t pop(scp_t *scope);

// Runtime context

typedef struct ctx_s {
	cell_t err, T, S, R;
	byte_t *PC;
	scp_t *scope;
} ctx_t;

void eval(ctx_t *c);
void dump_stack(ctx_t *c);

#define DUP(c)			push(c->scope, c->S); c->S = c->T
#define LIT(c, v)		push(c->scope, c->S); c->S = c->T; c->T = v
#define GT(c)				c->T = c->S > c->T; c->S = pop(c->scope)
#define DEC(c)			c->T = c->T - 1
#define SUB(c)			c->T = c->S - c->T; c->S = pop(c->scope)
#define SWAP(c)			{ cell_t x = c->T; c->T = c->S; c->S = x; }
#define ADD(c)			c->T = c->S + c->T; c->S = pop(c->scope)
