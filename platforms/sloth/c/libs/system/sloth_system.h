#ifndef SLOTH_SYSTEM
#define SLOTH_SYSTEM

#include <sloth.h>

/* DODO system-access words.
 *
 * SYSTEM ( c-addr u -- n ) runs a command through the host shell
 * (/bin/sh -c on POSIX, cmd /C on Windows) and returns its exit
 * status n, or -1 if the command could not be spawned. This is not
 * an ANS word (ANS SYSTEM returns nothing, implementation-defined),
 * so it lives in DODO rather than Sloth.
 *
 * OPEN-DIR ( c-addr u -- dirid ior ),
 * READ-DIR ( c-addr u1 dirid -- u2 flag ior ) and
 * CLOSE-DIR ( dirid -- ior ) are the directory counterparts of the
 * ANS file words. Like those, they never throw: they report through
 * ior, using -37 for a file/directory I/O error. The dirid is an
 * opaque handle (see sloth_system.c for its representation).
 *
 * Contract -- MUST be called only after ANS Forth (ans.4th) has been
 * loaded. The guard is the dictionary itself, so the call is
 * idempotent and per-VM, exactly like dodo_bootstrap_common. */

void dodo_system_(X* x);
void dodo_open_dir_(X* x);
void dodo_read_dir_(X* x);
void dodo_close_dir_(X* x);

void dodo_bootstrap_system(X* x);

#endif
