#ifndef IMAGE_H_
#define IMAGE_H_

struct color {
	unsigned char r, g, b;
};

struct colrange {
	int low, high;
	int rev;
	unsigned int rate;
	struct colrange *next;
};

struct image {
	int width, height;
	struct color palette[256];
	struct colrange *range;
	int num_ranges;
	unsigned char *pixels;
};

int gen_test_image(struct image *img);
int load_image(struct image *img, const char *fname);
void destroy_image(struct image *img);

#endif	/* IMAGE_H_ */
