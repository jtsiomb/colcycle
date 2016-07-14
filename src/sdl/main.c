#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "app.h"

static SDL_Surface *fbsurf;
static int quit;

int main(int argc, char **argv)
{
	unsigned long start_msec;
	unsigned int sdl_flags = SDL_HWPALETTE | SDL_HWSURFACE | SDL_FULLSCREEN;

	char *env = getenv("FULLSCREEN");
	if(env && !atoi(env)) {
		sdl_flags &= ~SDL_FULLSCREEN;
	}

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1) {
		fprintf(stderr, "failed to initialize SDL\n");
		return 1;
	}
	if(!(fbsurf = SDL_SetVideoMode(640, 480, 8, sdl_flags))) {
		fprintf(stderr, "failed to set video mode\n");
		SDL_Quit();
		return 1;
	}
	SDL_WM_SetCaption("colcycle", 0);

	SDL_LockSurface(fbsurf);
	fbwidth = fbsurf->w;
	fbheight = fbsurf->h;
	fbpixels = fbsurf->pixels;

	if(app_init(argc, argv) == -1) {
		SDL_Quit();
		return 1;
	}
	SDL_UnlockSurface(fbsurf);
	SDL_UpdateRect(fbsurf, 0, 0, 0, 0);

	if(sdl_flags & SDL_FULLSCREEN) {
		SDL_ShowCursor(0);
	}

	start_msec = SDL_GetTicks();

	while(!quit) {
		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_KEYDOWN:
				app_keyboard(ev.key.keysym.sym, 1);
				break;
			case SDL_KEYUP:
				app_keyboard(ev.key.keysym.sym, 0);
				break;

			default:
				break;
			}
			if(quit) goto break_evloop;
		}

		if(SDL_MUSTLOCK(fbsurf)) {
			SDL_LockSurface(fbsurf);
		}
		fbpixels = fbsurf->pixels;

		time_msec = SDL_GetTicks() - start_msec;
		app_draw();

		if(SDL_MUSTLOCK(fbsurf)) {
			SDL_UnlockSurface(fbsurf);
		}
	}

break_evloop:
	app_cleanup();
	SDL_ShowCursor(1);
	SDL_Quit();
	return 0;
}

void app_quit(void)
{
	quit = 1;
}

void set_palette(int idx, int r, int g, int b)
{
	SDL_Color col;
	col.r = r;
	col.g = g;
	col.b = b;
	SDL_SetPalette(fbsurf, SDL_PHYSPAL | SDL_LOGPAL, &col, idx, 1);
}
