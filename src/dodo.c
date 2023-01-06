#include<stdio.h>
#include "dodo.h"

void main() {
	CELL size = 8192;
	BYTE block[size];
	CTX* ctx = init(block, size);

	// Initialize required functions
	// dup 1 > if 1- dup 1- recurse swap recurse + then
	// dup 1 > [ 1- dup 1- recurse swap recurse + ] [ ] if
}
