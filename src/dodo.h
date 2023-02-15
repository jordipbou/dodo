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
typedef struct {
	BYTE *tib, *here;
	CELL there, size; 
	CELL free, stack, rstack, cpile, hstack;
	CELL ip, latest, state, token, in;
	CELL base;
} CTX;
typedef CELL (*FUNC)(CTX*);

#define CAR(pair)								(*((CELL*)pair))
#define CDR(pair)								(*(((CELL*)pair) + 1))
#define NEXT(pair)							(CDR(pair) & -4)
#define TYPE(pair)							(CDR(pair) & 3)

#define AS(type, cell)					((cell & -4) | type)

#define ATOM										0
#define LIST										1
#define PRIM										2
#define CALL										3

#define NFA(word)								(CAR(CAR(word)))
#define DFA(word)								(CAR(NEXT(CAR(word))))
#define XT(word)								(NEXT(NEXT(CAR(word))))
#define PRIMITIVE(word)					((TYPE(word) & 1) == 0)
#define IMMEDIATE(word)					((TYPE(word) & 2) == 2)

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_UNDEFINED_WORD			-3
#define ERR_NOT_ENOUGH_MEMORY		-4
#define ERR_ZERO_LENGTH_NAME		-5
#define ERR_ATOM_EXPECTED				-6
#define ERR_RSTACK_UNDERFLOW		-7

#define FREE(ctx)						(length(ctx->free) - 1)
#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))
#define RESERVED(ctx)				((ctx->there) - ((CELL)ctx->here))

#define BOTTOM(ctx)			(((BYTE*)ctx) + sizeof(CTX))
#define TOP(ctx)				(ALIGN(((BYTE*)ctx) + ctx->size - (2*sizeof(CELL)) - 1, (2*sizeof(CELL))))

CTX* init(BYTE* block, CELL size) {
	if (size < sizeof(CELL) + 2*(2*sizeof(CELL))) return 0;
	CTX* ctx = (CTX*)block;	
	ctx->size = size;
	ctx->here = BOTTOM(ctx);
	ctx->there = ALIGN(BOTTOM(ctx), 2*sizeof(CELL));
	ctx->free = TOP(ctx);

	for (CELL pair = ctx->there; pair <= ctx->free; pair += 2*sizeof(CELL)) {
		CAR(pair) = pair == ctx->free ? 0 : pair + 2*sizeof(CELL);
		CDR(pair) = pair == ctx->there ? 0 : pair - 2*sizeof(CELL);
	}

	ctx->stack = ctx->rstack = ctx->cpile = ctx->hstack = ctx->latest = ctx->state = 0;
	ctx->token = ctx->in = 0;
	ctx->tib = 0;
	ctx->base = 10;

	return ctx;
}

// MANAGED MEMORY

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
			case CALL:
				if (NEXT(ctx->ip) != 0) {
					if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
					ctx->rstack = cons(ctx, NEXT(ctx->ip), AS(ATOM, ctx->rstack));
				}
				ctx->ip = CAR(ctx->ip);
				break;
		}
	} while (1);
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
	printf("token %.*s\n", ctx->in - ctx->token, ctx->tib + ctx->token);
	printf("word %ld\n", word);
	while (
	word
	&& !(strlen(NFA(word)) == (ctx->in - ctx->token) 
	     && strncmp((BYTE*)NFA(word), ctx->tib + ctx->token, ctx->in - ctx->token) == 0i
	)) {
		word = NEXT(word);
		if (word) printf("NFA(word) %s\n", NFA(word));
	}
	if (word) {
		printf("WORD found: %s strlen(NFA(word)) %ld (ctx->in - ctx->token) %ld\n", NFA(word), strlen(NFA(word)), ctx->in - ctx->token);
	}
	return word;
}

// OUTER INTERPRETER

