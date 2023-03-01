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
	CELL free, pile, stack, rstack, latest;
	CELL ip, state, token, in, base;
} CTX;

#define S(ctx)									CAR(ctx->stack)
#define R(ctx)									ctx->rstack

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
	CDR(ctx->pile) = 0;
	ctx->stack = ctx->pile;
	ctx->free = TOP(ctx) - 2*sizeof(CELL);

	for (CELL pair = ctx->there; pair <= ctx->free; pair += 2*sizeof(CELL)) {
		CAR(pair) = pair == ctx->free ? 0 : pair + 2*sizeof(CELL);
		CDR(pair) = pair == ctx->there ? 0 : pair - 2*sizeof(CELL);
	}

	ctx->rstack = ctx->latest = 0;
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

#define PUSH(ctx, v)			S(ctx) = cons(ctx, (CELL)v, AS(ATOM, S(ctx)))

CELL pop(CTX* ctx) {
	if (S(ctx) == 0) { /* ERROR */ }
	CELL v = CAR(S(ctx));
	S(ctx) = reclaim(ctx, S(ctx));
	return v;
}

// Throughly tested until here

typedef CELL (*FUNC)(CTX*);

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_UNDEFINED_WORD			-3
#define ERR_NOT_ENOUGH_MEMORY		-4
#define ERR_ZERO_LENGTH_NAME		-5
#define ERR_ATOM_EXPECTED				-6
#define ERR_LIST_EXPECTED				-7
#define ERR_RSTACK_UNDERFLOW		-8
#define ERR_EXIT								-9

#define RESERVED(ctx)				((ctx->there) - ((CELL)ctx->here))

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

// INNER INTERPRETER

CELL execute(CTX* ctx, CELL xlist) {
	CELL result;
	ctx->ip = xlist;
	do {
		if (ctx->ip == 0) {
			if (ctx->rstack) {
				ctx->ip = CAR(ctx->rstack);
				ctx->rstack = reclaim(ctx, ctx->rstack);
			} else {
				return 0;
			}	
		}
		switch (TYPE(ctx->ip)) {
			case ATOM: 
				if ((ctx->stack = cons(ctx, CAR(ctx->ip), AS(ATOM, ctx->stack))) == 0) { 
					return ERR_STACK_OVERFLOW; 
				}
				ctx->ip = NEXT(ctx->ip); 
				break;
			case LIST:
				if ((ctx->stack = cons(ctx, clone(ctx, CAR(ctx->ip)), AS(LIST, ctx->stack))) == 0) {
					return ERR_STACK_OVERFLOW;
				}
				ctx->ip = NEXT(ctx->ip);
				break;
			case PRIM:
				result = ((FUNC)CAR(ctx->ip))(ctx);
				if (result < 0) { return result; }
				if (result != 1) { ctx->ip = NEXT(ctx->ip); }
				break;
			case WORD:
				if (NEXT(ctx->ip) != 0) {
					if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
					ctx->rstack = cons(ctx, NEXT(ctx->ip), AS(ATOM, ctx->rstack));
				}
				ctx->ip = CAR(ctx->ip);
				break;
		}
	} while (1);
}

// IP PRIMITIVES

CELL branch(CTX* ctx) {
	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
	CELL b = CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);
	if (NEXT(NEXT(NEXT(ctx->ip))) != 0) {
		if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
		ctx->rstack = cons(ctx, NEXT(NEXT(NEXT(ctx->ip))), AS(ATOM, ctx->rstack));
	}
	if (b) {
		ctx->ip = CAR(NEXT(ctx->ip));
	} else {
		ctx->ip = CAR(NEXT(NEXT(ctx->ip)));
	}

	return 1;
}

CELL jump(CTX* ctx) {
	ctx->ip = CAR(NEXT(ctx->ip));

	return 1;
}

CELL zjump(CTX* ctx) {
	if (ctx->stack == 0) { return ERR_STACK_OVERFLOW; }
	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
	CELL b = CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);
	if (b) {
		ctx->ip = NEXT(NEXT(ctx->ip));
	} else {
		ctx->ip = CAR(NEXT(ctx->ip));
	}

	return 1;
}

// STACK PRIMITIVES

