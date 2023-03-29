#ifndef __DODO_CORE__
#define __DODO_CORE__

#include "types.h"
#include "lists.h"

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

#endif

