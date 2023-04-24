#include<stdio.h>
#include"dodo.h"

void fib(CTX* x) {
	x->ip =
		cons(x, 30, AS(ATOM,
		cons(x, (CELL)
			cons(x, (CELL)&swap, AS(PRIM,
			cons(x, (CELL)&duplicate, AS(PRIM,
			cons(x, 1, AS(ATOM,
			cons(x, (CELL)&gt, AS(PRIM,
			cons(x, (CELL)
				cons(x, 1, AS(ATOM,
				cons(x, (CELL)&sub, AS(PRIM,
				cons(x, (CELL)&duplicate, AS(PRIM,
				cons(x, 1, AS(ATOM,
				cons(x, (CELL)&sub, AS(PRIM,
				cons(x, (CELL)&rot, AS(PRIM,
				cons(x, (CELL)&exec_x, AS(PRIM,
				cons(x, (CELL)&rot, AS(PRIM,
				cons(x, (CELL)&rot, AS(PRIM,
				cons(x, (CELL)&exec_x, AS(PRIM,
				cons(x, (CELL)&rot, AS(PRIM,
				cons(x, (CELL)&add, AS(PRIM, 0)))))))))))))))))))))))),
			AS(LIST,
			cons(x, 0, AS(LIST,
			cons(x, (CELL)&branch, AS(PRIM, 0)))))))))))))),
		AS(LIST,
		cons(x, (CELL)&exec_x, AS(PRIM,
		cons(x, (CELL)&swap, AS(PRIM,
		cons(x, (CELL)&drop, AS(PRIM, 0))))))))));

	while(step(x));

	printf("%ld\n", S(x)->val);
}

char* dump_stack(CTX* x, char* str, NODE* n) {
	if (N(n)) dump_stack(x, str, N(n));
	sprintf(str, "%s\n", print(str, n, 0, x));
	return str;		
}

int main(int argc, char *argv[]) {
	CELL sz = 120000;
	BYTE bk[sz];
	CTX* x = bootstrap(init(bk, sz));
		
	/*fib(x);*/

	FILE *fptr;
	char buf[255];
	char str[255];
	if (argc == 2 || argc == 3) {
		fptr = fopen(argv[1], "r");
		while (fgets(buf, 255, fptr)) {
			eval(x, (BYTE*)buf);
			if (x->err != 0 && x->err != -2) {
					printf("ERROR: %ld\n", x->err);
					return;
			}
			x->err = 0;
		}
	}

	if (argc == 1 || argc == 3) {
		do {
			printf("%c [%ld] ", x->compiling ? 'C' : 'I', FREE(x));
			fgets(buf, 255, stdin);
			eval(x, (BYTE*)buf);
			if (x->err != 0 && x->err != -2) {
					printf("ERROR: %ld\n", x->err);
					return;
			}
			if (S(x)) {
				str[0] = 0;
				printf("\n--- Data Stack\n");
				printf("%s", dump_stack(x, str, S(x)));
			}
			x->err = 0;
		} while(1);
	}

	printf("%ld\n", S(x)->val);

	return 0;
}
