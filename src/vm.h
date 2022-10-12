#include<stdio.h>
#include<stdint.h>
#include<stddef.h>

typedef int64_t cell_t;
typedef int8_t byte_t;

typedef struct pair_s {
	cell_t car;
	cell_t cdr;
} pair_t;

#define HEADER_SIZE						2
#define BLOCK_CELLS_SIZE(b)		b[0]
#define BLOCK_PAIRS_SIZE(b)		BLOCK_CELLS_SIZE(b) / 2
#define FIRST_FREE_PAIR(b)		b[1]
#define LAST_FREE_PAIR(b)			b[2]
#define HERE(b)								b[3]

int shrink_up(cell_t *cells, cell_t amount) {
	// Ensure last_free_pair is on HERE, if not, it means
	// that its not possible to take more cells.
	if (HERE(cells) != LAST_FREE_PAIR(cells)) return -1;

	// Allocates from HERE removing as much pairs from
	// free list as needed.
	cell_t required_pairs = amount / 2 + amount % 2;

	// Check if there are enough free cells contiguous to
	// remove.
	pair_t *p = (pair_t *)LAST_FREE_PAIR(cells);
	for (cell_t i = 0; i < required_pairs; i++) {
		if ((pair_t *)p->car != p + 1) {
			// There are not enough contiguous free cells starting at HERE
			return -2;
		} else {
			p = (pair_t *)p->car;
		}
	}

	// Remove the required cells
	p->cdr = (cell_t)NULL;
	HERE(cells) += required_pairs * sizeof(pair_t);
	LAST_FREE_PAIR(cells) = (cell_t)p;

	return 0;
}

cell_t *init(cell_t *cells, cell_t size) {
	if (size % 2 != 0) return NULL;

	// Treat block as made of pairs
	pair_t *pairs = (pair_t *)cells;

	// Initialize header
	BLOCK_CELLS_SIZE(cells) = size;								
	FIRST_FREE_PAIR(cells) = (cell_t)(&pairs[size / 2 - 1]);
	LAST_FREE_PAIR(cells) = (cell_t)(&pairs[2]);
	HERE(cells) = (cell_t)(&pairs[2]);

	// Initialize free pairs doubly linked list
	for (cell_t i = HEADER_SIZE; i < BLOCK_PAIRS_SIZE(cells); i++) {
		pairs[i].car = (cell_t)(&pairs[i] + 1);				// PREV
		pairs[i].cdr = (cell_t)(&pairs[i] - 1);				// NEXT
	}

	// Adjust first prev and last next to NULL
	pairs[HEADER_SIZE].cdr = (cell_t)NULL;
	pairs[BLOCK_PAIRS_SIZE(cells) - 1].car = (cell_t)NULL;

	return cells;
}	

// Takes one pair from the beginning of list of free pairs
pair_t *take(cell_t *cells) {
	if ((pair_t *)FIRST_FREE_PAIR(cells) == NULL) return NULL;

	pair_t *pair = (pair_t *)FIRST_FREE_PAIR(cells);
	FIRST_FREE_PAIR(cells) = pair->cdr;
	((pair_t *)FIRST_FREE_PAIR(cells))->car = (cell_t)NULL;

	pair->car = 0;
	pair->cdr = 0;

	return pair;
}

void put(cell_t *cells, pair_t *pair) {
	pair_t *first = (pair_t *)FIRST_FREE_PAIR(cells);
	FIRST_FREE_PAIR(cells) = (cell_t)pair;
	first->car = (cell_t)pair;
	pair->car = (cell_t)NULL;
	pair->cdr = (cell_t)first;
}

// TMP: Debug
void debug_block(cell_t *cells) {
	pair_t *pairs = (pair_t *)cells;

	printf("HEADER ----------------------------------------------- (%p)\n", cells);
	printf("[Size: %ld] [First free: %p] [Last free: %p] [ ]\n", BLOCK_CELLS_SIZE(cells), (pair_t *)FIRST_FREE_PAIR(cells), (pair_t *)LAST_FREE_PAIR(cells));
	printf("CONTIGUOUS REGION ------------------------------------ (%p)\n", &pairs[HEADER_SIZE]);
	printf("MANAGED REGION - HERE: (%p) - LAST FREE CELL: %p\n", (pair_t *)HERE(cells), (pair_t *)LAST_FREE_PAIR(cells));
	for (pair_t *p = (pair_t *)HERE(cells); p < (pair_t *)(cells + BLOCK_CELLS_SIZE(cells)); p++) {
		printf("[%p] FREE PAIR: PREV %p NEXT %p\n", p, (pair_t *)p->car, (pair_t *)p->cdr);
	}
	printf("END OF MEMORY----------------------------------------- (%p)\n", cells + BLOCK_CELLS_SIZE(cells));
}
