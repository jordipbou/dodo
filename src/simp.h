#ifndef __DODO__
#define __DODO__

// DATATYPES

#include <stdint.h>

typedef int8_t BYTE;
typedef intptr_t CELL;

typedef struct T_NODE {
	union {
		CELL value;
		struct T_NODE* ref;
	};
	struct T_NODE* next;
} NODE;

#define NEXT(node)								((NODE*)(((CELL)(node)->next) & -4))
#define TYPE(node)								(((CELL)node->next) & 3)

#define AS(type, node)						((NODE*)(((CELL)node) | type))
#define LINK(node, link)					(node->next = AS(TYPE(node), link))

enum Types { ATOM, PRIM, LIST, WORD };
enum RTypes { SAVED, IP, DISPOSABLE, HANDLER };

// HELPERS

BYTE* dump_list(BYTE*, NODE*);

BYTE* dump_cell(BYTE* str, NODE* node) {
	switch (TYPE(node)) {
		case ATOM: sprintf(str, "%s#%ld ", str, node->value); break;
		case PRIM: sprintf(str, "%sP:%ld ", str, node->value); break;
		case LIST: dump_list(str, node->ref); break;
		case WORD: /* TODO */; break;
	}

	return str;
}

BYTE* dump_list(BYTE* str, NODE* list) {
	sprintf(str, "%s{ ", str);
	while (list) {
		dump_cell(str, list);
		list = NEXT(list);
	}
	sprintf(str, "%s} ", str);

	return str;
}

CELL length(NODE* list, CELL dir) {
	CELL count = 0;
	if (dir) while (list) { count++; list = list->ref; }
	else while (list) { count++; list = NEXT(list); }
	return count;
}

NODE* reverse(NODE* list, NODE* acc) {
	if (list) {
		NODE* tail = NEXT(list);
		LINK(list, acc);
		return reverse(tail, list);
	} else {
		return acc;
	}
}

// CORE

#define ALIGN(addr, bound)				((((CELL)addr) + (bound - 1)) & ~(bound - 1))

NODE* init_free(BYTE* block, CELL size, CELL start) {
	NODE* node;

	NODE* there = (NODE*)ALIGN((block + start), sizeof(NODE));
	NODE* top = (NODE*)ALIGN((block + size - sizeof(NODE) - 1), sizeof(NODE));

	for (node = there; node <= top; node ++) {
		node->next = node == there ? 0 : node - 1;
		node->ref = node == top ? 0 : node + 1;
	}

	return top;
}

NODE* cons(NODE** free, CELL value, NODE* next) {
	NODE* node;

	if (*free == 0) return 0;

	node = *free;
	*free = NEXT(*free);
	(*free)->ref = 0;
	node->value = value;
	node->next = next;

	return node;
}

NODE* reclaim(NODE** free, NODE* node) {
	NODE* tail;

	if (!node) return 0;
	if (TYPE(node) == LIST) {
		while (node->ref) {
			node->ref = reclaim(free, node->ref);
		}
	}

	tail = NEXT(node);
	node->next = *free;
	node->value = 0;
	*free = node;
	NEXT(*free)->ref = *free;

	return tail;
}

NODE* clone(NODE** free, NODE* node) {
	if (!node) return 0;
	if (TYPE(node) == LIST) {
		return cons(free, (CELL)clone(free, node->ref), AS(LIST, clone(free, NEXT(node))));
	} else {
		return cons(free, node->value, AS(TYPE(node), clone(free, NEXT(node))));
	}
}

// CONTEXT

typedef struct {
	CELL size, err, compiling;
	NODE* fstack;
	NODE* mstack, * rstack, * nstack;
	NODE** dstack;
	BYTE* here;
	NODE* there;
	NODE* ip;
} CTX;

#define S(ctx)										(*(ctx->dstack))

typedef void (*FUNC)(CTX*);

