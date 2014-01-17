/*******************************************************************************
	producer.c: Producer type.

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

#include "log.h"
#include "producer.h"

struct producer * producer_new()
{
	struct producer * producer;

	producer = (struct producer *) calloc(1, sizeof(struct producer));
	if (!producer)
		error("producer: Failed to allocate memory");

	return producer;
}

void producer_exit(struct producer * producer)
{
	assert(producer);

	if (producer->exit)
		producer->exit(producer);

	free(producer);
}

void producer_set_module(struct producer * producer, producer_run_fn run,
		producer_stop_fn stop, producer_exit_fn exit, void * data)
{
	assert(producer);

	producer->run = run;
	producer->stop = stop;
	producer->exit = exit;
	producer->data = data;
}

void * producer_get_data(struct producer * producer)
{
	assert(producer);

	return producer->data;
}

int producer_run(struct producer * producer)
{
	assert(producer);

	if (producer->run)
		return producer->run(producer);

	return -ENOSYS;
}

int producer_stop(struct producer * producer, int condition)
{
	assert(producer);

	if (producer->stop)
		return producer->stop(producer, condition);

	return -ENOSYS;
}
