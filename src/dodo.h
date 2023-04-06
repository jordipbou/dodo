#ifndef __DODO_CORE__
#define __DODO_CORE__

#include<stdint.h>
#include<string.h>

// DATATYPES

typedef int8_t BYTE;
typedef intptr_t CELL;

enum DTypes { ATOM, LIST, PRIM, WORD };

typedef struct {
	BYTE *ibuf, *here;
	CELL there, size;
	CELL fstack, dstack, rstack, xstack;
	CELL ip, err, state, latest;
	CELL ipos, ilen;
	CELL free, ftotal, fmax;
} CTX;

typedef void (*FUNC)(CTX*);

// PAIRS

#define CDR(pair)						(*(((CELL*)pair)))
#define NEXT(pair)					(CDR(pair) & -4)
#define TYPE(pair)					(CDR(pair) & 3)

#define CAR(pair)						(*(((CELL*)pair) + 1))
#define REF(pair)						(CAR(pair) & -4)
#define SUBTYPE(pair)				(CAR(pair) & 3)

#define AS(type, next)			((next & -4) | type)
#define LINK(pair, next)		(CDR(pair) = AS(TYPE(pair), next))

CELL length(CELL list) {
	CELL count = 0;
	while (list) {
		count++;
		list = NEXT(list);
	}
	return count;
}

CELL reverse(CELL list, CELL acc) {
	if (list) {
		CELL tail = NEXT(list);
		LINK(list, acc);
		return reverse(tail, list);
	} else {
		return acc;
	}
}

// CONTEXT

#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))
#define BOTTOM(ctx)					(((BYTE*)ctx) + sizeof(CTX))
#define TOP(ctx)						(ALIGN(((BYTE*)ctx) + ctx->size - 2*sizeof(CELL) - 1, 2*sizeof(CELL)))
#define RESERVED(ctx)				((CELL)(((BYTE*)ctx->there) - ctx->here))

#define TOS(ctx)						(CAR(ctx->dstack))

CTX* init(BYTE* block, CELL size) {
	CELL pair;

	CTX* ctx = (CTX*)block;	

	ctx->size = size;

	ctx->here = BOTTOM(ctx);
	ctx->there = ALIGN(BOTTOM(ctx), 2*sizeof(CELL));

	ctx->dstack = TOP(ctx); CAR(ctx->dstack) = 0; CDR(ctx->dstack) = AS(LIST, 0);
	ctx->fstack = ctx->dstack - 2*sizeof(CELL);
	ctx->rstack = 0;
	ctx->xstack = 0;

	ctx->free = -1;
	for (pair = ctx->there; pair <= ctx->fstack; pair += 2*sizeof(CELL)) {
		ctx->free++;
		CAR(pair) = pair == ctx->fstack ? 0 : pair + 2*sizeof(CELL);
		CDR(pair) = pair == ctx->there ? 0 : pair - 2*sizeof(CELL);
	}

	ctx->fmax = 0;
	ctx->ftotal = ctx->free;
	ctx->ibuf = 0;
	ctx->ipos = ctx->ilen = ctx->state = ctx->err = ctx->latest = 0;
	ctx->ip = 0;

	return ctx;
}

CELL cons(CTX* ctx, CELL car, CELL cdr) {
	CELL pair;

	if (ctx->free == 0) return 0;

	ctx->free--;
	pair = ctx->fstack;
	ctx->fstack = CDR(ctx->fstack);
	CAR(pair) = car;
	CDR(pair) = cdr;

	return pair;
}

CELL reclaim(CTX* ctx, CELL pair) {
	CELL tail;

	if (TYPE(pair) == LIST) {
		while (CAR(pair)) {
			CAR(pair) = reclaim(ctx, CAR(pair));
		}
	}

	ctx->free++;
	tail = NEXT(pair);
	CDR(pair) = ctx->fstack;
	CAR(pair) = 0;
	ctx->fstack = pair;

	return tail;
}

CELL clone(CTX* ctx, CELL pair) {
	if (!pair) return 0;
	if (TYPE(pair) == LIST) {
		return cons(ctx, clone(ctx, CAR(pair)), AS(LIST, clone(ctx, NEXT(pair))));
	} else {
		return cons(ctx, CAR(pair), AS(TYPE(pair), clone(ctx, NEXT(pair))));
	}
}

// ERRORS

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_DIVISION_BY_ZERO		-3
#define ERR_NOT_ENOUGH_MEMORY		-4
#define ERR_NOT_ENOUGH_RESERVED	-5
#define ERR_UNDEFINED_WORD			-6
#define ERR_EXPECTED_LIST				-7
#define ERR_ZERO_LENGTH_NAME		-8
#define ERR_NO_INPUT_SOURCE			-9
#define ERR_END_OF_INPUT_SOURCE	-10
#define ERR_HEADER_NOT_FOUND		-11

