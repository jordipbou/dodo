#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include"core.h"

// -------------------------
C find_prim(X* x, C xt) {
	C word = x->dict;
	while (word && A(XT(word)) != xt) { 
		word = N(word); }
	return word;
}

void dump_list(X* x, C pair, C dir) { //, C order) {
	C word;
	if (pair) {
		if (!dir) dump_list(x, N(pair), dir);
		switch (T(pair)) {
			case ATOM: printf("#%ld ", A(pair)); break;
			case LIST: printf("{ "); dump_list(x, A(pair), 1); printf("} "); break;
			case PRIM: 
				word = find_prim(x, A(pair));
				if (word) {
					printf("P:%s ", (B*)(NFA(word)));
				} else {
					printf("PRIM_NOT_FOUND ");
				}
				break;
			case WORD: printf("W:%s ", NFA(A(pair))); break;
		}
		if (dir) dump_list(x, N(pair), 1);
		if (!dir) printf("\n");
	}
}

C dump_stack(X* x) {
	printf("\n");
	dump_list(x, S(x), 0);

	return 0;
}

C type(X* x) {
	UF2(x);
	C l = A(S(x));
	C a = A(N(S(x)));
	S(x) = rcl(x, rcl(x, S(x)));
	printf("%.*s", l, (B*)a);
	return 0;
}

C words(X* x) {
	C w = x->dict;
	printf("WORDS: ");
	while (w) { printf("%s ", NFA(w)); w = N(w); }
	printf("\n");
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
	ADD_PRIMITIVE(x, "words", &words, 0);
	ADD_PRIMITIVE(x, "type", &type, 0);

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
					//case -3: printf("Undefined word: %.*s\n", (int)(x->in - x->tk), x->ib + x->tk); break;
					//case -4: printf("Not enough memory\n"); break;
					//case -5: printf("Zero lth name\n"); break;
					case -3: printf("Atom expected\n"); break;
					//case -7: printf("Return s underflow\n"); break;
					//case -8: break;
					default: printf("ERROR: %ld\n", result); break;
				}
				//printf("TIB: %s\n", x->ib + x->tk);
				return;
			}
		}
	} else {
		printf("STATE: %ld FREE PAIRS: %ld CONTIGUOUS: %ld TRANSIENT: %ld PILE: %ld\n", x->state, FREE(x), (C)(x->here - BOTTOM(x)), (C)(x->t - (C)x->here), lth(x->p));
		do {
			printf("IN: ");
			fgets(buf, 255, stdin);
			result = evaluate(x, buf);
			if (result != 0) { 
				switch (result) {
					case -1: printf("Stack overflow\n"); break;
					case -2: 
						printf("Stack underflow\n"); 
						printf("STATE: %ld FREE PAIRS: %ld CONTIGUOUS: %ld TRANSIENT: %ld PILE: %ld\n", x->state, FREE(x), (C)(x->here - BOTTOM(x)), (C)(x->t - (C)x->here), lth(x->p));
						printf("TOKEN: %.*s\n", TL(x), TK(x));
						printf("x->p %ld x->s %ld x->o %ld\n", x->p, x->s, x->o);
						printf("lth(S(x)) %ld lth(O(x)) %ld\n", lth(S(x)), lth(O(x)));
						printf("S(x) "); dump_list(x, S(x), 1);
						printf("O(x) "); dump_list(x, O(x), 1);
						break;
					//case -3: printf("Undefined word: %.*s\n", (int)(x->in - x->tk), x->ib + x->tk); break;
					case -4: printf("Not enough memory\n"); break;
					case -5: printf("Zero lth name\n"); break;
					case -6: printf("Atom expected\n"); break;
					case -7: printf("Return s underflow\n"); break;
					case -8: break;
					default: printf("ERROR: %ld\n", result); break;
				}
				//printf("TIB: %s\n", x->ib + x->tk);
				return;
			}
		printf("STATE: %ld FREE PAIRS: %ld CONTIGUOUS: %ld TRANSIENT: %ld PILE: %ld\n", x->state, FREE(x), (C)(x->here - BOTTOM(x)), (C)(x->t - (C)x->here), lth(x->p));
			dump_stack(x);
		} while(1);
	}
}
