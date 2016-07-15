/*
colcycle - color cycling image viewer
Copyright (C) 2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "app.h"

static SDL_Surface *fbsurf;
static int quit;
static unsigned long start_msec;

int main(int argc, char **argv)
{
	unsigned long prev_msec = 0, frame_time;
	unsigned int sdl_flags = SDL_HWPALETTE | SDL_HWSURFACE;

	char *env = getenv("FULLSCREEN");
	if(env && atoi(env)) {
		sdl_flags |= SDL_FULLSCREEN;
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
	SDL_WM_SetCaption("colcycle/SDL", 0);

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

		time_msec = get_msec();
		app_draw();
		if(SDL_MUSTLOCK(fbsurf)) {
			SDL_UnlockSurface(fbsurf);
		}

		time_msec = get_msec();
		frame_time = time_msec - prev_msec;
		prev_msec = time_msec;
		if(frame_time < 16) {
			SDL_Delay(16 - frame_time);
		}
	}

break_evloop:
	app_cleanup();
	SDL_ShowCursor(1);
	SDL_Quit();
	return 0;
}

unsigned long get_msec(void)
{
	return SDL_GetTicks() - start_msec;
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
