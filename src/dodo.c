#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

int hello(H* bl) {
	printf("Hello world!\n");
}

int hello2(H* bl) {
	printf("Hello world again\n");
}

void main() {
	printf("sizeof C:%d\n", sizeof(C));
	printf("sizeof S:%d\n", sizeof(S));

	C b[262000];
	H* bl = init(b, 262000);
	F t[32];
	dump_dict(bl);
	assert(add_cfunc(bl, t, 0, &hello, "hello", 5) == 0);
	dump_dict(bl);
	assert(add_cfunc(bl, t, 1, &hello2, "hello2", 6) == 0);
	dump_dict(bl);
	//t[0] = &hello;
	//t[1] = &hello2;

	//B* c = (B*)(b + szHEADER);
	//*(c + 0) = 'c';
	//*(c + 1) = 0;
	//*(c + 2) = 'c';
	//*(c + 3) = 1;
	//*(c + 4) = '1';
	//*(c + 5) = '1';
	//*(c + 6) = '+';
	//*(c + 7) = ';';

	//eval(bl, t);
	//dump_stack(bl);
}
