/*******************************************************************************
	counter.c: Passthrough and count samples.

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

#include "consumer.h"
#include "counter.h"
#include "log.h"
#include "pulse.h"
#include "types.h"

struct counter {
	struct consumer *		target;
	counter_limit_callback_fn	limit_callback;
	void *				arg;

	uint				limit;
	uint				count;
};

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

void counter_exit(struct consumer * consumer)
{
	assert(consumer);

	struct counter * c = (struct counter *) consumer_get_data(consumer);

	msg("counter: Exiting, saw %u samples", c->count);
	free(c);
}

int counter_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);
	assert(buf);

	struct counter * c = (struct counter *) consumer_get_data(consumer);

	if (c->limit_callback && (c->count < c->limit) &&
			(c->count + count >= c->limit)) {
		int r, r2;
		uint prelimit = c->limit - c->count;
		uint postlimit = count - prelimit;

		/* Process samples upto c->limit and trigger the callback. */
		c->count += prelimit;
		r = consumer_write(c->target, buf, prelimit);
		msg("counter: Limit of %u samples hit", c->limit);
		r2 = c->limit_callback(c->arg);

		/* Check whether the write failed (r < 0) or the callback failed
		 * (r2 < 0) and return the error if either of these occurred.
		 * Then check whether the callback succeeded but indicated that
		 * further processing should not be performed (r2 > 0) in which
		 * case, return the number of samples processed so far.
		 */
		if (r < 0)
			return r;
		if (r2 < 0)
			return r2;
		if (r2 > 0)
			return r;

		/* Process samples after c->limit and update counter. */
		buf += prelimit;
		c->count += postlimit;
		r2 = consumer_write(c->target, buf, postlimit);
		if (r2 < 0)
			return r2;

		/* Return the total number of samples processed. */
		return r + r2;
	}

	c->count += count;
	return consumer_write(c->target, buf, count);
}

int counter_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	struct counter * c = (struct counter *) consumer_get_data(consumer);

	return consumer_start(c->target, sample_rate, ts);
}

int counter_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	struct counter * c = (struct counter *) consumer_get_data(consumer);

	return consumer_resync(c->target, ts);
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int counter_init(struct consumer * consumer, struct consumer * target,
		uint limit, counter_limit_callback_fn limit_callback,
		void * arg)
{
	assert(consumer);
	assert(target);

	struct counter * c = (struct counter *)malloc(sizeof(struct counter));
	if (!c) {
		error("counter: Failed to allocate memory");
		return -ENOMEM;
	}

	c->target = target;
	c->limit_callback = limit_callback;
	c->arg = arg;
	c->limit = limit;
	c->count = 0;

	consumer_set_module(consumer, counter_write, counter_start,
			counter_resync, counter_exit, c);

	return 0;
}
