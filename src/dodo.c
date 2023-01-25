#include<stdio.h>
#include "dodo.h"

void main() {
	C size = 8192;
	B block[size];
	X* x = init(block, size);

	push(x, 36);

	C xlist = 
		cons(x, cons(x, PRIMITIVE, (C)&_dup),
		cons(x, cons(x, ATOM, 1),
		cons(x, cons(x, PRIMITIVE, (C)&_gt),
		cons(x, cons(x, BRANCH, 
			// True branch
			cons(x,
				cons(x, cons(x, ATOM, 1),
				cons(x, cons(x, PRIMITIVE, (C)&_sub),
				cons(x, cons(x, PRIMITIVE, (C)&_dup),
				cons(x, cons(x, ATOM, 1),
				cons(x, cons(x, PRIMITIVE, (C)&_sub),
				cons(x, cons(x, RECURSION, 0),
				cons(x, cons(x, PRIMITIVE, (C)&_swap),
				cons(x, cons(x, RECURSION, 0),
				cons(x, cons(x, PRIMITIVE, (C)&_add), 0))))))))),
			// False branch
			cons(x, 0, 0))), 
		0))));
	
	inner(x, xlist);

	printf("%ld\n", T(x));
}
