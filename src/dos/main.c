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
#include "app.h"
#include "keyb.h"
#include "timer.h"
#include "gfx.h"

static int quit;

int main(int argc, char **argv)
{
	init_timer(100);
	kb_init(32);

	fbwidth = 640;
	fbheight = 480;
	if(!(fbpixels = set_video_mode(fbwidth, fbheight, 8))) {
		return 1;
	}

	if(app_init(argc, argv) == -1) {
		set_text_mode();
		return 1;
	}
	reset_timer();

	while(!quit) {
		int key;
		while((key = kb_getkey()) != -1) {
			app_keyboard(key, 1);
		}
		if(quit) goto break_evloop;

		wait_vsync();
		time_msec = get_msec();
		app_draw();
	}

break_evloop:
	set_text_mode();
	app_cleanup();
	kb_shutdown();
	return 0;
}

void app_quit(void)
{
	quit = 1;
}
