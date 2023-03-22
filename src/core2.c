#include <stdio.h>
#include "core2.h"

void main() {
	C sz = 16384;
	B bk[sz];
	X* x = init(bk, sz);

	R(x) =
		cons(x, 36, AS(ATM,
		cons(x,
			cons(x, (C)&swap, AS(PRM,
			cons(x, (C)&duplicate, AS(PRM,
			cons(x, 1, AS(ATM,
			cons(x, (C)&gt, AS(PRM,
			//cons(x, (C)&when, AS(PRM,
			cons(x,
				cons(x, 1, AS(ATM,
				cons(x, (C)&sub, AS(PRM,
				cons(x, (C)&duplicate, AS(PRM,
				cons(x, 1, AS(ATM,
				cons(x, (C)&sub, AS(PRM,
				cons(x, (C)&rot, AS(PRM,
				cons(x, (C)&exec_x, AS(PRM,
				cons(x, (C)&rot, AS(PRM,
				cons(x, (C)&rot, AS(PRM,
				cons(x, (C)&exec_x, AS(PRM,
				cons(x, (C)&rot, AS(PRM,
				cons(x, (C)&add, AS(PRM, 0)))))))))))))))))))))))),
			AS(LST,
			cons(x, 0, AS(LST,
			cons(x, (C)&branch, AS(PRM, 0)))))))))))))),
			//AS(LST, 0)))))))))))),
		AS (LST,
		cons(x, (C)&exec_x, AS(PRM,
		cons(x, (C)&swap, AS(PRM,
		cons(x, (C)&drop, AS(PRM, 0))))))))));

	inner(x);

	printf("<%ld> %ld\n", length(S(x)), A(S(x)));
}
