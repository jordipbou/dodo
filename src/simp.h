#ifndef __DODO__
#define __DODO__

// DATATYPES

#include <stdint.h>
#include <string.h>

typedef int8_t BYTE;
typedef intptr_t CELL;
#if INTPTR_MAX == INT64_MAX
	typedef struct T_NODE {
		struct T_NODE* next;
		union {
			CELL value;
			struct T_NODE* ref;
			struct {
				int16_t type;
				int16_t size;
				int32_t length;
			};
		};
		CELL data[];
	} NODE;
#else
	typedef struct T_NODE {
		struct T_NODE* next;
		union {
			CELL value;
			struct T_NODE* ref;
			struct {
				int8_t type;
				int8_t size;
				int16_t length;
			};
		};
		CELL data[];
	} NODE;
#endif

#define NEXT(node)								((NODE*)(((CELL)(node)->next) & -8))
#define TYPE(node)								(((CELL)node->next) & 7)

#define AS(type, node)						((NODE*)(((CELL)(node)) | type))
NODE* LINK(NODE* node, NODE* link) { 
	node->next = AS(TYPE(node), link); 
	return node; 
}

enum Types { ATOM, ARRAY, LIST, PRIM, FLOW, WORD, IWORD, CUSTOM };
enum ATypes	{ CELLS, BYTES, STRING };

#define IS_WORD(word)					(TYPE(word) == WORD || TYPE(word) == IWORD)

CELL length(NODE* list, CELL dir) {
	CELL count = 0;
	if (dir) while (list) { count++; list = list->ref; }
	else while (list) { count++; list = NEXT(list); }
	return count;
}

NODE* reverse(NODE* list, NODE* acc) {
	if (list) {
		NODE* tail = NEXT(list);
		LINK(list, acc);
		return reverse(tail, list);
	} else {
		return acc;
	}
}

#define ALIGN(addr, bound)				((((CELL)addr) + (bound - 1)) & ~(bound - 1))

NODE* init_free(BYTE* block, CELL size, CELL start) {
	NODE* node;

	NODE* there = (NODE*)ALIGN((block + start), sizeof(NODE));
	NODE* top = (NODE*)ALIGN((block + size - sizeof(NODE) - 1), sizeof(NODE));

	for (node = there; node <= top; node ++) {
		node->next = node == there ? 0 : node - 1;
		node->ref = node == top ? 0 : node + 1;
	}

	return top;
}

NODE* ncons(NODE** free, CELL n, NODE* next) {
	NODE** pred = free;
	NODE* node = *free;
	while (node) {
		CELL t = n;
		while (--t) {
			if (node->ref == (node + 1) || node->ref == 0) {
				node = node->next;
			} else {
				break;
			}
		}
		if (t == 0) {
			*pred = NEXT(node);
			(*pred)->ref = 0;
			node->next = next;
			node->size = n - 1;
			node->length = 0;
			return node;
		}
		pred = &node;
		node = node->next;
	}
}

NODE* cons(NODE** free, CELL value, NODE* next) {
	NODE* node;

	if (*free == 0) return 0;

	node = ncons(free, 1, next);
	if (node) {
		node->value = value;
	}

	return node;
}

NODE* reclaim(NODE** free, NODE* node) {
	CELL nodes = 1;
	NODE* tail = NEXT(node);

	if (TYPE(node) == LIST) {
		while (node->ref) {
			node->ref = reclaim(free, node->ref);
		}
	}

	if (TYPE(node) == ARRAY || TYPE(node) == FLOW) {
		nodes = node->size + 1;
	}

	while (nodes) {
		node->next = *free;
		node->ref = 0;
		(*free)->ref = node;
		*free = node;
		node++;
		nodes--;
	}

	return tail;
}

NODE* clone(NODE** free, NODE* node, CELL follow) {
	CELL i;
	if (!node) return 0;
	if (TYPE(node) == LIST) {
		return cons(free, (CELL)clone(free, node->ref, 1), AS(LIST, follow ? clone(free, NEXT(node), 1) : 0));
	} else {
		if (TYPE(node) == ARRAY || TYPE(node) == FLOW) {
			NODE* n = ncons(free, node->size + 1, AS(TYPE(node), follow ? clone(free, NEXT(node), 1) : 0));
			n->type = node->type;
			n->length = node->length;
			for (i = 0; i < (2*node->size); i++) {
				n->data[i] = node->data[i];
			}
			return n;
		} else {
			return cons(free, node->value, AS(TYPE(node), follow ? clone(free, NEXT(node), 1) : 0));	
		}
	}
}

