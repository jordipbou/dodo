#include <sloth.h>
#include <cpnbi.h>
#include <sloth_sdl3.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
	
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
	cpnbi_init();

	X* x = sloth_new();

	sloth_bootstrap(x);
	sloth_set_root_path(x, ROOT_PATH "4th/");
	sloth_include(x, "ans.4th");
	sloth_bootstrap_SDL3(x);

	sloth_repl(x);

	return SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	return SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
	return SDL_APP_SUCCESS;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}
