#ifndef __DODO_CORE__
#define __DODO_CORE__

#include<stdint.h>

// DATATYPES

typedef int8_t BYTE;
typedef intptr_t CELL;

enum DTypes { ATOM, LIST, PRIM, WORD };
enum SubTypes { LINKED };
enum WTypes { PL, CC, IMM, NDCS };

typedef struct {
	BYTE *ibuf, *here;
	CELL there, size;
	CELL fstack, dstack, rstack, xstack;
	CELL ip, err, status, latest;
	CELL ipos, ilen;
	CELL free, ftotal, fmax;
} CTX;

typedef void (*FUNC)(CTX*);

// PAIRS

#define CDR(pair)						(*(((CELL*)pair)))
#define NEXT(pair)					(CDR(pair) & -4)
#define TYPE(pair)					(CDR(pair) & 3)

#define CAR(pair)						(*(((CELL*)pair) + 1))
#define REF(pair)						(CAR(pair) & -4)
#define SUBTYPE(pair)				(CAR(pair) & 3)

#define AS(type, next)			((next & -4) | type)
#define LINK(pair, next)		(CDR(pair) = AS(TYPE(pair), next))

CELL length(CELL list) {
	CELL count = 0;
	while (list) {
		count++;
		list = NEXT(list);
	}
	return count;
}

CELL reverse(CELL list, CELL acc) {
	if (list) {
		CELL tail = NEXT(list);
		LINK(list, acc);
		return reverse(tail, list);
	} else {
		return acc;
	}
}

// TODO: depth

// CONTEXT

#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))
#define BOTTOM(ctx)					(((BYTE*)ctx) + sizeof(CTX))
#define TOP(ctx)						(ALIGN(((BYTE*)ctx) + ctx->size - 2*sizeof(CELL) - 1, 2*sizeof(CELL)))
#define RESERVED(ctx)				((CELL)(((BYTE*)ctx->there) - ctx->here))

#define TOS(ctx)						(CAR(ctx->dstack))

CTX* init(BYTE* block, CELL size) {
	CELL pair;

	CTX* ctx = (CTX*)block;	

	ctx->size = size;

	ctx->here = BOTTOM(ctx);
	ctx->there = ALIGN(BOTTOM(ctx), 2*sizeof(CELL));

	ctx->dstack = TOP(ctx); CAR(ctx->dstack) = 0; CDR(ctx->dstack) = AS(LIST, 0);
	ctx->fstack = ctx->dstack - 2*sizeof(CELL);
	ctx->rstack = 0;
	ctx->xstack = 0;

	ctx->free = -1;
	for (pair = ctx->there; pair <= ctx->fstack; pair += 2*sizeof(CELL)) {
		ctx->free++;
		CAR(pair) = pair == ctx->fstack ? 0 : pair + 2*sizeof(CELL);
		CDR(pair) = pair == ctx->there ? 0 : pair - 2*sizeof(CELL);
	}

	ctx->fmax = 0;
	ctx->ftotal = ctx->free;
	ctx->ibuf = 0;
	ctx->ipos = ctx->ilen = ctx->status = ctx->err = ctx->latest = 0;
	ctx->ip = 0;

	return ctx;
}

CELL cons(CTX* ctx, CELL car, CELL cdr) {
	CELL pair;

	ctx->free--;
	pair = ctx->fstack;
	ctx->fstack = CDR(ctx->fstack);
	CAR(pair) = car;
	CDR(pair) = cdr;

	return pair;
}

CELL reclaim(CTX* ctx, CELL pair) {
	CELL tail;

	if (TYPE(pair) == LIST && SUBTYPE(pair) == LINKED) {
		while (REF(pair)) {
			CAR(pair) = reclaim(ctx, CAR(pair));
		}
	}

	ctx->free++;
	tail = NEXT(pair);
	CDR(pair) = ctx->fstack;
	CAR(pair) = 0;
	ctx->fstack = pair;

	return tail;
}

CELL clone(CTX* ctx, CELL pair) {
	if (!pair) return 0;
	if (TYPE(pair) == LIST && SUBTYPE(pair) == LINKED) {
		return cons(ctx, clone(ctx, REF(pair)), AS(LIST, clone(ctx, NEXT(pair))));
	} else {
		return cons(ctx, CAR(pair), AS(TYPE(pair), clone(ctx, NEXT(pair))));
	}
}

// ERRORS

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_DIVISION_BY_ZERO		-3
#define ERR_NOT_ENOUGH_MEMORY		-4
#define ERR_NOT_ENOUGH_RESERVED	-5
#define ERR_UNDEFINED_WORD			-6
#define ERR_EXPECTED_LIST				-7
#define ERR_ZERO_LENGTH_WORD		-8

