#include<inttypes.h>
#include<string.h>
#include<stdio.h>
#include<ctype.h>

#ifdef _WIN32
	#include<conio.h>
#else
	#include<unistd.h>
	#include<termios.h>
#endif

typedef int8_t			BYTE;
typedef intptr_t		CELL;

#define CAR(pair)								(*((CELL*)pair))
#define CDR(pair)								(*(((CELL*)pair) + 1))
#define NEXT(pair)							(CDR(pair) & -4)
#define TYPE(pair)							(CDR(pair) & 3)

enum Types { ATOM, LIST, PRIM, WORD };

#define AS(type, ref)						((ref & -4) | type)
#define LINK(pair, next)				(CDR(pair) = AS(TYPE(pair), next))

enum Words { CMP_PRIMITIVE, CMP_COLON_DEF, IMM_PRIMITIVE, IMM_COLON_DEF };

#define NFA(word)								((BYTE*)CAR(CAR(word)))
#define XT(word)								(NEXT(CAR(word)))
#define PRIMITIVE(word)					((TYPE(word) & 1) == 0)
#define IMMEDIATE(word)					((TYPE(word) & 2) == 2)

typedef struct {
	BYTE *tib, *here;
	CELL there, size; 
	CELL free, pile, stack, cfstack, latest;
	CELL ip, state, token, in, base;
} CTX;

#define S(ctx)									CAR(ctx->stack)

#define FREE(ctx)						(length(ctx->free) - 1)		// Don't count ctx->there
#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))

#define BOTTOM(ctx)			(((BYTE*)ctx) + sizeof(CTX))
#define TOP(ctx)				(ALIGN(((BYTE*)ctx) + ctx->size - (2*sizeof(CELL)) - 1, (2*sizeof(CELL))))

CTX* init(BYTE* block, CELL size) {
	if (size < (sizeof(CTX) + 2*2*sizeof(CELL))) return 0;
	CTX* ctx = (CTX*)block;	
	ctx->size = size;
	ctx->here = BOTTOM(ctx);
	ctx->there = ALIGN(BOTTOM(ctx), 2*sizeof(CELL));
	ctx->pile = TOP(ctx);
	CAR(ctx->pile) = 0;
	CDR(ctx->pile) = AS(LIST, 0);
	ctx->stack = ctx->pile;
	ctx->free = TOP(ctx) - 2*sizeof(CELL);

	for (CELL pair = ctx->there; pair <= ctx->free; pair += 2*sizeof(CELL)) {
		CAR(pair) = pair == ctx->free ? 0 : pair + 2*sizeof(CELL);
		CDR(pair) = pair == ctx->there ? 0 : pair - 2*sizeof(CELL);
	}

	ctx->cfstack = ctx->latest = 0;
	ctx->state = ctx->token = ctx->in = 0;
	ctx->tib = 0;
	ctx->base = 10;

	return ctx;
}

CELL cons(CTX* ctx, CELL car, CELL cdr) {
	if (ctx->free == ctx->there) return 0;
	CELL pair = ctx->free;
	ctx->free = CDR(ctx->free);
	CAR(pair) = car;
	CDR(pair) = cdr;
	return pair;
}

CELL clone(CTX* ctx, CELL pair) {
	if (!pair) return 0;
	if (TYPE(pair) == LIST) {
		return cons(ctx, clone(ctx, CAR(pair)), AS(TYPE(pair), clone(ctx, NEXT(pair))));
	} else {
		return cons(ctx, CAR(pair), AS(TYPE(pair), clone(ctx, NEXT(pair))));
	}
}

CELL reclaim(CTX* ctx, CELL pair) {
	if (!pair) return 0;
	if (TYPE(pair) == LIST) { 
		while (CAR(pair) != 0) { 
			CAR(pair) = reclaim(ctx, CAR(pair)); 
		} 
	}
	CELL tail = NEXT(pair);
	CDR(pair) = ctx->free;
	CAR(pair) = 0;
	ctx->free = pair;
	return tail;
}

CELL reverse(CELL pair, CELL list) {
	if (pair != 0) {
		CELL t = NEXT(pair);
		CDR(pair) = AS(TYPE(pair), list);
		return reverse(t, pair);
	} else {
		return list;
	}
}

CELL length(CELL pair) { 
	CELL c = 0; 
	while (pair) { 
		c++; 
		pair = NEXT(pair); 
	} 
	return c; 
}

