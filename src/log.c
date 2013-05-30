/*******************************************************************************
	log.c: Generic logging support.

	Copyright (C) 2013 Paul Barker, Loughborough University

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*******************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/cdefs.h>

#include "log.h"
#include "timespec.h"

/*******************************************************************************
	Private variables and functions.
*******************************************************************************/

static FILE *file = NULL;
static char *app = NULL;

static const char * messages[LOG_MAX_LEVEL + 1] = {
	"FATAL",
	"ERROR",
	"WARNING",
	"MESSAGE",
	"(Bad Log Level)"
};

static int __log_printf(int level, const char * s, va_list va)
{
	int r, count;
	assert(file);

	count = 0;

	if ((level > LOG_MAX_LEVEL) || (level < 0))
		level = LOG_MAX_LEVEL;

	r = fprintf(file, "%s: ", messages[level]);
	if (r < 0)
		return r;
	count += r;

	r = vfprintf(file, s, va);
	if (r < 0)
		return r;
	count += r;

	r = fprintf(file, "\n");
	if (r < 0)
		return r;
	count += r;

	return count;
}

/*******************************************************************************
	Public functions.
*******************************************************************************/

int log_init(const char * output_file, const char * app_name)
{
	int r;

	if (!app_name)
		app = strdup("(null)");
	else
		app = strdup(app_name);
	if (!app)
		return -errno;

	if (!output_file) {
		file = stderr;
	} else {
		file = fopen(output_file, "a");
		if (!file) {
			free(app);
			return -errno;
		}
	}

	/* Get current time. */
	struct timespec ts;
	r = clock_gettime(CLOCK_REALTIME, &ts);
	if (r < 0)
		return r;

	r = fprintf(file, "%s started at ", app);
	if (r < 0)
		return r;

	r = timespec_fprint(&ts, file);
	if (r < 0)
		return r;

	r = fprintf(file, "\n");
	if (r < 0)
		return r;

	return 0;
}

void log_exit()
{
	int r;

	assert(file);

	/* Get current time. */
	struct timespec ts;
	r = clock_gettime(CLOCK_REALTIME, &ts);
	if (r < 0)
		memset(&ts, 0, sizeof(ts));

	fprintf(file, "%s finished at ", app);
	timespec_fprint(&ts, file);
	fprintf(file, "\n");

	fclose(file);
	file = NULL;

	free(app);
}

int log_printf(int level, const char * s, ...)
{
	va_list va;
	int r;
	assert(file);

	va_start(va, s);
	r = __log_printf(level, s, va);
	va_end(va);

	if (level == LOG_FATAL) {
		log_exit();
		exit(-1);
	}

	return r;
}

int msg(const char * s, ...)
{
	va_list va;
	int r;
	assert(file);

	va_start(va, s);
	r = __log_printf(LOG_MESSAGE, s, va);
	va_end(va);

	return r;
}

int warn(const char * s, ...)
{
	va_list va;
	int r;
	assert(file);

	va_start(va, s);
	r = __log_printf(LOG_WARNING, s, va);
	va_end(va);

	return r;
}

int error(const char * s, ...)
{
	va_list va;
	int r;
	assert(file);

	va_start(va, s);
	r = __log_printf(LOG_ERROR, s, va);
	va_end(va);

	return r;
}

void __noreturn fatal(const char * s, ...)
{
	va_list va;
	assert(file);

	va_start(va, s);
	__log_printf(LOG_FATAL, s, va);
	abort();

	/* Never reached. */
	va_end(va);

	/* Just incase. */
	while (1) ;
}

void log_sync()
{
	int f;
	f = fileno(file);

	fflush(file);
	fsync(f);
}
