#ifndef __DODO_ERROR__
#define __DODO_ERROR__

#include "types.h"
#include "core.h"

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_DIVISION_BY_ZERO		-3
#define ERR_NOT_ENOUGH_MEMORY		-4
#define ERR_NOT_ENOUGH_RESERVED	-5
#define ERR_UNDEFINED_WORD			-6
#define ERR_EXPECTED_ATOM				-7
#define ERR_EXPECTED_2_ATOMS		-8
#define ERR_EXPECTED_LIST				-9
#define ERR_EXPECTED_WORD				-10
#define ERR_ZERO_LENGTH_WORD		-11

void error(CTX* ctx) {
	CELL handler = ctx->xstack;
	while (handler) {
		if (CAR(CAR(handler)) == ctx->err || CAR(CAR(handler)) == 0) {
			((FUNC)CAR(NEXT(CAR(handler))))(ctx);
			if (ctx->err == 0) return;
		}
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
#define UF1(ctx)			ERR(ctx, !TOS(ctx), ERR_STACK_UNDERFLOW)
#define UF2(ctx)			UF1(ctx); ERR(ctx, !NEXT(TOS(ctx)), ERR_STACK_UNDERFLOW)
#define UF3(ctx)			UF2(ctx); ERR(ctx, !NEXT(NEXT(TOS(ctx))), ERR_STACK_UNDERFLOW)

#define EA(ctx)				UF1(ctx); ERR(ctx, TYPE(TOS(ctx)) != ATOM, ERR_EXPECTED_ATOM)
#define EL(ctx)				UF1(ctx); ERR(ctx, TYPE(TOS(ctx)) != LIST, ERR_EXPECTED_LIST)
#define EW(ctx)				UF1(ctx); ERR(ctx, TYPE(TOS(ctx)) != WORD, ERR_EXPECTED_WORD)
#define E2A(ctx)			UF2(ctx); ERR(ctx, TYPE(TOS(ctx)) != ATOM || TYPE(NEXT(TOS(ctx))) != ATOM, ERR_EXPECTED_2_ATOMS)

#endif
