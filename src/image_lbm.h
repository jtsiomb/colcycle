#ifndef IMAGE_ILBM_H_
#define IMAGE_ILBM_H_

#include <stdio.h>
#include "image.h"

int file_is_ilbm(FILE *fp);
int load_image_ilbm(struct image *img, FILE *fp);

#endif	/* IMAGE_ILBM_H_ */