// CONTEXT

typedef struct {
	CELL size, err, compiling, ipos;
	NODE* fstack;
	NODE* mstack, * rstack, * nstack;
	NODE** dstack;
	BYTE* here, * ibuf;
	NODE* there;
	NODE* ip;
} CTX;

#define S(ctx)										(*(ctx->dstack))

typedef void (*FUNC)(CTX*);

#define BOTTOM(ctx)								(((BYTE*)ctx) + sizeof(CTX))

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;

	ctx->size = size;

	ctx->fstack = init_free(block, size, sizeof(CTX));
	ctx->mstack = cons(&ctx->fstack, 0, AS(LIST, 0));
	ctx->rstack = ctx->nstack = 0;
	ctx->dstack = &ctx->mstack->ref;

	ctx->here = BOTTOM(ctx);
	ctx->there = (NODE*)ALIGN(ctx->here, sizeof(NODE));

	ctx->err = ctx->compiling = 0;

	ctx->ip = 0;

	return ctx;
}

// ERRORS

#define ERR_STACK_OVERFLOW				-1
#define ERR_STACK_UNDERFLOW				-2
#define ERR_DIVISION_BY_ZERO			-3
#define ERR_EXPECTED_LIST					-4
#define ERR_NO_INPUT_SOURCE				-5
#define ERR_ZERO_LENGTH_NAME			-6
#define ERR_UNDEFINED_WORD				-7
#define ERR_END_OF_INPUT_SOURCE		-8

