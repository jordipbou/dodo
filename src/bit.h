#ifndef __DODO_BIT__
#define __DOBO_BIT__

#include "core.h"
#include "error.h"

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

#endif
