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

#include "buffer.h"
#include "compiler.h"
#include "types.h"

struct buffer_head {
	uint		refs;
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

	assert(frames);
	sz = sizeof(struct buffer_head) + (*frames) * sizeof(sample_t);

	h = (struct buffer_head *)malloc(sz);
	if (!h)
		return NULL;

	h->refs = 1;
	sample_t * p = (sample_t *)ptr_offset(h, sizeof(struct buffer_head));

	return p;
}

void buffer_addref(sample_t * p)
{
	assert(p);
	struct buffer_head * h = (struct buffer_head *)ptr_offset(p, -sizeof(struct buffer_head));

	h->refs++;
}

void buffer_release(sample_t * p)
{
	assert(p);
	struct buffer_head * h = (struct buffer_head *)ptr_offset(p, -sizeof(struct buffer_head));

	h->refs--;

	if (!h->refs)
		free(h);
}
