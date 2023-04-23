#ifndef __DODO_CORE__
#define __DODO_CORE__

#include <stdint.h>

typedef int8_t BYTE;
typedef intptr_t CELL;

typedef struct T_NODE {
	struct T_NODE* next;
	union {
		CELL value;
		struct T_NODE* ref;
		CELL* cells;
		BYTE* bytes;
	};
} NODE;

#define NEXT(node)								((NODE*)(((CELL)(node)->next) & -8))
#define TYPE(node)								(((CELL)node->next) & 7)

#define AS(type, next)						((NODE*)(((CELL)(next)) | type))
NODE* LINK(NODE* node, NODE* link) { 
	node->next = AS(TYPE(node), link); 
	return node; 
}

enum Types { ATOM, REF, LIST, IP, PRIM, CODE, WORD, MACRO };
enum RTypes { ARRAY, BARRAY, STRING };

typedef struct {
	CELL type;
	CELL size;
	CELL length;
	CELL data[];
} DATA;

#define REF_DATA(ref)							((DATA*)(((void*)ref) - sizeof(DATA)))

typedef struct {
	CELL size;
	BYTE* here;
	NODE* there;
	NODE* free;
	BYTE* bottom[];
} BLOCK;

typedef struct {
	CELL size, err, compiling, ipos;
	NODE* mstack, * rstack, * nstack;
	NODE** dstack;
	NODE* ip;
	BYTE *ibuf;
	BLOCK* block;
} CTX;

#define S(ctx)	(&ctx->mstack->ref)

CELL length(NODE* list, CELL dir) {
	CELL count = 0;
	if (dir) while (list) { count++; list = list->ref; }
	else while (list) { count++; list = NEXT(list); }
	return count;
}

NODE* reverse(NODE* list, NODE* acc) {
	if (list) {
		NODE* tail = NEXT(list);
		list->next = AS(LIST, acc);
		return reverse(tail, list);
	} else {
		return acc;
	}
}

#define ALIGN(addr, bound)				((((CELL)addr) + (bound - 1)) & ~(bound - 1))
#define FREE(block)								(length(block->free, 0) - 1)

BLOCK* init_block(BYTE* block, CELL size, CELL start) {
	NODE* node;
	BLOCK* b = (BLOCK*)(block + start);

	b->size = size - start;
	b->here = (BYTE*)(((BYTE*)b) + sizeof(BLOCK));
	b->there = (NODE*)ALIGN(b->here, sizeof(NODE));
	b->free = (NODE*)ALIGN(b + (size - start) - sizeof(NODE) - 1, sizeof(NODE));

	for (node = b->there; node <= b->free; node++) {
		node->next = node == b->there ? 0 : node - 1;
		node->ref = node == b->free ? 0 : node + 1;
	}

	return b;
}

NODE* cons(BLOCK* block, CELL value, NODE* next) {
	NODE* node;

	if (block->free == block->there) return 0;

	node = block->free;
	block->free = NEXT(block->free);
	node->next = next;
	node->value = value;
	block->free->ref = 0;

	return node;
}

DATA* array(BLOCK* block, CELL req_size) {
	if (req_size == 0) return 0;
	NODE** pred = &block->free;
	NODE* node = block->free;
	req_size += sizeof(DATA);
	CELL size = req_size / sizeof(NODE) + ((req_size % sizeof(NODE)) == 0 ? 0 : 1);
	CELL t;
	DATA* arr;
	while (node && node != block->there) {
		t = size;
		while (--t && node && node != block->there) {
			if ((node->ref == (node + 1) || node->ref == 0)) {
				node = node->next;
			} else {
				break;
			}
		}
		if (t == 0) {
			*pred = NEXT(node);
			NEXT(node)->ref = *pred == block->free ? 0 : *pred;

			arr = (DATA*)node;	
			arr->type = ARRAY;
			arr->size = size * sizeof(NODE);

			return arr;
		}
		pred = &(node->next);
		node = NEXT(node);
	}
	return 0;
}

DATA* string(BLOCK* block, BYTE* str, CELL len) {
	DATA* data = array(block, len + 1);
	BYTE* dst = (BYTE*)data->data;
	strncpy(dst, str, len);
	dst[len] = 0;
	data->length = len;
	return data;
}

void recl_data(BLOCK* block, DATA* data) {
	if (!data) return;
	CELL nodes = data->size / sizeof(NODE);
	NODE* node;
	while (nodes) {
		node = ((NODE*)data) + --nodes;	
		node->next = block->free;
		node->ref = 0;
		block->free->ref = node;
		block->free = node;
	}
}

NODE* reclaim(BLOCK* block, NODE* node) {
	NODE* tail = NEXT(node);

	if (TYPE(node) == LIST) {
		while (node->ref) {
			node->ref = reclaim(block, node->ref);
		}
	}

	if (TYPE(node) == REF || TYPE(node) == CODE) {
		recl_data(block, REF_DATA(node->ref));
	}

	node->next = block->free;
	node->ref = 0;
	block->free->ref = node;
	block->free = node;

	return tail;
}

