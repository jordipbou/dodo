// TODO: Remove STR from compile, as it not needed!! That's something belonging
// to dictionary, no to C/ASM interoperability.

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

typedef int8_t B;
typedef int32_t H;
typedef int64_t C;

typedef struct _CTX CTX;
typedef void (*F)(CTX*);

typedef struct _CTX {
	C dsize, csize;	// Data size and Code size segments
	C chere;				// Relative address to free space on code segment
	F* Fx;					// Address of function to call from C
	C Lx;						// Index register and Literal register for C/ASM communication
	C TOS, SOS;			// Top of stack and Second of the stack registers
	B* code;				// Start of executable Code space
	B data[];				// Start of non-executable Data space
} CTX;

CTX* init(C dsize, C csize) {
#if __linux__
	C PAGESIZE = sysconf(_SC_PAGESIZE);
#elif _WIN32
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	C PAGESIZE = si.dwPageSize;
#endif

	csize = (csize + (PAGESIZE - 1)) & ~(PAGESIZE - 1);

	CTX* ctx = malloc(dsize);
	if (!ctx) return NULL;

	ctx->dsize = dsize;
	ctx->csize = csize;
	ctx->chere = 0;
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

// Compilation to code space

B* unprotect(CTX* ctx) {
#if __linux__
	if (!mprotect(ctx->code, ctx->csize, PROT_WRITE)) return ctx->code + ctx->chere;
	else return NULL; 
#elif _WIN32
	DWORD oldprot;
	if (VirtualProtect(ctx->code, ctx->csize, PAGE_READWRITE, &oldprot)) return ctx->code + ctx->chere;
	else return NULL;
#endif
}

B* protect(CTX* ctx) {
#if __linux__
	if (!mprotect(ctx->code, ctx->csize, PROT_READ|PROT_EXEC)) return ctx->code + ctx->chere;
	else return NULL;
#elif _WIN32
	DWORD oldprot;
	if (VirtualProtect(ctx->code, ctx->csize, PAGE_EXECUTE_READ, &oldprot)) return ctx->code + ctx->chere;
	else return NULL;
#endif
}

// TODO: protect and unprotect are called too much when compiling. It should be
// possible to start compile state and end compilation state.

B* compile_byte(CTX* ctx, B byte) {
	// TODO: Error checking for writing outside code block !!
	if (unprotect(ctx)) {
		*(ctx->code + ctx->chere) = byte;
		if (protect(ctx)) {
			ctx->chere++;
			return ctx->code + ctx->chere;
		}
	}
	return NULL;
}

B* compile_bytes(CTX* ctx, B* bytes, C len) {
	// TODO: Error checking for writing outside code block !!
	if (unprotect(ctx)) {
		memcpy(ctx->code + ctx->chere, bytes, len);
		if (protect(ctx)) {
			ctx->chere += len;
			return ctx->code + ctx->chere;
		}
	}
	return NULL;
}

B* compile_cell(CTX* ctx, C lit) {
	// TODO: Error checking for writing outside code block !!
	if (unprotect(ctx)) {
		*((C*)(ctx->code + ctx->chere)) = lit;
		if (protect(ctx)) {
			ctx->chere += sizeof(C);
			return ctx->code + ctx->chere;
		}
	}
	return NULL;
}

B* compile_halfcell(CTX* ctx, H lit) {
	// TODO: Error checking for writing outside code block !!
	if (unprotect(ctx)) {
		*((H*)(ctx->code + ctx->chere)) = lit;
		if (protect(ctx)) {
			ctx->chere += sizeof(H);
			return ctx->code + ctx->chere;
		}
	}
	return NULL;
}

B* compile_next(CTX* ctx) {
	// lea rax, [rcx + chere relative addressing of instruction after ret]
	// ret
	if (!compile_bytes(ctx, "\x48\x8D\x81", 3)) return NULL;
	if (!compile_halfcell(ctx, (H)(ctx->chere + 5))) return NULL;
	if (!compile_byte(ctx, 0xC3)) return NULL;
	return ctx->code + ctx->chere;
}

B* compile_cfunc(CTX* ctx, F func) {
	// movabs r10, <C Func address>
	// mov QWORD PTR [rdx + 32], r10
	if (!compile_bytes(ctx, "\x49\xBA", 2)) return NULL;
	if (!compile_cell(ctx, (C)func)) return NULL;
	if (!compile_bytes(ctx, "\x4C\x89\x52", 3)) return NULL;
	if (!compile_byte(ctx, offsetof(CTX, Fx))) return NULL;
	// lea rax, [rcx + chere relative addressing of instruction after ret]
	// ret
	if (!compile_next(ctx)) return NULL;
	return ctx->code + ctx->chere;
}

B* compile_push(CTX* ctx, F func, C lit) {
	// mov r10, <PUSH function address>
	// mov QWORD PTR [rdx + 32], r10
	if (!compile_bytes(ctx, "\x49\xBA", 2)) return NULL;
	if (!compile_cell(ctx, (C)func)) return NULL;
	if (!compile_bytes(ctx, "\x4C\x89\x52", 3)) return NULL;
	if (!compile_byte(ctx, offsetof(CTX, Fx))) return NULL;
	// mov r10, <64 bit literal>
	// mov QWORD PTR [rdx + 40], r10
	if (!compile_bytes(ctx, "\x49\xBA", 2)) return NULL;
	if (!compile_cell(ctx, lit)) return NULL;
	if (!compile_bytes(ctx, "\x4C\x89\x52", 3)) return NULL;
	if (!compile_byte(ctx, offsetof(CTX, Lx))) return NULL;
	// lea rax, [rcx + chere relative addressing of instruction after ret]
	// ret
	if (!compile_next(ctx)) return NULL;
	return ctx->code + ctx->chere;
}

#ifdef __linux__
#define CALL(f, ctx)	((B* (*)(void*, void*, CTX*, B*))(f))(NULL, NULL, ctx, ctx->code)
#elif _WIN32
#define CALL(f, ctx)	((B* (*)(B*, CTX*))(f))(ctx->code, ctx)
#endif
