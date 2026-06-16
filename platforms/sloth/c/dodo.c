#include <sloth.h>
#include <file.h>
#include <memory.h>
#include <cpnbi.h>
#include <sloth_sdl3.h>
#include <sloth_geninput.h>

#define SLOTH_APP_INIT			SLOTH_LAST_USER_VAR+sCELL
#define SLOTH_APP_EVENT			SLOTH_APP_INIT+sCELL
#define SLOTH_APP_ITERATE		SLOTH_APP_EVENT+sCELL
#define SLOTH_APP_QUIT			SLOTH_APP_ITERATE+sCELL

#define SLOTH_WINDOW				SLOTH_APP_QUIT+sCELL
#define SLOTH_RENDERER			SLOTH_WINDOW+sCELL

#define SLOTH_LAST_USED_VAR	SLOTH_RENDERER+sCELL

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
	
static X* ctx;

void show_exception(X* x, int err, char *msg) {
	if (err) {
		char *text;
		if (err == -38) {
			SDL_asprintf(&text, "Script (%s) not found.", msg);
		} else if (err == -13) {
			CELL ibuf = *((CELL*)(x->u+20*sCELL));
			CELL ipos = *((CELL*)(x->u+21*sCELL));
			CELL ilen = *((CELL*)(x->u+22*sCELL));
			SDL_asprintf(&text, "Word (%.*s) not found.", (int)(ilen - ipos), (char *)(ibuf + ipos));
		} else {
			SDL_asprintf(&text, "Exception %d.", err);
		}

		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"Sloth exception",
			text,
			NULL);
	}
}

/* The default functions for event, iterate and quit. */

void defaultAppEvent(X* x) {
	SDL_Event *event = (SDL_Event *)sloth_pop(x);

	if (event->type == SDL_EVENT_QUIT) {
		sloth_push(x, SDL_APP_SUCCESS);
	} else {
		sloth_push(x, SDL_APP_CONTINUE);
	}
}

void defaultAppIterate(X* x) {
	sloth_push(x, SDL_APP_CONTINUE);
}

void defaultAppQuit(X* x) {
}


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
	CELL err;
	CELL init, event, iterate, quit;

	/* CPNBI is used for terminal REPL */
	cpnbi_init();

	/* Bootstrap SLOTH. */
	ctx = sloth_new();

	sloth_bootstrap(ctx);
	sloth_bootstrap_file_word_set(ctx);
	sloth_bootstrap_memory_word_set(ctx);

	sloth_user_variable(ctx, "(APP-INIT)", SLOTH_APP_INIT, 0);
	sloth_user_variable(ctx, "(APP-EVENT)", SLOTH_APP_EVENT, 0);
	sloth_user_variable(ctx, "(APP-ITERATE)", SLOTH_APP_ITERATE, 0);
	sloth_user_variable(ctx, "(APP-QUIT)", SLOTH_APP_QUIT, 0);

	sloth_user_variable(ctx, "(WINDOW)", SLOTH_WINDOW, 0);
	sloth_user_variable(ctx, "(RENDERER)", SLOTH_RENDERER, 0);

	sloth_set_root_path(ctx, ROOT_PATH "4th/");
	sloth_include(ctx, "ans.4th");

	sloth_bootstrap_SDL3(ctx);
	sloth_bootstrap_geninput(ctx);

	if (argc == 1) {
		err = sloth_include(ctx, "main.4th");
		if (err) {
			printf("DODO/Sloth REPL\n");
			sloth_repl(ctx);
		}
	} else {
		err = sloth_include(ctx, argv[1]);
		if (err) {
			show_exception(ctx, err, NULL);
		}
	}

	event = sloth_find_word(ctx, "APPEVENT");
	iterate = sloth_find_word(ctx, "APPITERATE");
	quit = sloth_find_word(ctx, "APPQUIT");

	if (event || iterate || quit) {
		sloth_user_set(ctx,
			SLOTH_APP_EVENT,
			event ?
				sloth_get_xt(ctx, event)
				: sloth_primitive(ctx, &defaultAppEvent));

		sloth_user_set(ctx,
			SLOTH_APP_ITERATE,
			iterate ?
				sloth_get_xt(ctx, iterate)
				: sloth_primitive(ctx, &defaultAppIterate));

		sloth_user_set(ctx,
			SLOTH_APP_QUIT,
			quit ?
				sloth_get_xt(ctx, quit)
				: sloth_primitive(ctx, &defaultAppQuit));

		return SDL_APP_CONTINUE;
	} else {
		return SDL_APP_SUCCESS;
	}
}

/* This function runs when a new event (mouse input, */
/* keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	CELL err;

	sloth_push(ctx, (CELL)event);

	err = sloth_catch(ctx, sloth_user_get(ctx, SLOTH_APP_EVENT));
	if (err != 0) {
		return SDL_APP_FAILURE;
	}

	return sloth_pop(ctx);
}

/* This function runs once per frame, and is the heart */
/* of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
	CELL err;

	err = sloth_catch(ctx, sloth_user_get(ctx, SLOTH_APP_ITERATE));
	if (err != 0) {
		return SDL_APP_FAILURE;
	}

	return sloth_pop(ctx);
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	/* SDL will clean up the window/renderer for us. */
	CELL err;

	err = sloth_catch(ctx, sloth_user_get(ctx, SLOTH_APP_QUIT));
	if (err != 0) {
		/* TODO Manage the exception in some good way !!! */
	}
}
