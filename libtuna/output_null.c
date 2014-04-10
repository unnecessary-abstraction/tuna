/*******************************************************************************
	output_null.c: Consumer equivalent of /dev/null.

	Copyright (C) 2013, 2014 Paul Barker, Loughborough University

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

#include "compiler.h"
#include "consumer.h"
#include "log.h"
#include "output_null.h"
#include "types.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

void output_null_exit(struct consumer * consumer)
{
	assert(consumer);

	/* Nothing to do. */
	__unused consumer;
}

int output_null_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);
	assert(buf);

	__unused consumer;
	__unused buf;
	__unused count;
	
	return 0;
}

int output_null_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	__unused consumer;
	__unused ts;
	__unused sample_rate;

	return 0;
}

int output_null_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	__unused consumer;
	__unused ts;

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int output_null_init(struct consumer * consumer)
{
	assert(consumer);

	consumer_set_module(consumer, output_null_write, output_null_start,
			output_null_resync, output_null_exit, NULL);

	return 0;
}
