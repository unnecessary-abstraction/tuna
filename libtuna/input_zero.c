/*******************************************************************************
	input_zero.c: /dev/zero as a producer.

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
#include <errno.h>
#include <malloc.h>
#include <string.h>

#include "buffer.h"
#include "compiler.h"
#include "consumer.h"
#include "input_zero.h"
#include "log.h"
#include "producer.h"
#include "types.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

struct input_zero {
	struct producer		producer;
	struct consumer *	consumer;

	uint			sample_rate;
	volatile int		stop;
};

int input_zero_run(struct producer * producer)
{
	assert(producer);

	int		r;
	uint		frames;
	struct timespec ts;
	sample_t *	buf;

	struct input_zero * z = container_of(producer, struct input_zero, producer);

	memset(&ts, 0, sizeof(struct timespec));

	r = z->consumer->start(z->consumer, z->sample_rate, &ts);
	if (r < 0) {
		error("input_zero: consumer->start failed");
		return r;
	}

	while (1) {
		/* Check for termination signal. */
		if (z->stop) {
			msg("input_zero: Stop");
			return z->stop;
		}

		frames = 1<<16;
		buf = buffer_acquire(&frames);
		if (!buf) {
			error("input_zero: Failed to acquire buffer");
			return -ENOMEM;
		}

		memset(buf, 0, sizeof(*buf));
		
		r = z->consumer->write(z->consumer, buf, frames);
		if (r < 0) {
			error("input_zero: consumer->write failed");
			return r;
		}

		buffer_release(buf);
	}
}

void input_zero_exit(struct producer * producer)
{
	assert(producer);

	struct input_zero * z = container_of(producer, struct input_zero, producer);

	free(z);
}

int input_zero_stop(struct producer * producer, int condition)
{
	assert(producer);

	struct input_zero * z = container_of(producer, struct input_zero, producer);
	z->stop = condition;

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct producer * input_zero_init(struct consumer * c, uint sample_rate)
{
	assert(c);

	struct input_zero * z = (struct input_zero *)malloc(sizeof(struct input_zero));
	if (!z) {
		error("input_zero: Failed to allocate memory");
		return NULL;
	}

	z->stop = 0;
	z->sample_rate = sample_rate;
	z->consumer = c;

	z->producer.run = input_zero_run;
	z->producer.exit = input_zero_exit;
	z->producer.stop = input_zero_stop;

	return &z->producer;
}