NODE* clone(BLOCK* block, NODE* node, CELL follow) {
	CELL i;
	DATA* data;
	if (!node) return 0;
	if (TYPE(node) == LIST) {
		return cons(block, (CELL)clone(block, node->ref, 1), AS(LIST, follow ? clone(block, NEXT(node), 1) : 0));
	} else {
		if (TYPE(node) == REF || TYPE(node) == CODE) {
			data = array(block, REF_DATA(node->ref)->size);
			if (data == 0) return 0;
			memcpy(data, REF_DATA(node->ref), REF_DATA(node->ref)->size);
			return cons(block, (CELL)data, AS(TYPE(node), follow ? clone(block, NEXT(node), 1) : 0));
		} else {
			return cons(block, node->value, AS(TYPE(node), follow ? clone(block, NEXT(node), 1) : 0));	
		}
	}
}

// CONTEXT

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;

	ctx->size = size;

	ctx->block = init_block(block, size, offsetof(CTX, block));

	ctx->mstack = cons(ctx->block, 0, AS(LIST, 0));
	ctx->rstack = ctx->nstack = 0;
	ctx->dstack = &ctx->mstack->ref;

	ctx->err = ctx->compiling = 0;

	ctx->ip = 0;

	return ctx;
}

#define FSTACK(ctx)								(ctx->block->free)
#define BOTTOM(ctx)								(ctx->block->bottom)
#define HERE(ctx)									(ctx->block->here)
#define THERE(ctx)								(ctx->block->there)

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

#define OF1(ctx)			ERR(ctx, FSTACK(ctx) == THERE(ctx), ERR_STACK_OVERFLOW)
#define OF2(ctx)			ERR(ctx, FSTACK(ctx) == THERE(ctx) || NEXT(FSTACK(ctx)) == THERE(ctx), ERR_STACK_OVERFLOW)
#define OF3(ctx)			ERR(ctx, FSTACK(ctx) == THERE(ctx) || NEXT(FSTACK(ctx)) == THERE(ctx) || NEXT(NEXT(FSTACK(ctx))) == THERE(ctx), ERR_STACK_OVERFLOW)

#define UF1(ctx)			ERR(ctx, S(ctx) == 0, ERR_STACK_UNDERFLOW)
#define UF2(ctx)			ERR(ctx, S(ctx) == 0 || NEXT(S(ctx)) == 0, ERR_STACK_UNDERFLOW)
#define UF3(ctx)			ERR(ctx, S(ctx) == 0 || NEXT(S(ctx)) == 0 || NEXT(NEXT(S(ctx))) == 0, ERR_STACK_UNDERFLOW)

#define DO(ctx, primitive)		({ primitive(ctx); if (ctx->err < 0) { return; }; })

// STACK PRIMITIVES

void duplicate(CTX* ctx) { /* ( n -- n n ) */
	UF1(ctx);
	S(ctx) = LINK(clone(ctx->block, S(ctx), 0), S(ctx));
}

void swap(CTX* ctx) {	/* ( n2 n1 -- n1 n2 ) */
	UF2(ctx);
	NODE* temp = NEXT(S(ctx));
	LINK(S(ctx), NEXT(NEXT(S(ctx))));
	LINK(temp, S(ctx));
	S(ctx) = temp;
}

void drop(CTX* ctx) { /* ( n -- ) */
	S(ctx) = reclaim(ctx->block, S(ctx));
}

void over(CTX* ctx) { /* ( n2 n1 -- n2 n1 n2 ) */
	UF2(ctx); OF1(ctx);
	S(ctx) = LINK(clone(ctx->block, NEXT(S(ctx)), 0), S(ctx));
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
	S(ctx) = reclaim(ctx->block, S(ctx));
}

void incr(CTX* ctx) {	/* ( n -- n:(n + 1) ) */
	UF1(ctx);
	S(ctx)->value++;
}

void sub(CTX* ctx) {	/* ( n2 n1 -- n:(n2 - n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value - S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
}

void decr(CTX* ctx) {	/* ( n -- n:(n - 1) ) */
	UF1(ctx);
	S(ctx)->value--;
}

void mul(CTX* ctx) {	/* ( n2 n1 -- n:(n2 * n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value * S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
}

void division(CTX* ctx) {	/* ( n2 n1 -- n:(n2 / n1) ) */
	UF2(ctx); ERR(ctx, S(ctx)->value == 0, ERR_DIVISION_BY_ZERO);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value / S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
}

