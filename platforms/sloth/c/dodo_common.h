#ifndef DODO_COMMON
#define DODO_COMMON

#include <sloth.h>

/* DODO common C-interop words.
 *
 * These words expose the host C `int` type, whose size is
 * implementation-defined. They are only meaningful when interfacing
 * with C code, so they do not belong to Sloth: a non-C platform (for
 * example the Java implementation) has no use for them. They live here
 * instead, as the base shared by every DODO library that accesses C
 * structs containing `int` fields.
 *
 * INT@ / INT! fetch and store a single host C `int`. INTS scales by
 * sizeof(int), and INTALIGNED / INTFIELD: build struct fields of the
 * exact C `int` size, which is what a C library ABI requires.
 *
 * Contract -- MUST be called only after ANS Forth (ans.4th) has been
 * loaded. The guard is the dictionary itself, so it is per-VM: this
 * makes the call idempotent, so several libraries can share it without
 * registering the words twice, and it keeps separate VMs independent.
 * (A file-scope `static` flag would not work: it lives per translation
 * unit, so two libraries including this header would each register.) */

static inline void sloth_ints_(X* x) {
	if (!sloth__check_data_stack(x, 1, 1)) return;
	sloth_push(x, sloth_pop(x)*sizeof(int));
}

static inline void sloth_int_fetch_(X* x) {
	CELL a;
	if (!sloth__check_data_stack(x, 1, 1)) return;
	a = sloth_pop(x);
	sloth_push(x, (CELL)*((int*)a));
}

static inline void sloth_int_store_(X* x) {
	CELL a;
	int v;
	if (!sloth__check_data_stack(x, 2, 0)) return;
	a = sloth_pop(x);
	v = (int)sloth_pop(x);
	*((int*)a) = v;
}

static inline void dodo_bootstrap_common(X* x) {
	if (sloth_find_word(x, "INT@")) return;

	sloth_code(x, "INTS", sloth_primitive(x, &sloth_ints_));
	sloth_code(x, "INT@", sloth_primitive(x, &sloth_int_fetch_));
	sloth_code(x, "INT!", sloth_primitive(x, &sloth_int_store_));

	sloth_evaluate(x,
		": INTALIGNED ( addr -- a-addr ) "
		"  1 INTS + 1- 1 INTS 1- INVERT AND ; "
		": INTFIELD: ( n1 \"name\" -- n2 ; addr1 -- addr2 ) "
		"  INTALIGNED 1 INTS +FIELD ; ");
}

#endif
