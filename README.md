DODO

DODO is a layer on top of ANS Forth that offers functionality for working with video, audio, networking, and similar concerns.

DODO is built in two layers, a set of words that acts as the interface to the underlying system and has to be implemented separately for each Forth platform and a cross-platform layer built on top of it that works the same across all platforms.

Scope

* Audio
* Video
* Input (keyboard, mouse, touch, game controllers)
* CSP, actors and reactive programming

Target Forths

The primary target platform is [[https://github.com/jordipbou/sloth][SLOTH]] (both C and Java implementations) but it's also planned to work on compatibility with other ANS Forths like GForth, MPEForth, or SwiftForth.

Status

Early stage.
