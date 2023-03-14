#ifndef __CORE__
#define __CORE__

//#include<inttypes.h>
//#include<string.h>
//#include<stdio.h>
//#include<ctype.h>
//
//#ifdef _WIN32
//	#include<conio.h>
//#else
//	#include<unistd.h>
//	#include<termios.h>
//#endif
//
//typedef int8_t			BYTE;
//typedef intptr_t		CELL;
//
//#define CAR(pair)								(*((CELL*)pair))
//#define CDR(pair)								(*(((CELL*)pair) + 1))
//#define NEXT(pair)							(CDR(pair) & -4)
//#define TYPE(pair)							(CDR(pair) & 3)
//
//enum Types { ATOM, LIST, PRIM, WORD };
//
//#define AS(type, ref)						((ref & -4) | type)
//#define LINK(pair, next)				(CDR(pair) = AS(TYPE(pair), next))
//
//enum Words { CMP_PRIMITIVE, CMP_COLON_DEF, IMM_PRIMITIVE, IMM_COLON_DEF };
//
//enum Prims { ZJUMP, JUMP, ORIG, DEST, ORIG_DEST, RECURSE };
//
//#define NFA(word)								((BYTE*)CAR(CAR(word)))
//#define XT(word)								(NEXT(CAR(word)))
//#define PRIMITIVE(word)					((TYPE(word) & 1) == 0)
//#define IMMEDIATE(word)					((TYPE(word) & 2) == 2)
//
//typedef struct {
//	BYTE *tib, *here;
//	CELL there, size; 
//	CELL free, pile, cpile, xstack, latest;
//	CELL state, token, in, base;
//} CTX;
//
//#define S(ctx)									CAR(ctx->pile)
//#define C(ctx)									CAR(ctx->cpile)
//
//#define FREE(ctx)						(length(ctx->free) - 1)		// Don't count ctx->there
//#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))
//
//#define BOTTOM(ctx)			(((BYTE*)ctx) + sizeof(CTX))
//#define TOP(ctx)				(ALIGN(((BYTE*)ctx) + ctx->size - (2*sizeof(CELL)) - 1, (2*sizeof(CELL))))
//
//CTX* init(BYTE* block, CELL size) {
//	if (size < (sizeof(CTX) + 2*2*sizeof(CELL))) return 0;
//	CTX* ctx = (CTX*)block;	
//	ctx->size = size;
//	ctx->here = BOTTOM(ctx);
//	ctx->there = ALIGN(BOTTOM(ctx), 2*sizeof(CELL));
//	ctx->pile = TOP(ctx);
//	CAR(ctx->pile) = 0;
//	CDR(ctx->pile) = AS(LIST, 0);
//	ctx->free = ctx->pile - 2*sizeof(CELL);
//
//	for (CELL pair = ctx->there; pair <= ctx->free; pair += 2*sizeof(CELL)) {
//		CAR(pair) = pair == ctx->free ? 0 : pair + 2*sizeof(CELL);
//		CDR(pair) = pair == ctx->there ? 0 : pair - 2*sizeof(CELL);
//	}
//
//	ctx->xstack = ctx->cpile = ctx->latest = 0;
//	ctx->state = ctx->token = ctx->in = 0;
//	ctx->tib = 0;
//	ctx->base = 10;
//
//	return ctx;
//}
//
//CELL cons(CTX* ctx, CELL car, CELL cdr) {
//	if (ctx->free == ctx->there) return 0;
//	CELL pair = ctx->free;
//	ctx->free = CDR(ctx->free);
//	CAR(pair) = car;
//	CDR(pair) = cdr;
//	return pair;
//}
//
//CELL reclaim(CTX* ctx, CELL pair) {
//	if (!pair) return 0;
//	if (TYPE(pair) == LIST) { 
//		while (CAR(pair) != 0) { 
//			CAR(pair) = reclaim(ctx, CAR(pair)); 
//		} 
//	}
//	CELL tail = NEXT(pair);
//	CDR(pair) = ctx->free;
//	CAR(pair) = 0;
//	ctx->free = pair;
//	return tail;
//}
//
//#define ERR_STACK_OVERFLOW			-1
//#define ERR_RSTACK_OVERFLOW			-2
//#define ERR_STACK_UNDERFLOW			-3
//#define ERR_RSTACK_UNDERFLOW		-4
//#define ERR_UNDEFINED_WORD			-5
//#define ERR_NOT_ENOUGH_MEMORY		-6
//#define ERR_NOT_ENOUGH_RESERVED	-7
//#define ERR_ZERO_LENGTH_NAME		-8
//#define ERR_ATOM_EXPECTED				-9
//#define ERR_LIST_EXPECTED				-10
//#define ERR_END_OF_XLIST				-11
//#define ERR_EXIT								-12
//
//CELL error(CTX* ctx, CELL err) {
//	// TODO
//	/* Lookup on exception stack for a correct handler for current error */
//	/* The debugger must be installed on the exception stack for it to work */
//	/* If a handler its found then execute it and return its return value */
//	/* If none its found, just return the error */
//	// TODO
//	/* The error function should have the ability to not just modify the context */
//	/* but of returning a compleletely new context to work on. For example, in the case */
//	/* of a memory error, a new context with a bigger buffer could be created, cloning */
//	/* the current context on it and returning it as the new context. */
//	return err;
//}
//
//#define INFO(msg) \
//    fprintf(stderr, "info: %s:%d: ", __FILE__, __LINE__); \
//    fprintf(stderr, "%s", msg);
//
//#define ERR(ctx, err)		{ INFO(""); CELL __err__ = error(ctx, err); if (__err__) { return __err__; } }
//
//#define ERR_POP(ctx, v) \
//	if (S(ctx) == 0) { \
//		ERR(ctx, ERR_STACK_UNDERFLOW); \
//	} else { \
//		v = pop(ctx); \
//	}
//
//typedef CELL (*FUNC)(CTX*);
//
//#define EXECUTE_PRIMITIVE(ctx, primitive) { \
//	CELL result = ((FUNC)primitive)(ctx); \
//	if (result < 0) { ERR(ctx, result); } \
//}
//
//CELL execute(CTX* ctx, CELL xlist) {
//	CELL result;
//	CELL p = xlist;
//	while (p) {
//		switch (TYPE(p)) {
//			case ATOM:
//				if ((S(ctx) = cons(ctx, CAR(p), AS(ATOM, S(ctx)))) == 0) { 
//					ERR(ctx, ERR_STACK_OVERFLOW); 
//				}
//				p = NEXT(p);
//				break;
//			case LIST:
//				if ((S(ctx) = cons(ctx, clone(ctx, CAR(p)), AS(LIST, S(ctx)))) == 0) { 
//					ERR(ctx, ERR_STACK_OVERFLOW); 
//				}
//				p = NEXT(p);
//				break;
//			case PRIM:
//				switch (CAR(p)) {
//					case ZJUMP: /* ZJUMP */
//						if (pop(ctx) == 0) {
//							p = CAR(NEXT(p));
//						} else {
//							p = NEXT(NEXT(p));
//						}
//						break;
//					case JUMP: /* JUMP */
//						p = CAR(NEXT(p));
//						break;
//					default:
//						EXECUTE_PRIMITIVE(ctx, CAR(p));
//						p = NEXT(p);
//						break;
//				}
//				break;
//			case WORD:
//				if (NEXT(p)) { 
//					execute(ctx, XT(CAR(p))); p = NEXT(p); 
//				} else { 
//					p = XT(CAR(p));
//				}
//				break;
//		}
//	}
//}
//
//#define TC(ctx)									(*(ctx->tib + ctx->in))
//#define TK(ctx)									(ctx->tib + ctx->token)
//#define TL(ctx)									(ctx->in - ctx->token)
//
//CELL parse_token(CTX* ctx) {
//	ctx->token = ctx->in;	while (TC(ctx) != 0 && isspace(TC(ctx))) { ctx->in++;	}
//	ctx->token = ctx->in;	while (TC(ctx) != 0 && !isspace(TC(ctx))) { ctx->in++; }
//	return ctx->in - ctx->token;
//}
//
//CELL find_token(CTX* ctx) {
//	CELL w = ctx->latest;
//	while (w && !(strlen(NFA(w)) == TL(ctx) && strncmp(NFA(w), TK(ctx), TL(ctx)) == 0)) {
//		w = NEXT(w);
//	}
//	return w;
//}
//
//CELL evaluate(CTX* ctx, BYTE* str) { 
//	CELL word, result; 
//	char *endptr;
//	ctx->tib = str; 
//	ctx->token = ctx->in = 0;
//	do {
//		if (parse_token(ctx) == 0) { return 0; }
//		if ((word = find_token(ctx)) != 0) {
//			if (!ctx->state || IMMEDIATE(word)) {
//				if ((result = execute(ctx, XT(word))) != 0) { ERR(ctx, result); }
//			} else {
//				if (compile_word(ctx, word) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//			}
//		} else {
//			intmax_t number = strtoimax(TK(ctx), &endptr, 10);
//			if (number == 0 && endptr == (char*)(TK(ctx))) {
//				ERR(ctx, ERR_UNDEFINED_WORD);
//			} else if (ctx->state) {
//				if (compile_number(ctx, number) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//			} else {
//				if (PUSH(ctx, number) == 0) { ERR(ctx, ERR_STACK_OVERFLOW); }
//			}
//		}
//	} while (1);
//}
//
//CTX* bootstrap(CTX* ctx) {
//	return ctx;
//}

#endif
