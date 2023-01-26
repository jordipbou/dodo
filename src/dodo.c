#include<stdio.h>
#include "dodo.h"

void main() {
	C size = 8192;
	B block[size];
	X* x = init(block, size);

	push(x, 36);

	C xlist =
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
	
	//inner(x, xlist, xlist);
	inner(x, xlist);

	printf("%ld\n", T(x));
}
