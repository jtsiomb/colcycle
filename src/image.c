#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "image.h"

int load_image(struct image *img, const char *fname)
{
	int i, j;
	unsigned char *pptr;

	img->width = 640;
	img->height = 480;

	if(!(img->range = malloc(sizeof *img->range))) {
		return -1;
	}
	img->num_ranges = 1;
	img->range[0].low = 0;
	img->range[0].high = 255;
	img->range[0].rev = 0;
	img->range[0].rate = 500;

	if(!(img->pixels = malloc(img->width * img->height))) {
		free(img->range);
		return -1;
	}

	for(i=0; i<256; i++) {
		float theta = M_PI * 2.0 * (float)i / 256.0;
		float r = cos(theta) * 0.5 + 0.5;
		float g = sin(theta) * 0.5 + 0.5;
		float b = -cos(theta) * 0.5 + 0.5;
		img->palette[i].r = (int)(r * 255.0);
		img->palette[i].g = (int)(g * 255.0);
		img->palette[i].b = (int)(b * 255.0);
	}

	pptr = img->pixels;
	for(i=0; i<img->height; i++) {
		int c = (i << 8) / img->height;
		for(j=0; j<img->width; j++) {
			int chess = ((i >> 6) & 1) == ((j >> 6) & 1);
			*pptr++ = (chess ? c : c + 128) & 0xff;
		}
	}
	return 0;
}

void destroy_image(struct image *img)
{
	free(img->pixels);
	free(img->range);
	memset(img, 0, sizeof *img);
}
