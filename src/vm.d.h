#include<stdio.h>
#include "vm.h"

// Here we have things that are not totally necessary for the VM, or that
// depend on printf

void dump_stack(H* bl) {
	printf("<%ld> ", length(bl->sp));
	for (P* p = bl->sp; p != NULL; p = p->cdr) { printf("%ld ", p->car); }
	printf("\n");
}

void dump_string(S* s) {
	printf("<%ld>%s\n", s->len, s->data.str);
}

void dump_dict(H* bl) {
	printf("Dictionary length: %ld\n", length((P*)(bl->dp)));
	W* w = (W*)bl->dp;
	while (w != NULL) {
		if (w->source) {
			printf(": %s %s ;\n", w->name->data.str, w->source->data.str);
		} else {
			printf(": %s ;\n", w->name->data.str);
		}
		//if (IS(w, PRIMITIVE)) {	printf("PRIMITIVE "); }
		//if (IS(w, CFUNC)) {	printf("CFUNC %d ", *(w->name.str + bytes(cells(w->name.len)) + 3)); }
		w = (W*)w->cdr;
		//printf("\n");
	}
}