#define BOTTOM(ctx)								(((BYTE*)ctx) + sizeof(CTX))

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;

	ctx->size = size;

	ctx->fstack = init_free(block, size, sizeof(CTX));
	ctx->mstack = ctx->rstack = ctx->nstack = 0;
	ctx->dstack = &ctx->mstack;

	ctx->here = BOTTOM(ctx);
	ctx->there = (NODE*)ALIGN(ctx->here, sizeof(NODE));

	ctx->err = ctx->compiling = 0;

	ctx->ip = 0;

	return ctx;
}

// ERRORS

#define ERR_STACK_OVERFLOW				-1
#define ERR_STACK_UNDERFLOW				-2
#define ERR_DIVISION_BY_ZERO			-3
#define ERR_EXPECTED_LIST					-4

void error(CTX* ctx) {
	// TODO: Find in return stack error handlers and call them if same type as error
}

#define ERR(ctx, cond, ...)	({ \
	if (cond) { \
		ctx->err = __VA_ARGS__; \
		error(ctx); \
		if (ctx->err < 0) { \
			return; \
		} \
	} \
})

#define OF1(ctx)			ERR(ctx, ctx->fstack == ctx->there, ERR_STACK_OVERFLOW)
#define OF2(ctx)			ERR(ctx, ctx->fstack == ctx->there || NEXT(ctx->fstack) == ctx->there, ERR_STACK_OVERFLOW)
#define OF3(ctx)			ERR(ctx, ctx->fstack == ctx->there || NEXT(ctx->fstack) == ctx->there || NEXT(NEXT(ctx->fstack)) == ctx->there, ERR_STACK_OVERFLOW)

#define UF1(ctx)			ERR(ctx, S(ctx) == 0, ERR_STACK_UNDERFLOW)
#define UF2(ctx)			ERR(ctx, S(ctx) == 0 || NEXT(S(ctx)) == 0, ERR_STACK_UNDERFLOW)
#define UF3(ctx)			ERR(ctx, S(ctx) == 0 || NEXT(S(ctx)) == 0 || NEXT(NEXT(S(ctx))) == 0, ERR_STACK_UNDERFLOW)

// STACK PRIMITIVES

void duplicate(CTX* ctx) { /* ( n -- n n ) */
	UF1(ctx);
	if (TYPE(S(ctx)) == LIST) {
		S(ctx) = cons(&ctx->fstack, (CELL)clone(&ctx->fstack, S(ctx)->ref), AS(LIST, S(ctx)));
	} else {
		S(ctx) = cons(&ctx->fstack, S(ctx)->value, AS(TYPE(S(ctx)), S(ctx)));
	}
}

void swap(CTX* ctx) {	/* ( n2 n1 -- n1 n2 ) */
	UF2(ctx);
	NODE* temp = NEXT(S(ctx));
	LINK(S(ctx), NEXT(NEXT(S(ctx))));
	LINK(temp, S(ctx));
	S(ctx) = temp;
}

