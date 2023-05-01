#ifndef DODO_TYPES
#define DODO_TYPES

typedef char BYTE;
typedef intptr_t CELL;

typedef struct NODE_T { 
	CELL next; 
	CELL value; 
} NODE;

typedef enum {
	ATOM,
	LIST,
	PRIM
} TYPE;

CELL as(TYPE type, NODE* node) { 
	return (((CELL)(node)) | (type)); 
}

NODE* next(NODE* node) { 
	return ((NODE*)(node->next & -8)); 
}

TYPE type(NODE* node) { 
	return ((TYPE)(node->next & 7)); 
}

NODE* link(NODE* node, NODE* next) { 
	node->next = as(T(node), next); 
	return node; 
}

NODE* set_value(NODE* node, CELL value) { 
	node->value = value; 
	return node; 
}

NODE* set_next(NODE* node, CELL next) { 
	node->next = next; 
	return node; 
}



#endif
