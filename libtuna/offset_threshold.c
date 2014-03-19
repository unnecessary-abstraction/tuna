/*******************************************************************************
	offset_threshold.c: Offset detection threshold tracking.

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
#include <math.h>
#include <stddef.h>

#include "cbuf.h"
#include "log.h"
#include "offset_threshold.h"
#include "types.h"

struct offset_threshold {
	struct cbuf *			delay_line;

	env_t				current_min;
	env_t				delayed_min;
	env_t				limit;
	env_t				ratio;
	env_t				threshold;
};

struct offset_threshold * offset_threshold_init(float Td, uint sample_rate,
		env_t ratio)
{
	uint Td_w;

	struct offset_threshold * o = (struct offset_threshold *)
		malloc(sizeof(struct offset_threshold));
	if (!o) {
		error("offset_threshold: Failed to allocate memory");
		return NULL;
	}

	Td_w = (uint) floor(Td * sample_rate);
	o->delay_line = cbuf_init(Td_w);
	if (!o->delay_line) {
		error("offset_threshold: Failed to initialise delay line");
		free(o);
		return NULL;
	}

	o->ratio = ratio;
	o->limit = ENV_MAX / ratio;

	return o;
}

void offset_threshold_exit(struct offset_threshold * o)
{
	assert(o);

	if (o->delay_line)
		cbuf_exit(o->delay_line);
	free(o);
}

env_t offset_threshold_next(struct offset_threshold * o, env_t env)
{
	assert(o);

	env_t old = cbuf_rotate(o->delay_line, env);

	/* Update delayed minimum. */
	if (old < o->delayed_min)
		o->delayed_min = old;

	/* Update current minimum and threshold. */
	if (env < o->current_min) {
		o->current_min = env;
		if (env <= o->limit)
			o->threshold = env * o->ratio;
		else
			/* Set threshold such that delayed_min is never less
			 * than the threshold.
			 */
			o->threshold = 0;
	}

	return o->threshold;
}

void offset_threshold_reset(struct offset_threshold * o, env_t env)
{
	assert(o);

	cbuf_reset(o->delay_line);

	o->delayed_min = env;
	o->current_min = env;

	if (env <= o->limit)
		o->threshold = env * o->ratio;
	else
		/* Set threshold such that delayed_min is never less than the
		 * threshold.
		 */
		o->threshold = 0;
}

env_t offset_threshold_delayed_min(struct offset_threshold * o)
{
	assert(o);

	return o->delayed_min;
}
