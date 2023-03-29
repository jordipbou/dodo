#ifndef __DODO_STACK__
#define __DODO_STACK__

#include "types.h"
#include "core.h"

void dupl(CTX* ctx) { /* ( n -- n n ) */
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

#endif
