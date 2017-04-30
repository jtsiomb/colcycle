#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <alloca.h>
#include "image_lbm.h"

#ifndef __BYTE_ORDER__
#error "__BYTE_ORDER__ undefined"
#endif
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LENDIAN
#else
#define BENDIAN
#endif

#define MKID(a, b, c, d)	(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define IS_IFF_CONTAINER(id)	((id) == IFF_FORM || (id) == IFF_CAT || (id) == IFF_LIST)

enum {
	IFF_FORM = MKID('F', 'O', 'R', 'M'),
	IFF_CAT = MKID('C', 'A', 'T', ' '),
	IFF_LIST = MKID('L', 'I', 'S', 'T'),
	IFF_ILBM = MKID('I', 'L', 'B', 'M'),
	IFF_PBM = MKID('P', 'B', 'M', ' '),
	IFF_BMHD = MKID('B', 'M', 'H', 'D'),
	IFF_CMAP = MKID('C', 'M', 'A', 'P'),
	IFF_BODY = MKID('B', 'O', 'D', 'Y'),
	IFF_CRNG = MKID('C', 'R', 'N', 'G')
};


struct chdr {
	uint32_t id;
	uint32_t size;
};

struct bitmap_header {
	uint16_t width, height;
	int16_t xoffs, yoffs;
	uint8_t nplanes;
	uint8_t masking;
	uint8_t compression;
	uint8_t padding;
	uint16_t colorkey;
	uint8_t aspect_num, aspect_denom;
	int16_t pgwidth, pgheight;
} __attribute__ ((packed));

enum {
	MASK_NONE,
	MASK_PLANE,
	MASK_COLORKEY,
	MASK_LASSO
};

struct crng {
	uint16_t padding;
	uint16_t rate;
	uint16_t flags;
	uint8_t low, high;
};

enum {
	CRNG_ENABLE = 1,
	CRNG_REVERSE = 2
};

static int read_header(FILE *fp, struct chdr *hdr);
static int read_ilbm_pbm(FILE *fp, uint32_t type, uint32_t size, struct image *img);
static int read_bmhd(FILE *fp, struct bitmap_header *bmhd);
static int read_crng(FILE *fp, struct crng *crng);
static int read_body_ilbm(FILE *fp, struct bitmap_header *bmhd, struct image *img);
static int read_body_pbm(FILE *fp, struct bitmap_header *bmhd, struct image *img);
static int read_compressed_scanline(FILE *fp, unsigned char *scanline, int width);
static int read16(FILE *fp, uint16_t *res);
static int read32(FILE *fp, uint32_t *res);
static inline uint16_t swap16(uint16_t x);
static inline uint32_t swap32(uint32_t x);

int file_is_ilbm(FILE *fp)
{
	uint32_t type;
	struct chdr hdr;

	while(read_header(fp, &hdr) != -1) {
		if(IS_IFF_CONTAINER(hdr.id)) {
			if(read32(fp, &type) == -1) {
				break;
			}

			if(type == IFF_ILBM || type == IFF_PBM ) {
				rewind(fp);
				return 1;
			}
			hdr.size -= sizeof type;	/* so we will seek fwd correctly */
		}
		fseek(fp, hdr.size, SEEK_CUR);
	}
	fseek(fp, 0, SEEK_SET);
	return 0;
}

void print_chunkid(uint32_t id)
{
	char str[5] = {0};
#ifdef LENDIAN
	id = swap32(id);
#endif
	memcpy(str, &id, 4);
	puts(str);
}

int load_image_ilbm(struct image *img, FILE *fp)
{
	uint32_t type;
	struct chdr hdr;

	while(read_header(fp, &hdr) != -1) {
		if(IS_IFF_CONTAINER(hdr.id)) {
			if(read32(fp, &type) == -1) {
				break;
			}
			hdr.size -= sizeof type;	/* to compensate for having advanced 4 more bytes */

			if(type == IFF_ILBM) {
				if(read_ilbm_pbm(fp, type, hdr.size, img) == -1) {
					return -1;
				}
				return 0;
			}
			if(type == IFF_PBM) {
				if(read_ilbm_pbm(fp, type, hdr.size, img) == -1) {
					return -1;
				}
				return 0;
			}
		}
		fseek(fp, hdr.size, SEEK_CUR);
	}
	return 0;
}

