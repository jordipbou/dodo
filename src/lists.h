#ifndef __DODO_LISTS__
#define __DODO_LISTS__

#include "types.h"

#define CDR(pair)						(*(((CELL*)pair)))
#define NEXT(pair)					(CDR(pair) & -4)
#define TYPE(pair)					(CDR(pair) & 3)

#define CAR(pair)						(*(((CELL*)pair) + 1))
#define REF(pair)						(CAR(pair) & -4)
#define SUBTYPE(pair)				(CAR(pair) & 3)

#define AS(type, next)			((next & -4) | type)
#define LINK(pair, next)		(CDR(pair) = AS(TYPE(pair), next))

CELL length(CELL list) {
	CELL count = 0;
	while (list) {
		count++;
		list = NEXT(list);
	}
	return count;
}

CELL reverse(CELL list, CELL acc) {
	if (list) {
		CELL tail = NEXT(list);
		LINK(list, acc);
		return reverse(tail, list);
	} else {
		return acc;
	}
}

// TODO: depth, equal, disjoint

#endif
