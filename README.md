# DODO

DODO offers functionality for working with video, audio, input, database, inter-process communication in top of ANS Forth.

DODO is built in two layers, a set of words that acts as the interface to the underlying system and has to be implemented separately for each Forth platform and a cross-platform layer built on top of it that works the same across all platforms.

## Scope

* Audio
* Video
* Input (keyboard, mouse, touch, game controllers)
* Database (SQLite)
* CSP, actors and reactive programming

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
- `4th/dodo/` — cross-platform Forth layer (`media.4th`, `sqlite.4th`, `media_examples/`, `sdl3_examples/`).

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