CELL evaluate(CTX* ctx, BYTE* str) {
	CELL word, result;
	char *endptr;
	ctx->tib = str;
	ctx->token = 0;
	ctx->in = 0;
	do {
		if (parse_token(ctx) == 0) { return 0; }
		printf("TOKEN: %.*s\n", ctx->in - ctx->token, ctx->tib + ctx->token);
		if ((word = find_token(ctx)) != 0) {
			if (!ctx->state || IMMEDIATE(word)) {
				if ((result = execute(ctx, XT(word))) != 0) {
					return result;
				}
			} else {
				if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
				if (PRIMITIVE(word)) {
					CAR(ctx->cpile) = cons(ctx, CAR(XT(word)), AS(PRIM, CAR(ctx->cpile)));
				} else {
					CAR(ctx->cpile) = cons(ctx, XT(word), AS(CALL, CAR(ctx->cpile)));
				}
			}
		} else {
			intmax_t number = strtoimax(ctx->tib + ctx->token, &endptr, ctx->base);
			if (number == 0 && endptr == (char*)(ctx->tib + ctx->token)) {
				return ERR_UNDEFINED_WORD;
			} else {
				if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
				if (ctx->state) {
					CAR(ctx->cpile) = cons(ctx, number, AS(ATOM, CAR(ctx->cpile)));
				} else {
					ctx->stack = cons(ctx, number, AS(ATOM, ctx->stack));
				}
			}
		}
	} while (1);
}

// CONTIGUOUS MEMORY

CELL allot(CTX* ctx) {
	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
	CELL bytes = CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);
 	BYTE* here = ctx->here;
 	if (!bytes) { 
		return 0;
 	} else if (bytes < 0) { 
 		ctx->here = (ctx->here + bytes) > BOTTOM(ctx) ? ctx->here + bytes : BOTTOM(ctx);
 		while (ctx->there - (2*sizeof(CELL)) >= ALIGN(ctx->here, (2*sizeof(CELL)))) { 
 			CELL t = ctx->there;
 			ctx->there -= (2*sizeof(CELL));
 			CAR(ctx->there) = t;
 			CDR(ctx->there) = 0;
 			CDR(t) = ctx->there;
 		}
 	} else {
 		CELL p = ctx->there;
 		while(CAR(p) == (p + (2*sizeof(CELL))) && p < (CELL)(ctx->here + bytes) && p < TOP(ctx)) { 
			p = CAR(p);	
		}
 		if (p >= (CELL)(here + bytes)) {
 			ctx->there = p;
 			CDR(ctx->there) = 0;
 			ctx->here += bytes;
 		} else {
 			return ERR_NOT_ENOUGH_MEMORY;
 		}
 	}
 	return 0;
}

CELL compile_str(CTX* ctx) {
	CELL str_start;
	while (*(ctx->tib + ctx->in) != 0 && isspace(*(ctx->tib + ctx->in))) {
		ctx->in++;
	}
	str_start = ctx->in;
	while (*(ctx->tib + ctx->in) != 0 && *(ctx->tib + ctx->in) != '"') {
		ctx->in++;
	}
	BYTE* str = ctx->here;
	ctx->stack = cons(ctx, sizeof(CELL) + (ctx->in - str_start) + 1, AS(ATOM, ctx->stack));
	if (allot(ctx) != 0) { return ERR_NOT_ENOUGH_MEMORY; }
	*((CELL*)str) = ctx->in - str_start;
	str += sizeof(CELL);
	for (CELL i = 0; i < ctx->in - str_start; i++) {
		str[i] = ctx->tib[str_start + i];
	}
	str[ctx->in - str_start] = 0;
	if (ctx->state) {
		CAR(ctx->cpile) = cons(ctx, (CELL)str, AS(ATOM, CAR(ctx->cpile)));
		//CAR(ctx->cpile) = cons(ctx, ctx->in - str_start, AS(ATOM, CAR(ctx->cpile)));
	} else {
		ctx->stack = cons(ctx, (CELL)str, AS(ATOM, ctx->stack));
		//ctx->stack = cons(ctx, ctx->in - str_start, AS(ATOM, ctx->stack));
	}
	ctx->in++;
	return 0;
}

// LIST OPERATIONS

CELL append(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	if (TYPE(NEXT(ctx->stack)) == LIST) {
		CELL t = NEXT(ctx->stack);
		CDR(ctx->stack) = AS(TYPE(ctx->stack), CAR(NEXT(ctx->stack)));
		CAR(t) = ctx->stack;
		ctx->stack = t;
	} else if (TYPE(NEXT(ctx->stack)) == ATOM) {
		CELL* dest = (CELL*)CAR(NEXT(ctx->stack));	
		CELL t = NEXT(ctx->stack);
		CDR(ctx->stack) = AS(TYPE(ctx->stack), *dest);
		*dest = ctx->stack;
		ctx->stack = reclaim(ctx, t);
	}

	return 0;
}

