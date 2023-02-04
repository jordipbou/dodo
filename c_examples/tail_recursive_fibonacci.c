#include<stdio.h>
#include "../src/dodo.h"

void main() {
	C size = 8192;
	B block[size];
	X* x = init(block, size);

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
	C code = 
		ATOM(x, 36, 
		ATOM(x, 0, 
		ATOM(x, 1, 
		LAMBDA(x, loop, 
		PRIMITIVE(x, &_drop, 
		PRIMITIVE(x, &_drop, 0))))));
	
	inner(x, code);

	printf("%ld\n", pop(x));
}
