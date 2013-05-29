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

#include <malloc.h>

#include "buffer.h"
#include "compiler.h"
#include "types.h"
#include "uara.h"

struct buffer_head {
	uint		refs;
};

sample_t * buffer_acquire(uint * frames)
{
	uint count = 1<<16;
	size_t sz = sizeof(struct buffer_head) + count * sizeof(sample_t);

	struct buffer_head * h = (struct buffer_head *)malloc(sz);
	if (!h)
		return NULL;

	h->refs = 1;
	sample_t * p = (sample_t *)ptr_offset(h, sizeof(struct buffer_head));

	if (frames)
		*frames = count;
	return p;
}

void buffer_addref(sample_t * p)
{
	struct buffer_head * h = (struct buffer_head *)ptr_offset(p, -sizeof(struct buffer_head));

	h->refs++;
}

void buffer_release(sample_t * p)
{
	struct buffer_head * h = (struct buffer_head *)ptr_offset(p, -sizeof(struct buffer_head));

	h->refs--;

	if (!h->refs)
		free(h);
}
