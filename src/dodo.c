#include<stdio.h>
#include "dodo.h"

void main() {
	C size = 8192;
	B block[size];
	X* x = init(block, size);

	push(x, 36);

	// recursive fibonacci
	C code =
		PRIMITIVE(x, &_dup,
		ATOM(x, 1,
		PRIMITIVE(x, &_gt,
		BRANCH(x,
			// True
			ATOM(x, 1,
			PRIMITIVE(x, &_sub,
			PRIMITIVE(x, &_dup,
			ATOM(x, 1,
			PRIMITIVE(x, &_sub,
			RECURSION(x,
			PRIMITIVE(x, &_swap,
			RECURSION(x,
			PRIMITIVE(x, &_add, 0))))))))),
			// False
			0,
		0))));

	inner(x, code);

	printf("%ld\n", pop(x));

	// tail call optimized recursive fibonacci
	C loop = 
		PRIMITIVE(x, &_rot,
		PRIMITIVE(x, &_dup,
		ATOM(x, 0,
		PRIMITIVE(x, &_gt,
		BRANCH(x,
			ATOM(x, 1,
			PRIMITIVE(x, &_sub,
			PRIMITIVE(x, &_rot,
			PRIMITIVE(x, &_rot,
			PRIMITIVE(x, &_swap,
			PRIMITIVE(x, &_over,
			PRIMITIVE(x, &_add,
			RECURSION(x, 0)))))))),
			0,
			0)))));
	code = ATOM(x, 36, ATOM(x, 0, ATOM(x, 1, WORD(x, loop, PRIMITIVE(x, &_drop, PRIMITIVE(x, &_drop, 0))))));
	
	inner(x, code);

	printf("%ld\n", pop(x));
}
