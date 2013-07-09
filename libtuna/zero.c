/*******************************************************************************
	zero.c: /dev/zero as a producer.

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
#include "log.h"
#include "producer.h"
#include "types.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

struct zero {
	struct producer		producer;
	struct consumer *	consumer;

	uint			sample_rate;
};

int zero_run(struct producer * producer)
{
	assert(producer);

	int		r;
	uint		frames;
	struct timespec ts;
	sample_t *	buf;

	struct zero * z = container_of(producer, struct zero, producer);

	memset(&ts, 0, sizeof(struct timespec));

	r = z->consumer->start(z->consumer, z->sample_rate, &ts);
	if (r < 0) {
		error("zero: consumer->start failed");
		return r;
	}

	while (1) {
		frames = 1<<16;
		buf = buffer_acquire(&frames);
		if (!buf) {
			error("zero: Failed to acquire buffer");
			return -ENOMEM;
		}

		memset(buf, 0, sizeof(*buf));
		
		r = z->consumer->write(z->consumer, buf, frames);
		if (r < 0) {
			error("zero: consumer->write failed");
			return r;
		}

		buffer_release(buf);
	}
}

void zero_exit(struct producer * producer)
{
	assert(producer);

	struct zero * z = container_of(producer, struct zero, producer);

	free(z);
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct producer * zero_init(uint sample_rate, struct consumer * c)
{
	assert(c);

	struct zero * z = (struct zero *)malloc(sizeof(struct zero));
	if (!z) {
		error("zero: Failed to allocate memory");
		return NULL;
	}

	z->sample_rate = sample_rate;
	z->consumer = c;

	z->producer.run = zero_run;
	z->producer.exit = zero_exit;

	return &z->producer;
}
