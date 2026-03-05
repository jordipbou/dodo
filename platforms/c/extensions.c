#include "extensions.h"

#define SLOTH_CODE(w, f) sloth_code(x, w, sloth_primitive(x, &sloth_##f##_));

/* Memory-stack transfer operations */

void sloth_b_fetch_(X* x) { 
	BYTE* addr = (BYTE*)sloth_pop(x);
	sloth_push(x, *addr);
}
void sloth_b_store_(X* x) { 
	BYTE* addr = (BYTE*)sloth_pop(x);
	BYTE v = (BYTE)sloth_pop(x);
	*addr = v;
}

void sloth_w_fetch_(X* x) {
	WYDE* addr = (WYDE*)sloth_pop(x);
	sloth_push(x, *addr);
}
void sloth_w_store_(X* x) { 
	WYDE* addr = (WYDE*)sloth_pop(x);
	WYDE v = (WYDE)sloth_pop(x);
	*addr = v;
}

void sloth_l_fetch_(X* x) {
	LONG* addr = (LONG*)sloth_pop(x);
	sloth_push(x, *addr);
}
void sloth_l_store_(X* x) {
	LONG* addr = (LONG*)sloth_pop(x);
	LONG v = (LONG)sloth_pop(x);
	*addr = v;
}

void sloth_x_fetch_(X* x) { 
	EXTENDED* addr = (EXTENDED*)sloth_pop(x);
	sloth_push(x, *addr);
}
void sloth_x_store_(X* x) {
	EXTENDED* addr = (EXTENDED*)sloth_pop(x);
	EXTENDED v = (EXTENDED)sloth_pop(x);
	*addr = v;
}

/* Allows creation of the INTFIELDS word that allows creating */
/* structures that use C int field sizes for compatibility */
/* with C libraries. */
void sloth_ints_(X* x) {
	sloth_push(x, sloth_pop(x)*sizeof(int));
}

void sloth_bootstrap_extensions(X* x) {

	/* Memory-stack transfer operations */

	SLOTH_CODE("B@", b_fetch);
	SLOTH_CODE("B!", b_store);
	SLOTH_CODE("W@", w_fetch);
	SLOTH_CODE("W!", w_store);
	SLOTH_CODE("L@", l_fetch);
	SLOTH_CODE("L!", l_store);
	SLOTH_CODE("X@", x_fetch);
	SLOTH_CODE("X!", x_store);

	SLOTH_CODE("INTS", ints);

	if (sizeof(int) == 2) {
		SLOTH_CODE("INT@", w_fetch);
		SLOTH_CODE("INT!", w_store);
	} else if (sizeof(int) == 4) {
		SLOTH_CODE("INT@", l_fetch);
		SLOTH_CODE("INT!", l_store);
	} else if (sizeof(int) == 8) {
		SLOTH_CODE("INT@", x_fetch);
		SLOTH_CODE("INT!", x_store);
	}
}
