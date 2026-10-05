#include "sloth_system.h"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>

#ifndef WINDOWS
#include <sys/wait.h>
#endif

#define DODO_CODE(w, f) sloth_code(x, w, sloth_primitive(x, &dodo_##f##_))

#define DODO_FILE_IO_EXCEPTION -37

/* Directory handle.
 *
 * OPEN-DIR materializes the listing once (SDL_GlobDirectory returns a
 * single NULL-terminated allocation) and READ-DIR walks it one entry per
 * call. The cursor below is the only mutable state.
 *
 * The handle is the address of this struct cast to CELL, the same opaque
 * convention the ANS file words use for their FILE* fileid (file.c:64).
 * Nothing in the Forth API exposes the layout, so a formal opaque handle
 * type can be introduced later without breaking code that treats a dirid
 * as an opaque cell. */
typedef struct dodo_dir {
	char **names;
	int index;
} dodo_dir;

/* Copy a counted string from the dictionary into a fresh NUL-terminated
 * heap buffer of exactly len+1 bytes.
 *
 * DODO deliberately avoids a fixed path/command buffer. Sloth's file words
 * use char buf[512] and memcpy without bounds checking (file.c:51-57), so a
 * long name can overflow, while a legitimate long name can be truncated
 * silently. Sizing from the actual length is cheap and removes both
 * problems; SDL_malloc/SDL_free keep it in the SDL3 dependency DODO
 * already carries. */
static char *dodo_copy_string(char *a, CELL len) {
	char *buf;
	if (len < 0) {
		len = 0;
	}
	buf = (char *)SDL_malloc((size_t)len + 1);
	if (!buf) {
		return 0;
	}
	memcpy(buf, a, (size_t)len);
	buf[len] = 0;
	return buf;
}

/* SYSTEM ( c-addr u -- n ) */
void dodo_system_(X* x) {
	char *cmd;
	CELL a, u;
	int status;

	if (!sloth__check_data_stack(x, 2, 1)) return;
	u = sloth_pop(x);
	a = sloth_pop(x);

	cmd = dodo_copy_string((char *)a, u);
	if (!cmd) {
		sloth_push(x, -1);
		return;
	}

	status = system(cmd);
	SDL_free(cmd);

	if (status == -1) {
		/* Could not spawn the shell. */
		sloth_push(x, -1);
		return;
	}

#ifdef WINDOWS
	/* cmd /C already reports the command's exit status. */
	sloth_push(x, status);
#else
	if (WIFEXITED(status)) {
		sloth_push(x, WEXITSTATUS(status));
	} else if (WIFSIGNALED(status)) {
		/* Shell convention for death by signal. */
		sloth_push(x, 128 + WTERMSIG(status));
	} else {
		sloth_push(x, -1);
	}
#endif
}

/* OPEN-DIR ( c-addr u -- dirid ior ) */
void dodo_open_dir_(X* x) {
	char *path;
	char **names;
	int count;
	CELL a, u;
	dodo_dir *dir;
	SDL_PathInfo info;

	if (!sloth__check_data_stack(x, 2, 2)) return;
	u = sloth_pop(x);
	a = sloth_pop(x);

	path = dodo_copy_string((char *)a, u);
	if (!path) {
		sloth_push(x, 0);
		sloth_push(x, DODO_FILE_IO_EXCEPTION);
		return;
	}

	if (!SDL_GetPathInfo(path, &info)
			|| info.type != SDL_PATHTYPE_DIRECTORY) {
		SDL_free(path);
		sloth_push(x, 0);
		sloth_push(x, DODO_FILE_IO_EXCEPTION);
		return;
	}

	count = 0;
	names = SDL_GlobDirectory(path, 0, 0, &count);
	SDL_free(path);

	dir = (dodo_dir *)SDL_malloc(sizeof(dodo_dir));
	if (!dir) {
		if (names) SDL_free(names);
		sloth_push(x, 0);
		sloth_push(x, DODO_FILE_IO_EXCEPTION);
		return;
	}
	/* A valid but empty directory yields no names. */
	dir->names = names;
	dir->index = 0;

	sloth_push(x, (CELL)dir);
	sloth_push(x, 0);
}

/* READ-DIR ( c-addr u1 dirid -- u2 flag ior ) */
void dodo_read_dir_(X* x) {
	dodo_dir *dir;
	char *name;
	CELL a, u1;
	size_t n, c;

	if (!sloth__check_data_stack(x, 3, 3)) return;
	dir = (dodo_dir *)sloth_pop(x);
	u1 = sloth_pop(x);
	a = sloth_pop(x);

	if (!dir || !dir->names || !dir->names[dir->index]) {
		/* End of directory. */
		sloth_push(x, 0);
		sloth_push(x, 0);
		sloth_push(x, 0);
		return;
	}

	name = dir->names[dir->index];
	dir->index++;

	n = strlen(name);
	if (u1 < 0) {
		u1 = 0;
	}
	c = (n < (size_t)u1) ? n : (size_t)u1;
	memcpy((void *)a, name, c);

	/* READ-LINE semantics: a true flag means an entry was delivered,
	 * even if it did not fit and the remainder was discarded. */
	sloth_push(x, (CELL)c);
	sloth_push(x, 1);
	sloth_push(x, 0);
}

/* CLOSE-DIR ( dirid -- ior ) */
void dodo_close_dir_(X* x) {
	dodo_dir *dir;

	if (!sloth__check_data_stack(x, 1, 1)) return;
	dir = (dodo_dir *)sloth_pop(x);

	if (dir) {
		if (dir->names) SDL_free(dir->names);
		SDL_free(dir);
	}
	sloth_push(x, 0);
}

void dodo_bootstrap_system(X* x) {
	if (sloth_find_word(x, "SYSTEM")) return;

	DODO_CODE("SYSTEM", system);
	DODO_CODE("OPEN-DIR", open_dir);
	DODO_CODE("READ-DIR", read_dir);
	DODO_CODE("CLOSE-DIR", close_dir);
}