void error(CTX* ctx) {
	CELL handler = ctx->xstack;
	while (handler) {
		if (CAR(CAR(handler)) == ctx->err || CAR(CAR(handler)) == 0) {
			((FUNC)CAR(NEXT(CAR(handler))))(ctx);
			if (ctx->err == 0) return;
		}
		handler = NEXT(handler);
	}
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

#define OF(ctx, n)		ERR(ctx, ctx->free < n, ERR_STACK_OVERFLOW)
#define UF1(ctx)			ERR(ctx, TOS(ctx) == 0, ERR_STACK_UNDERFLOW)
#define UF2(ctx)			ERR(ctx, TOS(ctx) == 0 || NEXT(TOS(ctx)) == 0, ERR_STACK_UNDERFLOW)
#define UF3(ctx)			ERR(ctx, TOS(ctx) == 0 || NEXT(TOS(ctx)) == 0 || NEXT(NEXT(TOS(ctx))) == 0, ERR_STACK_UNDERFLOW)

#define EL(ctx)				UF1(ctx); ERR(ctx, TYPE(TOS(ctx)) != LIST, ERR_EXPECTED_LIST)

// STACK PRIMITIVES

void duplicate(CTX* ctx) { /* ( n -- n n ) */
	if (TYPE(TOS(ctx)) == LIST && SUBTYPE(TOS(ctx)) == LINKED) {
		TOS(ctx) = cons(ctx, AS(LINKED, clone(ctx, REF(TOS(ctx)))), AS(LIST, TOS(ctx)));
	} else {
		TOS(ctx) = cons(ctx, CAR(TOS(ctx)), AS(TYPE(TOS(ctx)), TOS(ctx)));
	}
}

void swap(CTX* ctx) {	/* ( n2 n1 -- n1 n2 ) */
	CELL temp = NEXT(TOS(ctx));
	LINK(TOS(ctx), NEXT(NEXT(TOS(ctx))));
	LINK(temp, TOS(ctx));
	TOS(ctx) = temp;
}

void drop(CTX* ctx) { /* ( n -- ) */
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void over(CTX* ctx) { /* ( n2 n1 -- n2 n1 n2 ) */
	if (TYPE(NEXT(TOS(ctx))) == LIST && SUBTYPE(NEXT(TOS(ctx))) == LINKED) {
		TOS(ctx) = cons(ctx, AS(LINKED, clone(ctx, REF(NEXT(TOS(ctx))))), AS(LIST, TOS(ctx)));
	} else {
		TOS(ctx) = cons(ctx, CAR(NEXT(TOS(ctx))), AS(TYPE(NEXT(TOS(ctx))), TOS(ctx)));
	}
}

void rot(CTX* ctx) { /* ( n3 n2 n1 -- n1 n3 n2 ) */
	CELL temp = NEXT(NEXT(TOS(ctx)));
	LINK(NEXT(TOS(ctx)), NEXT(NEXT(NEXT(TOS(ctx)))));
	LINK(temp, TOS(ctx));
	TOS(ctx) = temp;
}


// ARITHMETIC PRIMITIVES

void add(CTX* ctx) {	/* ( n2 n1 -- n:(n2 + n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) + CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void sub(CTX* ctx) {	/* ( n2 n1 -- n:(n2 - n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) - CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void mul(CTX* ctx) {	/* ( n2 n1 -- n:(n2 * n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) * CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void division(CTX* ctx) {	/* ( n2 n1 -- n:(n2 / n1) ) */
	UF2(ctx); ERR(ctx, CAR(TOS(ctx)) == 0, ERR_DIVISION_BY_ZERO);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) / CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void mod(CTX* ctx) {	/* ( n2 n1 -- n:(n2 mod n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) % CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

// TODO: Add inc and dec

// COMPARISON PRIMITIVES

void gt(CTX* ctx) {	/* ( n2 n1 -- n:(n2 > n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) > CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void lt(CTX* ctx) { /* ( n2 n1 -- n:(n2 < n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) < CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void eq(CTX* ctx) { /* ( n2 n1 -- n:(n2 = n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) == CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void neq(CTX* ctx) { /* ( n2 n1 -- n:(n2 <> n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) != CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

// BIT PRIMITIVES

void and(CTX* ctx) { /* ( n2 n1 -- n:(n2 and n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) & CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void or(CTX* ctx) { /* ( n2 n1 -- n:(n2 or n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) | CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void invert(CTX* ctx) { /* ( n -- n:(inverted bits) ) */
	UF1(ctx);
	CAR(TOS(ctx)) = ~(CAR(TOS(ctx)));
}

// LIST PRIMITIVES

void empty(CTX* ctx) { /* ( -- {} ) */
	OF(ctx, 1);
	TOS(ctx) = cons(ctx, 0, AS(LIST, TOS(ctx)));
}

void list_to_stack(CTX* ctx) { /* ( { ... } -- ... ) */
	EL(ctx);
	CELL temp = TOS(ctx);
	TOS(ctx) = NEXT(TOS(ctx));
	LINK(temp, ctx->dstack);
	ctx->dstack = temp;
}

void stack_to_list(CTX* ctx) { /* ( ... -- { ... } ) */
	TOS(ctx) = reverse(TOS(ctx), 0);
	if (NEXT(ctx->dstack) == 0) {
		OF(ctx, 1);
		ctx->dstack = cons(ctx, ctx->dstack, AS(LIST, 0));
	} else {
		CELL temp = ctx->dstack;
		ctx->dstack = NEXT(ctx->dstack);
		LINK(temp, TOS(ctx));
		TOS(ctx) = temp;
	}
}

// INNER INTERPRETER


#endif

