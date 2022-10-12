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

// TMP: Debug
void debug_block(cell_t *cells) {
	pair_t *pairs = (pair_t *)cells;

	printf("HEADER ----------------------------------------------- (%p)\n", cells);
	printf("[Size: %ld] [First free: %p] [Last free: %p] [ ]\n", BLOCK_CELLS_SIZE(cells), (pair_t *)FIRST_FREE_PAIR(cells), (pair_t *)LAST_FREE_PAIR(cells));
	printf("CONTIGUOUS REGION ------------------------------------ (%p)\n", &pairs[HEADER_SIZE]);
	printf("[HERE: %p]\n", (cell_t *)HERE(cells));
	printf("MANAGED REGION---------------------------------------- (%p)\n", (pair_t *)LAST_FREE_PAIR(cells));
	for (cell_t i = HEADER_SIZE; i < BLOCK_PAIRS_SIZE(cells); i++) {
		printf("[%p] FREE PAIR: PREV %p NEXT %p\n", (pair_t *)&pairs[i], (pair_t *)pairs[i].car, (pair_t *)pairs[i].cdr);
	}
	printf("END OF MEMORY----------------------------------------- (%p)\n", cells + BLOCK_CELLS_SIZE(cells));
}
