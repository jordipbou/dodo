#include<inttypes.h>
#include<string.h>
#include<stdio.h>
#include<ctype.h>

typedef int8_t			BYTE;
typedef intptr_t		CELL;
typedef struct {
	BYTE *tib, *here;
	CELL there, size, free, stack, ip, rstack, compiled, defs, latest, compiling, token, in;
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
#define XT(word)								(CDR(NEXT(CAR(word))))
#define PRIMITIVE(word)					((TYPE(word) & 1) == 0)
#define IMMEDIATE(word)					((TYPE(word) & 2) == 2)

#define WORD(name, block)				CELL name(CTX* ctx) { block; ctx->ip = NEXT(ctx->ip); return 0; }

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_UNDEFINED_WORD			-3
#define ERR_NOT_ENOUGH_MEMORY		-4
#define ERR_ZERO_LENGTH_NAME		-5

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

	ctx->stack = ctx->rstack = ctx->compiled = ctx->defs = ctx->latest = ctx->compiling = 0;
	ctx->token = ctx->in = 0;
	ctx->tib = 0;

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

CELL length(CELL pair) { 
	CELL c = 0; 
	while (pair) { 
		c++; 
		pair = NEXT(pair); 
	} 
	return c; 
}

CELL depth(CELL pair) { 
	CELL c = 0; 
	while (pair) { 
		if (TYPE(pair) == LIST) {
			c += depth(CAR(pair)) + 1;
		} else {
			c++;
		}
		pair = NEXT(pair);
	}
	return c;
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
				if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
				ctx->stack = cons(ctx, CAR(ctx->ip), AS(ATOM, ctx->stack));
				ctx->ip = NEXT(ctx->ip); 
				break;
			case LIST:
				if (FREE(ctx) < depth(CAR(ctx->ip))) { return ERR_STACK_OVERFLOW; }
				ctx->stack = cons(ctx, clone(ctx, CAR(ctx->ip)), AS(LIST, ctx->stack));
				ctx->ip = NEXT(ctx->ip);
				break;
			case PRIM:
				result = ((FUNC)CAR(ctx->ip))(ctx);
				if (result != 0) { return result; }
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

// END OF CORE - Everything above this line is the bare minimum

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
	while (word && strncmp((BYTE*)NFA(word), ctx->tib + ctx->token, ctx->in - ctx->token)) {
		word = NEXT(word);
	}
	return word;
}

CELL evaluate(CTX* ctx, BYTE* str) {
	CELL word, result;
	char *endptr;
	ctx->tib = str;
	ctx->token = 0;
	ctx->in = 0;
	do {
		if (parse_token(ctx) == 0) { return 0; }
		if ((word = find_token(ctx)) != 0) {
			if (!ctx->compiling || IMMEDIATE(word)) {
				if ((result = execute(ctx, XT(word))) != 0) {
					return result;
				}
			} else {
				if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
				if (PRIMITIVE(word)) {
					CAR(ctx->compiled) = cons(ctx, CAR(XT(word)), AS(PRIM, CAR(ctx->compiled)));
				} else {
					CAR(ctx->compiled) = cons(ctx, XT(word), AS(CALL, CAR(ctx->compiled)));
				}
			}
		} else {
			intmax_t number = strtoimax(ctx->tib + ctx->token, &endptr, 10);
			if (number == 0 && endptr == (char*)(ctx->tib + ctx->token)) {
				return ERR_UNDEFINED_WORD;
			} else {
				if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
				if (ctx->compiling) {
					CAR(ctx->compiled) = cons(ctx, number, AS(ATOM, CAR(ctx->compiled)));
				} else {
					ctx->stack = cons(ctx, number, AS(ATOM, ctx->stack));
				}
			}
		}
	} while (1);
}

// END OF EVALUATOR (Outer interpreter)

CELL header(CTX* ctx, BYTE* name) {
	if (ctx->free == 0 || NEXT(ctx->free) == 0 || NEXT(NEXT(ctx->free)) == 0) {
		return ERR_STACK_OVERFLOW; 
	}
	return 
		cons(ctx, 
			cons(ctx, (CELL)name, AS(ATOM, 
			cons(ctx, (CELL)ctx->here, AS(ATOM, 0)))), 
		AS(LIST, 0));	
}

CELL primitive(CELL word) {
	CDR(word) = CDR(word) & 2;
	return word;
}

CELL immediate(CELL word) {
	CDR(word) = CDR(word) | 2;
	return word;
}

CELL body(CELL word, CELL xt) {
	CDR(NEXT(CAR(word))) = xt;
	return word;
}

CELL reveal(CTX* ctx, CELL word) {
	CDR(word) = AS(TYPE(word), ctx->latest);
	ctx->latest = word;
	return word;
}

// END OF WORD DEFINITION

CELL find_prim(CTX* ctx, CELL addr) {
	CELL word = ctx->latest;
	while (word && CAR(XT(word)) != addr) {
		word = NEXT(word);
	}
	return word;
}

CELL allot(CTX* ctx, CELL bytes) {
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

CELL align(CTX* ctx) {
	return allot(ctx, ALIGN(ctx->here, sizeof(CELL)) - (CELL)ctx->here);
}

// END OF CONTIGUOUS MEMORY

WORD(_lbrace,
	ctx->compiling = 1;
	ctx->compiled = cons(ctx, 0, AS(LIST, ctx->compiled));
)

WORD(_rbrace,
	CELL list = reverse(CAR(ctx->compiled), 0);
	CAR(ctx->compiled) = 0;
	ctx->compiled = reclaim(ctx, ctx->compiled);
	if (ctx->compiled == 0) {
		ctx->stack = cons(ctx, list, AS(LIST, ctx->stack));
		ctx->compiling = 0;
	} else {
		CAR(ctx->compiled) = cons(ctx, list, AS(LIST, CAR(ctx->compiled)));
	}
)

//WORD(_colon,
CELL _colon(CTX* ctx) {
	if (parse_token(ctx) == 0) { return ERR_ZERO_LENGTH_NAME; }
	BYTE* str = ctx->here;
	if (allot(ctx, sizeof(CELL) + (ctx->in - ctx->token) + 1) != 0) { return ERR_NOT_ENOUGH_MEMORY; }
	*((CELL*)str) = ctx->in - ctx->token;
	str += sizeof(CELL);
	strncpy(str, ctx->tib + ctx->token, ctx->in - ctx->token);
	str[ctx->in - ctx->token] = 0;
	CELL h = header(ctx, str);
	ctx->defs = cons(ctx, h, AS(ATOM, ctx->defs));
	_lbrace(ctx);

	//ctx->ip = NEXT(ctx->ip);
	return 0;
}
//)

//WORD(_semicolon,
CELL _semicolon(CTX* ctx) {
	_rbrace(ctx);
	CELL h = CAR(ctx->defs);
	ctx->defs = reclaim(ctx, ctx->defs);
	body(h, CAR(ctx->stack));
	CAR(ctx->stack) = 0;
	ctx->stack = reclaim(ctx, ctx->stack);
	reveal(ctx, h);

	return 0;
}
//)

WORD(_add,
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) + CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);
)