void error(CTX* ctx) {
	// TODO: Find in return stack error handlers and call them if same type as error
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

#define OF1(ctx)			ERR(ctx, ctx->fstack == ctx->there, ERR_STACK_OVERFLOW)
#define OF2(ctx)			ERR(ctx, ctx->fstack == ctx->there || NEXT(ctx->fstack) == ctx->there, ERR_STACK_OVERFLOW)
#define OF3(ctx)			ERR(ctx, ctx->fstack == ctx->there || NEXT(ctx->fstack) == ctx->there || NEXT(NEXT(ctx->fstack)) == ctx->there, ERR_STACK_OVERFLOW)

#define UF1(ctx)			ERR(ctx, S(ctx) == 0, ERR_STACK_UNDERFLOW)
#define UF2(ctx)			ERR(ctx, S(ctx) == 0 || NEXT(S(ctx)) == 0, ERR_STACK_UNDERFLOW)
#define UF3(ctx)			ERR(ctx, S(ctx) == 0 || NEXT(S(ctx)) == 0 || NEXT(NEXT(S(ctx))) == 0, ERR_STACK_UNDERFLOW)

#define DO(ctx, primitive)		({ primitive(ctx); if (ctx->err < 0) { return; }; })

// STACK PRIMITIVES

void duplicate(CTX* ctx) { /* ( n -- n n ) */
	UF1(ctx);
	S(ctx) = LINK(clone(&ctx->fstack, S(ctx), 0), S(ctx));
}

void swap(CTX* ctx) {	/* ( n2 n1 -- n1 n2 ) */
	UF2(ctx);
	NODE* temp = NEXT(S(ctx));
	LINK(S(ctx), NEXT(NEXT(S(ctx))));
	LINK(temp, S(ctx));
	S(ctx) = temp;
}

void drop(CTX* ctx) { /* ( n -- ) */
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void over(CTX* ctx) { /* ( n2 n1 -- n2 n1 n2 ) */
	UF2(ctx); OF1(ctx);
	S(ctx) = LINK(clone(&ctx->fstack, NEXT(S(ctx)), 0), S(ctx));
}

void rot(CTX* ctx) { /* ( n3 n2 n1 -- n1 n3 n2 ) */
	UF3(ctx);
	NODE* temp = NEXT(NEXT(S(ctx)));
	LINK(NEXT(S(ctx)), NEXT(NEXT(NEXT(S(ctx)))));
	LINK(temp, S(ctx));
	S(ctx) = temp;
}

// ARITHMETIC PRIMITIVES

void add(CTX* ctx) {	/* ( n2 n1 -- n:(n2 + n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value + S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void incr(CTX* ctx) {	/* ( n -- n:(n + 1) ) */
	UF1(ctx);
	S(ctx)->value++;
}

void sub(CTX* ctx) {	/* ( n2 n1 -- n:(n2 - n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value - S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void decr(CTX* ctx) {	/* ( n -- n:(n - 1) ) */
	UF1(ctx);
	S(ctx)->value--;
}

void mul(CTX* ctx) {	/* ( n2 n1 -- n:(n2 * n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value * S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void division(CTX* ctx) {	/* ( n2 n1 -- n:(n2 / n1) ) */
	UF2(ctx); ERR(ctx, S(ctx)->value == 0, ERR_DIVISION_BY_ZERO);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value / S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void mod(CTX* ctx) {	/* ( n2 n1 -- n:(n2 mod n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value % S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

// COMPARISON PRIMITIVES

void gt(CTX* ctx) {	/* ( n2 n1 -- n:(n2 > n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value > S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void lt(CTX* ctx) { /* ( n2 n1 -- n:(n2 < n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value < S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void eq(CTX* ctx) { /* ( n2 n1 -- n:(n2 = n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value == S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void neq(CTX* ctx) { /* ( n2 n1 -- n:(n2 <> n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value != S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

// BIT PRIMITIVES

void and(CTX* ctx) { /* ( n2 n1 -- n:(n2 and n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value & S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void or(CTX* ctx) { /* ( n2 n1 -- n:(n2 or n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value | S(ctx)->value;
	S(ctx) = reclaim(&ctx->fstack, S(ctx));
}

void invert(CTX* ctx) { /* ( n -- n:(inverted bits) ) */
	UF1(ctx);
	S(ctx)->value = ~(S(ctx)->value);
}

// LIST PRIMITIVES

void reverse_stack(CTX* ctx) { /* ( a b c -- c b a ) */
	UF1(ctx);
	S(ctx) = reverse(S(ctx), 0);
}

void stack(CTX* ctx) {
	ctx->mstack = cons(&ctx->fstack, 0, AS(LIST, ctx->mstack));
	ctx->dstack = &ctx->mstack->ref;
}

void unstack(CTX* ctx) {
	if (length(ctx->mstack, 0) > 1) {
		NODE* t = ctx->mstack;
		NODE* s = NEXT(ctx->mstack);
		LINK(t, s->ref);
		s->ref = t;
		ctx->mstack = s;
		ctx->dstack = &ctx->mstack->ref;
	} else {
		S(ctx) = cons(&ctx->fstack, (CELL)S(ctx), AS(LIST, 0));
	}
}

// WORDS

BYTE* NFA(NODE* word) {
	if (!word) return 0;
	else {
		if (TYPE(word->ref) == ATOM) return (BYTE*)(word->ref->value);
		else if (TYPE(word->ref) == ARRAY) return (BYTE*)(word->ref->data);
	}
}

//#define NFA(word)							((BYTE*)REF(word)->value)
#define XT(word)							(NEXT(word->ref))

#define IS_HEADER(word)				(TYPE(word) == WORD && XT(word) == 0)
#define IS_PRIMITIVE(word)		(TYPE(word) == WORD && length(XT(word), 0) == 1 && TYPE(XT(word)) == PRIM)
#define IS_IMMEDIATE(word)		(TYPE(word) == IWORD)

#define ADD_PRIMITIVE(ctx, name, func, type) \
	ctx->nstack = \
		cons(&ctx->fstack, (CELL) \
			cons(&ctx->fstack, (CELL)name, AS(ATOM, \
			cons(&ctx->fstack, (CELL)func, AS(PRIM, 0)))), \
		AS(type, ctx->nstack));

// INNER INTERPRETER

void incrIP(CTX* ctx) {
	if (ctx->ip) {
		ctx->ip = NEXT(ctx->ip);
	}
	while (!ctx->ip && ctx->rstack) {
		if (TYPE(ctx->rstack) == PRIM) {
			ctx->ip = ctx->rstack->ref;
		}
		ctx->rstack = reclaim(&ctx->fstack, ctx->rstack);
	}
}

#define CALL(ctx, dest, ret) \
	({ \
		if (ret) { \
			ctx->rstack = cons(&ctx->fstack, (CELL)ret, AS(PRIM, ctx->rstack)); \
		} \
		ctx->ip = dest; \
	})

NODE* step(CTX* ctx) {
	NODE* r;
	if (ctx->ip) {
		switch (TYPE(ctx->ip)) {
			case ATOM:
				S(ctx) = cons(&ctx->fstack, ctx->ip->value, AS(ATOM, S(ctx)));
				incrIP(ctx);
				break;
			case LIST:
				S(ctx) = LINK(clone(&ctx->fstack, ctx->ip, 0), S(ctx));
				incrIP(ctx);
				break;
			case PRIM:
				r = ctx->ip;
				((FUNC)(ctx->ip->value))(ctx);
				if (r == ctx->ip || ctx->ip == 0) {
					incrIP(ctx);
				}
				break;
			case WORD:
			case IWORD:
				CALL(ctx, XT(ctx->ip->ref), NEXT(ctx->ip));
				break;
		}
	}
	return ctx->ip;
}

void exec_i(CTX* ctx) { /* ( xt -- ) */
	CELL p;
	NODE* n;
	UF1(ctx);
	switch (TYPE(S(ctx))) {
		case ATOM: 
			// This executes the ATOM to be compatible with Forth, is it correct?
			p = S(ctx)->value; 
			S(ctx) = reclaim(&ctx->fstack, S(ctx)); 
			((FUNC)p)(ctx);
			break;
		case LIST: 
			if (S(ctx)->ref) {
				NODE* t = S(ctx);
				S(ctx) = NEXT(S(ctx));
				CALL(ctx, t->ref, NEXT(ctx->ip));
				LINK(t, ctx->rstack);
				ctx->rstack = t;
				step(ctx);
			} else {
				S(ctx) = reclaim(&ctx->fstack, S(ctx));
			}
			break;
		case PRIM: 
			p = S(ctx)->value; 
			S(ctx) = reclaim(&ctx->fstack, S(ctx)); 
			((FUNC)p)(ctx); 
			break;
		case WORD:
		case IWORD:
			n = ctx->ip == 0 ? 0 : NEXT(ctx->ip);
			CALL(ctx, XT(S(ctx)->ref), n);
			S(ctx) = reclaim(&ctx->fstack, S(ctx));
			while (step(ctx) != n);
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
	if (NEXT(NEXT(S(ctx)))->value == 0) { 
		swap(ctx); 
	} 
	drop(ctx); 
	swap(ctx); 
	drop(ctx); 
	exec_i(ctx); 
}

// 

NODE* find_primitive(CTX* ctx, CELL addr) {
	NODE* word = ctx->nstack;
	while (word) {
		if (IS_PRIMITIVE(word) && XT(word)->value == addr) {
			return word;
		}
		word = NEXT(word);
	}
	return 0;
}

BYTE* print(BYTE* str, NODE* node, CELL follow, CTX* ctx) {
	NODE* word;

	if (!node) return str;

	switch (TYPE(node)) {
		case ATOM: 
			sprintf(str, "%s#%ld ", str, node->value); 
			break;
		case PRIM: 
			if (ctx != 0) {
				if ((word = find_primitive(ctx, node->value)) != 0) {
					sprintf(str, "%sP:%s ", str, NFA(word));
					break;
				}
			}
			sprintf(str, "%sP:%ld ", str, node->value); 
			break;
		case LIST: 
			sprintf(str, "%s{ ", str); 
			print(str, node->ref, 1, ctx); 
			sprintf(str, "%s} ", str); 
			break;
		case WORD:
			sprintf(str, "%sW:%s ", str, NFA(node->ref));
			break;
		case ARRAY:
			switch (node->type) {
				case CELLS:
					break;
				case BYTES:
					break;
				case STRING:
					sprintf(str, "%s\"%.*s\" ", str, node->length, (BYTE*)(node->data));
					break;
			}
	}

	if (follow && NEXT(node)) print(str, NEXT(node), 1, ctx);

	return str;
}

void parse_name(CTX* ctx) {
	ERR(ctx, ctx->ibuf == 0, ERR_NO_INPUT_SOURCE);
	OF2(ctx);
	while(*(ctx->ibuf + ctx->ipos) && isspace(*(ctx->ibuf + ctx->ipos))) {
		ctx->ipos++;
	}
	S(ctx) = cons(&ctx->fstack, (CELL)(ctx->ibuf + ctx->ipos), AS(ATOM, S(ctx)));
	while(*(ctx->ibuf + ctx->ipos) && !isspace(*(ctx->ibuf + ctx->ipos))) {
		ctx->ipos++;
	}
	S(ctx) = cons(&ctx->fstack, (CELL)((ctx->ibuf + ctx->ipos) - S(ctx)->value), AS(ATOM, S(ctx)));
}

void to_str(CTX* ctx) {
	UF2(ctx);

	CELL len = S(ctx)->value;
	BYTE* addr = (BYTE*)NEXT(S(ctx))->ref;
	S(ctx) = reclaim(&ctx->fstack, reclaim(&ctx->fstack, S(ctx)));

	CELL size = ((len + 1) / sizeof(NODE)) + (((len + 1) % sizeof(NODE)) == 0 ? 0 : 1);
	S(ctx) = ncons(&ctx->fstack, size + 1, AS(ARRAY, S(ctx)));
	S(ctx)->type = STRING;
	S(ctx)->size = size;
	S(ctx)->length = len;
	memcpy((BYTE*)S(ctx)->data, addr, len);
	((BYTE*)S(ctx)->data)[len] = 0;
}

void find_name(CTX* ctx) {
	CELL len;
	BYTE* addr;

	UF1(ctx);
	//ERR(ctx, S(ctx)->value == 0, ERR_ZERO_LENGTH_NAME);

	if (TYPE(S(ctx)) == ATOM) {
		UF2(ctx);
		len = S(ctx)->value;
		addr = (BYTE*)(NEXT(S(ctx))->ref);
		S(ctx) = reclaim(&ctx->fstack, S(ctx));
	} else if (TYPE(S(ctx)) == ARRAY) {
		addr = (BYTE*)(S(ctx)->data);
		len = strlen(addr);
		//S(ctx) = reclaim(&ctx->fstack, S(ctx));
		printf("Finding word: %.*s\n", len, addr);
	}

	NODE* t = S(ctx);
	S(ctx) = NEXT(S(ctx));
	LINK(t, 0);

	ERR(ctx, len == 0, ERR_ZERO_LENGTH_NAME);
	//CELL len = S(ctx)->value;
	//BYTE *addr = (BYTE*)(NEXT(S(ctx))->value);
	//S(ctx) = reclaim(&ctx->fstack, reclaim(&ctx->fstack, S(ctx)));

	printf("ctx->nstack: %ld\n", ctx->nstack);

	NODE* word = ctx->nstack;
	while (word 
	&& !(strlen(NFA(word)) == len 
	&& !strncmp(NFA(word), addr, len))) {
		word = NEXT(word);
	}

	if (word) {
		S(ctx) = cons(&ctx->fstack, (CELL)word, AS(TYPE(word), S(ctx)));
	} else {
		char* endptr;
		intmax_t n = strtoimax((char*)addr, &endptr, 0);
		ERR(ctx, n == 0 && endptr == (char*)addr, ERR_UNDEFINED_WORD);
		S(ctx) = cons(&ctx->fstack, n, AS(ATOM, S(ctx)));
	}

	reclaim(&ctx->fstack, t);
}

void lbracket(CTX* ctx) {
	ctx->compiling = 0;
}

void rbracket(CTX* ctx) {
	ctx->compiling = 1;
}

void postpone(CTX* ctx) {
	DO(ctx, parse_name);
	DO(ctx, find_name);
}

void compile(CTX* ctx) {
	if (IS_PRIMITIVE(S(ctx)->ref)) {
		NODE* word = S(ctx)->ref;
		S(ctx)->next = AS(PRIM, NEXT(S(ctx)));
		S(ctx)->value = XT(word)->value;
	}
}

void eval(CTX* ctx, BYTE* str) {
	ctx->ibuf = str;
	ctx->ipos = 0;
	while (!ctx->err) {
		DO(ctx, parse_name);
		ERR(ctx, S(ctx)->value == 0, ERR_END_OF_INPUT_SOURCE; drop(ctx); drop(ctx));
		DO(ctx, to_str);
		DO(ctx, find_name);
		if (IS_WORD(S(ctx))) {
			if (!ctx->compiling || TYPE(S(ctx)) == IWORD) {
				exec_i(ctx);
			} else {
				compile(ctx);
			}
		}
	}
}

// STRINGS

void string_literal(CTX* ctx) {
	CELL len = 0;
	CELL pos = ++ctx->ipos;
	while(*(ctx->ibuf + pos) && *(ctx->ibuf + pos) != '"') {
		len++;
		pos++;
	}
	CELL size = (len / sizeof(NODE)) + ((len % sizeof(NODE)) == 0 ? 0 : 1);
	S(ctx) = ncons(&ctx->fstack, size + 1, AS(ARRAY, S(ctx)));
	S(ctx)->type = STRING;
	S(ctx)->size = size;
	S(ctx)->length = len;
	memcpy(S(ctx)->data, ctx->ibuf + ctx->ipos, len);
	ctx->ipos = pos + 1;
}

void words(CTX* ctx) {
	NODE* w = ctx->nstack;
	while (w) {
		printf("%s ", NFA(w));
		w = NEXT(w);
	}
	printf("\n");
}

// BOOTSTRAPING

CTX* bootstrap(CTX* ctx) {
	ADD_PRIMITIVE(ctx, "dup", &duplicate, WORD);
	ADD_PRIMITIVE(ctx, "swap", &swap, WORD);
	ADD_PRIMITIVE(ctx, "drop", &drop, WORD);
	ADD_PRIMITIVE(ctx, "over", &over, WORD);
	ADD_PRIMITIVE(ctx, "rot", &rot, WORD);

	ADD_PRIMITIVE(ctx, "+", &add, WORD);
	ADD_PRIMITIVE(ctx, "1+", &incr, WORD);
	ADD_PRIMITIVE(ctx, "-", &sub, WORD);
	ADD_PRIMITIVE(ctx, "1-", &decr, WORD);
	ADD_PRIMITIVE(ctx, "*", &mul, WORD);
	ADD_PRIMITIVE(ctx, "/", &division, WORD);
	ADD_PRIMITIVE(ctx, "%", &mod, WORD);

	ADD_PRIMITIVE(ctx, ">", &gt, WORD);
	ADD_PRIMITIVE(ctx, "<", &lt, WORD);
	ADD_PRIMITIVE(ctx, "=", &eq, WORD);
	ADD_PRIMITIVE(ctx, "<>", &neq, WORD);

	ADD_PRIMITIVE(ctx, "and", &and, WORD);
	ADD_PRIMITIVE(ctx, "or", &or, WORD);
	ADD_PRIMITIVE(ctx, "invert", &invert, WORD);

	ADD_PRIMITIVE(ctx, "reverse", &reverse_stack, WORD);
	ADD_PRIMITIVE(ctx, "stack", &stack, WORD);
	ADD_PRIMITIVE(ctx, "unstack", &unstack, WORD);

	ADD_PRIMITIVE(ctx, "i", &exec_i, WORD);
	ADD_PRIMITIVE(ctx, "x", &exec_x, WORD);
	ADD_PRIMITIVE(ctx, "branch", &branch, WORD);

	ADD_PRIMITIVE(ctx, "]", &rbracket, WORD);
	ADD_PRIMITIVE(ctx, "[", &lbracket, IWORD);
	ADD_PRIMITIVE(ctx, "postpone", &postpone, IWORD);

	ADD_PRIMITIVE(ctx, "parse-name", &parse_name, WORD);
	ADD_PRIMITIVE(ctx, ">str", &to_str, WORD);
	ADD_PRIMITIVE(ctx, "\"", &string_literal, WORD);

	ADD_PRIMITIVE(ctx, "words", &words, WORD);

	return ctx;
}

#endif
