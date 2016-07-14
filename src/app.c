#include <stdio.h>
#include <math.h>
#include "app.h"
#include "image.h"

#ifndef M_PI
#define M_PI 3.141593
#endif

int fbwidth, fbheight;
unsigned char *fbpixels;
unsigned long time_msec;

static struct image img;
static int blend = 1;

int app_init(int argc, char **argv)
{
	int i, j;
	unsigned char *dptr = fbpixels;

	if(argv[1]) {
		if(load_image(&img, argv[1]) == -1) {
			fprintf(stderr, "failed to load image: %s\n", argv[1]);
			return -1;
		}
	} else {
		if(gen_test_image(&img) == -1) {
			fprintf(stderr, "failed to generate test image\n");
			return -1;
		}
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

#define LERP(a, b, t)	((a) + ((b) - (a)) * (t))

void app_draw(void)
{
	int i, j;

	for(i=0; i<img.num_ranges; i++) {
		int rev, rsize, ioffs;
		float offs, tm;

		if(!img.range[i].rate) continue;
		rev = img.range[i].rev;
		rsize = img.range[i].high - img.range[i].low + 1;

		/* TODO reverse engineer and rewrite this block -------------------- */
		tm = (float)time_msec / (1000.0 / (img.range[i].rate / 280.0));

		if(rev < 3) {
			offs = fmod(tm, (float)rsize);
		} else if(rev == 3) {	/* ping-pong */
			offs = fmod(tm, (float)(rsize * 2));
			if(offs >= rsize) offs = (rsize * 2) - offs;
		} else if(rev < 6) {	/* sine */
			float x = fmod(tm, (float)rsize);
			offs = sin((x * M_PI * 2.0) / (float)rsize) + 1.0;
			if(rev == 4) {
				offs *= rsize / 4;
			} else {
				offs *= rsize / 2;
			}
		}
		/* ----------------------------------------------------------------- */
		ioffs = (int)floor(offs);

		/* reverse when rev is 2 */
		rev = rev == 2 ? 1 : 0;

		for(j=0; j<rsize; j++) {
			int pidx, to, next;

			pidx = j + img.range[i].low;

			if(rev) {
				to = (j + ioffs) % rsize;
				next = (to + 1) % rsize;
			} else {
				if((to = (j - ioffs) % rsize) < 0) {
					to += rsize;
				}
				if((next = to - 1) < 0) {
					next += rsize;
				}
			}
			to += img.range[i].low;

			if(blend) {
				float t, r, g, b;

				next += img.range[i].low;

				t = offs - (int)offs;
				r = LERP(img.palette[to].r, img.palette[next].r, t);
				g = LERP(img.palette[to].g, img.palette[next].g, t);
				b = LERP(img.palette[to].b, img.palette[next].b, t);

				set_palentry(pidx, (int)r, (int)g, (int)b);
			} else {
				set_palentry(pidx, img.palette[to].r, img.palette[to].g, img.palette[to].b);
			}
		}
	}
}

void app_keyboard(int key, int state)
{
	if(state) {
		switch(key) {
		case 27:
			app_quit();
			break;

		case 'b':
		case 'B':
			blend = !blend;
			break;

		default:
			break;
		}
	}
}
