#include <stdio.h>

#include "dodo.h"

void main() {
	DODO* dodo = init_dodo();

	_free_pairs(dodo);

	_dump_stack(dodo);

	POP(dodo);
	POP(dodo);

	deinit_dodo(dodo);
}