#define PUSH(ctx, v)				(S(ctx) = cons(ctx, (CELL)v, AS(ATOM, S(ctx))))
#define PUSHL(ctx, v)				(S(ctx) = cons(ctx, (CELL)v, AS(LIST, S(ctx))))

CELL pop(CTX* ctx) {
	if (S(ctx) == 0) { return 0; /* ERROR */ }
	CELL v = CAR(S(ctx));
	S(ctx) = reclaim(ctx, S(ctx));
	return v;
}

#define ERR_STACK_OVERFLOW			-1
#define ERR_RSTACK_OVERFLOW			-2
#define ERR_STACK_UNDERFLOW			-3
#define ERR_RSTACK_UNDERFLOW		-4
#define ERR_UNDEFINED_WORD			-5
#define ERR_NOT_ENOUGH_MEMORY		-6
#define ERR_NOT_ENOUGH_RESERVED	-7
#define ERR_ZERO_LENGTH_NAME		-8
#define ERR_ATOM_EXPECTED				-9
#define ERR_LIST_EXPECTED				-10
#define ERR_END_OF_XLIST				-11
#define ERR_EXIT								-12

CELL error(CTX* ctx, CELL err) {
	// TODO
	/* Lookup on exception stack for a correct handler for current error */
	/* The debugger must be installed on the exception stack for it to work */
	/* If a handler its found then execute it and return its return value */
	/* If none its found, just return the error */
	// TODO
	/* The error function should have the ability to not just modify the context */
	/* but of returning a compleletely new context to work on. For example, in the case */
	/* of a memory error, a new context with a bigger buffer could be created, cloning */
	/* the current context on it and returning it as the new context. */
	return err;
}

#define ERR(ctx, err)		{ CELL __err__ = error(ctx, err); if (__err__) { return __err__; } }

#define ERR_POP(ctx, v) \
	if (S(ctx) == 0) { \
		ERR(ctx, ERR_STACK_UNDERFLOW); \
	} else { \
		v = pop(ctx); \
	}

typedef CELL (*FUNC)(CTX*);

#define EXECUTE_PRIMITIVE(ctx, primitive) { \
	CELL result = ((FUNC)primitive)(ctx); \
	if (result < 0) { ERR(ctx, result); } \
}