CELL lbrace(CTX* ctx) {
 ctx->state = 1;
 ctx->cpile = cons(ctx, 0, AS(LIST, ctx->cpile));

 return 0;
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

CELL rbrace(CTX* ctx) {
  CELL list = reverse(CAR(ctx->cpile), 0);
  CAR(ctx->cpile) = 0;
  ctx->cpile = reclaim(ctx, ctx->cpile);
  if (ctx->cpile == 0) {
		ctx->stack = cons(ctx, list, AS(LIST, ctx->stack));
		ctx->state = 0;
  } else {
		CAR(ctx->cpile) = cons(ctx, list, AS(LIST, CAR(ctx->cpile)));
  }

  return 0;
}

CELL length(CELL pair) { 
	CELL c = 0; 
	while (pair) { 
		c++; 
		pair = NEXT(pair); 
	} 
	return c; 
}

CELL find_prim(CTX* ctx, CELL xt) {
	CELL word = ctx->latest;
	while (word && XT(word) != xt) { word = NEXT(word); }
	return word;
}

void dump_list(CTX* ctx, CELL pair, CELL order) {
	CELL word;
	if (pair) {
		if (order) dump_list(ctx, NEXT(pair), order);
		switch (TYPE(pair)) {
			case ATOM: printf("%ld ", CAR(pair)); break;
			case LIST: printf("{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
			case PRIM: 
				word = find_prim(ctx, CAR(pair));
				printf("%s ", (BYTE*)(word ? (NFA(word)) : "")); break;
			case CALL: printf("X::{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
		}
		if (!order) dump_list(ctx, NEXT(pair), order);
	}
}

CELL dump_stack(CTX* ctx) {
	printf("<%ld> ", length(ctx->stack));
	dump_list(ctx, ctx->stack, 1);
	printf("\n");

	return 0;
}

CELL words(CTX* ctx) {
	printf("WORDS: ");
	CELL p = ctx->latest;
	while (p) { printf("%s ", (BYTE*)NFA(p)); p = NEXT(p); }
	printf("\n");
}

CTX* bootstrap(CTX* ctx) {
	ctx->latest = 
		cons(ctx, 
			cons(ctx, (CELL)"s\"", AS(ATOM, 
			cons(ctx, (CELL)ctx->here, AS(ATOM, 
			cons(ctx, (CELL)&compile_str, AS(PRIM, 0)))))), 
		AS(CALL, ctx->latest));	

	ctx->latest = 
		cons(ctx,
			cons(ctx, (CELL)"allot", AS(ATOM,
			cons(ctx, (CELL)ctx->here, AS(ATOM,
			cons(ctx, (CELL)&allot, AS(PRIM, 0)))))),
		AS(LIST, ctx->latest));

	ctx->latest =
		cons(ctx,
			cons(ctx, (CELL)"append", AS(ATOM,
			cons(ctx, (CELL)ctx->here, AS(ATOM,
			cons(ctx, (CELL)&append, AS(PRIM, 0)))))),
		AS(LIST, ctx->latest));

	ctx->latest =
		cons(ctx,
			cons(ctx, (CELL)".s", AS(ATOM,
			cons(ctx, (CELL)ctx->here, AS(ATOM,
			cons(ctx, (CELL)&dump_stack, AS(PRIM, 0)))))),
		AS(LIST, ctx->latest));

	ctx->latest =
		cons(ctx,
			cons(ctx, (CELL)"{", AS(ATOM,
			cons(ctx, (CELL)ctx->here, AS(ATOM,
			cons(ctx, (CELL)&lbrace, AS(PRIM, 0)))))),
		AS(CALL, ctx->latest));

	ctx->latest =
		cons(ctx,
			cons(ctx, (CELL)"}", AS(ATOM,
			cons(ctx, (CELL)ctx->here, AS(ATOM,
			cons(ctx, (CELL)&rbrace, AS(PRIM, 0)))))),
		AS(CALL, ctx->latest));

	ctx->latest =
		cons(ctx,
			cons(ctx, (CELL)"words", AS(ATOM,
			cons(ctx, (CELL)ctx->here, AS(ATOM,
			cons(ctx, (CELL)&words, AS(PRIM, 0)))))),
		AS(LIST, ctx->latest));

	// VARIABLES

	ctx->latest =
		cons(ctx,
			cons(ctx, (CELL)"latest", AS(ATOM,
			cons(ctx, (CELL)ctx->here, AS(ATOM,
			cons(ctx, (CELL)&ctx->latest, AS(ATOM, 0)))))),
		AS(LIST, ctx->latest));
	return ctx;
}

//// PRIMITIVES
//
//CELL P_lbrace(CTX* ctx) {
// ctx->state = 1;
// ctx->cpile = cons(ctx, 0, AS(LIST, ctx->cpile));
//
// return 0;
//}
//
//CELL D_rbrace(CTX* ctx) {
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
//CELL D_colon(CTX* ctx) {
//	if (parse_token(ctx) == 0) { return ERR_ZERO_LENGTH_NAME; }
// 	BYTE* str = ctx->here;
// 	if (allot(ctx, sizeof(CELL) + (ctx->in - ctx->token) + 1) != 0) { return ERR_NOT_ENOUGH_MEMORY; }
// 	*((CELL*)str) = ctx->in - ctx->token;
// 	str += sizeof(CELL);
// 	strncpy(str, ctx->tib + ctx->token, ctx->in - ctx->token);
// 	str[ctx->in - ctx->token] = 0;
// 	CELL h = header(ctx, str);
// 	ctx->defs = cons(ctx, h, AS(ATOM, ctx->defs));
// 	D_lbrace(ctx);
//
// 	return 0;
//}
//
//CELL D_semicolon(CTX* ctx) {
//	D_rbrace(ctx);
// 	CELL h = CAR(ctx->defs);
// 	ctx->defs = reclaim(ctx, ctx->defs);
// 	body(ctx, h, CAR(ctx->stack));
// 	CAR(ctx->stack) = 0;
// 	ctx->stack = reclaim(ctx, ctx->stack);
// 	reveal(ctx, h);
//
// 	return 0;
//}

//CELL depth(CELL pair) { 
//	CELL c = 0; 
//	while (pair) { 
//		if (TYPE(pair) == LIST) {
//			c += depth(CAR(pair)) + 1;
//		} else {
//			c++;
//		}
//		pair = NEXT(pair);
//	}
//	return c;
//}
//
//CELL allot(CTX* ctx, CELL bytes) {
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
//CELL align(CTX* ctx) {
//	return allot(ctx, ALIGN(ctx->here, sizeof(CELL)) - (CELL)ctx->here);
//}
//
//CELL header(CTX* ctx, BYTE* name) {
//	return 
//		cons(ctx, 
//			cons(ctx, (CELL)name, AS(ATOM, 
//			cons(ctx, (CELL)ctx->here, AS(ATOM, 
//			cons(ctx, (CELL)ctx->here, AS(ATOM, 0)))))), // Default XT, push DFA
//		AS(LIST, 0));	
//}
//
//CELL primitive(CELL word) {
//	CDR(word) = CDR(word) & 2;
//	return word;
//}
//
//CELL immediate(CELL word) {
//	CDR(word) = CDR(word) | 2;
//	return word;
//}
//
//CELL body(CTX* ctx, CELL word, CELL xt) {
//	while (XT(word)) { XT(word) = reclaim(ctx, XT(word)); }
//	XT(word) = xt;
//	return word;
//}
//
//CELL reveal(CTX* ctx, CELL word) {
//	CDR(word) = AS(TYPE(word), ctx->latest);
//	ctx->latest = word;
//	return word;
//}
//
//CELL D_add(CTX* ctx) {
//	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
//	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
//	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) + CAR(ctx->stack);
//	ctx->stack = reclaim(ctx, ctx->stack);
//
//	return 0;
//}
//
//
////// END OF EVALUATOR (Outer interpreter)
////
////// END OF WORD DEFINITION
////
////// END OF CONTIGUOUS MEMORY
////
////CELL _dup(CTX* ctx) {
////	switch(TYPE(ctx->stack)) {
////		case ATOM:
////			ctx->stack = cons(ctx, CAR(ctx->stack), AS(ATOM, ctx->stack));
////			break;
////		case LIST:
////			ctx->stack = cons(ctx, clone(ctx, CAR(ctx->stack)), AS(LIST, ctx->stack));
////			break;
////		case PRIM:
////			ctx->stack = cons(ctx, CAR(ctx->stack), AS(PRIM, ctx->stack));
////			break;
////		case CALL:
////			ctx->stack = cons(ctx, CAR(ctx->stack), AS(CALL, ctx->stack));
////			break;
////	}
////
////	return 0;
////}
////
////CELL _dump_cpile(CTX* ctx) {
////	printf("COMPILED: <%ld> ", length(ctx->cpile));
////	dump_list(ctx, ctx->cpile, 1);
////	printf("\n");
////}
////
////CELL _dump_hstack(CTX* ctx) {
////	printf("DEFS: <%ld> ", length(ctx->hstack));
////	dump_list(ctx, ctx->hstack, 1);
////	printf("\n");
////}
////
////CELL _dump_body(CTX* ctx) {
////	parse_token(ctx);
////	CELL w = find_token(ctx);
////	printf("BODY for %s ", (BYTE*)NFA(w));
////	dump_list(ctx, XT(w), 1);
////	return 0;
////}
////
////CELL _execute(CTX* ctx) {
////	CELL xlist = CAR(ctx->stack);
////	CAR(ctx->stack) = 0;
////	ctx->stack = reclaim(ctx, ctx->stack);
////	if (NEXT(ctx->ip) != 0) {
////		if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////		ctx->rstack = cons(ctx, NEXT(ctx->ip), AS(ATOM, ctx->rstack));
////	}
////	ctx->ip = xlist;
////
////	return 1;
////}
////
////// Basic WORDS needed for executing SECTOR FORTH
////
////CELL D_parse(CTX* ctx) {
////	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
////	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
////	CELL delimiter = CAR(ctx->stack);
////	ctx->stack = reclaim(ctx, ctx->stack);
////	printf("PARSE::UNTIL %ld FROM %ld ", delimiter, ctx->in);
////	while (*(ctx->tib + ctx->in) != 0 && *(ctx->tib + ctx->in) != delimiter) {
////		ctx->in++;
////	}
////	ctx->in++;
////	printf("LAST %d\n", ctx->in);
////}
////
////CELL D_immediate(CTX* ctx) {
////	CDR(ctx->latest) = CDR(ctx->latest) | 2;
////	return 0;	
////}
////
////// SECTOR FORTH Primitives
////
////CELL D_fetch(CTX* ctx) {
////	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
////	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
////	CAR(ctx->stack) = *((CELL*)(CAR(ctx->stack)));
////
////	return 0;
////}
////
////CELL D_store(CTX* ctx) {
////	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
////	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
////	CELL addr = CAR(ctx->stack);
////	CELL x = CAR(NEXT(ctx->stack));
////	ctx->stack = reclaim(ctx, reclaim(ctx, ctx->stack));
////	*((CELL*)addr) = x;
////
////	return 0;
////}
////
////CELL D_sp_fetch(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, ctx->stack, AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////CELL D_rp_fetch(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, ctx->rstack, AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////CELL D_zeroeq(CTX* ctx) {
////	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
////	if (TYPE(ctx->stack) != ATOM) { return ERR_ATOM_EXPECTED; }
////	CAR(ctx->stack) = CAR(ctx->stack) == 0 ? -1 : 0;
////
////	return 0;
////}
////
////CELL D_nand(CTX* ctx) {
////	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
////	if (TYPE(ctx->stack) != ATOM || TYPE(NEXT(ctx->stack)) != ATOM) { return ERR_ATOM_EXPECTED; }
////	CAR(NEXT(ctx->stack)) = !(CAR(NEXT(ctx->stack)) && CAR(ctx->stack));
////	ctx->stack = reclaim(ctx, ctx->stack);
////
////	return 0;
////}
////
////CELL D_exit(CTX* ctx) {
////	if (ctx->rstack == 0) { return ERR_RSTACK_UNDERFLOW; }
////	CELL addr = CAR(ctx->rstack);
////	ctx->rstack = reclaim(ctx, ctx->rstack);
////
////	ctx->ip = addr;
////	return 1;
////}
////
////// Source code for getch is taken from:
////// Crossline readline (https://github.com/jcwangxp/Crossline).
////// It's a fantastic readline cross-platform replacement, but only getch was
////// needed and there's no need to include everything else.
////#ifdef _WIN32	// Windows
////int dodo_getch (void) {	fflush (stdout); return _getch(); }
////#else
////int dodo_getch ()
////{
////	char ch = 0;
////	struct termios old_term, cur_term;
////	fflush (stdout);
////	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
////	cur_term = old_term;
////	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
////	cur_term.c_cc[VMIN] = 1;
////	cur_term.c_cc[VTIME] = 0;
////	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
////	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
////	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
////	return ch;
////}
////#endif
////
////CELL D_key(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, dodo_getch(), AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////CELL D_emit(CTX* ctx) { 
////	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
////	CELL k = CAR(ctx->stack);
////	ctx->stack = reclaim(ctx, ctx->stack);
////	printf("%c", (char)k);
////
////	return 0;
////}
////
////// SECTORFORTH Variables
////
////CELL D_state(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, (CELL)&ctx->state, AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////CELL D_tib(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, (CELL)&ctx->tib, AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////CELL D_in(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, (CELL)&ctx->in, AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////CELL D_here(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, (CELL)&ctx->here, AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////CELL D_latest(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, (CELL)&ctx->latest, AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////// FORTH TESTER primitives
////
////CELL D_base(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, (CELL)&ctx->base, AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////CELL D_cell(CTX* ctx) {
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	ctx->stack = cons(ctx, sizeof(CELL), AS(ATOM, ctx->stack));
////
////	return 0;
////}
////
////CELL D_over(CTX* ctx) {
////	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
////	if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
////	if (TYPE(NEXT(ctx->stack)) == LIST) {
////		ctx->stack = cons(ctx, clone(ctx, CAR(NEXT(ctx->stack))), AS(LIST, ctx->stack));
////	} else {
////		ctx->stack = cons(ctx, CAR(NEXT(ctx->stack)), AS(TYPE(NEXT(ctx->stack)), ctx->stack));
////	}
////}
////
////CELL D_parse_name(CTX* ctx) {
////	if (ctx->free == ctx->there || NEXT(ctx->free) == ctx->there) { return ERR_STACK_OVERFLOW; }
////	parse_token(ctx);
////	ctx->stack = cons(ctx, (CELL)(ctx->tib + ctx->token), AS(ATOM, ctx->stack));
////	ctx->stack = cons(ctx, ctx->in - ctx->token, AS(ATOM, ctx->stack));
////	return 0;
////}
////
////CELL D_s_literal(CTX* ctx) {
////	/* TODO: Compile only words ?! */
////	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
////	CELL len = CAR(ctx->stack);
////	ctx->stack = reclaim(ctx, ctx->stack);
////	BYTE* addr = (BYTE*)CAR(ctx->stack);
////	ctx->stack = reclaim(ctx, ctx->stack);
////	BYTE* str = ctx->here;
////	if (allot(ctx, sizeof(CELL) + len + 1) != 0) { return ERR_NOT_ENOUGH_MEMORY; }
////	*((CELL*)str) = len;
////	str += sizeof(CELL);
////	strncpy(str, addr, len);
////	str[len] = 0;
////
////	ctx->cpile = cons(ctx, (CELL)str, AS(ATOM, ctx->cpile));
////	ctx->cpile = cons(ctx, len, AS(ATOM, ctx->cpile));
////
////	return 0;
////}
////
////CELL D_header(CTX* ctx) {
////	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
////	CELL len = CAR(ctx->stack);
////	ctx->stack = reclaim(ctx, ctx->stack);
////	BYTE* addr = (BYTE*)CAR(ctx->stack);
////	ctx->stack = reclaim(ctx, ctx->stack);
////
////	header(ctx, addr);
////
////	return 0;
////}
////
////CELL D_reveal(CTX* ctx) {
////	CELL word = ctx->hstack;
////	ctx->hstack = NEXT(ctx->hstack);
////	reveal(ctx, word);
////
////	return 0;
////}
////
////CELL D_comma(CTX* ctx) {
////	if (ctx->stack == 0) { return ERR_STACK_UNDERFLOW; }
////	BYTE* here = ctx->here;
////	if (allot(ctx, sizeof(CELL)) == 0) { return ERR_NOT_ENOUGH_MEMORY; }
////	CELL x = CAR(ctx->stack);
////	ctx->stack = reclaim(ctx, ctx->stack);
////	*((CELL*)here) = x;
////	
////	return 0;
////}
