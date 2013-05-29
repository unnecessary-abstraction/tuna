/*******************************************************************************
	bufq.c: Buffer queue to decouple input from output.

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

#include "compiler.h"
#include "log.h"
#include "uara.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

struct bufq {
	struct consumer		consumer;

	struct consumer *	target;
};

void bufq_exit(struct consumer * consumer)
{
	struct bufq * b = container_of(consumer, struct bufq, consumer);

	free(b);
}

int bufq_write(struct consumer * consumer, sample_t * buf, uint count)
{
	struct bufq * b = container_of(consumer, struct bufq, consumer);

	/* TODO */
	__unused b;
	__unused buf;
	__unused count;

	return 0;
}

int bufq_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	struct bufq * b = container_of(consumer, struct bufq, consumer);

	/* TODO */
	__unused b;
	__unused sample_rate;
	__unused ts;

	return 0;
}

int bufq_resync(struct consumer * consumer, struct timespec * ts)
{
	struct bufq * b = container_of(consumer, struct bufq, consumer);

	/* TODO */
	__unused b;
	__unused ts;

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct consumer * bufq_init(struct consumer * target)
{
	struct bufq * b = (struct bufq *)malloc(sizeof(struct bufq));
	if (!b) {
		error("bufq_init: Failed to allocate memory");
		return NULL;
	}

	b->target = target;

	/* Setup consumer and return. */
	b->consumer.write = bufq_write;
	b->consumer.start = bufq_start;
	b->consumer.resync = bufq_resync;
	b->consumer.exit = bufq_exit;

	return &b->consumer;
}