static int read_header(FILE *fp, struct chdr *hdr)
{
	if(fread(hdr, 1, sizeof *hdr, fp) < sizeof *hdr) {
		return -1;
	}
#ifdef LENDIAN
	hdr->id = swap32(hdr->id);
	hdr->size = swap32(hdr->size);
#endif
	return 0;
}

static int read_ilbm_pbm(FILE *fp, uint32_t type, uint32_t size, struct image *img)
{
	int i, res = -1;
	struct chdr hdr;
	struct bitmap_header bmhd;
	struct crng crng;
	struct colrange *crnode;
	unsigned char pal[3 * 256];
	unsigned char *pptr;
	long start = ftell(fp);

	memset(img, 0, sizeof *img);

	while(read_header(fp, &hdr) != -1 && ftell(fp) - start < size) {
		switch(hdr.id) {
		case IFF_BMHD:
			assert(hdr.size == 20);
			if(read_bmhd(fp, &bmhd) == -1) {
				return -1;
			}
			img->width = bmhd.width;
			img->height = bmhd.height;
			if(bmhd.nplanes != 8) {
				fprintf(stderr, "only 256-color ILBM files supported\n");
				return -1;
			}
			if(!(img->pixels = malloc(img->width * img->height))) {
				fprintf(stderr, "failed to allocate %dx%d image\n", img->width, img->height);
				return -1;
			}
			break;

		case IFF_CMAP:
			assert(hdr.size / 3 <= 256);

			if(fread(pal, 1, hdr.size, fp) < hdr.size) {
				fprintf(stderr, "failed to read colormap\n");
				return -1;
			}
			pptr = pal;
			for(i=0; i<256; i++) {
				img->palette[i].r = *pptr++;
				img->palette[i].g = *pptr++;
				img->palette[i].b = *pptr++;
			}
			break;

		case IFF_CRNG:
			assert(hdr.size == sizeof crng);

			if(read_crng(fp, &crng) == -1) {
				fprintf(stderr, "failed to read color cycling range chunk\n");
				return -1;
			}
			if(crng.low != crng.high && crng.rate > 0) {
				if(!(crnode = malloc(sizeof *crnode))) {
					fprintf(stderr, "failed to allocate color range node\n");
					return -1;
				}
				crnode->low = crng.low;
				crnode->high = crng.high;
				crnode->cmode = (crng.flags & CRNG_REVERSE) ? CYCLE_REVERSE : CYCLE_NORMAL;
				crnode->rate = crng.rate;
				crnode->next = img->range;
				img->range = crnode;
				++img->num_ranges;
			}
			break;

		case IFF_BODY:
			if(!img->pixels) {
				fprintf(stderr, "malformed ILBM image: encountered BODY chunk before BMHD\n");
				return -1;
			}
			if(type == IFF_ILBM) {
				if(read_body_ilbm(fp, &bmhd, img) == -1) {
					fprintf(stderr, "failed to read interleaved pixel data\n");
					return -1;
				}
			} else {
				assert(type == IFF_PBM);
				if(read_body_pbm(fp, &bmhd, img) == -1) {
					fprintf(stderr, "failed to read linear pixel data\n");
					return -1;
				}
			}
			res = 0;	/* sucessfully read image */
			break;

		default:
			/* skip unknown chunks */
			fseek(fp, hdr.size, SEEK_CUR);
			if(ftell(fp) & 1) {
				/* chunks must start at even offsets */
				fseek(fp, 1, SEEK_CUR);
			}
		}
	}

	return res;
}


