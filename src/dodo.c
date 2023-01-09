#include<stdio.h>
#include "dodo.h"

void main() {
	CELL size = 8192;
	BYTE block[size];
	CTX* ctx = init(block, size);

	PAIR* fib = header(ctx, "fib", 3, NIL);
	printf("fib %p\n", &fib);
	printf("fib nfa: %p dfa: %p cfa: %p\n", (PAIR*)fib->value, NEXT(((PAIR*)fib->value)), NEXT((NEXT(((PAIR*)fib->value)))));
	printf("fib NFA: %p DFA: %p CFA: %p\n", NFA(fib), DFA(fib), CFA(fib));
	body(ctx, fib, 
		pcons(ctx, &_dup,
		ncons(ctx, 1,
		pcons(ctx, &_gt,
		bcons(ctx, 
			ncons(ctx, 1,
			pcons(ctx, &_sub,
			pcons(ctx, &_dup,
			ncons(ctx, 1,
			pcons(ctx, &_sub,
			wcons(ctx, CFA(fib),
			pcons(ctx, &_swap,
			wcons(ctx, CFA(fib),
			pcons(ctx, &_add, NIL))))))))),
				NIL)))));
				
	ctx->dstack = ncons(ctx, 36, NIL);

	inner(ctx, CFA(fib));

	printf("%ld\n", ctx->dstack->value);
}
