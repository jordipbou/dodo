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
}
