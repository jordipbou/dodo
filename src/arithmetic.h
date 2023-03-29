#ifndef __DODO_ARITHMETIC__
#define __DODO_ARITHMETIC__

#include "core.h"
#include "error.h"

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

#endif
