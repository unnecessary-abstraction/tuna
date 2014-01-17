/*******************************************************************************
	consumer.c: Consumer type.

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
#include "log.h"
#include "types.h"

struct consumer * consumer_new()
{
	struct consumer * consumer;

	consumer = (struct consumer *) calloc(1, sizeof(struct consumer));
	if (!consumer)
		error("consumer: Failed to allocate memory");

	return consumer;
}

void consumer_exit(struct consumer * consumer)
{
	assert(consumer);

	if (consumer->exit)
		consumer->exit(consumer);

	free(consumer);
}

void consumer_set_module(struct consumer * consumer, consumer_write_fn write,
		consumer_start_fn start, consumer_resync_fn resync,
		consumer_exit_fn exit, void * data)
{
	assert(consumer);

	consumer->write = write;
	consumer->start = start;
	consumer->resync = resync;
	consumer->exit = exit;
	consumer->data = data;
}

void * consumer_get_data(struct consumer * consumer)
{
	assert(consumer);

	return consumer->data;
}

int consumer_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);

	if (consumer->write)
		return consumer->write(consumer, buf, count);

	return -ENOSYS;
}

int consumer_start(struct consumer * consumer, uint sample_rate,
		struct timespec * ts)
{
	assert(consumer);

	if (consumer->start)
		return consumer->start(consumer, sample_rate, ts);

	return -ENOSYS;
}

int consumer_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);

	if (consumer->resync)
		return consumer->resync(consumer, ts);

	return -ENOSYS;
}