CELL duplicate(CTX* ctx) {
	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
	switch(TYPE(ctx->stack)) {
		case ATOM:
			if ((ctx->stack = cons(ctx, CAR(ctx->stack), AS(ATOM, ctx->stack))) == 0) {
				return ERR_STACK_OVERFLOW;
			}
			break;
		case LIST:
			if ((ctx->stack = cons(ctx, clone(ctx, CAR(ctx->stack)), AS(LIST, ctx->stack))) == 0) {
				return ERR_STACK_OVERFLOW;
			}
			break;
		case PRIM:
			if ((ctx->stack = cons(ctx, CAR(ctx->stack), AS(PRIM, ctx->stack))) == 0) {
				return ERR_STACK_OVERFLOW;
			}
			break;
		case WORD:
			if ((ctx->stack = cons(ctx, CAR(ctx->stack), AS(WORD, ctx->stack))) == 0) {
				return ERR_STACK_OVERFLOW;
			}
			break;
	}

	return 0;
}

CELL swap(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	CELL t = NEXT(ctx->stack);
	CDR(ctx->stack) = AS(TYPE(ctx->stack), NEXT(NEXT(ctx->stack)));
	CDR(t) = AS(TYPE(t), ctx->stack);
	ctx->stack = t;

	return 0;
}

CELL drop(CTX* ctx) {
	ctx->stack = reclaim(ctx, ctx->stack);
	return 0;
}

CELL over(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
	if (TYPE(NEXT(ctx->stack)) == ATOM) {
		ctx->stack = cons(ctx, CAR(NEXT(ctx->stack)), AS(ATOM, ctx->stack));
	} else if (TYPE(NEXT(ctx->stack)) == LIST) {
		ctx->stack = cons(ctx, clone(ctx, CAR(NEXT(ctx->stack))), AS(LIST, ctx->stack));
	}
	return 0;
}

CELL rot(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0 || NEXT(NEXT(ctx->stack)) == 0) {
		return ERR_STACK_UNDERFLOW;
	}
	CELL t = ctx->stack;
	ctx->stack = NEXT(NEXT(ctx->stack));
	CDR(NEXT(t)) = AS(TYPE(NEXT(t)), NEXT(ctx->stack));
	CDR(ctx->stack) = AS(TYPE(ctx->stack), t);

	return 0;
}

// ARITHMETIC PRIMITIVES

CELL add(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) + CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

CELL sub(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) - CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

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
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) > CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	return 0;
}

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



// PARSING

CELL parse_token(CTX* ctx) {
	while (*(ctx->tib + ctx->in) != 0 && isspace(*(ctx->tib + ctx->in))) {
		ctx->in++;
	}
	ctx->token = ctx->in;
	while (*(ctx->tib + ctx->in) != 0 && !isspace(*(ctx->tib + ctx->in))) {
		ctx->in++;
	}
	return ctx->in - ctx->token;
}

CELL find_token(CTX* ctx) {
	CELL word = ctx->latest;
	while (
	word
	&& !(strlen((BYTE*)NFA(word)) == (ctx->in - ctx->token) 
	     && strncmp((BYTE*)NFA(word), ctx->tib + ctx->token, ctx->in - ctx->token) == 0
	)) {
		word = NEXT(word);
	}
	return word;
}

// OUTER INTERPRETER

