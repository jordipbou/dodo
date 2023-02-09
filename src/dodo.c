#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<inttypes.h>
#include<errno.h>
#include"dodo.h"

void main() {
	C size = 16384;
	B block[size];
	X* x = bootstrap(init(block, size));

	B ibuf[255];
	B* tok;

	while (1) {
		fgets(ibuf, 255, stdin);
		outer(x, ibuf);
		switch (x->err) {
			case 0: printf("ok\n"); break;
			case ERR_OVERFLOW: printf("Stack overflow\n"); x->err = 0; break;
			case ERR_UNDERFLOW: printf("Stack underflow\n"); x->err = 0; break;
			case ERR_UNDEFINED_WORD: printf("Undefined word\n"); x->err = 0; break;
			case ERR_BYE: exit(0); break;
		}
	}
}
