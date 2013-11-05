/*******************************************************************************
	bufhold.c: List of held buffers.

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
#include <stddef.h>

#include "buffer.h"
#include "bufhold.h"
#include "compiler.h"
#include "list.h"
#include "log.h"
#include "slab.h"
#include "types.h"

struct held_buffer * bufhold_oldest(struct bufhold * bh)
{
	assert(bh);
	struct list_entry * e = list_head(&bh->buffers);

	if (e)
		return container_of(e, struct held_buffer, e);
	else
		return NULL;
}

struct held_buffer * bufhold_newest(struct bufhold * bh)
{
	assert(bh);
	struct list_entry * e = list_tail(&bh->buffers);

	if (e)
		return container_of(e, struct held_buffer, e);
	else
		return NULL;
}

struct held_buffer * bufhold_next(struct held_buffer * h)
{
	assert(h);
	struct list_entry * e = list_next(&h->e);

	if (e)
		return container_of(e, struct held_buffer, e);
	else
		return NULL;
}

struct held_buffer * bufhold_prev(struct held_buffer * h)
{
	assert(h);
	struct list_entry * e = list_prev(&h->e);

	if (e)
		return container_of(e, struct held_buffer, e);
	else
		return NULL;
}

sample_t * bufhold_data(struct held_buffer * h)
{
	assert(h);

	return h->data;
}

uint bufhold_count(struct held_buffer * h)
{
	assert(h);

	return h->count;
}

int bufhold_advance(struct bufhold * bh, struct held_buffer * h, uint offset)
{
	assert(h);

	if (offset <= h->count) {
		h->count -= offset;
		h->data += offset;
		return h->count;
	} else {
		/* This buffer is no longer needed. */
		bufhold_release(bh, h);
		return -1;
	}
}

void bufhold_release(struct bufhold * bh, struct held_buffer * h)
{
	assert(bh);
	assert(h);

	list_remove(&h->e);
	buffer_release(h->base);
	slab_free(&bh->allocator, h);
}

void bufhold_release_all(struct bufhold * bh)
{
	assert(bh);

	struct held_buffer * h;
	struct list_entry * e;

	while ((e = list_pop(&bh->buffers))) {
		h = container_of(e, struct held_buffer, e);
		buffer_release(h->base);
		slab_free(&bh->allocator, h);
	}
}

void bufhold_add(struct bufhold * bh, sample_t * buf, uint count)
{
	assert(bh);
	assert(buf);

	struct held_buffer * h = (struct held_buffer *)slab_alloc(&bh->allocator);

	h->base = buf;
	h->data = buf;
	h->count = count;
	buffer_addref(buf);
	list_enqueue(&bh->buffers, &h->e);
}

void bufhold_init(struct bufhold * bh)
{
	assert(bh);

	list_init(&bh->buffers);
	slab_init(&bh->allocator, sizeof(struct held_buffer), offsetof(struct held_buffer, e));
}

void bufhold_exit(struct bufhold * bh)
{
	assert(bh);

	if (list_head(&bh->buffers))
		error("bufhold: Destroying a non-empty bufhold will leak memory");

	list_exit(&bh->buffers);
	slab_exit(&bh->allocator);
}