//CELL evaluate(CTX* ctx, BYTE* str) {
//	CELL word, result;
//	char *endptr;
//	ctx->tib = str;
//	ctx->token = 0;
//	ctx->in = 0;
//	do {
//		if (parse_token(ctx) == 0) { return 0; }
//		if ((word = find_token(ctx)) != 0) {
//			if (!ctx->state || IMMEDIATE(word)) {
//				if ((result = execute(ctx, XT(word))) != 0) {
//					return result;
//				}
//			} else {
//				if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
//				if (PRIMITIVE(word)) {
//					CAR(ctx->cpile) = cons(ctx, CAR(XT(word)), AS(PRIM, CAR(ctx->cpile)));
//				} else {
//					CAR(ctx->cpile) = cons(ctx, XT(word), AS(WORD, CAR(ctx->cpile)));
//				}
//			}
//		} else {
//			intmax_t number = strtoimax(ctx->tib + ctx->token, &endptr, ctx->base);
//			if (number == 0 && endptr == (char*)(ctx->tib + ctx->token)) {
//				return ERR_UNDEFINED_WORD;
//			} else {
//				if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
//				if (ctx->state) {
//					CAR(ctx->cpile) = cons(ctx, number, AS(ATOM, CAR(ctx->cpile)));
//				} else {
//					ctx->stack = cons(ctx, number, AS(ATOM, ctx->stack));
//				}
//			}
//		}
//	} while (1);
//}
//
//// CONTIGUOUS MEMORY
//
//CELL allot(CTX* ctx) {
//	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
//	CELL bytes = CAR(ctx->stack);
//	ctx->stack = reclaim(ctx, ctx->stack);
// 	BYTE* here = ctx->here;
// 	if (!bytes) { 
//		return 0;
// 	} else if (bytes < 0) { 
// 		ctx->here = (ctx->here + bytes) > BOTTOM(ctx) ? ctx->here + bytes : BOTTOM(ctx);
// 		while (ctx->there - (2*sizeof(CELL)) >= ALIGN(ctx->here, (2*sizeof(CELL)))) { 
// 			CELL t = ctx->there;
// 			ctx->there -= (2*sizeof(CELL));
// 			CAR(ctx->there) = t;
// 			CDR(ctx->there) = 0;
// 			CDR(t) = ctx->there;
// 		}
// 	} else {
// 		CELL p = ctx->there;
// 		while(CAR(p) == (p + (2*sizeof(CELL))) && p < (CELL)(ctx->here + bytes) && p < TOP(ctx)) { 
//			p = CAR(p);	
//		}
// 		if (p >= (CELL)(here + bytes)) {
// 			ctx->there = p;
// 			CDR(ctx->there) = 0;
// 			ctx->here += bytes;
// 		} else {
// 			return ERR_NOT_ENOUGH_MEMORY;
// 		}
// 	}
// 	return 0;
//}
//
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
//CELL rbracket(CTX* ctx) {
//	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
//	if (ctx->cpile == 0) { ctx->cpile = cons(ctx, 0, AS(LIST, ctx->cpile)); }
//	ctx->state = 1;
//
//	return 0;
//}
//
//CELL lbracket(CTX* ctx) {
//	ctx->state = 0;
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
//CELL parse(CTX* ctx) {
//	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
//	if (ctx->free == ctx->there || NEXT(ctx->free) == ctx->there) { return ERR_STACK_OVERFLOW; }
//	CELL c = CAR(ctx->stack);
//	ctx->stack = reclaim(ctx, ctx->stack);
//	ctx->stack = cons(ctx, (CELL)(ctx->tib + ctx->in), AS(ATOM, ctx->stack));
//	CELL start = ctx->in;
//	while (*(ctx->tib + ctx->in) != 0 && *(ctx->tib + ctx->in) != c) {
//		ctx->in++;
//	}
//	if (*(ctx->tib + ctx->in) == c) { 
//		ctx->in++;
//	}
//	ctx->stack = cons(ctx, ctx->in - start, AS(ATOM, ctx->stack));
//
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
//CELL parse_name(CTX* ctx) {
//	if (ctx->free == ctx->there || NEXT(ctx->free) == ctx->there) { return ERR_STACK_OVERFLOW; }
//	parse_token(ctx);
//	ctx->stack = cons(ctx, (CELL)(ctx->tib + ctx->token), AS(ATOM, ctx->stack));
//	ctx->stack = cons(ctx, ctx->in - ctx->token, AS(ATOM, ctx->stack));
//	return 0;
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
//CELL postpone(CTX* ctx) {
//	CELL word;
//	parse_token(ctx);
//	if ((word = find_token(ctx)) == 0) { return ERR_UNDEFINED_WORD; }
//	if (ctx->state) {
//		if (PRIMITIVE(word)) {
//			CAR(ctx->cpile) = cons(ctx, CAR(XT(word)), AS(PRIM, CAR(ctx->cpile)));
//		} else {
//			CAR(ctx->cpile) = cons(ctx, XT(word), AS(LIST, CAR(ctx->cpile)));
//		}
//	} else {
//		if (PRIMITIVE(word)) {
//			ctx->stack = cons(ctx, CAR(XT(word)), AS(PRIM, ctx->stack));
//		} else {
//			ctx->stack = cons(ctx, XT(word), AS(LIST, ctx->stack));
//		}
//	}
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
