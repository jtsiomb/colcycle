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

#define CHUNKID(s)	(((uint32_t)(s)[0] << 24) | ((uint32_t)(s)[1] << 16) | \
		((uint32_t)(s)[2] << 8) | (uint32_t)(s)[3])

#define IS_IFF_CONTAINER(id) \
	((id) == CHUNKID("FORM") || (id) == CHUNKID("CAT") || (id) == CHUNKID("LIST"))


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

static int read_header(FILE *fp, struct chdr *hdr);
static int read_ilbm(FILE *fp, int ilbm_size, struct image *img);
static int read_bmhd(FILE *fp, struct bitmap_header *bmhd);
static int read_body(FILE *fp, struct bitmap_header *bmhd, struct image *img);
static inline uint16_t swap16(uint16_t x);
static inline uint32_t swap32(uint32_t x);

int file_is_ilbm(FILE *fp)
{
	int found_iff = 0;
	struct chdr hdr;

	while(read_header(fp, &hdr) != -1) {
		if(IS_IFF_CONTAINER(hdr.id)) {
			found_iff = 1;
			continue;	/* skip seeking to read child chunk next iter */
		}
		if(!found_iff) break;	/* read at least one chunk and not found IFF container */

		if(hdr.id == CHUNKID("ILBM")) {
			rewind(fp);
			return 1;
		}
		fseek(fp, hdr.size, SEEK_CUR);
	}
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
	int found_iff = 0;
	struct chdr hdr;

	while(read_header(fp, &hdr) != -1) {
		if(IS_IFF_CONTAINER(hdr.id)) {
			found_iff = 1;
			continue;
		}
		if(!found_iff) return -1;

		if(hdr.id == CHUNKID("ILBM")) {
			if(read_ilbm(fp, hdr.size, img) == -1) {
				return -1;
			}
			return 0;	/* TODO support multiple ILBM chunks */
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

static int read_ilbm(FILE *fp, int ilbm_size, struct image *img)
{
	int i, res = -1;
	struct chdr hdr;
	struct bitmap_header bmhd;
	unsigned char pal[3 * 256];
	long start = ftell(fp);

	memset(img, 0, sizeof *img);

	while(read_header(fp, &hdr) != -1 && ftell(fp) - start < ilbm_size) {
		if(hdr.id == CHUNKID("BMHD")) {
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
			if(bmhd.compression) {
				fprintf(stderr, "only uncompressed ILBM files supported\n");
				return -1;
			}
			if(!(img->pixels = malloc(img->width * img->height))) {
				fprintf(stderr, "failed to allocate %dx%d image\n", img->width, img->height);
				return -1;
			}

		} else if(hdr.id == CHUNKID("CMAP")) {
			unsigned char *pptr = pal;

			assert(hdr.size / 3 <= 256);

			if(fread(pal, 1, hdr.size, fp) < hdr.size) {
				fprintf(stderr, "failed to read colormap\n");
				return -1;
			}
			for(i=0; i<256; i++) {
				img->palette[i].r = *pptr++;
				img->palette[i].g = *pptr++;
				img->palette[i].b = *pptr++;
			}

		} else if(hdr.id == CHUNKID("BODY")) {
			if(!img->pixels) {
				fprintf(stderr, "malformed ILBM image: encountered BODY chunk before BMHD\n");
				return -1;
			}
			if(read_body(fp, &bmhd, img) == -1) {
				fprintf(stderr, "failed to read pixel data\n");
				return -1;
			}
			res = 0;	/* sucessfully read image */

		} else {
			/* skip unknown chunks */
			fseek(fp, hdr.size, SEEK_CUR);
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

static int read_body(FILE *fp, struct bitmap_header *bmhd, struct image *img)
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

static inline uint16_t swap16(uint16_t x)
{
	return (x << 8) | (x >> 8);
}

static inline uint32_t swap32(uint32_t x)
{
	return (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
}
