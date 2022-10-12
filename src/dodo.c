#include "vm.h"

#include <stdio.h>		// printf
#include <stdlib.h>		// malloc

void main() {
	cell_t b[32];

	if (!init(b, 32)) {
		printf ("Bad block size\n");
	} else {
		debug_block(b);
	}

	pair_t *p = take(b);
	p->car = 13;
	p->cdr = 17;
	printf("Taken %p from free list\n", p);

	int err = shrink_up(b, 3);
	if (err == -1 ) {
		printf ("HERE is not at last free pair, is not possible to grow continuous region\n");
	} else if (err == -2) {
		printf ("There are not enough contiguous pairs to grow continuous region\n");
	} else {
		debug_block(b);
	}

	put(b, p);
	debug_block(b);
}
