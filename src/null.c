/*******************************************************************************
	null.c: Consumer equivalent of /dev/null.

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
#include <malloc.h>

#include "compiler.h"
#include "log.h"
#include "null.h"
#include "types.h"
#include "uara.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

void null_exit(struct consumer * consumer)
{
	assert(consumer);

	free(consumer);
}

int null_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);
	assert(buf);

	__unused count;
	
	return 0;
}

int null_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	__unused sample_rate;

	return 0;
}

int null_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct consumer * null_init()
{
	struct consumer * c = (struct consumer *)malloc(sizeof(struct consumer));
	if (!c) {
		error("null_init: Failed to allocate memory");
		return NULL;
	}

	c->write = null_write;
	c->start = null_start;
	c->resync = null_resync;
	c->exit = null_exit;

	return c;
}
