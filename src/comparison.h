#ifndef __DODO_COMPARISON__
#define __DODO_COMPARISON__

#include "core.h"
#include "error.h"

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

#endif
