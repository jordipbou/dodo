# DODO

DODO offers functionality for working with video, audio, midi, files, input, database, inter-process communication in top of ANS Forth.

DODO is built in two layers, a set of words that acts as the interface to the underlying system and has to be implemented separately for each Forth platform and a cross-platform layer built on top of it that works the same across all platforms.

## Scope

* System (shell, file access)
* Audio
* Video
* MIDI
* Input (keyboard, mouse, touch, game controllers)
* Concurrency and communication (CSP, actors and reactive programming)
* Database (SQLite)

## Target Forths

The primary target platform is [[https://github.com/jordipbou/sloth][SLOTH]] (both C and Java implementations) but it's also planned to work on compatibility with other ANS Forths like GForth, MPEForth, or SwiftForth.

On constrained hosts that only offer a high-level engine (a smartwatch with a JavaScript engine, for example) there is no Forth VM. There the DODO cross-platform layer and the application are compiled to native JavaScript by the Sloth word-to-native compiler, and the per-platform interface is implemented in JavaScript; the API is unchanged, so the same DODO code is reused without losing performance.

## Built on ANS Forth

DODO sits on top of ANS Forth and must never be loaded without it. Every DODO library is initialized only after the ANS Forth system (Sloth `ans.4th`) is up: its bootstrap functions rely on ANS words (for example `+FIELD`). This is a hard contract for all DODO libraries, not a convention.

## Core Principles

1. **The Path of Least Effort.** Interfaces use whatever approach minimizes total concepts: expose raw buffers when memory access is simplest; provide clean abstractions (e.g. `CIRCLE`, `TONE`) when raw math creates boilerplate.

2. **Single Canonical Path.** Every operation has exactly one way to be performed. Alternate modes, helper variants and duplicate paths are explicitly omitted.

3. **The Three-Line Rule.** All operations must initialize, execute and present results in three lines of code or fewer:

   ```forth
   \ Multimedia example
   440 FREQ TONE       \ Line 1: play an audio tone
   100 100 50 CIRCLE   \ Line 2: draw visual output
   SYNC                \ Line 3: present the frame
   ```

   ```forth
   \ IPC example
   Z" PAYLOAD"         \ Line 1: define the message
   CHAN-OUT POST       \ Line 2: transmit data
   FLUSH               \ Line 3: flush the stream
   ```

4. **Zero Unnecessary Moving Parts.** Interfaces eliminate setup ceremony, handles, context creation and non-essential configuration flags.

## Layout

- `platforms/sloth/c/` — Sloth C platform (`dodo.c`, `CMakeLists.txt`).
  - `dodo_common.h` — header-only base shared by the libraries: exposes the host C `int` type (`INTS`, `INT@`, `INT!`, `INTALIGNED`, `INTFIELD:`). Not a linkable library.
  - `libs/sdl3/` — SDL3-backed video, audio and input.
  - `libs/geninput/` — input generation from code (keyboard)
  - `libs/system/` — host shell and directory access.
- `4th/dodo/` — cross-platform Forth layer (`media.4th`, `sqlite.4th`, `media_examples/`, `sdl3_examples/`).

### System (v1)

Registered by `dodo_bootstrap_system`, called from `dodo.c` after ANS Forth
is loaded. There are no `4th/dodo/system.4th` helpers: the four words below
are primitives.

- `SYSTEM ( c-addr u -- n )` — runs the command through the host shell
  (`/bin/sh -c` on POSIX, `cmd /C` on Windows) and returns its exit status
  `n`; `-1` means the command could not be spawned. Not ANS (ANS `SYSTEM`
  returns nothing), so it lives in DODO, not Sloth.
- `OPEN-DIR ( c-addr u -- dirid ior )`,
  `READ-DIR ( c-addr u1 dirid -- u2 flag ior )`,
  `CLOSE-DIR ( dirid -- ior )` — directory counterparts of the ANS file
  words. They never throw: `ior` is `0` or `-37` (file/directory I/O error).
  `READ-DIR` uses `READ-LINE` semantics: `flag` is true when an entry was
  delivered even if it was longer than `u1` and the remainder was discarded;
  at end of directory `u2 = 0`, `flag = false`, `ior = 0`.

`dirid` is the address of a heap iterator struct cast to `CELL`, the same
opaque-pointer convention the ANS file words use for their `FILE*` fileid
(`file.c`). Nothing in the API exposes the layout, so a formal opaque handle
type can be introduced later without breaking code that treats a dirid as an
opaque cell.

There is no fixed path, command or entry buffer. Counted strings are copied
into a heap buffer of exactly `len + 1` bytes, and the directory listing is
allocated by SDL3, so long names are neither truncated nor able to overflow
(as Sloth's `char buf[512]` can). `READ-DIR` writes at most `u1` bytes into
the caller's buffer and discards any remainder.

On constrained hosts with no shell or filesystem (a JavaScript-engine
smartwatch, for example) the per-platform implementation of these words is
responsible for reporting the missing capability: return `-1` from `SYSTEM`
and `-37` from the directory words. Verified for now; a capability query is
a follow-up.

## Dependencies

`platforms/sloth/c/CMakeLists.txt` pulls **Sloth** and **cpnbi** via `FetchContent`. 

## Build

```sh
cmake -B build -G "Ninja Multi-Config" -S platforms/sloth/c
cmake --build build
```

## Status

Early stage.

## FAQ

**Why the name DODO?**
Because DODO is fearless. It trusts you. It will let you do anything you please with your system.

