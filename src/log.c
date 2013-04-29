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

static char * ascnow()
{
	time_t t;
	struct tm * local;

	t = time(NULL);
	local = localtime(&t);
	return asctime(local);
}

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

	fprintf(file, "%s started at %s\n", app, ascnow());

	return 0;
}

void log_exit()
{
	assert(file);
	fprintf(file, "%s finished at %s\n", app, ascnow());

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
