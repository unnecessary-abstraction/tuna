/*******************************************************************************
	analysis.c: Run data through both analysis modules.

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
#include <errno.h>
#include <malloc.h>
#include <time.h>

#include "analysis.h"
#include "consumer.h"
#include "fft.h"
#include "log.h"
#include "pulse.h"
#include "time_slice.h"
#include "types.h"

struct analysis {
	struct consumer * pulse;
	struct consumer * time_slice;
};

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

void analysis_exit(struct consumer * consumer)
{
	assert(consumer);

	struct analysis * a = (struct analysis *) consumer_get_data(consumer);

	consumer_exit(a->pulse);
	consumer_exit(a->time_slice);
	free(a);
}

int analysis_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);
	assert(buf);

	int r;
	struct analysis * a = (struct analysis *) consumer_get_data(consumer);

	r = consumer_write(a->pulse, buf, count);
	if (r < 0)
		return r;

	r = consumer_write(a->time_slice, buf, count);

	return r;
}

int analysis_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	int r;
	struct analysis * a = (struct analysis *) consumer_get_data(consumer);

	r = consumer_start(a->pulse, sample_rate, ts);
	if (r < 0)
		return r;

	r = consumer_start(a->time_slice, sample_rate, ts);

	return r;
}

int analysis_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	int r;
	struct analysis * a = (struct analysis *) consumer_get_data(consumer);

	r = consumer_resync(a->pulse, ts);
	if (r < 0)
		return r;

	r = consumer_resync(a->time_slice, ts);

	return r;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int analysis_init(struct consumer * consumer, const char * pulse_csv_name,
		const char * time_slice_csv_name,
		const struct pulse_params * pulse_params)
{
	assert(consumer);

	int r;

	struct analysis * a = (struct analysis *)
		calloc(sizeof(struct analysis), 1);
	if (!a) {
		error("analysis: Failed to allocate memory");
		r = -ENOMEM;
		goto err;
	}

	a->pulse = consumer_new();
	if (!a->pulse) {
		error("analysis: Failed to create consumer object for pulse processing");
		r = -1;
		goto err;
	}

	r = pulse_init(a->pulse, pulse_csv_name, pulse_params);
	if (r < 0) {
		error("analysis: Failed to initialise pulse processing");
		goto err;
	}

	a->time_slice = consumer_new();
	if (!a->time_slice) {
		error("analysis: Failed to create consumer object for time slice processing");
		r = -1;
		goto err;
	}

	r = time_slice_init(a->time_slice, time_slice_csv_name, pulse_params->out_mode);
	if (r < 0) {
		error("analysis: Failed to initialise time slice processing");
		goto err;
	}

	consumer_set_module(consumer, analysis_write, analysis_start,
			analysis_resync, analysis_exit, a);

	return 0;

err:
	if (a) {
		if (a->pulse)
			consumer_exit(a->pulse);
		if (a->time_slice)
			consumer_exit(a->time_slice);
		free(a);
	}

	return r;
}
