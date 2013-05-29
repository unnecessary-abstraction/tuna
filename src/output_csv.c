/*******************************************************************************
	output_csv.c: Output to CSV files.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compiler.h"
#include "log.h"
#include "output_csv.h"
#include "timespec.h"
#include "types.h"
#include "uara.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

struct csv {
	struct consumer		consumer;
	
	FILE *			f;
	uint			sample_rate;
	struct timespec		last_ts;
	uint			offset;
};

void output_csv_exit(struct consumer * consumer)
{
	struct csv * c = container_of(consumer, struct csv, consumer);
	fputc('\n', c->f);
	fclose(c->f);

	free(c);
}

int output_csv_write(struct consumer * consumer, sample_t * buf, uint count)
{
	uint i;
	struct csv * c = container_of(consumer, struct csv, consumer);

	for (i = 0; i < count; i++) {
		if ((++c->offset) % c->sample_rate)
			fprintf(c->f, "%f, ", buf[i]);
		else
			fprintf(c->f, "%f\n", buf[i]);
	}
	
	return 0;
}

int output_csv_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	struct csv * c = container_of(consumer, struct csv, consumer);

	c->sample_rate = sample_rate;
	c->last_ts = *ts;
	c->offset = 0;

	fprintf(c->f, "START ");
	timespec_fprint(ts, c->f);
	fprintf(c->f, "\n");
	
	return 0;
}

int output_csv_resync(struct consumer * consumer, struct timespec * ts)
{
	struct csv * c = container_of(consumer, struct csv, consumer);

	c->last_ts = *ts;
	c->offset = 0;

	fprintf(c->f, "RESYNC ");
	timespec_fprint(ts, c->f);
	fprintf(c->f, "\n");
	
	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct consumer * output_csv_init(const char * fname)
{
	struct csv * c = (struct csv *)malloc(sizeof(struct csv));
	if (!c) {
		error("output_csv: Failed to allocate memory");
		return NULL;
	}

	memset(c, 0, sizeof(struct csv));
	
	c->f = fopen(fname, "w");
	if (!c->f) {
		error("output_csv: Failed to open file %s", fname);
		return NULL;
	}

	c->consumer.write = output_csv_write;
	c->consumer.start = output_csv_start;
	c->consumer.resync = output_csv_resync;
	c->consumer.exit = output_csv_exit;

	return &c->consumer;
}
