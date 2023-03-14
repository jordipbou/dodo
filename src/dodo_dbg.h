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
		printf("[%ld] ", pair);
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
			case WORD: break;
				//printf("W:%s (%ld) ", NFA(CAR(pair)), CAR(pair)); break;
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
			case ATOM: ERR_PUSH(ctx, CAR(p)); break;
			case LIST: ERR_PUSHL(ctx, clone(ctx, CAR(p))); break;
				break;
			case PRIM:
				switch (CAR(p)) {
					case ZJUMP: /* ZJUMP */
						if (pop(ctx) == 0) {
							p = CAR(NEXT(p));
						} else {
							p = NEXT(NEXT(p));
						}
						break;
					case JUMP: /* JUMP */
						p = CAR(NEXT(p));
						break;
					default:
						printf("Executing primitive: %ld\n", CAR(p));
						CELL prim = find_primitive(ctx, CAR(p));
						if (prim) {
							printf("Primitive: %s\n", NFA(prim));
						} else {
							printf("NULL primitive!!!!!\n");
						}
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
		// TODO: Atom should be executed as a primitive
		case ATOM: ERR_PUSH(ctx, CAR(S(ctx))); break; 
		case LIST: dump_list(ctx, CAR(S(ctx)), 1); result = execute(ctx, CAR(S(ctx))); break;
		case PRIM: result = ((FUNC)CAR(S(ctx)))(ctx); break;
		case WORD: result = dbg_execute(ctx, XT(CAR(S(ctx)))); break;
	}

	if (result < 0) { ERR(ctx, result); } else { return result; }
}

void dump_context(CTX* ctx) {
	printf("STATE: %ld\n", ctx->state);
	printf("FREE NODES: %ld\n", FREE(ctx));
	printf("STACK PILE:\n");
	for (CELL s = ctx->pile; s; s = NEXT(s)) {
		printf("[%ld] ", s); dump_list(ctx, s, 1); printf("\n");
	}
	printf("\n");
	printf("COMPILING PILE:\n");
	for (CELL s = ctx->cpile; s; s = NEXT(s)) {
		if (TYPE(s) == LIST) {
			printf("List:   [%ld] ", CAR(s)); dump_list(ctx, CAR(s), 1); printf("\n");
		} else if (TYPE(s) == WORD) {
			printf("Header: [%ld] %s", CAR(s), NFA(s));
			if (XT(s) != 0) {
				printf(" "); dump_list(ctx, XT(s), 1);
			}
			printf("\n");
		}
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
		printf("TOKEN: %.*s\n\n", (int)TL(ctx), TK(ctx));
		if ((word = find_token(ctx)) != 0) {
			if (strcmp(NFA(word), "fib") == 0) {
				printf("Executing FIB: %ld XT: %ld \n", word, XT(word));
				dump_list(ctx, XT(word), 1);
			}
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
				if (compile_number(ctx, number) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
			} else {
				ERR_PUSH(ctx, number);
			}
		}
		dump_context(ctx);
		printf("----------------------------\n");
	} while (1);
}

CELL words(CTX* ctx) {
	CELL w = ctx->latest;
	while (w) {
		printf("([%ld] %s {%ld}) ", w, NFA(w), XT(w));
		w = NEXT(w);
	}
}

CELL sp_fetch(CTX* ctx) {
	ERR_PUSH(ctx, S(ctx));
	return 0;
}

CELL pair(CTX* ctx) {
	CELL p; ERR_POP(ctx, p);
	if (p) printf("[%ld] NEXT: %ld | TYPE: %ld | CAR: %ld\n", p, NEXT(p), TYPE(p), CAR(p));
	else printf("Not a pair\n");
	return 0;
}

CELL next(CTX* ctx) {
	CELL p; ERR_POP(ctx, p);
	ERR_PUSH(ctx, NEXT(p));
	return 0;
}

CELL see(CTX* ctx) {
	CELL result;
	if ((result = parse_name(ctx)) != 0) { ERR(ctx, result); }
	CELL w = ctx->latest;
	while (w) {
		if (strncmp((BYTE*)CAR(NEXT(S(ctx))), NFA(w), CAR(S(ctx))) == 0) {
			printf("[%ld] %s ", w, NFA(w)); 
			dump_list(ctx, XT(w), 1); 
			printf("\n"); 
			return 0;
		} else {
			w = NEXT(w);
		}
	}
	drop(ctx);
	drop(ctx);
	return 0;
}

CELL nfa(CTX* ctx) {
	CELL w; ERR_POP(ctx, w);
	printf("NAME: %s\n", NFA(w));
	return 0;
}

CELL xt(CTX* ctx) {
	CELL w; ERR_POP(ctx, w);
	printf("WORD: %ld\n", w);
	printf("XT(w): %ld\n", XT(w));
	//dump_list(ctx, XT(w), 1);
	return 0;
}

#endif
