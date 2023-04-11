#ifndef __DODO__
#define __DODO__

#include<stdint.h>

typedef int8_t BYTE;
#if INTPTR_MAX == INT64_MAX
	typedef int32_t HALF;
#else
	typedef int16_t HALF;
#endif
typedef intptr_t CELL;

typedef struct T_NODE {
	struct T_NODE* next;
	union {
		CELL value;
		struct T_NODE* ref;
		struct {
			HALF size;		// Always in cells, to ensure correct reclaiming
			HALF length;
		};
	};
	CELL data[];
} NODE;

#define BYTES(node)					((BYTE*)node->data)

#define NEXT(node)					((NODE*)(((CELL)node->next) & -8))
#define TYPE(node)					(((CELL)node->next) & 7)

#define REF(node)						((NODE*)(((CELL)node->ref) & -8))
#define SUBTYPE(node)				(((CELL)node->ref) & 7)

#define AS(type, node)			((NODE*)(((CELL)node) | type))
#define LINK(node, _next_)	(node->next = AS(TYPE(node), _next_))

enum Types { ATOM, PRIM, PLIT, STRING, BARRAY, ARRAY, COLL };
enum CollTypes { LIST, WORD, DUAL, USER };

#define IS_ARRAY(node)			(TYPE(node) == PLIT || TYPE(node) == STRING || TYPE(node) == BARRAY || TYPE(node) == ARRAY)

CELL length(NODE* list) {
	CELL count = 0;
	while (list) {
		count++;
		list = NEXT(list);
	}
	return count;
}

#define ALIGN(addr, bound)	((((CELL)addr) + (bound - 1)) & ~(bound - 1))

NODE* init_free(BYTE* block, CELL size, CELL start) {
	NODE* node;

	NODE* there = (NODE*)ALIGN((block + start), 2*sizeof(CELL));
	NODE* top = (NODE*)ALIGN((block + size - 2*sizeof(CELL) - 1), 2*sizeof(CELL));

	for (node = there; node <= top; node ++) {
		node->next = node == there ? 0 : node - 1;
		node->ref = node == top ? 0 : node + 1;
	}

	return top;
}

NODE* cons(NODE** free, CELL n, NODE* next) {
	NODE* pred = (NODE*)free;
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
			pred->next = node->next;
			node->next->ref = pred == (NODE*)free ? 0 : pred;
			node->next = next;
			return node;
		}
		node = node->next;
	}
}

#define CONS1(f, v, x)			({ NODE* _n_ = cons(f, 1, x); _n_->value = v; _n_; })
#define CONS2(f, d1, d2, x)	({ NODE* _n_ = cons(f, 2, x); _n_->size = 1; _n_->data[0] = d1; _n_->data[1] = d2; _n_; })
#define CONSn(f, n, x)			({ NODE* _n_ = cons(f, n, x); _n_->size = n - 1; _n_; })

NODE* reclaim(NODE** free, NODE* node) {
	CELL nodes = 1;
	NODE* tail = NEXT(node);

	if (TYPE(node) == COLL && (SUBTYPE(node) == LIST || SUBTYPE(node) == USER)) {
		while (node->ref) {
			node->ref = AS(SUBTYPE(node), reclaim(free, node->ref));
		}
	}

	if (IS_ARRAY(node)) {
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

NODE* clone(NODE** free, NODE* node) {
	CELL i;
	if (!node) return 0;
	if (TYPE(node) == COLL && (SUBTYPE(node) == LIST || SUBTYPE(node) == USER)) {
		return CONS1(free, (CELL)AS(SUBTYPE(node), clone(free, REF(node))), AS(COLL, clone(free, NEXT(node))));
	} else {
		if (IS_ARRAY(node)) {
			NODE* n = CONSn(free, node->size + 1, AS(TYPE(node), clone(free, NEXT(node))));
			for (i = 0; i < (2*node->size); i++) {
				n->data[i] = node->data[i];
			}
			return n;
		} else {
			return CONS1(free, node->value, AS(TYPE(node), clone(free, NEXT(node))));	
		}
	}
}

typedef struct {
	CELL size;
	HALF err, state;
	BYTE* here;
	NODE* there;
	// TODO: I don't understand if dstack should be like this....
	NODE* fstack, * mstack, ** dstack, * rstack, * xstack;
	NODE* ip, * latest;
} CTX;

typedef CTX* (*FUNC)(CTX*);

#define BOTTOM(ctx)					(((BYTE*)ctx) + sizeof(CTX))
#define TOP(ctx)						(ALIGN(((BYTE*)ctx) + ctx->size - sizeof(NODE) - 1, sizeof(NODE)))
#define RESERVED(ctx)				((CELL)(((BYTE*)ctx->there) - ctx->here))

CTX* init(BYTE* block, CELL size) {
	CTX* ctx = (CTX*)block;	

	ctx->size = size;
	ctx->here = BOTTOM(ctx);
	ctx->there = (NODE*)ALIGN(BOTTOM(ctx), sizeof(NODE));
	ctx->mstack = ctx->rstack = ctx->xstack = 0;
	ctx->dstack = &ctx->mstack;

	ctx->fstack = init_free(block, size, ((CELL)ctx->there) - ((CELL)ctx));

	ctx->err = ctx->state = 0;

	return ctx;
}

#define F(ctx)		(ctx->fstack)
#define S(ctx)		(*ctx->dstack)
#define R(ctx)		(ctx->rstack)

#define ERR_STACK_OVERFLOW			-1
#define ERR_STACK_UNDERFLOW			-2
#define ERR_DIVISION_BY_ZERO		-3

// STACK PRIMITIVES

CTX* duplicate(CTX* ctx) { /* ( n -- n n ) */
	// TODO: Error management !!
	if (TYPE(S(ctx)) == COLL && (SUBTYPE(S(ctx)) == LIST || SUBTYPE(S(ctx)) == USER)) {
		S(ctx) = CONS1(&F(ctx), (CELL)AS(SUBTYPE(S(ctx)), clone(&F(ctx), S(ctx)->ref)), AS(COLL, S(ctx)));
	} else {
		S(ctx) = CONS1(&F(ctx), S(ctx)->value, AS(TYPE(S(ctx)), S(ctx)));
	}
}

//CTX* swap(CTX* ctx) {	/* ( n2 n1 -- n1 n2 ) */
//	CELL temp = NEXT(S(ctx));
//	LINK(TOS(ctx), NEXT(NEXT(TOS(ctx))));
//	LINK(temp, TOS(ctx));
//	TOS(ctx) = temp;
//}
//
//CTX* drop(CTX* ctx) { /* ( n -- ) */
//	TOS(ctx) = reclaim(ctx, TOS(ctx));
//}
//
//CTX* over(CTX* ctx) { /* ( n2 n1 -- n2 n1 n2 ) */
//	if (TYPE(NEXT(TOS(ctx))) == LIST) {
//		TOS(ctx) = cons(ctx, clone(ctx, CAR(NEXT(TOS(ctx)))), AS(LIST, TOS(ctx)));
//	} else {
//		TOS(ctx) = cons(ctx, CAR(NEXT(TOS(ctx))), AS(TYPE(NEXT(TOS(ctx))), TOS(ctx)));
//	}
//}
//
//CTX* rot(CTX* ctx) { /* ( n3 n2 n1 -- n1 n3 n2 ) */
//	CELL temp = NEXT(NEXT(TOS(ctx)));
//	LINK(NEXT(TOS(ctx)), NEXT(NEXT(NEXT(TOS(ctx)))));
//	LINK(temp, TOS(ctx));
//	TOS(ctx) = temp;
//}

#endif
