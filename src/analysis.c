/*******************************************************************************
	analysis.c: Real-time analysis.

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

#include <malloc.h>
#include <string.h>
#include <time.h>

#include "analysis.h"
#include "compiler.h"
#include "log.h"
#include "types.h"
#include "uara.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

struct analysis {
	struct consumer		consumer;

	struct consumer *	out_time_slice;
	struct consumer *	out_impulse;
};

void analysis_exit(struct consumer * consumer)
{
	struct analysis * a = container_of(consumer, struct analysis, consumer);

	free(a);
}

int analysis_write(struct consumer * consumer, sample_t * buf, uint count)
{
	/* TODO */
	__unused consumer;
	__unused buf;
	__unused count;

	return 0;
}

int analysis_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	/* TODO */
	__unused consumer;
	__unused sample_rate;
	__unused ts;

	return 0;
}

int analysis_resync(struct consumer * consumer, struct timespec * ts)
{
	/* Pass through the resync. */
	struct analysis * a = container_of(consumer, struct analysis, consumer);

	a->out_time_slice->resync(a->out_time_slice, ts);
	a->out_impulse->resync(a->out_impulse, ts);

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct consumer * analysis_init(struct consumer * out_time_slice, struct consumer * out_impulse)
{
	struct analysis * a = (struct analysis *)malloc(sizeof(struct analysis));
	if (!a) {
		error("analysis_init: Failed to allocate memory");
		return NULL;
	}

	a->out_time_slice = out_time_slice;
	a->out_impulse = out_impulse;

	/* Setup consumer and return. */
	a->consumer.write = analysis_write;
	a->consumer.start = analysis_start;
	a->consumer.resync = analysis_resync;
	a->consumer.exit = analysis_exit;

	return &a->consumer;
}
