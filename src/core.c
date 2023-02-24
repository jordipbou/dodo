#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include"core.h"

// -------------------------
C find_prim(X* x, C xt) {
	C word = x->d;
	while (word && A(XT(word)) != xt) { 
		word = N(word); }
	return word;
}

void dump_list(X* x, C pair, C dir) { //, C order) {
	C word;
	if (pair) {
		dump_list(x, N(pair), dir);
		switch (T(pair)) {
			case ATOM: printf("%ld ", A(pair)); break;
			case LIST: printf("{ "); dump_list(x, A(pair), 1); printf("} "); break;
			case PRIM: 
				word = find_prim(x, A(pair));
				if (word) {
					printf("%s ", (B*)(NFA(word)));
				} else {
					printf("PRIM_NOT_FOUND ");
				}
				break;
			case WORD: printf("%s ", NFA(A(pair))); break;
		}
		if (!dir) printf("\n");
	}
}

C dump_stack(X* x) {
	printf("\n");
	dump_list(x, S(x), 0);

	return 0;
}
// ----------------------------

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}

void main(int argc, char *argv[]) {
	C size = 4096;
	B block[size];
	X* x = bootstrap(init(block, size));

	ADD_PRIMITIVE(x, ".s", &dump_stack, 0);

	FILE *fptr;
	B buf[255];
	C result;
	if (argc == 2) {
		fptr = fopen(argv[1], "r");
		while (fgets(buf, 255, fptr)) {
			result = evaluate(x, strlwr(buf));
			if (result != 0) { 
				switch (result) {
					case -1: printf("Stack overflow\n"); break;
					case -2: printf("Stack underflow\n"); break;
					//case -3: printf("Undefined word: %.*s\n", (int)(x->in - x->token), x->tib + x->token); break;
					//case -4: printf("Not enough memory\n"); break;
					//case -5: printf("Zero length name\n"); break;
					case -3: printf("Atom expected\n"); break;
					//case -7: printf("Return s underflow\n"); break;
					//case -8: break;
					default: printf("ERROR: %ld\n", result); break;
				}
				printf("TIB: %s\n", x->tib + x->token);
				return;
			}
		}
	} else {
		do {
			printf("IN: ");
			fgets(buf, 255, stdin);
			result = evaluate(x, buf);
			if (result != 0) { 
				switch (result) {
					case -1: printf("Stack overflow\n"); break;
					case -2: printf("Stack underflow\n"); break;
					case -3: printf("Undefined word: %.*s\n", (int)(x->in - x->token), x->tib + x->token); break;
					case -4: printf("Not enough memory\n"); break;
					case -5: printf("Zero length name\n"); break;
					case -6: printf("Atom expected\n"); break;
					case -7: printf("Return s underflow\n"); break;
					case -8: break;
					default: printf("ERROR: %ld\n", result); break;
				}
				printf("TIB: %s\n", x->tib + x->token);
				return;
			}
			dump_stack(x);
		} while(1);
	}
}
