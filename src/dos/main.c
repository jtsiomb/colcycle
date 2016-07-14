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
	fbpixels = set_video_mode(fbwidth, fbheight, 8);

	if(app_init(argc, argv) == -1) {
		set_text_mode();
		return 1;
	}
	reset_timer();

	for(;;) {
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
	app_cleanup();
	set_text_mode();
	kb_shutdown();
	return 0;
}

void app_quit(void)
{
	quit = 1;
}

void set_palentry(int idx, unsigned char r, unsigned char g, unsigned char b)
{
	set_palette(idx, r, g, b);
}
