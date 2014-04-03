/*******************************************************************************
	dat.c: Raw data file output.

	Copyright (C) 2014 Paul Barker, Loughborough University

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
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "dat.h"
#include "log.h"
#include "timespec.h"
#include "types.h"

static int dat_write_header(FILE * dat)
{
	int buf[2];

	assert(dat);

	/* Write magic number followed by byte order indicator. */
	buf[0] = htonl(TUNA_DAT_MAGIC);
	buf[1] = TUNA_DAT_ENDIAN_INDICATOR;

	return fwrite(buf, 2 * sizeof(int), 1, dat);
}

FILE * dat_open(const char * filename)
{
	assert(filename);

	FILE * dat;
	int r;

	dat = fopen(filename, "w");
	if (!dat)
		return NULL;

	r = dat_write_header(dat);
	if (r < 0) {
		fclose(dat);
		return NULL;
	}

	return dat;
}

void dat_close(FILE * dat)
{
	assert(dat);

	fclose(dat);
}

int dat_write_record(FILE * dat, int record_type, void * data, size_t count)
{
	assert(dat);

	int r;
	int buf[2];

	if (!record_type)
		/* Write a single NULL byte. */
		return putc(0, dat);

	/* Write record header: type in big endian order followed by length. */
	buf[0] = htonl(record_type);
	buf[1] = count;
	r = fwrite(buf, 2 * sizeof(int), 1, dat);
	if (r < 0)
		return r;

	/* Write record body. */
	return fwrite(data, count, 1, dat);
}

int dat_write_null(FILE * dat, size_t count)
{
	assert(dat);

	void * buf;
	int r;
	long lr;
	size_t page_size;

	lr = sysconf(_SC_PAGESIZE);
	if (lr < 0)
		fatal("dat: Could not determine page size");

	page_size = (size_t)lr;

	if (count > page_size)
		buf = calloc(1, page_size);
	else
		buf = calloc(1, count);
	if (!buf) {
		error("dat: Failed to allocate memory for write buffer");
		return -1;
	}

	while (count > page_size) {
		r = fwrite(buf, page_size, 1, dat);
		if (r < 0)
			goto cleanup;
		count -= page_size;
	}

	r = fwrite(buf, count, 1, dat);

cleanup:
	free(buf);
	return r;
}

int dat_write_start(FILE * dat, struct timespec * ts)
{
	return dat_write_record(dat, TUNA_DAT_START, ts, sizeof(*ts));
}

int dat_write_resync(FILE * dat, struct timespec * ts)
{
	return dat_write_record(dat, TUNA_DAT_RESYNC, ts, sizeof(*ts));
}
