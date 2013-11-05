/*******************************************************************************
	csv.c: CSV file output.

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
#include <stdio.h>
#include <time.h>

#include "csv.h"
#include "timespec.h"
#include "types.h"

FILE * csv_open(const char * filename)
{
	return fopen(filename, "w");
}

void csv_close(FILE * csv)
{
	assert(csv);

	fclose(csv);
}

int csv_write_float(FILE * csv, float f)
{
	assert(csv);

	return fprintf(csv, "%f, ", f);
}

int csv_write_sample(FILE * csv, sample_t s)
{
	assert(csv);

	return fprintf(csv, "%d, ", s);
}

int csv_write_uint(FILE * csv, uint u)
{
	assert(csv);

	return fprintf(csv, "%u, ", u);
}

int csv_next(FILE * csv)
{
	assert(csv);

	return fprintf(csv, "\n");
}

int csv_write_start(FILE * csv, struct timespec * ts)
{
	int r, total;

	assert(csv);
	assert(ts);

	r = fprintf(csv, "START ");
	if (r < 0)
		return r;
	total = r;

	r = timespec_fprint(ts, csv);
	if (r < 0)
		return r;
	total += r;

	r = fprintf(csv, "\n");
	if (r < 0)
		return r;
	total += r;

	return total;
}

int csv_write_resync(FILE * csv, struct timespec * ts)
{
	int r, total;

	assert(csv);
	assert(ts);

	r = fprintf(csv, "RESYNC ");
	if (r < 0)
		return r;
	total = r;

	r = timespec_fprint(ts, csv);
	if (r < 0)
		return r;
	total += r;

	r = fprintf(csv, "\n");
	if (r < 0)
		return r;
	total += r;

	return total;
}