void error(CTX* ctx) {
	CELL handler = ctx->xstack;
	while (handler) {
		if (CAR(CAR(handler)) == ctx->err || CAR(CAR(handler)) == 0) {
			((FUNC)CAR(NEXT(CAR(handler))))(ctx);
			if (ctx->err == 0) return;
		}
		handler = NEXT(handler);
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
#define UF1(ctx)			ERR(ctx, TOS(ctx) == 0, ERR_STACK_UNDERFLOW)
#define UF2(ctx)			ERR(ctx, TOS(ctx) == 0 || NEXT(TOS(ctx)) == 0, ERR_STACK_UNDERFLOW)
#define UF3(ctx)			ERR(ctx, TOS(ctx) == 0 || NEXT(TOS(ctx)) == 0 || NEXT(NEXT(TOS(ctx))) == 0, ERR_STACK_UNDERFLOW)

#define EL(ctx)				UF1(ctx); ERR(ctx, TYPE(TOS(ctx)) != LIST, ERR_EXPECTED_LIST)

#define DO(ctx, primitive)		({ primitive(ctx); if (ctx->err < 0) { return; }; })

// STACK PRIMITIVES

void duplicate(CTX* ctx) { /* ( n -- n n ) */
	if (TYPE(TOS(ctx)) == LIST) {
		TOS(ctx) = cons(ctx, clone(ctx, CAR(TOS(ctx))), AS(LIST, TOS(ctx)));
	} else {
		TOS(ctx) = cons(ctx, CAR(TOS(ctx)), AS(TYPE(TOS(ctx)), TOS(ctx)));
	}
}

void swap(CTX* ctx) {	/* ( n2 n1 -- n1 n2 ) */
	CELL temp = NEXT(TOS(ctx));
	LINK(TOS(ctx), NEXT(NEXT(TOS(ctx))));
	LINK(temp, TOS(ctx));
	TOS(ctx) = temp;
}

void drop(CTX* ctx) { /* ( n -- ) */
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void over(CTX* ctx) { /* ( n2 n1 -- n2 n1 n2 ) */
	if (TYPE(NEXT(TOS(ctx))) == LIST) {
		TOS(ctx) = cons(ctx, clone(ctx, CAR(NEXT(TOS(ctx)))), AS(LIST, TOS(ctx)));
	} else {
		TOS(ctx) = cons(ctx, CAR(NEXT(TOS(ctx))), AS(TYPE(NEXT(TOS(ctx))), TOS(ctx)));
	}
}

void rot(CTX* ctx) { /* ( n3 n2 n1 -- n1 n3 n2 ) */
	CELL temp = NEXT(NEXT(TOS(ctx)));
	LINK(NEXT(TOS(ctx)), NEXT(NEXT(NEXT(TOS(ctx)))));
	LINK(temp, TOS(ctx));
	TOS(ctx) = temp;
}


// ARITHMETIC PRIMITIVES

void add(CTX* ctx) {	/* ( n2 n1 -- n:(n2 + n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) + CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void incr(CTX* ctx) {	/* ( n -- n:(n + 1) ) */
	UF1(ctx);
	CAR(TOS(ctx))++;
}

void sub(CTX* ctx) {	/* ( n2 n1 -- n:(n2 - n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) - CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void decr(CTX* ctx) {	/* ( n -- n:(n - 1) ) */
	UF1(ctx);
	CAR(TOS(ctx))--;
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

// COMPARISON PRIMITIVES

void gt(CTX* ctx) {	/* ( n2 n1 -- n:(n2 > n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) > CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void lt(CTX* ctx) { /* ( n2 n1 -- n:(n2 < n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) < CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void eq(CTX* ctx) { /* ( n2 n1 -- n:(n2 = n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) == CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void neq(CTX* ctx) { /* ( n2 n1 -- n:(n2 <> n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) != CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

// BIT PRIMITIVES

void and(CTX* ctx) { /* ( n2 n1 -- n:(n2 and n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) & CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void or(CTX* ctx) { /* ( n2 n1 -- n:(n2 or n1) ) */
	UF2(ctx);
	CAR(NEXT(TOS(ctx))) = CAR(NEXT(TOS(ctx))) | CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
}

void invert(CTX* ctx) { /* ( n -- n:(inverted bits) ) */
	UF1(ctx);
	CAR(TOS(ctx)) = ~(CAR(TOS(ctx)));
}

// LIST PRIMITIVES

void empty(CTX* ctx) { /* ( -- {} ) */
	OF(ctx, 1);
	TOS(ctx) = cons(ctx, 0, AS(LIST, TOS(ctx)));
}

void list_to_stack(CTX* ctx) { /* ( { ... } -- ... ) */
	EL(ctx);
	CELL temp = TOS(ctx);
	TOS(ctx) = NEXT(TOS(ctx));
	LINK(temp, ctx->dstack);
	ctx->dstack = temp;
}

void stack_to_list(CTX* ctx) { /* ( ... -- { ... } ) */
	TOS(ctx) = reverse(TOS(ctx), 0);
	if (NEXT(ctx->dstack) == 0) {
		OF(ctx, 1);
		ctx->dstack = cons(ctx, ctx->dstack, AS(LIST, 0));
	} else {
		CELL temp = ctx->dstack;
		ctx->dstack = NEXT(ctx->dstack);
		LINK(temp, TOS(ctx));
		TOS(ctx) = temp;
	}
}

// TODO: Analyze cons
void _cons(CTX* ctx) { /* ( i1 i2 -- l:(i1 ..i2) */
	UF2(ctx);
	if (TYPE(TOS(ctx)) == LIST) {
		CELL h = NEXT(TOS(ctx));
		LINK(TOS(ctx), NEXT(NEXT(TOS(ctx))));
		LINK(h, CAR(TOS(ctx)));
		CAR(TOS(ctx)) = h;
	} else {
		CELL a = TOS(ctx);
		CELL b = NEXT(TOS(ctx));
		CELL t = NEXT(NEXT(TOS(ctx)));
		LINK(b, a);
		LINK(a, 0);
		TOS(ctx) = cons(ctx, b, AS(LIST, t));
	}
}

void uncons(CTX* ctx) { /* ( l -- h t ) */
	EL(ctx);
	if (CAR(TOS(ctx)) == 0) {
		TOS(ctx) = reclaim(ctx, TOS(ctx));
	} else {
		CELL head = CAR(TOS(ctx));
		CELL tail = NEXT(CAR(TOS(ctx)));
		CAR(TOS(ctx)) = tail;
		LINK(head, TOS(ctx));
		TOS(ctx) = head;
		DO(ctx, swap);
	}
}

void append(CTX* ctx) { /* ( l i -- l:(i ..l) ) */
	UF2(ctx);
	CELL i = TOS(ctx);
	TOS(ctx) = NEXT(TOS(ctx));
	LINK(i, CAR(TOS(ctx)));
	CAR(TOS(ctx)) = i;
}

// MEMORY

void fetch(CTX* ctx) {	/* ( addr -- n ) */
	UF1(ctx);
	CAR(TOS(ctx)) = *((CELL*)CAR(TOS(ctx)));
}

void store(CTX* ctx) {	/* ( n addr -- ) */
	UF2(ctx);
	*((CELL*)CAR(TOS(ctx))) = CAR(NEXT(TOS(ctx)));
	TOS(ctx) = reclaim(ctx, reclaim(ctx, TOS(ctx)));
}

void bfetch(CTX* ctx) {	/* ( c-addr -- c ) */
	UF1(ctx);
	CAR(TOS(ctx)) = (CELL)*((BYTE*)CAR(TOS(ctx)));
}

void bstore(CTX* ctx) {	/* ( c c-addr -- ) */
	UF2(ctx);
	*((BYTE*)CAR(TOS(ctx))) = (BYTE)CAR(NEXT(TOS(ctx)));
	TOS(ctx) = reclaim(ctx, reclaim(ctx, TOS(ctx)));
}

void shrink(CTX* ctx) { 
	ERR(ctx, CAR(ctx->there) != (ctx->there + 2*sizeof(CELL)), ERR_NOT_ENOUGH_MEMORY);
	ctx->there += 2*sizeof(CELL);
	CDR(ctx->there) = 0;
	ctx->free--;
	ctx->ftotal--;
}

void grow(CTX* ctx) {
	ERR(ctx, RESERVED(ctx) < 2*sizeof(CELL), ERR_NOT_ENOUGH_RESERVED);
	CDR(ctx->there) = ctx->there - 2*sizeof(CELL);
	ctx->there -= 2*sizeof(CELL);
	CAR(ctx->there) = ctx->there + 2*sizeof(CELL);
	CDR(ctx->there) = 0;
	ctx->free++;
	ctx->ftotal++;
}

void allot(CTX* ctx) { 
	UF1(ctx);
	CELL bytes = CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
	if (bytes > 0) {
		ERR(ctx, bytes >= (TOP(ctx) - ((CELL)ctx->here)), ERR_NOT_ENOUGH_MEMORY);
		while (RESERVED(ctx) < bytes) {
			shrink(ctx); if (ctx->err < 0) return;
		}
		ctx->here += bytes;
	} else if (bytes < 0) {
		if (bytes < (BOTTOM(ctx) - ctx->here)) {
			ctx->here = BOTTOM(ctx);
		} else {
			ctx->here += bytes;
		}
		while (RESERVED(ctx) >= 2*sizeof(CELL)) {
			grow(ctx); if (ctx->err < 0) return;
		}
	}
}

// PARSING

void parse(CTX* ctx) {
	ERR(ctx, ctx->ibuf == 0, ERR_NO_INPUT_SOURCE);
	OF(ctx, 1);
	CELL k = CAR(TOS(ctx));
	while(*(ctx->ibuf + ctx->ipos) && *(ctx->ibuf + ctx->ipos) != k && ctx->ipos < ctx->ilen) {
		ctx->ipos++;
	}
	CAR(TOS(ctx)) = (CELL)ctx->ibuf;
	TOS(ctx) = cons(ctx, ctx->ipos, AS(ATOM, TOS(ctx)));
	ctx->ipos++;
}

void parse_name(CTX* ctx) {
	ERR(ctx, ctx->ibuf == 0, ERR_NO_INPUT_SOURCE);
	OF(ctx, 2);
	while(*(ctx->ibuf + ctx->ipos) && isspace(*(ctx->ibuf + ctx->ipos)) && ctx->ipos < ctx->ilen) {
		ctx->ipos++;
	}
	TOS(ctx) = cons(ctx, (CELL)(ctx->ibuf + ctx->ipos), AS(ATOM, TOS(ctx)));
	while(*(ctx->ibuf + ctx->ipos) && !isspace(*(ctx->ibuf + ctx->ipos)) && ctx->ipos < ctx->ilen) {
		ctx->ipos++;
	}
	TOS(ctx) = cons(ctx, (CELL)((ctx->ibuf + ctx->ipos) - CAR(TOS(ctx))), AS(ATOM, TOS(ctx)));
}

void parse_name_list(CTX* ctx) {
	ERR(ctx, ctx->ibuf == 0, ERR_NO_INPUT_SOURCE);
	while(*(ctx->ibuf + ctx->ipos) && isspace(*(ctx->ibuf + ctx->ipos)) && ctx->ipos < ctx->ilen) {
		ctx->ipos++;
	}
	// TODO: Check stack overflow
	DO(ctx, empty);
	while(*(ctx->ibuf + ctx->ipos) && !isspace(*(ctx->ibuf + ctx->ipos)) && ctx->ipos < ctx->ilen) {
		if (*(ctx->ibuf + ctx->ipos) != 10) {
			// TODO: Check stack overflow
			CAR(TOS(ctx)) = cons(ctx, (CELL)*(ctx->ibuf + ctx->ipos), AS(ATOM, CAR(TOS(ctx))));
		}
		ctx->ipos++;
	}
	CAR(TOS(ctx)) = reverse(CAR(TOS(ctx)), 0);
}

// DICTIONARY

enum WordSubtypes { NDNN, DNN, NDN, DN };

#define NFA(word)							((BYTE*)CAR(REF(word)))

#define IS_DUAL(word)					(TYPE(word) == WORD && (SUBTYPE(word) == DNN || SUBTYPE(word) == DN))
#define IS_IMMEDIATE(word)		(IS_DUAL(word) && CT(word) == IT(word))
#define HAS_NAMESPACE(word)		(TYPE(word) == WORD && (SUBTYPE(word) == NDN || SUBTYPE(word) == DN))

CELL XT(CTX* ctx, CELL word) {
	if (TYPE(word) == LIST) {
		return CAR(word);
	} else if (TYPE(word) == WORD) {
		if (IS_DUAL(word)) {
			if (ctx->state) {
				return CAR(NEXT(REF(word)));
			} else {
				return NEXT(NEXT(REF(word)));
			}
		} else {
			return NEXT(REF(word));
		}
	}
}

#define IS_PRIMITIVE(ctx, word)	(length(XT(ctx, word)) == 1 && TYPE(XT(ctx, word)) == PRIM)

#define ADD_PRIMITIVE(ctx, name, func) \
	ctx->latest = \
		cons(ctx, AS(NDNN, \
			cons(ctx, (CELL)name, AS(ATOM, \
			cons(ctx, (CELL)func, AS(PRIM, 0))))), \
		AS(WORD, ctx->latest));

#define ADD_DUAL_PRIMITIVE(ctx, name, cfunc, ifunc) \
	ctx->latest = \
		cons(ctx, AS(DNN, \
			cons(ctx, (CELL)name, AS(ATOM, \
			cons(ctx, \
				cons(ctx, (CELL)cfunc, AS(PRIM, 0)), \
			AS(LIST, \
			cons(ctx, (CELL)ifunc, AS(PRIM, 0))))))), \
		AS(WORD, ctx->latest));

// TODO: Adapt find name to find from a LIST of chars or a SLITERAL string and to words
// with an SLITERAL NFA or with a LIST based NFA
void find_name(CTX* ctx) {
	UF2(ctx);
	ERR(ctx, CAR(TOS(ctx)) == 0, ERR_ZERO_LENGTH_NAME);
	CELL word = ctx->latest;
	while (word && 
	!(strlen(NFA(word)) == CAR(TOS(ctx)) && 
	  !strncmp(NFA(word), (BYTE*)CAR(NEXT(TOS(ctx))), CAR(TOS(ctx)))
	)) {
		word = NEXT(word);
	}
	if (word) {
		TOS(ctx) = reclaim(ctx, TOS(ctx));
		CAR(TOS(ctx)) = word;
		CDR(TOS(ctx)) = AS(WORD, NEXT(TOS(ctx)));
	} else {
		OF(ctx, 1);
		TOS(ctx) = cons(ctx, 0, AS(ATOM, TOS(ctx)));
	}
}

void header(CTX* ctx) {
	OF(ctx, 1);
	TOS(ctx) = cons(ctx, 0, AS(WORD, TOS(ctx)));
}

// INNER INTERPRETER

#define IP(ctx)				(ctx->ip)

void next(CTX* ctx) {
	if (IP(ctx)) {
		IP(ctx) = NEXT(IP(ctx));
	}
	while (!IP(ctx) && ctx->rstack) {
		if (TYPE(ctx->rstack) == ATOM) {
			IP(ctx) = NEXT(CAR(ctx->rstack));
		}
		ctx->rstack = reclaim(ctx, ctx->rstack);
	}
}

#define JUMP(ctx, list)			(ctx->ip = list)
#define CALL(ctx, ip) \
	({ \
		if (IP(ctx)) { \
			ctx->rstack = cons(ctx, IP(ctx), AS(ATOM, ctx->rstack)); \
		} \
		IP(ctx) = ip; \
	})

void dump_context(CTX* ctx);

CELL step(CTX* ctx) {
	CELL r;
	if (IP(ctx)) {
		switch (TYPE(IP(ctx))) {
			case ATOM:
				TOS(ctx) = cons(ctx, CAR(IP(ctx)), AS(ATOM, TOS(ctx)));
				next(ctx);
				break;
			case LIST:
				TOS(ctx) = cons(ctx, clone(ctx, CAR(IP(ctx))), AS(LIST, TOS(ctx)));
				next(ctx);
				break;
			case PRIM:
				r = IP(ctx);
				((FUNC)CAR(IP(ctx)))(ctx);
				if (r == IP(ctx)) {
					next(ctx);
				}
				break;
			case WORD:
				CALL(ctx, XT(ctx, REF(IP(ctx))));
				break;
		}
	}
	if ((ctx->ftotal - ctx->free) > ctx->fmax) {
		ctx->fmax = ctx->ftotal - ctx->free;
	}
	//dump_context(ctx);
	//getchar();
	return IP(ctx);
}

void exec_i(CTX* ctx) { /* ( xt -- ) */
	CELL p;
	UF1(ctx);
	switch (TYPE(TOS(ctx))) {
		case ATOM: 
			// This executes to be compatible with Forth, is it correct?
			p = CAR(TOS(ctx)); 
			TOS(ctx) = reclaim(ctx, TOS(ctx)); 
			((FUNC)p)(ctx);
			break;
		case LIST: 
			if (CAR(TOS(ctx))) {
				CELL t = TOS(ctx);
				TOS(ctx) = NEXT(TOS(ctx));
				CALL(ctx, CAR(t));
				LINK(t, ctx->rstack);
				ctx->rstack = t;
				step(ctx);
			} else {
				TOS(ctx) = reclaim(ctx, TOS(ctx));
			}
			break;
		case PRIM: 
			p = CAR(TOS(ctx)); 
			TOS(ctx) = reclaim(ctx, TOS(ctx)); 
			((FUNC)p)(ctx); 
			break;
		case WORD: 
			p = IP(ctx);
			CALL(ctx, XT(ctx, CAR(TOS(ctx))));
			TOS(ctx) = reclaim(ctx, TOS(ctx));
			while (step(ctx) != p);
			break;
	}
}

void exec_x(CTX* ctx) { /* ( xt -- xt ) */
	UF1(ctx); 
	duplicate(ctx); 
	exec_i(ctx); 
}

// CONTROL FLOW

void branch(CTX* ctx) { /* ( b xt_true xt_false -- ) */
	UF3(ctx); 
	if (CAR(NEXT(NEXT(TOS(ctx)))) == 0) { 
		swap(ctx); 
	} 
	drop(ctx); 
	swap(ctx); 
	drop(ctx); 
	exec_i(ctx); 
}

void recurse(CTX* ctx) {
	OF(ctx, 1);
	CELL s = ctx->dstack;
	while (s != 0) {
		CELL i = CAR(s);
		while (i && !(TYPE(i) == WORD && CAR(i) == 0)) {
			i = NEXT(i);
		}
		if (i) {
			TOS(ctx) = cons(ctx, i, AS(WORD, TOS(ctx)));
			return;
		}
		s = NEXT(s);
	}
	ctx->err = ERR_HEADER_NOT_FOUND;
}

// COMPILATION

void compile(CTX* ctx) {
	UF1(ctx);
	CELL word = CAR(TOS(ctx));
	TOS(ctx) = reclaim(ctx, TOS(ctx));
	if (IS_PRIMITIVE(ctx, word)) {
		TOS(ctx) = cons(ctx, CAR(XT(ctx, word)), AS(PRIM, TOS(ctx)));
	} else {
		TOS(ctx) = cons(ctx, word, AS(WORD, TOS(ctx)));
	}
}

void postpone(CTX* ctx) {
	DO(ctx, parse_name);
	DO(ctx, find_name);
	compile(ctx);
}

void lbracket(CTX* ctx) {
	ctx->state = 0;
}

void rbracket(CTX* ctx) {
	ctx->state = 1;
}

// OUTER INTERPRETER

void outer(CTX* ctx) {
	char *endptr;
	do {
		//dump_context(ctx);
		parse_name(ctx); ERR(ctx, CAR(TOS(ctx)) == 0, ERR_END_OF_INPUT_SOURCE; drop(ctx); drop(ctx));
		find_name(ctx); if (ctx->err < 0) return;
		if (CAR(TOS(ctx)) == 0) {
			CELL addr = CAR(NEXT(NEXT(TOS(ctx))));
			TOS(ctx) = reclaim(ctx, reclaim(ctx, reclaim(ctx, TOS(ctx))));
			intmax_t n = strtoimax((BYTE*)addr, &endptr, 10);
			ERR(ctx, n == 0 && endptr == (char*)addr, ERR_UNDEFINED_WORD);
			TOS(ctx) = cons(ctx, n, AS(ATOM, TOS(ctx)));
		} else {
			if (ctx->state == 0 || IS_DUAL(REF(TOS(ctx)))) {
				exec_i(ctx);
			} else {
				compile(ctx);
			}
		}
		if ((ctx->ftotal - ctx->free) > ctx->fmax) {
			ctx->fmax = ctx->ftotal - ctx->free;
		}
	} while(1);
}

void evaluate(CTX* ctx) {
	UF2(ctx);
	ctx->ipos = 0;
	ctx->ilen = CAR(TOS(ctx));
	ctx->ibuf = (BYTE*)CAR(NEXT(TOS(ctx)));
	TOS(ctx) = reclaim(ctx, reclaim(ctx, TOS(ctx)));
	outer(ctx);
}

// TODO: Organize

void sliteral(CTX* ctx) {
	// TODO: Should a string literal have a count?
	CELL u = CAR(TOS(ctx));
	BYTE* addr = (BYTE*)CAR(NEXT(TOS(ctx)));
	DO(ctx, incr);
	BYTE* here = ctx->here;
	DO(ctx, allot);
	CELL i;
	for (i = 0; i < u; i++) {
		here[i] = addr[i];
	}
	here[u] = 0;
	CAR(TOS(ctx)) = (CELL)here;
	TOS(ctx) = cons(ctx, u, AS(ATOM, TOS(ctx)));
}

//void header(CTX* ctx) {
//	DO(ctx, parse_name);
//	DO(ctx, sliteral);
//	DO(ctx, drop);
//	TOS(ctx) = cons(ctx, 0, AS(WORD, TOS(ctx)));
//	DO(ctx, swap);
//}

void reveal(CTX* ctx) {
	UF1(ctx);
	EL(ctx);
	CELL w = CAR(TOS(ctx));
	CAR(TOS(ctx)) = 0;
	TOS(ctx) = reclaim(ctx, TOS(ctx));
	CAR(w) = NEXT(w);
	LINK(w, ctx->latest);
	ctx->latest = w;
}

void immediate(CTX* ctx) {
	if (SUBTYPE(ctx->latest) == NDNN) {
		CELL xt = NEXT(CAR(ctx->latest));
		LINK(CAR(ctx->latest), cons(ctx, clone(ctx, xt), AS(LIST, xt)));
		CAR(ctx->latest) = AS(DNN, REF(ctx->latest));
	} else if (SUBTYPE(ctx->latest) == NDN) {
		CELL xt = NEXT(CAR(ctx->latest));
		LINK(CAR(ctx->latest), cons(ctx, clone(ctx, xt), AS(LIST, xt)));
		CAR(ctx->latest) = AS(DN, REF(ctx->latest));
	}
}

void type(CTX* ctx) {
	UF1(ctx);
	if (TYPE(TOS(ctx)) == LIST) {
		CELL c = CAR(TOS(ctx));
		while (c) {
			printf("%c", (BYTE)CAR(c));
			c = NEXT(c);
		}
		TOS(ctx) = reclaim(ctx, TOS(ctx));
	} else {
		UF2(ctx);
		CELL u = CAR(TOS(ctx));
		BYTE* addr = (BYTE*)CAR(NEXT(TOS(ctx)));
		TOS(ctx) = reclaim(ctx, reclaim(ctx, TOS(ctx)));
		printf("%.*s", (int)u, addr);
	}
}

// INSPECTION

void _next(CTX* ctx) {
	UF1(ctx);
	CAR(TOS(ctx)) = NEXT(CAR(TOS(ctx)));
}

void _type(CTX* ctx) {
	UF1(ctx);
	printf("\nTYPE: ");
	switch (TYPE(CAR(TOS(ctx)))) {
		case ATOM: printf("ATOM\n"); break;
		case LIST: printf("LIST\n"); break;
		case PRIM: printf("PRIM\n"); break;
		case WORD: printf("WORD\n"); break;
	}
}

void _subtype(CTX* ctx) {
	UF1(ctx);
	printf("\nSUBTYPE: ");
	switch (SUBTYPE(CAR(TOS(ctx)))) {
		case NDNN: printf("NDNN\n"); break;
		case DNN: printf("DNN\n"); break;
		case NDN: printf("NDN\n"); break;
		case DN: printf("DN\n"); break;
	}
}

void _latest(CTX* ctx) {
	OF(ctx, 1);
	TOS(ctx) = cons(ctx, ctx->latest, AS(WORD, TOS(ctx)));
}

// BOOTSTRAPING

CTX* bootstrap(CTX* ctx) {
	ADD_PRIMITIVE(ctx, "latest", (CELL)&_latest);
	ADD_PRIMITIVE(ctx, "T", (CELL)&_type);
	ADD_PRIMITIVE(ctx, "N", (CELL)&_next);
	ADD_PRIMITIVE(ctx, "ST", (CELL)&_subtype);

	ADD_PRIMITIVE(ctx, "dup", (CELL)&duplicate);
	ADD_PRIMITIVE(ctx, "swap", (CELL)&swap);
	ADD_PRIMITIVE(ctx, "drop", (CELL)&drop);
	ADD_PRIMITIVE(ctx, "over", (CELL)&over);
	ADD_PRIMITIVE(ctx, "rot", (CELL)&rot);

	ADD_PRIMITIVE(ctx, "+", (CELL)&add);
	ADD_PRIMITIVE(ctx, "1+", (CELL)&incr);
	ADD_PRIMITIVE(ctx, "-", (CELL)&sub);
	ADD_PRIMITIVE(ctx, "1-", (CELL)&decr);
	ADD_PRIMITIVE(ctx, "*", (CELL)&mul);
	ADD_PRIMITIVE(ctx, "/", (CELL)&div);
	ADD_PRIMITIVE(ctx, "mod", (CELL)&mod);

	ADD_PRIMITIVE(ctx, ">", (CELL)&gt);
	ADD_PRIMITIVE(ctx, "<", (CELL)&lt);
	ADD_PRIMITIVE(ctx, "=", (CELL)&eq);
	ADD_PRIMITIVE(ctx, "<>", (CELL)&neq);

	ADD_PRIMITIVE(ctx, "and", (CELL)&and);
	ADD_PRIMITIVE(ctx, "or", (CELL)&or);
	ADD_PRIMITIVE(ctx, "invert", (CELL)&invert);

	ADD_PRIMITIVE(ctx, "{}", (CELL)&empty);
	ADD_PRIMITIVE(ctx, "l>s", (CELL)&list_to_stack);
	ADD_PRIMITIVE(ctx, "s>l", (CELL)&stack_to_list);
	ADD_PRIMITIVE(ctx, "uncons", (CELL)&uncons);
	ADD_PRIMITIVE(ctx, "cons", (CELL)&_cons);
	ADD_PRIMITIVE(ctx, "append", (CELL)&append);

	ADD_PRIMITIVE(ctx, "@", (CELL)&fetch);
	ADD_PRIMITIVE(ctx, "!", (CELL)&store);
	ADD_PRIMITIVE(ctx, "c@", (CELL)&bfetch);
	ADD_PRIMITIVE(ctx, "c!", (CELL)&bstore);

	ADD_PRIMITIVE(ctx, "shrink", (CELL)&shrink);
	ADD_PRIMITIVE(ctx, "grow", (CELL)&grow);
	ADD_PRIMITIVE(ctx, "allot", (CELL)&allot);

	ADD_PRIMITIVE(ctx, "parse", (CELL)&parse);
	ADD_PRIMITIVE(ctx, "parse-name", (CELL)&parse_name);
	ADD_PRIMITIVE(ctx, "parse-name-list", (CELL)&parse_name_list);

	ADD_PRIMITIVE(ctx, "find-name", (CELL)&find_name);

	ADD_PRIMITIVE(ctx, "i", (CELL)&exec_i);
	ADD_PRIMITIVE(ctx, "x", (CELL)&exec_x);
	ADD_PRIMITIVE(ctx, "branch", (CELL)&branch);

	ADD_PRIMITIVE(ctx, "compile,", (CELL)&compile);
	ADD_DUAL_PRIMITIVE(ctx, "postpone", (CELL)&postpone, (CELL)&postpone);
	ADD_DUAL_PRIMITIVE(ctx, "[", (CELL)&lbracket, (CELL)&lbracket);
	ADD_PRIMITIVE(ctx, "]", (CELL)&rbracket);

	ADD_PRIMITIVE(ctx, "evaluate", (CELL)&evaluate);

	ADD_DUAL_PRIMITIVE(ctx, "sliteral", (CELL)&sliteral, (CELL)&sliteral);
	ADD_PRIMITIVE(ctx, "header", (CELL)&header);
	ADD_PRIMITIVE(ctx, "reveal", (CELL)&reveal);
	ADD_DUAL_PRIMITIVE(ctx, "recurse", (CELL)&recurse, (CELL)&recurse);
	ADD_PRIMITIVE(ctx, "immediate", (CELL)&immediate);

	ADD_PRIMITIVE(ctx, "type", (CELL)&type);

	ADD_PRIMITIVE(ctx, ".c", (CELL)&dump_context);

	ctx->fmax = ctx->ftotal - ctx->free;

	return ctx;
}

// DUMPS

void dump_cell(CTX* ctx, CELL cell, CELL direction);

void dump_list(CTX* ctx, CELL list, CELL direction) {
	if (!list) return;
	if (direction == 0) { 
		while (list) { 
			dump_cell(ctx, list, direction); 
			list = NEXT(list); 
		} 
	} else {
		if (NEXT(list)) { 
			dump_list(ctx, NEXT(list), direction); 
		}
		dump_cell(ctx, list, direction - 1);
	}
}

void dump_cell(CTX* ctx, CELL cell, CELL direction) {
	if (!cell) return;
	switch (TYPE(cell)) {
		case ATOM: 
			printf("#%ld ", CAR(cell)); 
			break;
		case LIST: 
			printf("{ "); 
			dump_list(ctx, CAR(cell), direction); 
			printf("} "); 
			break;
		case PRIM:  // TODO: This should use find_primitive
			if (CAR(cell) == (CELL)&duplicate) printf("P:dup ");
			else if (CAR(cell) == (CELL)&swap) printf("P:swap ");
			else if (CAR(cell) == (CELL)&drop) printf("P:drop ");
			else if (CAR(cell) == (CELL)&over) printf("P:over ");
			else if (CAR(cell) == (CELL)&rot) printf("P:rot ");
			else if (CAR(cell) == (CELL)&add) printf("P:+ ");
			else if (CAR(cell) == (CELL)&sub) printf("P:- ");
			else if (CAR(cell) == (CELL)&mul) printf("P:* ");
			else if (CAR(cell) == (CELL)&div) printf("P:/ ");
			else if (CAR(cell) == (CELL)&mod) printf("P:mod ");
			else if (CAR(cell) == (CELL)&gt) printf("P:> ");
			else if (CAR(cell) == (CELL)&lt) printf("P:< ");
			else if (CAR(cell) == (CELL)&eq) printf("P:= ");
			else if (CAR(cell) == (CELL)&neq) printf("P:<> ");
			else if (CAR(cell) == (CELL)&and) printf("P:and ");
			else if (CAR(cell) == (CELL)&or) printf("P:or ");
			else if (CAR(cell) == (CELL)&invert) printf("P:invert ");
			else if (CAR(cell) == (CELL)&empty) printf("P:{} ");
			else if (CAR(cell) == (CELL)&list_to_stack) printf("P:l>s ");
			else if (CAR(cell) == (CELL)&stack_to_list) printf("P:s>l ");
			else if (CAR(cell) == (CELL)&fetch) printf("P:@ ");
			else if (CAR(cell) == (CELL)&store) printf("P:! ");
			else if (CAR(cell) == (CELL)&bfetch) printf("P:c@ ");
			else if (CAR(cell) == (CELL)&bstore) printf("P:c! ");
			else if (CAR(cell) == (CELL)&shrink) printf("P:shrink ");
			else if (CAR(cell) == (CELL)&grow) printf("P:grow ");
			else if (CAR(cell) == (CELL)&allot) printf("P:allot ");
			else if (CAR(cell) == (CELL)&parse) printf("P:parse ");
			else if (CAR(cell) == (CELL)&parse_name) printf("P:parse-name ");
			else if (CAR(cell) == (CELL)&find_name) printf("P:find-name ");
			else if (CAR(cell) == (CELL)&exec_i) printf("P:i ");
			else if (CAR(cell) == (CELL)&exec_x) printf("P:x ");
			else if (CAR(cell) == (CELL)&branch) printf("P:branch ");
			else if (CAR(cell) == (CELL)&compile) printf("P:compile, ");
			//else if (CAR(cell) == (CELL)&postpone) printf("P:postpone ");
			else if (CAR(cell) == (CELL)&lbracket) printf("P:[ ");
			else if (CAR(cell) == (CELL)&rbracket) printf("P:] ");
			else if (CAR(cell) == (CELL)&uncons) printf("P:uncons ");
			else printf("P:%ld ", CAR(cell));
			break;
		case WORD: 
			if (cell != 0) {
				if (CAR(cell) != 0) {
					if (CAR(CAR(cell)) != 0) {
						printf("W:%s ", (BYTE*)NFA(CAR(cell)));
					} else {
						printf("W:RECURSE ");
					}
				} else {
					printf("HEADER ");
				}
			}
			break;
	}
}

void dump_context(CTX* ctx) {
	CELL i;
	printf("%c/%ld/[%ld:%ld(%ld)]", ctx->state ? 'C' : 'I', ctx->err, ctx->ftotal, ctx->free, ctx->fmax);
	if (ctx->ibuf) {
		printf("'");
		for (i = 0; i < ctx->ipos; i++) {
			if (ctx->ibuf[i] != 0 && ctx->ibuf[i] != 10 && ctx->ibuf[i] != 13) {
				printf("%c", ctx->ibuf[i]);
			}
		}
		printf("^");
		for (i = ctx->ipos; i < ctx->ilen; i++) {
			if (ctx->ibuf[i] != 0 && ctx->ibuf[i] != 10 && ctx->ibuf[i] != 13) {
				printf("%c", ctx->ibuf[i]);
			}
		}
		printf("'");
	}
	printf("\n");
	dump_list(ctx, ctx->dstack, 2);
	printf("‖ ");
	printf("{ ");
	dump_list(ctx, IP(ctx), 0);
	printf("} ");
	CELL p = ctx->rstack;
	while (p) {
		printf("{ ");
		dump_list(ctx, CAR(p), 0);
		printf("} ");
		p = NEXT(p);
	}
	printf("\n");
}

#endif

