#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include"dodo.h"

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
	CELL size = 4096;
	BYTE block[size];
	CTX* ctx = bootstrap(init(block, size));

	FILE *fptr;
	BYTE buf[255];
	CELL result;
	if (argc == 2) {
		fptr = fopen(argv[1], "r");
		while (fgets(buf, 255, fptr)) {
			result = evaluate(ctx, strlwr(buf));
			if (result != 0) { 
				switch (result) {
					case -1: printf("Stack overflow\n"); break;
					case -2: printf("Stack underflow\n"); break;
					case -3: printf("Undefined word: %.*s\n", (int)(ctx->in - ctx->token), ctx->tib + ctx->token); break;
					case -4: printf("Not enough memory\n"); break;
					case -5: printf("Zero length name\n"); break;
					case -6: printf("Atom expected\n"); break;
					case -7: printf("Return stack underflow\n"); break;
					case -8: break;
					default: printf("ERROR: %ld\n", result); break;
				}
				printf("TIB: %s\n", ctx->tib + ctx->token);
				return;
			}
		}
	} else {
		do {
			fgets(buf, 255, stdin);
			result = evaluate(ctx, buf);
			if (result != 0) { 
				switch (result) {
					case -1: printf("Stack overflow\n"); break;
					case -2: printf("Stack underflow\n"); break;
					case -3: printf("Undefined word: %.*s\n", (int)(ctx->in - ctx->token), ctx->tib + ctx->token); break;
					case -4: printf("Not enough memory\n"); break;
					case -5: printf("Zero length name\n"); break;
					case -6: printf("Atom expected\n"); break;
					case -7: printf("Return stack underflow\n"); break;
					case -8: break;
					default: printf("ERROR: %ld\n", result); break;
				}
				printf("TIB: %s\n", ctx->tib + ctx->token);
				return;
			}
		} while(1);
	}
}
