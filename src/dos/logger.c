#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

#define LOGFNAME	"colcycle.log"

static FILE *logfile;

void logger_output(FILE *fp)
{
	if(logfile) fclose(logfile);
	logfile = fp;
}

void printlog(const char *fmt, ...)
{
	va_list ap;

	if(!logfile) {
		if(!(logfile = fopen(LOGFNAME, "w"))) {
			return;
		}
		setvbuf(logfile, 0, _IOLBF, 0);
	}

	va_start(ap, fmt);
	vfprintf(logfile, fmt, ap);
	va_end(ap);
}
