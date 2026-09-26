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

## Status

Early stage.

## FAQ

**Why the name DODO?**
Because DODO is fearless. It trusts you. It will let you do anything you please with your system.

