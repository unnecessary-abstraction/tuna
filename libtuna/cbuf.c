/*******************************************************************************
	cbuf.c: Circular buffering.

	Copyright (C) 2013,2014 Paul Barker, Loughborough University
	
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
#include <stddef.h>
#include <string.h>

#include "cbuf.h"
#include "log.h"
#include "types.h"

struct cbuf {
	uint index;
	uint len;

	sample_t data[];
};

struct cbuf * cbuf_init(uint len)
{
	/* Use calloc so that the index and all data values are initialised to
	 * zero.
	 */
	struct cbuf * c = calloc(1, sizeof(struct cbuf) + len * sizeof(sample_t));
	if (!c) {
		error("cbuf_init: Failed to allocate memory");
		return NULL;
	}

	c->len = len;

	return c;
}

void cbuf_reset(struct cbuf * c)
{
	assert(c);

	c->index = 0;
	memset(c->data, 0, c->len * sizeof(sample_t));
}

sample_t cbuf_index(struct cbuf * c, uint i)
{
	assert(c);

	return c->data[(c->len + c->index - i) % c->len];
}

sample_t cbuf_get(struct cbuf * c)
{
	assert(c);

	return c->data[c->index];
}

void cbuf_put(struct cbuf * c, sample_t s)
{
	assert(c);

	c->data[c->index] = s;
	c->index = (c->index + 1) % c->len;
}

sample_t cbuf_rotate(struct cbuf *c, sample_t s)
{
	assert(c);

	sample_t tmp;

	tmp = c->data[c->index];
	c->data[c->index] = s;
	c->index = (c->index + 1) % c->len;

	return tmp;
}

void cbuf_exit(struct cbuf * c)
{
	assert(c);

	free(c);
}
