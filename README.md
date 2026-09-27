# DODO

DODO offers functionality for working with video, audio, input, inter-process communication in top of ANS Forth.

DODO is built in two layers, a set of words that acts as the interface to the underlying system and has to be implemented separately for each Forth platform and a cross-platform layer built on top of it that works the same across all platforms.

## Scope

* Audio
* Video
* Input (keyboard, mouse, touch, game controllers)
* CSP, actors and reactive programming

## Target Forths

The primary target platform is [[https://github.com/jordipbou/sloth][SLOTH]] (both C and Java implementations) but it's also planned to work on compatibility with other ANS Forths like GForth, MPEForth, or SwiftForth.

On constrained hosts that only offer a high-level engine (a smartwatch with a JavaScript engine, for example) there is no Forth VM. There the DODO cross-platform layer and the application are compiled to native JavaScript by the Sloth word-to-native compiler, and the per-platform interface is implemented in JavaScript; the API is unchanged, so the same DODO code is reused without losing performance.

## Layout

- `platforms/sloth/c/` — Sloth C platform (`dodo.c`, `CMakeLists.txt`, `CMakePresets.json`).
  - `libs/sdl3/` — SDL3-backed video, audio and input.
  - `libs/geninput/` — input generation from code (keyboard)

## Dependencies

`platforms/sloth/c/CMakeLists.txt` pulls **Sloth** and **cpnbi** via `FetchContent`. 

## Build

```sh
cmake --preset dev
cmake --build build
```

## Status

Early stage.

## FAQ

**Why the name DODO?**
Because DODO is fearless. It trusts you. It will let you do anything you please with your system.