static int read_bmhd(FILE *fp, struct bitmap_header *bmhd)
{
	if(fread(bmhd, sizeof *bmhd, 1, fp) < 1) {
		return -1;
	}
#ifdef LENDIAN
	bmhd->width = swap16(bmhd->width);
	bmhd->height = swap16(bmhd->height);
	bmhd->xoffs = swap16(bmhd->xoffs);
	bmhd->yoffs = swap16(bmhd->yoffs);
	bmhd->colorkey = swap16(bmhd->colorkey);
	bmhd->pgwidth = swap16(bmhd->pgwidth);
	bmhd->pgheight = swap16(bmhd->pgheight);
#endif
	return 0;
}

static int read_crng(FILE *fp, struct crng *crng)
{
	if(fread(crng, sizeof *crng, 1, fp) < 1) {
		return -1;
	}
#ifdef LENDIAN
	crng->rate = swap16(crng->rate);
	crng->flags = swap16(crng->flags);
#endif
	return 0;
}

static int read_body_ilbm(FILE *fp, struct bitmap_header *bmhd, struct image *img)
{
	int i, j, k, npix;
	unsigned char *dest = img->pixels;
	unsigned char *scanbuf = alloca(img->width);

	assert(bmhd->width == img->width);
	assert(bmhd->height == img->height);
	assert(img->pixels);
	assert(bmhd->nplanes = 8);

	/* TODO handle compression */
	for(i=0; i<img->height; i++) {
		/* read a scanline */
		if(fread(scanbuf, 1, img->width, fp) < img->width) {
			return -1;
		}
		if(bmhd->masking & MASK_PLANE) {
			/* skip the mask (1bpp) */
			fseek(fp, img->width / 8, SEEK_CUR);
		}

		/* reorder planes to make the scanline linear */
		npix = 0;
		for(j=0; j<img->width; j++) {
			unsigned char pixel = 0;

			for(k=0; k<bmhd->nplanes; k++) {
				pixel |= ((scanbuf[k] >> npix) & 1) << k;
			}
			*dest++ = pixel;
			scanbuf += bmhd->nplanes;
			npix = (npix + 1) & 7;	/* 8 planes, see assert above */
		}
	}
	return 0;
}

static int read_body_pbm(FILE *fp, struct bitmap_header *bmhd, struct image *img)
{
	int i;
	int npixels = img->width * img->height;
	unsigned char *dptr = img->pixels;

	assert(bmhd->width == img->width);
	assert(bmhd->height == img->height);
	assert(img->pixels);
	assert(bmhd->nplanes = 1);

	if(bmhd->compression) {
		for(i=0; i<img->height; i++) {
			if(read_compressed_scanline(fp, dptr, img->width) == -1) {
				return -1;
			}
			dptr += img->width;
		}

	} else {
		/* uncompressed */
		if(fread(img->pixels, 1, npixels, fp) < npixels) {
			return -1;
		}
	}
	return 0;
}

static int read_compressed_scanline(FILE *fp, unsigned char *scanline, int width)
{
	int i, count, x = 0;
	signed char ctl;

	while(x < width) {
		if(fread(&ctl, 1, 1, fp) < 1) return -1;

		if(ctl == -128) continue;

		if(ctl >= 0) {
			count = ctl + 1;
			if(fread(scanline, 1, count, fp) < count) return -1;
			scanline += count;

		} else {
			unsigned char pixel;
			count = 1 - ctl;
			if(fread(&pixel, 1, 1, fp) < 1) return -1;

			for(i=0; i<count; i++) {
				*scanline++ = pixel;
			}
		}

		x += count;
	}

	return 0;
}

static int read16(FILE *fp, uint16_t *res)
{
	if(fread(res, sizeof *res, 1, fp) < 1) {
		return -1;
	}
#ifdef LENDIAN
	*res = swap16(*res);
#endif
	return 0;
}

static int read32(FILE *fp, uint32_t *res)
{
	if(fread(res, sizeof *res, 1, fp) < 1) {
		return -1;
	}
#ifdef LENDIAN
	*res = swap32(*res);
#endif
	return 0;
}

static inline uint16_t swap16(uint16_t x)
{
	return (x << 8) | (x >> 8);
}

static inline uint32_t swap32(uint32_t x)
{
	return (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
}