void mod(CTX* ctx) {	/* ( n2 n1 -- n:(n2 mod n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value % S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
}

// COMPARISON PRIMITIVES

void gt(CTX* ctx) {	/* ( n2 n1 -- n:(n2 > n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value > S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
}

void lt(CTX* ctx) { /* ( n2 n1 -- n:(n2 < n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value < S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
}

void eq(CTX* ctx) { /* ( n2 n1 -- n:(n2 = n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value == S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
}

void neq(CTX* ctx) { /* ( n2 n1 -- n:(n2 <> n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value != S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
}

// BIT PRIMITIVES

void and(CTX* ctx) { /* ( n2 n1 -- n:(n2 and n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value & S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
}

void or(CTX* ctx) { /* ( n2 n1 -- n:(n2 or n1) ) */
	UF2(ctx);
	NEXT(S(ctx))->value = NEXT(S(ctx))->value | S(ctx)->value;
	S(ctx) = reclaim(ctx->block, S(ctx));
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
	ctx->mstack = cons(ctx->block, 0, AS(LIST, ctx->mstack));
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
		S(ctx) = cons(ctx->block, (CELL)S(ctx), AS(LIST, 0));
	}
}

// WORDS

#define NFA(word)							((BYTE*)(word->ref->ref))

#define XT(word)							(NEXT(word->ref))

#define IS_WORD(word)					(TYPE(word) == WORD || TYPE(word) == MACRO)
#define IS_HEADER(word)				(IS_WORD(word) && XT(word) == 0)
#define IS_PRIMITIVE(word)		(IS_WORD(word) && length(XT(word), 0) == 1 && TYPE(XT(word)) == PRIM)
#define IS_IMMEDIATE(word)		(TYPE(word) == MACRO)

#define ADD_PRIMITIVE(ctx, name, func, type) \
	ctx->nstack = \
		cons(ctx->block, (CELL) \
			cons(ctx->block, (CELL)name, AS(ATOM, \
			cons(ctx->block, (CELL)func, AS(PRIM, 0)))), \
		AS(type, ctx->nstack));

// INNER INTERPRETER

void incrIP(CTX* ctx) {
	if (ctx->ip) {
		ctx->ip = NEXT(ctx->ip);
	}
	while (!ctx->ip && ctx->rstack) {
		if (TYPE(ctx->rstack) == IP) {
			ctx->ip = ctx->rstack->ref;
		}
		ctx->rstack = reclaim(ctx->block, ctx->rstack);
	}
}

#define CALL(ctx, dest, ret) \
	({ \
		if (ret) { \
			ctx->rstack = cons(ctx->block, (CELL)ret, AS(IP, ctx->rstack)); \
		} \
		ctx->ip = dest; \
	})

NODE* step(CTX* ctx) {
	NODE* r;
	if (ctx->ip) {
		switch (TYPE(ctx->ip)) {
			case ATOM:
				S(ctx) = cons(ctx->block, ctx->ip->value, AS(ATOM, S(ctx)));
				incrIP(ctx);
				break;
			case LIST:
				S(ctx) = LINK(clone(ctx->block, ctx->ip, 0), S(ctx));
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
			case MACRO:
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
			S(ctx) = reclaim(ctx->block, S(ctx)); 
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
				S(ctx) = reclaim(ctx->block, S(ctx));
			}
			break;
		case PRIM: 
			p = S(ctx)->value; 
			S(ctx) = reclaim(ctx->block, S(ctx)); 
			((FUNC)p)(ctx); 
			break;
		case WORD:
		case MACRO:
			n = ctx->ip == 0 ? 0 : NEXT(ctx->ip);
			CALL(ctx, XT(S(ctx)->ref), n);
			S(ctx) = reclaim(ctx->block, S(ctx));
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

// EVALUATION

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
	case REF:
		switch (REF_DATA(node->ref)->type) {
		case ARRAY: break;
		case BARRAY: break;
		case STRING: 
			sprintf(str, "%s\"%.*s\" ", str, (int)REF_DATA(node->ref)->length, (BYTE*)node->bytes); 
			break;
		}
		break;
	case LIST: 
		sprintf(str, "%s{ ", str); 
		print(str, node->ref, 1, ctx); 
		sprintf(str, "%s} ", str); 
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
	case WORD: 
		sprintf(str, "%sW:%s ", str, NFA(node->ref)); 
		break;
	case MACRO: 
		sprintf(str, "%sM:%s ", str, NFA(node->ref)); 
		break;
	}

	if (follow && NEXT(node)) print(str, NEXT(node), 1, ctx);

	return str;
}

void parse_token(CTX* ctx) {
	ERR(ctx, ctx->ibuf == 0, ERR_NO_INPUT_SOURCE);
	OF2(ctx);
	CELL start, end, i;
	while(*(ctx->ibuf + ctx->ipos) && isspace(*(ctx->ibuf + ctx->ipos))) {
		ctx->ipos++;
	}
	start = ctx->ipos;
	while(*(ctx->ibuf + ctx->ipos) && !isspace(*(ctx->ibuf + ctx->ipos))) {
		ctx->ipos++;
	}
	end = ctx->ipos;
	S(ctx) = cons(ctx->block, (CELL)array(ctx->block, end - start + 1), AS(REF, S(ctx)));
	for (i = 0; i < (end - start); i++) {
		S(ctx)->bytes[i] = (ctx->ibuf + start)[i];
	}
	S(ctx)->bytes[end - start]  = 0;
	REF_DATA(S(ctx)->value)->length = end-start;
}

void compile(CTX* ctx) {
}

#endif
