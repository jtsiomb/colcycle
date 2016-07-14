#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void logger_output(FILE *fp);
void printlog(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif	/* LOGGER_H_ */
