#ifndef __DODO_LISTS__
#define __DODO_LISTS__

#include "core.h"
#include "error.h"

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

#endif