CELL execute(CTX* ctx, CELL xlist) {
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

#define TC(ctx)									(*(ctx->tib + ctx->in))
#define TK(ctx)									(ctx->tib + ctx->token)
#define TL(ctx)									(ctx->in - ctx->token)

CELL parse_token(CTX* ctx) {
	ctx->token = ctx->in;	while (TC(ctx) != 0 && isspace(TC(ctx))) { ctx->in++;	}
	ctx->token = ctx->in;	while (TC(ctx) != 0 && !isspace(TC(ctx))) { ctx->in++; }
	return ctx->in - ctx->token;
}

CELL find_token(CTX* ctx) {
	CELL w = ctx->latest;
	while (w && !(strlen(NFA(w)) == TL(ctx) && strncmp(NFA(w), TK(ctx), TL(ctx)) == 0)) {
		w = NEXT(w);
	}
	return w;
}

CELL compile_word(CTX* ctx, CELL word) {
	if (TYPE(word) == ATOM || TYPE(word) == PRIM) {
		return (S(ctx) = cons(ctx, CAR(XT(word)), AS(PRIM, S(ctx))));
	} else {
		return (S(ctx) = cons(ctx, word, AS(WORD, S(ctx))));
	}
}

CELL evaluate(CTX* ctx, BYTE* str) { 
	CELL word, result; 
	char *endptr;
	ctx->tib = str; 
	ctx->token = ctx->in = 0;
	do {
		if (parse_token(ctx) == 0) { return 0; }
		if ((word = find_token(ctx)) != 0) {
			if (!ctx->state || IMMEDIATE(word)) {
				if ((result = execute(ctx, XT(word))) != 0) { ERR(ctx, result); }
			} else {
				if (compile_word(ctx, word) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
			}
		} else {
			intmax_t number = strtoimax(TK(ctx), &endptr, 10);
			if (number == 0 && endptr == (char*)(TK(ctx))) {
				ERR(ctx, ERR_UNDEFINED_WORD);
			} else {
				if (PUSH(ctx, number) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
			}
		}
	} while (1);
}

CELL grow(CTX* ctx) { 
	if (CAR(ctx->there) == (ctx->there + 2*sizeof(CELL))) { 
		ctx->there += 2*sizeof(CELL);
		CDR(ctx->there) = 0;
		return 0;
	} else {
		return ERR_NOT_ENOUGH_MEMORY;
	}
}

#define RESERVED(ctx)				((ctx->there) - ((CELL)ctx->here))

CELL shrink(CTX* ctx) {
	if (RESERVED(ctx) >= 2*sizeof(CELL)) {
		CDR(ctx->there) = ctx->there - 2*sizeof(CELL);
		ctx->there -= 2*sizeof(CELL);
		CAR(ctx->there) = ctx->there + 2*sizeof(CELL);
		CDR(ctx->there) = 0;
		return 0;
	} else {
		return ERR_NOT_ENOUGH_RESERVED;
	}
}

CELL allot(CTX* ctx) {
	CELL bytes; ERR_POP(ctx, bytes);
	if (bytes == 0) {
		return 0;
	} else if (bytes > 0) { 
		if (bytes < (TOP(ctx) - ((CELL)ctx->here))) {
			while (RESERVED(ctx) < bytes) { 
				if (grow(ctx) != 0) { ERR(ctx, ERR_NOT_ENOUGH_MEMORY); } 
			}
			ctx->here += bytes;
		} else {
			ERR(ctx, ERR_NOT_ENOUGH_MEMORY);
		}
	} else if (bytes < 0) {
		if (bytes < (BOTTOM(ctx) - ctx->here)) {
			ctx->here = BOTTOM(ctx);
		} else {
			ctx->here += bytes;
		}
		while (RESERVED(ctx) >= 2*sizeof(CELL)) { 
			if (shrink(ctx) != 0) { ERR(ctx, ERR_NOT_ENOUGH_RESERVED); } 
		}
	}
	return 0;
}

CELL postpone(CTX* ctx) {
	CELL word;
	if (parse_token(ctx) == 0) { ERR(ctx, ERR_ZERO_LENGTH_NAME); }
	if ((word = find_token(ctx)) == 0) { ERR(ctx, ERR_UNDEFINED_WORD); }
	if (compile_word(ctx, word) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
	return 0;
}

CELL parse(CTX* ctx) { 
	CELL c; ERR_POP(ctx, c);
	ctx->token = ctx->in;	
	if (PUSH(ctx, TK(ctx)) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
	while (TC(ctx) != 0 && TC(ctx) != c) { ctx->in++;	}
	if (PUSH(ctx, TL(ctx)) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
	if (TC(ctx) != 0) ctx->in++;
	return 0;
}

CELL parse_name(CTX* ctx) { 
	parse_token(ctx);
	if (PUSH(ctx, TK(ctx)) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
	if (PUSH(ctx, TL(ctx)) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
	return 0; 
}

CELL spush(CTX* ctx) {
	if ((ctx->pile = cons(ctx, 0, AS(LIST, ctx->pile))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
	ctx->stack = ctx->pile;
	return 0;
}

CELL clear(CTX* ctx) {
	while (S(ctx)) S(ctx) = reclaim(ctx, S(ctx));
}

CELL sdrop(CTX* ctx) {
	if (NEXT(ctx->pile) == 0) { return clear(ctx); }
	ctx->stack = ctx->pile = reclaim(ctx, ctx->pile);
	return 0;
}

CELL stack_to_list(CTX* ctx) {
	if (NEXT(ctx->pile) == 0) {
		ctx->stack = ctx->pile = cons(ctx, ctx->pile, AS(LIST, 0));
	} else {
		CELL t = ctx->pile;
		ctx->stack = ctx->pile = NEXT(ctx->pile);
		LINK(t, CAR(ctx->pile));
		CAR(ctx->pile) = t;
	}
	return 0;
}

CELL list_to_stack(CTX* ctx) {
	if (S(ctx) == 0) {
		spush(ctx);
	} else {
		CELL t = S(ctx);
		S(ctx) = NEXT(S(ctx));
		LINK(t, ctx->pile);
		ctx->stack = ctx->pile = t;
	}
}

CELL lbracket(CTX* ctx) {
	ctx->state = 0;
	return 0;
}

CELL rbracket(CTX* ctx) {
	ctx->state = 1;
	return 0;
}

CELL reverse_stack(CTX* ctx) {
	S(ctx) = reverse(S(ctx), 0);
	return 0;
}

CELL lbrace(CTX* ctx) {
	spush(ctx);
	rbracket(ctx);
	return 0;
}

CELL rbrace(CTX* ctx) {
	lbracket(ctx);
	reverse_stack(ctx);
	stack_to_list(ctx);
	return 0;
}

CELL duplicate(CTX* ctx) {
	if (S(ctx) == 0) { ERR(ctx, ERR_STACK_UNDERFLOW); }
	switch(TYPE(S(ctx))) {
		case ATOM: if (PUSH(ctx, CAR(S(ctx))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); } break;
		case LIST: if (PUSHL(ctx, clone(ctx, CAR(S(ctx)))) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); } break;
		case PRIM:
			if ((S(ctx) = cons(ctx, CAR(S(ctx)), AS(PRIM, S(ctx)))) == 0) {
				ERR(ctx, ERR_STACK_OVERFLOW);
			}
			break;
		case WORD:
			if ((S(ctx) = cons(ctx, CAR(S(ctx)), AS(WORD, S(ctx)))) == 0) {
				ERR(ctx, ERR_STACK_OVERFLOW);
			}
			break;
	}

	return 0;
}

CELL swap(CTX* ctx) {
	CELL t = S(ctx);
	S(ctx) = NEXT(S(ctx));
	LINK(t, NEXT(S(ctx)));
	LINK(S(ctx), t);
	return 0;
}

CELL drop(CTX* ctx) {
	S(ctx) = reclaim(ctx, S(ctx));
	return 0;
}

CELL rot(CTX* ctx) {
	CELL t = NEXT(NEXT(S(ctx)));
	LINK(NEXT(S(ctx)), NEXT(t));
	LINK(t, S(ctx));
	S(ctx) = t;
	return 0;
}

CELL over(CTX* ctx) {
	if (S(ctx) == 0 || NEXT(S(ctx)) == 0) { ERR(ctx, ERR_STACK_UNDERFLOW); }
	switch(TYPE(NEXT(S(ctx)))) {
		case ATOM: 
			if (PUSH(ctx, CAR(NEXT(S(ctx)))) == 0) { 
				ERR(ctx, ERR_STACK_OVERFLOW); 
			} 
			break;
		case LIST: 
			if (PUSHL(ctx, clone(ctx, CAR(NEXT(S(ctx))))) == 0) { 
				ERR( ctx, ERR_STACK_OVERFLOW); 
			} 
			break;
		case PRIM: 
			if ((S(ctx) = cons(ctx, CAR(S(ctx)), AS(PRIM, S(ctx)))) == 0) {
				ERR(ctx, ERR_STACK_OVERFLOW);
			}
			break;
		case WORD:
			if ((S(ctx) = cons(ctx, CAR(S(ctx)), AS(WORD, S(ctx)))) == 0) {
				ERR(ctx, ERR_STACK_OVERFLOW);
			}
			break;
	}
	return 0;
}

CELL exec(CTX* ctx) {
	CELL result = 0;
	CELL t = S(ctx);
	S(ctx) = NEXT(S(ctx));
	LINK(t, 0);

	switch (TYPE(t)) {
		case ATOM: if (PUSH(ctx, CAR(t)) == 0) result = ERR_STACK_OVERFLOW; break;
		case LIST: result = execute(ctx, CAR(t)); break;
		case PRIM: result = ((FUNC)CAR(t))(ctx); break;
		case WORD: result = execute(ctx, XT(CAR(t))); break;
	}

	reclaim(ctx, t);
	if (result < 0) { ERR(ctx, result); } else { return result; }
}

CELL branch(CTX* ctx) {
	rot(ctx);
	CELL b = pop(ctx);
	if (!b) {
		swap(ctx);
	}
	drop(ctx);
	return exec(ctx);
}

// --------------------------------------------------------------- Throughly tested until here

// STACK PRIMITIVES

//CELL rot(CTX* ctx) {
//	if (ctx->stack == 0 || NEXT(ctx->stack) == 0 || NEXT(NEXT(ctx->stack)) == 0) {
//		return ERR_STACK_UNDERFLOW;
//	}
//	CELL t = ctx->stack;
//	ctx->stack = NEXT(NEXT(ctx->stack));
//	CDR(NEXT(t)) = AS(TYPE(NEXT(t)), NEXT(ctx->stack));
//	CDR(ctx->stack) = AS(TYPE(ctx->stack), t);
//
//	return 0;
//}

// ARITHMETIC PRIMITIVES

CELL add(CTX* ctx) {
	CELL a = pop(ctx);
	CELL b = pop(ctx);
	PUSH(ctx, b + a);
	return 0;
}

CELL sub(CTX* ctx) {
	CELL a = pop(ctx);
	CELL b = pop(ctx);
	PUSH(ctx, b - a);
	return 0;
}

//CELL add(CTX* ctx) {
//	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
//	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) + CAR(ctx->stack);
//	ctx->stack = reclaim(ctx, ctx->stack);
//
//	return 0;
//}
//
//CELL sub(CTX* ctx) {
//	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
//	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) - CAR(ctx->stack);
//	ctx->stack = reclaim(ctx, ctx->stack);
//
//	return 0;
//}

CELL mul(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) * CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

CELL division(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) / CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

CELL mod(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) % CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

// COMPARISON PRIMITIVES

CELL gt(CTX* ctx) {
	CELL a = pop(ctx);
	CELL b = pop(ctx);
	PUSH(ctx, b > a);
	return 0;
}

//CELL gt(CTX* ctx) {
//	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
//	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) > CAR(ctx->stack);
//	ctx->stack = reclaim(ctx, ctx->stack);
//
//	return 0;
//}

CELL lt(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) < CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

CELL eq(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) == CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

// BIT PRIMITIVES

CELL and(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) & CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

CELL or(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) | CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

CELL invert(CTX* ctx) {
	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(ctx->stack) = ~CAR(ctx->stack);

	return 0;
}



//CELL compile_str(CTX* ctx) {
//	CELL str_start;
//	while (*(ctx->tib + ctx->in) != 0 && isspace(*(ctx->tib + ctx->in))) {
//		ctx->in++;
//	}
//	str_start = ctx->in;
//	while (*(ctx->tib + ctx->in) != 0 && *(ctx->tib + ctx->in) != '"') {
//		ctx->in++;
//	}
//	BYTE* str = ctx->here;
//	ctx->stack = cons(ctx, sizeof(CELL) + (ctx->in - str_start) + 1, AS(ATOM, ctx->stack));
//	if (allot(ctx) != 0) { return ERR_NOT_ENOUGH_MEMORY; }
//	*((CELL*)str) = ctx->in - str_start;
//	str += sizeof(CELL);
//	for (CELL i = 0; i < ctx->in - str_start; i++) {
//		str[i] = ctx->tib[str_start + i];
//	}
//	str[ctx->in - str_start] = 0;
//	if (ctx->state) {
//		CAR(ctx->cpile) = cons(ctx, (CELL)str, AS(ATOM, CAR(ctx->cpile)));
//		//CAR(ctx->cpile) = cons(ctx, ctx->in - str_start, AS(ATOM, CAR(ctx->cpile)));
//	} else {
//		ctx->stack = cons(ctx, (CELL)str, AS(ATOM, ctx->stack));
//		//ctx->stack = cons(ctx, ctx->in - str_start, AS(ATOM, ctx->stack));
//	}
//	ctx->in++;
//	return 0;
//}
//
//// PRIMITIVES
//
//// LIST OPERATIONS
//
//CELL append(CTX* ctx) {
//	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(NEXT(ctx->stack)) == LIST) {
//		CELL t = NEXT(ctx->stack);
//		CDR(ctx->stack) = AS(TYPE(ctx->stack), CAR(NEXT(ctx->stack)));
//		CAR(t) = ctx->stack;
//		ctx->stack = t;
//	} else if (TYPE(NEXT(ctx->stack)) == ATOM) {
//		CELL* dest = (CELL*)CAR(NEXT(ctx->stack));	
//		CELL t = NEXT(ctx->stack);
//		CDR(ctx->stack) = AS(TYPE(ctx->stack), *dest);
//		*dest = ctx->stack;
//		ctx->stack = reclaim(ctx, t);
//	}
//
//	return 0;
//}
//
//CELL lbrace(CTX* ctx) {
// ctx->state = 1;
// ctx->cpile = cons(ctx, 0, AS(LIST, ctx->cpile));
//
// return 0;
//}
//
//CELL rbrace(CTX* ctx) {
//  CELL list = reverse(CAR(ctx->cpile), 0);
//  CAR(ctx->cpile) = 0;
//  ctx->cpile = reclaim(ctx, ctx->cpile);
//  if (ctx->cpile == 0) {
//		ctx->stack = cons(ctx, list, AS(LIST, ctx->stack));
//		ctx->state = 0;
//  } else {
//		CAR(ctx->cpile) = cons(ctx, list, AS(LIST, CAR(ctx->cpile)));
//  }
//
//  return 0;
//}
//
//CELL find_prim(CTX* ctx, CELL xt) {
//	CELL word = ctx->latest;
//	while (word && CAR(XT(word)) != xt) { 
//		word = NEXT(word); }
//	return word;
//}
//
//void dump_list(CTX* ctx, CELL pair, CELL order) {
//	CELL word;
//	if (pair) {
//		if (order) dump_list(ctx, NEXT(pair), order);
//		switch (TYPE(pair)) {
//			case ATOM: printf("%ld ", CAR(pair)); break;
//			case LIST: printf("{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
//			case PRIM: 
//				word = find_prim(ctx, CAR(pair));
//				if (word) {
//					printf("%s ", (BYTE*)(NFA(word)));
//				} else {
//					printf("PRIM_NOT_FOUND ");
//				}
//				break;
//			case WORD: printf("X{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
//		}
//		if (!order) dump_list(ctx, NEXT(pair), order);
//	}
//}
//
//CELL dump_stack(CTX* ctx) {
//	printf("<%ld> ", length(ctx->stack));
//	dump_list(ctx, ctx->stack, 1);
//	printf("\n");
//
//	return 0;
//}
//
//CELL dump_cpile(CTX* ctx) {
//	printf("COMPILED: <%ld> ", length(ctx->cpile));
//	dump_list(ctx, ctx->cpile, 1);
//	printf("\n");
//}
//
//CELL words(CTX* ctx) {
//	printf("WORDS: ");
//	CELL p = ctx->latest;
//	while (p) { printf("%s ", (BYTE*)NFA(p)); p = NEXT(p); }
//	printf("\n");
//}
//
//// PRIMITIVES
//
//CELL fetch(CTX* ctx) {
//	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
//	CAR(ctx->stack) = *((CELL*)(CAR(ctx->stack)));
//
//	return 0;
//}
//
//CELL store(CTX* ctx) {
//	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
//	CELL addr = CAR(ctx->stack);
//	CELL x = CAR(NEXT(ctx->stack));
//	ctx->stack = reclaim(ctx, reclaim(ctx, ctx->stack));
//	*((CELL*)addr) = x;
//
//	return 0;
//}
//
//CELL literal(CTX* ctx) {
//	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
//	CELL x = CAR(ctx->stack);
//	ctx->stack = reclaim(ctx, ctx->stack);
//	CAR(ctx->cpile) = cons(ctx, x, AS(ATOM, CAR(ctx->cpile)));
//	return 0;
//}
//
//CELL see(CTX* ctx) {
//	// TODO
//	parse_token(ctx);
//	CELL word = find_token(ctx);
//	if (word) {
//		printf(": %s ", (BYTE*)NFA(word));
//		if (PRIMITIVE(word)) {
//			printf("; PRIMITIVE");
//		} else {
//			dump_list(ctx, XT(word), 1);
//			printf("; ");
//		}
//		printf("\n");
//	}
//	return 0;
//}
//
//CELL exec(CTX* ctx) {
//	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
//	CELL xt = CAR(ctx->stack);
//	CAR(ctx->stack) = 0;
//	ctx->stack = reclaim(ctx, ctx->stack);
//	if (NEXT(ctx->ip) != 0) {
//		if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
//		ctx->rstack = cons(ctx, NEXT(ctx->ip), AS(ATOM, ctx->rstack));
//	}
//	ctx->ip = xt;
//
//	return 1;
//}
//
//CELL exec_x(CTX* ctx) {
//	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
//	CELL xt = CAR(ctx->stack);
//	if (NEXT(ctx->ip) != 0) {
//		if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
//		ctx->rstack = cons(ctx, NEXT(ctx->ip), AS(ATOM, ctx->rstack));
//	}
//	ctx->ip = xt;
//
//	return 1;
//}
//
//CELL type(CTX* ctx) {
//	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
//	CELL l = CAR(ctx->stack);
//	BYTE* s = (BYTE*)CAR(NEXT(ctx->stack));
//	ctx->stack = reclaim(ctx, reclaim(ctx, ctx->stack));
//	printf("%.*s", (int)l, s);
//	return 0;
//}
//
//CELL sliteral(CTX* ctx) {
//	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
//	CELL l = CAR(ctx->stack);
//	BYTE* s = (BYTE*)CAR(NEXT(ctx->stack));
//	ctx->stack = reclaim(ctx, reclaim(ctx, ctx->stack));
//	BYTE* str = ctx->here;
//	ctx->stack = cons(ctx, sizeof(CELL) + l + 1, AS(ATOM, ctx->stack));
//	if (allot(ctx) != 0) { return ERR_NOT_ENOUGH_MEMORY; }
//	*((CELL*)str) = l;
//	str += sizeof(CELL);
//	for (CELL i = 0; i < l; i++) { str[i] = s[i]; }
//	str[l] = 0;
//	if (ctx->state) {
//		CAR(ctx->cpile) = cons(ctx, (CELL)str, AS(ATOM, CAR(ctx->cpile)));
//	} else {
//		ctx->stack = cons(ctx, (CELL)str, AS(ATOM, ctx->stack));
//	}
//	//ctx->stack = cons(ctx, l, AS(ATOM, ctx->stack));
//	return 0;
//}
//
//CELL here(CTX* ctx) {
//	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
//	ctx->stack = cons(ctx, (CELL)ctx->here, AS(ATOM, ctx->stack));
//	return 0;
//}
//
//CELL carcdr(CTX* ctx) {
//	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != LIST) { return ERR_LIST_EXPECTED; }
//	if (CAR(ctx->stack) == 0) { return 0; }
//	CELL h = CAR(ctx->stack);
//	CAR(ctx->stack) = NEXT(CAR(ctx->stack));
//	CDR(h) = AS(TYPE(h), ctx->stack);
//	ctx->stack = h;
//	return 0;
//}
//
//CELL comma(CTX* ctx) {
//	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
//	CELL v = CAR(ctx->stack);
//	ctx->stack = reclaim(ctx, ctx->stack);
//	CELL* here = (CELL*)ctx->here;
//	ctx->stack = cons(ctx, sizeof(CELL), AS(ATOM, ctx->stack));
//	if (allot(ctx) != 0) { return ERR_NOT_ENOUGH_MEMORY; }
//	*here = v;
//	return 0;
//}
//
//CELL align(CTX* ctx) {
//	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
//	ctx->stack = cons(ctx, ALIGN(ctx->here, sizeof(CELL)) - (CELL)ctx->here, AS(ATOM, ctx->stack));
//	return allot(ctx);
//}
//
//CELL cells(CTX* ctx) {
//	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
//	CAR(ctx->stack) = CAR(ctx->stack) * sizeof(CELL);
//	return 0;
//}
//
//CELL immediate(CTX* ctx) {
//	if (ctx->latest == 0) return 0;
//	CDR(ctx->latest) = CDR(ctx->latest) | 2;
//	return 0;
//}
//
//#define WORD(n, f) \
//	ctx->latest = \
//		cons(ctx, \
//			cons(ctx, (CELL)n, AS(ATOM, \
//			cons(ctx, (CELL)f, AS(PRIM, 0)))), \
//		AS(ATOM, ctx->latest));
//
//#define IMM(n, f) \
//	ctx->latest = \
//		cons(ctx, \
//			cons(ctx, (CELL)n, AS(ATOM, \
//			cons(ctx, (CELL)f, AS(PRIM, 0)))), \
//		AS(PRIM, ctx->latest));
//
//CTX* bootstrap(CTX* ctx) {
//	// STACK PRIMITIVES
//	WORD("dup", &duplicate);
//	WORD("swap", &swap);
//	WORD("over", &over);
//	WORD("rot", &rot);
//	WORD("drop", &drop);
//
//	// IP PRIMITIVES
//	WORD("branch", &branch);
//	WORD("jump", &jump);
//	WORD("zjump", &zjump);
//
//	// ARITHMETIC PRIMITIVES
//	WORD("+", &add);
//	WORD("-", &sub);
//	WORD("*", &mul);
//	WORD("/", &division);
//	WORD("%", &mod);
//
//	// COMPARISON PRIMITIVES
//	WORD(">", &gt);
//	WORD("<", &lt);
//	WORD("=", &eq);
//
//	// BIT PRIMITIVES
//	WORD("and", &and);
//	WORD("or", &or);
//	WORD("invert", &invert);
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"immediate", AS(ATOM, 
//			cons(ctx, (CELL)&immediate, AS(PRIM, 0)))), 
//		AS(PRIM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"postpone", AS(ATOM, 
//			cons(ctx, (CELL)&postpone, AS(PRIM, 0)))), 
//		AS(PRIM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"cells", AS(ATOM, 
//			cons(ctx, (CELL)&cells, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)",", AS(ATOM, 
//			cons(ctx, (CELL)&comma, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"align", AS(ATOM, 
//			cons(ctx, (CELL)&align, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"carcdr", AS(ATOM, 
//			cons(ctx, (CELL)&carcdr, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"sliteral", AS(ATOM, 
//			cons(ctx, (CELL)&sliteral, AS(PRIM, 0)))), 
//		AS(PRIM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"type", AS(ATOM, 
//			cons(ctx, (CELL)&type, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"parse-name", AS(ATOM, 
//			cons(ctx, (CELL)&parse_name, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"x", AS(ATOM, 
//			cons(ctx, (CELL)&exec_x, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"execute", AS(ATOM, 
//			cons(ctx, (CELL)&exec, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"see", AS(ATOM, 
//			cons(ctx, (CELL)&see, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"parse", AS(ATOM, 
//			cons(ctx, (CELL)&parse, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"literal", AS(ATOM, 
//			cons(ctx, (CELL)&literal, AS(PRIM, 0)))), 
//		AS(PRIM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"[", AS(ATOM, 
//			cons(ctx, (CELL)&lbracket, AS(PRIM, 0)))), 
//		AS(PRIM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"]", AS(ATOM, 
//			cons(ctx, (CELL)&rbracket, AS(PRIM, 0)))), 
//		AS(PRIM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"@", AS(ATOM, 
//			cons(ctx, (CELL)&fetch, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"!", AS(ATOM, 
//			cons(ctx, (CELL)&store, AS(PRIM, 0)))), 
//		AS(ATOM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx, 
//			cons(ctx, (CELL)"s\"", AS(ATOM, 
//			cons(ctx, (CELL)&compile_str, AS(PRIM, 0)))), 
//		AS(PRIM, ctx->latest));	
//
//	ctx->latest = 
//		cons(ctx,
//			cons(ctx, (CELL)"allot", AS(ATOM,
//			cons(ctx, (CELL)&allot, AS(PRIM, 0)))),
//		AS(ATOM, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"append", AS(ATOM,
//			cons(ctx, (CELL)&append, AS(PRIM, 0)))),
//		AS(ATOM, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)".s", AS(ATOM,
//			cons(ctx, (CELL)&dump_stack, AS(PRIM, 0)))),
//		AS(ATOM, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)".c", AS(ATOM,
//			cons(ctx, (CELL)&dump_cpile, AS(PRIM, 0)))),
//		AS(ATOM, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"{", AS(ATOM,
//			cons(ctx, (CELL)&lbrace, AS(PRIM, 0)))),
//		AS(PRIM, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"}", AS(ATOM,
//			cons(ctx, (CELL)&rbrace, AS(PRIM, 0)))),
//		AS(PRIM, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"words", AS(ATOM,
//			cons(ctx, (CELL)&words, AS(PRIM, 0)))),
//		AS(ATOM, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"here", AS(ATOM,
//			cons(ctx, (CELL)&here, AS(PRIM, 0)))),
//		AS(ATOM, ctx->latest));
//
//	// VARIABLES
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"there", AS(ATOM,
//			cons(ctx, (CELL)&ctx->there, AS(ATOM, 0)))),
//		AS(LIST, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"state", AS(ATOM,
//			cons(ctx, (CELL)&ctx->state, AS(ATOM, 0)))),
//		AS(LIST, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"latest", AS(ATOM,
//			cons(ctx, (CELL)&ctx->latest, AS(ATOM, 0)))),
//		AS(LIST, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"hstack", AS(ATOM,
//			cons(ctx, (CELL)&ctx->hstack, AS(ATOM, 0)))),
//		AS(LIST, ctx->latest));
//
//	ctx->latest =
//		cons(ctx,
//			cons(ctx, (CELL)"base", AS(ATOM,
//			cons(ctx, (CELL)&ctx->base, AS(ATOM, 0)))),
//		AS(LIST, ctx->latest));
//
//	return ctx;
//}
