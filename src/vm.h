// TODO: Adapt this code to work on Windows also

#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<sys/mman.h>
#include<unistd.h>

typedef int8_t B;
typedef int32_t H;
typedef int64_t C;

typedef struct {
	C len;
	union {
		B* code;
		B str[sizeof(C)];
	} data;
} STR;

typedef struct {
	C dsize, csize;	// Data size and Code size segments
	C dhere, chere;	// Indexes to free space on data segment and on code segment
	C Ix, Lx;				// Index register and Literal register for C/ASM communication
	B* code;				// Start of executable Code space
	B data[];				// Start of non-executable Data space
} CTX;

#define PAGESIZE	sysconf(_SC_PAGESIZE)

CTX* init(int dsize, int csize) {
	csize = (csize + (PAGESIZE - 1)) & ~(PAGESIZE - 1);

	CTX* ctx = malloc(dsize);
	if (!ctx) return NULL;

	ctx->dsize = dsize;
	ctx->csize = csize;
	ctx->dhere = sizeof(CTX);
	ctx->chere = 0;
	ctx->Ix = ctx->Lx = 0;

	ctx->code = mmap(NULL, csize, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (ctx->code == (void *)-1) {
		free(ctx);
		return NULL;
	}

	return ctx;
}

void deinit(CTX* ctx) {
	munmap(ctx->code, ctx->csize);
	free(ctx);
}

// Compilation to code space

B* unprotect(CTX* ctx) {
	if (!mprotect(ctx->code, ctx->csize, PROT_WRITE)) return ctx->code + ctx->chere;
	else return NULL; 
}

B* protect(CTX* ctx) {
	if (!mprotect(ctx->code, ctx->csize, PROT_READ|PROT_EXEC)) return ctx->code + ctx->chere;
	else return NULL;
}

B* compile_byte(CTX* ctx, B byte, STR* str) {
	// TODO: Error checking for writing outside code block !!
	if (unprotect(ctx)) {
		*(ctx->code + ctx->chere) = byte;
		if (protect(ctx)) {
			if (str != NULL) str->len++;
			ctx->chere++;
			return ctx->code + ctx->chere;
		}
	}
	return NULL;
}

B* compile_bytes(CTX* ctx, B* bytes, C len, STR* str) {
	// TODO: Error checking for writing outside code block !!
	if (unprotect(ctx)) {
		memcpy(ctx->code + ctx->chere, bytes, len);
		if (protect(ctx)) {
			if (str != NULL) str->len += len;
			ctx->chere += len;
			return ctx->code + ctx->chere;
		}
	}
	return NULL;
}

B* compile_literal(CTX* ctx, H lit, STR* str) {
	// TODO: Error checking for writing outside code block !!
	if (unprotect(ctx)) {
		*((H*)(ctx->code + ctx->chere)) = lit;
		if (protect(ctx)) {
			if (str != NULL) str->len += sizeof(H);
			ctx->chere += sizeof(H);
			return ctx->code + ctx->chere;
		}
	}
	return NULL;
}

B* compile_cfunc(CTX* ctx, H idx, H lit, STR* str) {
	// mov DWORD PTR [rdi + 32], idx
	// mov DWORD PTR [rdi + 40], lit
	// mov rax, chere + 20
	// ret
	if (!compile_bytes(ctx, "\xC7\x42\x20", 3, str)) return NULL;
	if (!compile_literal(ctx, idx, str)) return NULL;
	if (!compile_bytes(ctx, "\xC7\x42\x28", 3, str)) return NULL;
	if (!compile_literal(ctx, lit, str)) return NULL;
	if (!compile_byte(ctx, 0xB8, str)) return NULL;
	if (!compile_literal(ctx, ((H)(ctx->chere) + 5), str)) return NULL;
	if (!compile_byte(ctx, 0xC3, str)) return NULL;
}

#ifdef __linux__
#define CALL(f, ctx)	((C (*)(void*, void*, CTX*))f)(NULL, NULL, ctx)
#elif _WIN32
#define CALL(f, ctx)	((C (*)(void*, CTX*))f)(NULL, ctx)
#endif
