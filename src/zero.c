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

#include <malloc.h>
#include <string.h>

#include "buffer.h"
#include "compiler.h"
#include "log.h"
#include "types.h"
#include "uara.h"

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
	uint		frames;
	struct timespec ts;
	sample_t *	buf;

	struct zero * z = container_of(producer, struct zero, producer);

	memset(&ts, 0, sizeof(struct timespec));
	z->consumer->start(z->consumer, z->sample_rate, &ts);

	while (1) {
		frames = 1<<16;
		buf = buffer_acquire(&frames);
		if (!buf) {
			error("zero: Failed to acquire buffer");
			return -1;
		}

		memset(buf, 0, sizeof(*buf));
		
		z->consumer->write(z->consumer, buf, frames);

		buffer_release(buf);
	}
}

void zero_exit(struct producer * producer)
{
	struct zero * z = container_of(producer, struct zero, producer);

	free(z);
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct producer * zero_init(uint sample_rate, struct consumer * c)
{
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