void drop(CTX* ctx) { /* ( n -- ) */
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void over(CTX* ctx) { /* ( n2 n1 -- n2 n1 n2 ) */
	UF2(ctx); OF1(ctx);
	if (TYPE(NEXT(S(ctx))) == LIST) {
		S(ctx) = cons(&ctx->fstack, (CELL)clone(&ctx->fstack, NEXT(S(ctx))->ref), AS(LIST, S(ctx)));
	} else {
		S(ctx) = cons(&ctx->fstack, NEXT(S(ctx))->value, AS(TYPE(NEXT(S(ctx))), S(ctx)));
	}
}

void rot(CTX* ctx) { /* ( n3 n2 n1 -- n1 n3 n2 ) */
	UF3(ctx);
	NODE* temp = NEXT(NEXT(S(ctx)));
	LINK(NEXT(S(ctx)), NEXT(NEXT(NEXT(S(ctx)))));
	LINK(temp, S(ctx));
	S(ctx) = temp;
}

// ARITHMETIC PRIMITIVES

void add(CTX* ctx) {	/* ( n2 n1 -- n:(n2 + n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value + S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void incr(CTX* ctx) {	/* ( n -- n:(n + 1) ) */
	UF1(ctx);
	S(ctx)->value++;
}

void sub(CTX* ctx) {	/* ( n2 n1 -- n:(n2 - n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value - S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void decr(CTX* ctx) {	/* ( n -- n:(n - 1) ) */
	UF1(ctx);
	S(ctx)->value--;
}

void mul(CTX* ctx) {	/* ( n2 n1 -- n:(n2 * n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value * S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void division(CTX* ctx) {	/* ( n2 n1 -- n:(n2 / n1) ) */
	UF2(ctx); ERR(ctx, S(ctx)->value == 0, ERR_DIVISION_BY_ZERO);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value / S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void mod(CTX* ctx) {	/* ( n2 n1 -- n:(n2 mod n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value % S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

// COMPARISON PRIMITIVES

void gt(CTX* ctx) {	/* ( n2 n1 -- n:(n2 > n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value > S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void lt(CTX* ctx) { /* ( n2 n1 -- n:(n2 < n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value < S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void eq(CTX* ctx) { /* ( n2 n1 -- n:(n2 = n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value == S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void neq(CTX* ctx) { /* ( n2 n1 -- n:(n2 <> n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value != S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

// BIT PRIMITIVES

void and(CTX* ctx) { /* ( n2 n1 -- n:(n2 and n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value & S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void or(CTX* ctx) { /* ( n2 n1 -- n:(n2 or n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value | S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void invert(CTX* ctx) { /* ( n -- n:(inverted bits) ) */
	UF1(ctx);
	S(ctx)->value = ~(S(ctx)->value);
}

// LIST PRIMITIVES

void empty(CTX* ctx) { /* ( -- {} ) */
	OF1(ctx);
	S(ctx) = cons(&ctx->fstack, 0, AS(LIST, S(ctx)));
}

void list_to_stack(CTX* ctx) { /* ( { a b c } -- c b a ) */
	UF1(ctx); ERR(ctx, TYPE(S(ctx)) != LIST, ERR_EXPECTED_LIST);
	ctx->dstack = &(S(ctx)->ref);
}

void reverse_list(CTX* ctx) { /* ( { a b c } -- { c b a } ) */
	UF1(ctx);  ERR(ctx, TYPE(S(ctx)) != LIST, ERR_EXPECTED_LIST);
	S(ctx)->ref = reverse(S(ctx)->ref, 0);
}

void main_stack(CTX* ctx) { /* ( -- ) */
	ctx->dstack = &ctx->mstack;
}

// INNER INTERPRETER

void incrIP(CTX* ctx) {
	if (ctx->ip) {
		ctx->ip = NEXT(ctx->ip);
	}
	while (!ctx->ip && ctx->rstack) {
		if (TYPE(ctx->rstack) == IP) {
			ctx->ip = NEXT(ctx->rstack->ref);
		}
		ctx->rstack = reclaim(&ctx->fstack, ctx->rstack);
	}
}

#define CALL(ctx, ip) \
	({ \
		if (ctx->ip && NEXT(ctx->ip)) { \
			ctx->rstack = cons(&ctx->fstack, ctx->ip, AS(IP, ctx->rstack)); \
		} \
		ctx->ip = ip; \
	})

NODE* step(CTX* ctx) {
	NODE* r;
	if (ctx->ip) {
		switch (TYPE(ctx->ip)) {
			case ATOM:
				S(ctx) = cons(&ctx->fstack, ctx->ip->value, AS(ATOM, S(ctx)));
				incrIP(ctx);
				break;
			case LIST:
				S(ctx) = cons(&ctx->fstack, (CELL)clone(&ctx->fstack, ctx->ip->ref), AS(LIST, S(ctx)));
				incrIP(ctx);
				break;
			case PRIM:
				r = ctx->ip;
				((FUNC)(ctx->ip->value))(ctx);
				if (r == ctx->ip) {
					incrIP(ctx);
				}
				break;
			case WORD:
				//CALL(ctx, XT(ctx, REF(IP(ctx))));
				break;
		}
	}
	return ctx->ip;
}

#endif
