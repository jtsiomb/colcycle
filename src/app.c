#include <stdio.h>
#include "app.h"
#include "image.h"

static struct image img;

int app_init(int argc, char **argv)
{
	int i, j;
	unsigned char *dptr = fbpixels;

	if(!argv[1]) {
		fprintf(stderr, "pass the filename of a color cycling image\n");
		return -1;
	}
	if(load_image(&img, argv[1]) == -1) {
		fprintf(stderr, "failed to load image: %s\n", argv[1]);
		return -1;
	}

	for(i=0; i<256; i++) {
		set_palentry(i, img.palette[i].r, img.palette[i].g, img.palette[i].b);
	}

	for(i=0; i<fbheight; i++) {
		for(j=0; j<fbwidth; j++) {
			unsigned char c = 0;
			if(i < img.height && j < img.width) {
				c = img.pixels[i * img.width + j];
			}
			*dptr++ = c;
		}
	}

	return 0;
}

void app_cleanup(void)
{
}

void app_draw(void)
{
	int i;
	for(i=0; i<256; i++) {
		int idx = (i + time_msec * img.range[0].rate / 10000) & 0xff;
		set_palentry(i, img.palette[idx].r, img.palette[idx].g, img.palette[idx].b);
	}
}

void app_keyboard(int key, int state)
{
	if(state) {
		switch(key) {
		case 27:
			app_quit();
			break;

		default:
			break;
		}
	}
}
