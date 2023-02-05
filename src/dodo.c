#include<stdio.h>
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

	while (x->err != ERR_BYE) {
		scanf("%s", ibuf);
		outer(x, ibuf);
		switch (x->err) {
			case ERR_OVERFLOW: printf("Stack overflow\n"); x->err = 0; break;
			case ERR_UNDERFLOW: printf("Stack underflow\n"); x->err = 0; break;
		}
	}
}
