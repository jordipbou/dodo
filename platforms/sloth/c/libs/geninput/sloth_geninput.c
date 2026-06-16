#include "sloth_geninput.h"

#ifdef WINDOWS
WinScancode SDLScancodeToWinScancode(SDL_Scancode sdl)
{
    if ((unsigned)sdl >= SDL_SCANCODE_COUNT)
        return (WinScancode){0, 0};

    return s_scancode_table[sdl];
}

void SendKey(SDL_Scancode sdl, int pressed)
{
    WinScancode w = SDLScancodeToWinScancode(sdl);
    if (!w.scancode)
        return;

    INPUT in = {0};
    in.type = INPUT_KEYBOARD;
    in.ki.wScan = w.scancode;
    in.ki.dwFlags = KEYEVENTF_SCANCODE |
                    (pressed ? 0 : KEYEVENTF_KEYUP) |
                    (w.extended ? KEYEVENTF_EXTENDEDKEY : 0);

    SendInput(1, &in, sizeof(INPUT));
}

void sloth_geninput_press_key_(X* x) { 
	SendKey(sloth_pop(x), 1); 
}
void sloth_geninput_release_key_(X* x) { 
	SendKey(sloth_pop(x), 0); 
}
#else
void sloth_geninput_press_key_(X* x) { 
	/* To be implemented */
}
void sloth_geninput_release_key_(X* x) { 
	/* To be implemented */
}

#endif
void sloth_bootstrap_geninput(X* x) {
	SLOTH_GENINPUT_CODE("PRESS-KEY", press_key);
	SLOTH_GENINPUT_CODE("RELEASE-KEY", release_key);
}

