#include<stdlib.h>
#include<string.h>
#include<stddef.h>

#ifdef __linux__
#include<sys/mman.h>
#include<unistd.h>
#elif _WIN32
#include<windows.h>
#include<memoryapi.h>
#endif

// DATATYPES ------------------------------------------------------------------

#ifdef __linux__
typedef int8_t BYTE;
#endif
typedef int16_t QUARTER;
typedef int32_t HALF;
typedef int64_t CELL;

typedef struct _CTX CTX;
typedef void (*FUNC)(CTX*);

typedef struct _CTX {
	CELL dsize, csize;	// Data size and Code size segments
	BYTE* chere;				// Address to free space on code segment
	FUNC* Fx;						// Address of function to call from C 
	CELL Lx;						// Literal value register
	BYTE* code;					// Start of executable Code space
	BYTE data[];				// Start of non-executable Data space
} CTX;

// CONTEXT CREATION AND DESTRUCTION -------------------------------------------

#define ALIGN(addr, bound)	((CELL)csize + ((CELL)bound-1)) & ~((CELL)bound-1)

CTX* init(CELL dsize, CELL csize) {
#if __linux__
	CELL PAGESIZE = sysconf(_SC_PAGESIZE);
#elif _WIN32
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	CELL PAGESIZE = si.dwPageSize;
#endif

	csize = ALIGN(csize, PAGESIZE);

	CTX* ctx = malloc(dsize);
	if (!ctx) return NULL;

	ctx->dsize = dsize;
	ctx->csize = csize;
	ctx->Fx = NULL;
	ctx->Lx = 0;

#if __linux__
	ctx->code = mmap(NULL, csize, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (ctx->code == (void *)-1) {
		free(ctx);
		return NULL;
	}
#elif _WIN32
	ctx->code = VirtualAlloc(NULL, csize, MEM_COMMIT, PAGE_EXECUTE_READ);
	if (!ctx->code) {
		free(ctx);
		return NULL;
	}
#endif

	ctx->chere = ctx->code;

	return ctx;
}

void deinit(CTX* ctx) {
#if __linux__
	munmap(ctx->code, ctx->csize);
#elif _WIN32
	VirtualFree(ctx->code, 0, MEM_RELEASE);
#endif
	free(ctx);
}

// COMPILATION ----------------------------------------------------------------

// Modifying compilation state

BYTE* unprotect(CTX* ctx) {
#if __linux__
	if (!mprotect(ctx->code, ctx->csize, PROT_WRITE)) 
		return ctx->chere;
	else 
		return NULL; 
#elif _WIN32
	DWORD oldprot;
	if (VirtualProtect(ctx->code, ctx->csize, PAGE_READWRITE, &oldprot)) 
		return ctx->chere;
	else 
		return NULL;
#endif
}

BYTE* protect(CTX* ctx, BYTE* old_chere) {
#if __linux__
	if (!mprotect(ctx->code, ctx->csize, PROT_READ|PROT_EXEC)) 
		return ctx->chere;
	else {
		ctx->chere = old_chere;
		return NULL;
	}
#elif _WIN32
	DWORD oldprot;
	if (VirtualProtect(ctx->code, ctx->csize, PAGE_EXECUTE_READ, &oldprot)) 
		return ctx->chere;
	else {
		ctx->chere = old_chere;
		return NULL;
	}
#endif
}

// Compile helpers

#define compile_lit(ctx, type, lit)\
	{ *((type*)(ctx->chere)) = ((type)(lit)); ctx->chere += sizeof(type); }

void compile_bytes(CTX* ctx, BYTE* bytes, CELL len) {
	memcpy(ctx->chere, bytes, len);
	ctx->chere += len;
}

void compile_next(CTX* ctx) {
	// 0:  48 8d 81 <H:Address after ret>	lea    rax,[rcx + <address after ret>]
	// 7:  c3															ret
	// 8 bytes
	compile_bytes(ctx, "\x48\x8D\x81", 3);
	compile_lit(ctx, HALF, ctx->chere - ctx->code + 5);
	compile_lit(ctx, BYTE, 0xC3);
}

void compile_reg(CTX* ctx, CELL lit, BYTE offset) {
	// 0:  49 ba <C:Address of cfunc>			movabs r10,0xff00ff11ff22ff33
	// a:  4c 89 52 <B:Fx offset>					mov    QWORD PTR [rdx+<Fx offset>],r10
	// 14 bytes
	compile_bytes(ctx, "\x49\xBA", 2);
	compile_lit(ctx, CELL, lit);
	compile_bytes(ctx, "\x4C\x89\x52", 3);
	compile_lit(ctx, BYTE, offset);
}

#define compile_cfunc(ctx, cfunc)	compile_reg(ctx, (CELL)cfunc, offsetof(CTX, Fx))
#define compile_push(ctx, lit)		compile_reg(ctx, lit, offsetof(CTX, Lx))

// RDX: Pointer to context
// RCX: Pointer to code space
// R8: Top of the stack
// R9: Second of the stack
#ifdef __linux__
#define CALL(f, ctx)\
	((BYTE* (*)(void*, void*, CTX*, BYTE*, void*, void*))(f))\
		(NULL, NULL, ctx, ctx->code, NULL, NULL)
#elif _WIN32
#define CALL(f, ctx)\
	((BYTE* (*)(BYTE*, CTX*, void*, void*))(f))\
		(ctx->code, ctx, NULL, NULL)
#endif
