#ifndef __DODO_DBG__
#define __DODO_DBG__

#include "dodo.h"

CELL find_primitive(CTX* ctx, CELL f) {
	CELL w = ctx->latest;
	while (w && !(CAR(XT(w)) == f && NEXT(XT(w)) == 0)) {
		w = NEXT(w);
	}
	return w;
}

void dump_list(CTX* ctx, CELL pair, CELL dir) {
	if (pair) {
		if (!dir) dump_list(ctx, NEXT(pair), dir);
		switch (TYPE(pair)) {
			case ATOM: printf("#%ld ", CAR(pair)); break;
			case LIST: printf("{ "); dump_list(ctx, CAR(pair), 1); printf("} "); break;
			case PRIM: {
				CELL word = find_primitive(ctx, CAR(pair));
				if (word) {
					printf("P:%s ", (BYTE*)(NFA(word)));
				} else {
					printf("PRIM_NOT_FOUND ");
				}
			} break;
			case WORD: printf("W:%s ", NFA(CAR(pair))); break;
		}
		if (dir) dump_list(ctx, NEXT(pair), 1);
		if (!dir) printf("\n");
	}
}

CELL dump_stack(CTX* ctx) {
	printf("\n");
	dump_list(ctx, S(ctx), 0);

	return 0;
}

CELL dbg_execute(CTX* ctx, CELL xlist) {
	CELL result;
	CELL p = xlist;
	while (p) {
		switch (TYPE(p)) {
			case ATOM:
				if (PUSH(ctx, CAR(p)) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
				p = NEXT(p);
				break;
			case LIST:
				if (PUSHL(ctx, clone(ctx, CAR(p))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
				p = NEXT(p);
				break;
			case PRIM:
				switch (CAR(p)) {
					case 0: /* ZJUMP */
						if (pop(ctx) == 0) {
							p = CAR(NEXT(p));
						} else {
							p = NEXT(NEXT(p));
						}
						break;
					case 1: /* JUMP */
						p = CAR(NEXT(p));
						break;
					case 2:	/* AHEAD */
						if ((ctx->cfstack = cons(ctx, p, AS(ATOM, ctx->cfstack))) == 0) {
							ERR(ctx, ERR_STACK_OVERFLOW);
						}
						p = NEXT(p);
						break;
					default:
						EXECUTE_PRIMITIVE(ctx, CAR(p));
						p = NEXT(p);
						break;
				}
				break;
			case WORD:
				if (NEXT(p)) { 
					execute(ctx, XT(CAR(p))); p = NEXT(p); 
				} else { 
					p = XT(CAR(p));
				}
				break;
		}
	}
}

CELL dbg_exec_x(CTX* ctx) {
	printf("CAR(S(ctx)) %ld\n", CAR(S(ctx)));
	dump_list(ctx, CAR(S(ctx)), 1);
	printf("AFTER\n");
	CELL result = 0;

	switch (TYPE(S(ctx))) {
		case ATOM: if (PUSH(ctx, CAR(S(ctx))) == 0) result = ERR_STACK_OVERFLOW; break;
		case LIST: dump_list(ctx, CAR(S(ctx)), 1); result = execute(ctx, CAR(S(ctx))); break;
		case PRIM: result = ((FUNC)CAR(S(ctx)))(ctx); break;
		case WORD: result = dbg_execute(ctx, XT(CAR(S(ctx)))); break;
	}

	if (result < 0) { ERR(ctx, result); } else { return result; }
}

void dump_context(CTX* ctx) {
	printf("STATE: %ld\n", ctx->state);
	printf("STACK PILE:\n");
	for (CELL s = ctx->pile; s; s = NEXT(s)) {
		printf("[%ld] ", s); dump_list(ctx, s, 1); printf("\n");
	}
	printf("\n");
	printf("COMPILING PILE:\n");
	for (CELL s = ctx->cpile; s; s = NEXT(s)) {
		printf("[%ld] ", s); dump_list(ctx, s, 1); printf("\n");
	}
	printf("\n");
}

CELL dbg_evaluate(CTX* ctx, BYTE* str) {
	CELL word, result; 
	char *endptr;
	ctx->tib = str; 
	ctx->token = ctx->in = 0;
	do {
		if (parse_token(ctx) == 0) { return 0; }
		printf("============================\n");
		dump_context(ctx);
		printf("TOKEN: %.*s\n\n", TL(ctx), TK(ctx));
		if ((word = find_token(ctx)) != 0) {
			if (!ctx->state || IMMEDIATE(word)) {
				if ((result = dbg_execute(ctx, XT(word))) != 0) { ERR(ctx, result); }
			} else {
				printf("COMPILING: %s\n", NFA(word));
				if (compile_word(ctx, word) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
			}
		} else {
			intmax_t number = strtoimax(TK(ctx), &endptr, 10);
			if (number == 0 && endptr == (char*)(TK(ctx))) {
				ERR(ctx, ERR_UNDEFINED_WORD);
			} else if (ctx->state) {
				printf("COMPILING: %ld\n", number);
				//if (PUSH(ctx, number) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
				if ((C(ctx) = cons(ctx, number, AS(ATOM, C(ctx)))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
			} else {
				if (PUSH(ctx, number) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
			}
		}
		dump_context(ctx);
		printf("----------------------------\n");
	} while (1);
}

#endif
