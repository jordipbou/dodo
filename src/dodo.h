#include<inttypes.h>
#include<string.h>
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

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_UNDEFINED_WORD			-3
#define ERR_NOT_ENOUGH_MEMORY		-4

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
				if (FREE(ctx) < (depth(CAR(ctx->ip)) + 1)) { return ERR_STACK_OVERFLOW; }
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

#define NFA(word)								(CAR(CAR(word)))
#define DFA(word)								(NFA(word) + *((CELL*)(NFA(word) - sizeof(CELL))) + 1)
#define XT(word)								(NEXT(CAR(word)))
#define IMMEDIATE(word)					(TYPE(word) == CALL)

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
				CAR(ctx->compiled) = cons(ctx, XT(word), AS(CALL, CAR(ctx->compiled)));
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

CELL header(CTX* ctx, BYTE* name, CELL length) {
	BYTE* str = ctx->here;
	if (ctx->free == 0 || NEXT(ctx->free) == 0) {	return ERR_STACK_OVERFLOW; }
	if ((allot(ctx, length + sizeof(CELL) + 1)) != 0) { return ERR_NOT_ENOUGH_MEMORY; }
	*((CELL*)str) = length;
	str += sizeof(CELL);
	strcpy(str, name);
	return cons(ctx, cons(ctx, (CELL)str, AS(ATOM, 0)), AS(LIST, 0));
}

CELL body(CTX* ctx, CELL word, CELL list) {
	CDR(CAR(word)) = AS(ATOM, list);	
	return word;
}

CELL reveal(CTX* ctx, CELL word) {
	CDR(word) = AS(LIST, ctx->latest);
	ctx->latest = word;
	return word;
}

CELL immediate(CELL word) {
	CDR(word) = AS(CALL, NEXT(word));
	return word;
}

// END OF WORD DEFINITION

CELL _add(CTX* ctx) {
	if (ctx->stack == 0 || NEXT(ctx->stack) == 0) { return ERR_STACK_UNDERFLOW; }
	CAR(NEXT(ctx->stack)) = CAR(NEXT(ctx->stack)) + CAR(ctx->stack);
	ctx->stack = reclaim(ctx, ctx->stack);

	ctx->ip = NEXT(ctx->ip); 
	return 0;
}

//
//
//void dump_list(CELL pair) {
//	if (pair) {
//		dump_list(NEXT(pair));
//		switch (TYPE(pair)) {
//			case ATOM: printf("%ld ", CAR(pair)); break;
//			case LIST: printf("{ "); dump_list(CAR(pair)); printf("} "); break;
//			case PRIM: printf("%p ", CAR(pair)); break;
//			case CALL: printf("%p ", CAR(pair)); break;
//		}
//	}
//}
//
//CELL _dump_stack(CTX* ctx) {
//	printf("<%ld> ", length(ctx->stack));
//	dump_list(ctx->stack);
//	printf("\n");
//}
//
//CELL _lbrace(CTX* ctx) {
//	ctx->compiling = 1;
//	ctx->compiled = cons(ctx, 0, AS(LIST, ctx->compiled));
//
//	ctx->ip = NEXT(ctx->ip);
//	return 0;
//}
//
//CELL _rbrace(CTX* ctx) {
//	CELL list = CAR(ctx->compiled);
//	CAR(ctx->compiled) = 0;
//	ctx->compiled = reclaim(ctx, ctx->compiled);
//	if (ctx->compiled == 0) {
//		ctx->stack = cons(ctx, list, AS(LIST, ctx->stack));
//		ctx->compiling = 0;
//	} else {
//		CAR(ctx->compiled) = cons(ctx, list, AS(LIST, CAR(ctx->compiled)));
//	}
//
//	ctx->ip = NEXT(ctx->ip);
//	return 0;
//}

// Source code for getch is taken from:
// Crossline readline (https://github.com/jcwangxp/Crossline).
// It's a fantastic readline cross-platform replacement, but only getch was
// needed and there's no need to include everything else.
//#ifdef _WIN32	// Windows
//int dodo_getch (void) {	fflush (stdout); return _getch(); }
//#else
//int dodo_getch ()
//{
//	char ch = 0;
//	struct termios old_term, cur_term;
//	fflush (stdout);
//	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
//	cur_term = old_term;
//	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
//	cur_term.c_cc[VMIN] = 1;
//	cur_term.c_cc[VTIME] = 0;
//	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
//	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
//	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
//	return ch;
//}
//#endif
//
//void _key(X* x) { push(x, dodo_getch()); }
//void _emit(X* x) { C K = T(x); pop(x); K == 127 ? printf ("\b \b") : printf ("%c", (char)K); }