WORD(_dup, 
	switch(TYPE(ctx->stack)) {
		case ATOM:
			ctx->stack = cons(ctx, CAR(ctx->stack), AS(ATOM, ctx->stack));
			break;
		case LIST:
			ctx->stack = cons(ctx, clone(ctx, CAR(ctx->stack)), AS(LIST, ctx->stack));
			break;
		case PRIM:
			ctx->stack = cons(ctx, CAR(ctx->stack), AS(PRIM, ctx->stack));
			break;
		case CALL:
			ctx->stack = cons(ctx, CAR(ctx->stack), AS(CALL, ctx->stack));
			break;
	}
)

void dump_list(CTX* ctx, CELL pair, CELL order) {
	if (pair) {
		if (order) dump_list(ctx, NEXT(pair), order);
		switch (TYPE(pair)) {
			case ATOM: printf("%ld ", CAR(pair)); break;
			case LIST: printf("{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
			case PRIM: printf("%s ", (BYTE*)NFA(find_prim(ctx, CAR(pair)))); break;
			case CALL: printf("X::{ "); dump_list(ctx, CAR(pair), 0); printf("} "); break;
		}
		if (!order) dump_list(ctx, NEXT(pair), order);
	}
}

WORD(_dump_stack,
	printf("<%ld> ", length(ctx->stack));
	dump_list(ctx, ctx->stack, 1);
	printf("\n");
)

CELL _dump_compiled(CTX* ctx) {
	printf("COMPILED: <%ld> ", length(ctx->compiled));
	dump_list(ctx, ctx->compiled, 1);
	printf("\n");
}

CELL _dump_defs(CTX* ctx) {
	printf("DEFS: <%ld> ", length(ctx->defs));
	dump_list(ctx, ctx->defs, 1);
	printf("\n");
}

CELL _dump_body(CTX* ctx) {
	parse_token(ctx);
	CELL w = find_token(ctx);
	printf("BODY for %s ", (BYTE*)NFA(w));
	dump_list(ctx, XT(w), 1);
	return 0;
}

CELL _execute(CTX* ctx) {
	CELL xlist = CAR(ctx->stack);
	CAR(ctx->stack) = 0;
	ctx->stack = reclaim(ctx, ctx->stack);
	if (NEXT(ctx->ip) != 0) {
		if (ctx->free == ctx->there) { return ERR_STACK_OVERFLOW; }
		ctx->rstack = cons(ctx, NEXT(ctx->ip), AS(ATOM, ctx->rstack));
	}
	ctx->ip = xlist;
	return 0;
}

//// Source code for getch is taken from:
//// Crossline readline (https://github.com/jcwangxp/Crossline).
//// It's a fantastic readline cross-platform replacement, but only getch was
//// needed and there's no need to include everything else.
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
////void _key(X* x) { push(x, dodo_getch()); }
////void _emit(X* x) { C K = T(x); pop(x); K == 127 ? printf ("\b \b") : printf ("%c", (char)K); }
//
