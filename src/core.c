#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include"core.h"

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
}
