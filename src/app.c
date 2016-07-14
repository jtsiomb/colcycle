#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#ifdef __WATCOMC__
#include <direct.h>
#else
#include <dirent.h>
#endif
#include <sys/stat.h>
#include "app.h"
#include "image.h"

#ifndef M_PI
#define M_PI 3.141593
#endif

struct ss_node {
	char *path;
	struct image *img;
	struct ss_node *next;
};

static void set_image_palette(struct image *img);
static void show_image(struct image *img);
static int load_slideshow(const char *path);
static int load_slide(void);

int fbwidth, fbheight;
unsigned char *fbpixels;
unsigned long time_msec;

static struct image *img;
static int blend = 1;
static int fade_dir;
static unsigned long fade_start, fade_dur = 600;
static int change_pending;
static unsigned long showing_since, show_time = 15000;

static struct ss_node *sslist;

int app_init(int argc, char **argv)
{
	if(argv[1]) {
		struct stat st;

		if(stat(argv[1], &st) == -1) {
			fprintf(stderr, "failed to stat path: %s\n", argv[1]);
			return -1;
		}

		if(S_ISDIR(st.st_mode)) {
			if(load_slideshow(argv[1]) == -1) {
				fprintf(stderr, "failed to load slideshow in dir: %s\n", argv[1]);
				return -1;
			}
			if(load_slide() == -1) {
				return -1;
			}

		} else {
			if(!(img = malloc(sizeof *img))) {
				perror("failed to allocate image structure");
				return -1;
			}
			if(load_image(img, argv[1]) == -1) {
				fprintf(stderr, "failed to load image: %s\n", argv[1]);
				return -1;
			}
		}
	} else {
		if(!(img = malloc(sizeof *img))) {
			perror("failed to allocate image structure");
			return -1;
		}
		if(gen_test_image(img) == -1) {
			fprintf(stderr, "failed to generate test image\n");
			return -1;
		}
	}

	set_image_palette(img);
	show_image(img);
	return 0;
}

void app_cleanup(void)
{
	if(sslist) {
		struct ss_node *start = sslist;
		sslist = sslist->next;
		start->next = 0;	/* break the circle */

		while(sslist) {
			struct ss_node *node = sslist;
			sslist = sslist->next;
			destroy_image(node->img);
			free(node->path);
			free(node);
		}

	} else {
		destroy_image(img);
		free(img);
	}
}

#define LERP(a, b, t)	((a) + ((b) - (a)) * (t))

/* tx is fixed point with 10 bits decimal */
static void palfade(int dir, long tx)
{
	int i;

	if(!img) return;

	if(dir == -1) {
		tx = 1024 - tx;
	}

	for(i=0; i<256; i++) {
		int r = (img->palette[i].r * tx) >> 10;
		int g = (img->palette[i].g * tx) >> 10;
		int b = (img->palette[i].b * tx) >> 10;
		set_palette(i, r, g, b);
	}
}

void app_draw(void)
{
	int i, j;

	if(!img) return;

	if(sslist) {
		if(!fade_dir && (change_pending || time_msec - showing_since > show_time)) {
			fade_dir = -1;
			fade_start = time_msec;
			change_pending = 0;
		}

		if(fade_dir) {
			unsigned long dt = time_msec - fade_start;

			if(dt >= fade_dur) {
				if(fade_dir == -1) {
					sslist = sslist->next;
					if(load_slide() == -1) {
						app_quit();
						return;
					}
					show_image(img);
					fade_dir = 1;
					fade_start = time_msec;
					dt = 0;
				} else {
					set_image_palette(img);
					fade_dir = 0;
				}
			}

			if(fade_dir) {
				long tx = ((long)dt << 10) / fade_dur;
				palfade(fade_dir, tx);
			}
			showing_since = time_msec;
			return;
		}
	}

	for(i=0; i<img->num_ranges; i++) {
		int rev, rsize, ioffs;
		float offs, tm;

		if(!img->range[i].rate) continue;
		rev = img->range[i].rev;
		rsize = img->range[i].high - img->range[i].low + 1;

		/* TODO rewrite this block -------------------- */
		tm = (float)time_msec / (1000.0 / (img->range[i].rate / 280.0));

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
		ioffs = (int)floor(offs);

		/* reverse when rev is 2 */
		rev = rev == 2 ? 1 : 0;
		/* -------------------------------------------- */

		for(j=0; j<rsize; j++) {
			int pidx, to, next;

			pidx = j + img->range[i].low;

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
			to += img->range[i].low;

			if(blend) {
				float t, r, g, b;

				next += img->range[i].low;

				t = offs - (int)offs;
				r = LERP(img->palette[to].r, img->palette[next].r, t);
				g = LERP(img->palette[to].g, img->palette[next].g, t);
				b = LERP(img->palette[to].b, img->palette[next].b, t);

				set_palette(pidx, (int)r, (int)g, (int)b);
			} else {
				set_palette(pidx, img->palette[to].r, img->palette[to].g, img->palette[to].b);
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

		case ' ':
			change_pending = 1;
			break;

		default:
			break;
		}
	}
}

static void set_image_palette(struct image *img)
{
	int i;
	for(i=0; i<256; i++) {
		set_palette(i, img->palette[i].r, img->palette[i].g, img->palette[i].b);
	}
}

static void show_image(struct image *img)
{
	int i, j;
	unsigned char *dptr = fbpixels;

	for(i=0; i<fbheight; i++) {
		for(j=0; j<fbwidth; j++) {
			unsigned char c = 0;
			if(i < img->height && j < img->width) {
				c = img->pixels[i * img->width + j];
			}
			*dptr++ = c;
		}
	}

	showing_since = time_msec;
}

static int load_slideshow(const char *path)
{
	DIR *dir;
	struct dirent *dent;
	struct ss_node *head = 0, *tail = 0, *node;

	if(!(dir = opendir(path))) {
		fprintf(stderr, "failed to open directory: %s: %s\n", path, strerror(errno));
		return -1;
	}

	while((dent = readdir(dir))) {
		int sz;

		if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0) {
			continue;
		}
		sz = strlen(path) + strlen(dent->d_name) + 1;	/* +1 for a slash */

		if(!(node = malloc(sizeof *node))) {
			perror("failed to allocate slideshow node");
			goto err;
		}
		if(!(node->path = malloc(sz + 1))) {
			perror("failed to allocate image path");
			free(node);
			goto err;
		}
		sprintf(node->path, "%s/%s", path, dent->d_name);
		node->img = 0;
		node->next = 0;

		if(head) {
			tail->next = node;
			tail = node;
		} else {
			head = tail = node;
		}
	}
	closedir(dir);

	sslist = head;
	tail->next = head;	/* make circular */
	return 0;

err:
	closedir(dir);
	while(head) {
		node = head;
		head = head->next;
		free(node->path);
		free(node);
	}
	return -1;
}

static int load_slide(void)
{
	struct ss_node *start = sslist;

	img = 0;
	do {
		if(sslist->path) {
			if(!sslist->img) {
				if(!(sslist->img = malloc(sizeof *sslist->img))) {
					perror("failed to allocate image structure");
					return -1;
				}
				if(load_image(sslist->img, sslist->path) == -1) {
					fprintf(stderr, "failed to load image: %s\n", sslist->path);
					free(sslist->path);
					sslist->path = 0;
					free(sslist->img);
					sslist->img = 0;
				}
			}
			img = sslist->img;
		}

		sslist = sslist->next;
	} while(!img && sslist != start);

	return img ? 0 : -1;
}
