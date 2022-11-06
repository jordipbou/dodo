#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "vm.d.h"

int hello(H* bl) {
	printf("Hello world!\n");
}

int hello2(H* bl) {
	printf("Hello world again\n");
}

void main() {
	printf("Sizeof S: %ld\n", sizeof(S));

	//C* b = mmap(NULL, 65536 * sizeof(C), PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
	////C b[262000];

	//assert(b != NULL);


	//H* bl = init(b, 262000);
	//F t[32];

	//assert(add_cfunc(bl, t, 15, &hello, "hello", 5) == 0);

	//S* s = malloc(sizeof(S));
	//s->len = 5;
	//s->str[0] = 'h'; s->str[1] = 'e'; s->str[2] = 'l';
	//s->str[3] = 'l'; s->str[4] = 'o'; s->str[6] = 0;

	//call(bl, t, s);
}
