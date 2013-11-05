/*******************************************************************************
	bufhold.h: List of held buffers.

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

#ifndef __TUNA_BUFHOLD_H_INCLUDED__
#define __TUNA_BUFHOLD_H_INCLUDED__

#include "list.h"
#include "slab.h"
#include "types.h"

struct bufhold {
	struct list		buffers;
	struct slab		allocator;
};

struct held_buffer {
	/* We may be starting at an offset into the buffer due to previous
	 * processing or other reasons. If so, we will have a data pointer which
	 * is different to the base pointer which references the base of the
	 * allocated memory for the buffer. This is because we need to pass the
	 * base address of the allocated buffer to free(), not a data pointer
	 * which may have been subject to an offset.
	 *
	 * Note that count refers to the number of samples actually in the
	 * buffer, beginning at the data pointer, not the total length of the
	 * whole buffer itself.
	 */
	sample_t *		base;
	sample_t *		data;
	uint			count;

	struct list_entry	e;
};

struct held_buffer * bufhold_oldest(struct bufhold * bh);
struct held_buffer * bufhold_newest(struct bufhold * bh);
struct held_buffer * bufhold_next(struct held_buffer * h);
struct held_buffer * bufhold_prev(struct held_buffer * h);
sample_t * bufhold_data(struct held_buffer * h);
uint bufhold_count(struct held_buffer * h);
int bufhold_advance(struct bufhold * bh, struct held_buffer * h, uint offset);
void bufhold_release(struct bufhold * bh, struct held_buffer * h);
void bufhold_release_all(struct bufhold * bh);
void bufhold_add(struct bufhold * bh, sample_t * buf, uint count);
void bufhold_init(struct bufhold * bh);
void bufhold_exit(struct bufhold * bh);

#endif /* !__TUNA_BUFHOLD_H_INCLUDED__ */
