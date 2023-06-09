/*******************************************************************************
	buffer.c: Sample buffering.

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
#include <stdlib.h>

#include "buffer.h"
#include "compiler.h"
#include "types.h"

struct buffer_head {
	uint		refs;
	sample_t	data		__attribute__ ((aligned(16)));
};

/* Acquire a buffer of at least (*frames) samples. The actual number of samples
 * which can be stored in the buffer is written back to (*frames).
 *
 * Buffers currently aren't cached so we will allocate exactly the number of
 * frames requested.
 */
sample_t * buffer_acquire(uint * frames)
{
	size_t sz;
	struct buffer_head * h;
	int r;

	assert(frames);
	sz = sizeof(struct buffer_head) + (*frames) * sizeof(sample_t);

	r = posix_memalign((void **)&h, 16, sz);
	if (r)
		return NULL;

	h->refs = 1;
	return &h->data;
}

void buffer_addref(sample_t * p)
{
	assert(p);
	struct buffer_head * h = container_of(p, struct buffer_head, data);

	h->refs++;
}

int buffer_release(sample_t * p)
{
	assert(p);
	struct buffer_head * h = container_of(p, struct buffer_head, data);

	h->refs--;

	if (!h->refs) {
		free(h);
		return 1;
	}

	return 0;
}

uint buffer_refcount(sample_t * p)
{
	assert(p);
	struct buffer_head * h = container_of(p, struct buffer_head, data);

	return h->refs;
}
