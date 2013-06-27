/*******************************************************************************
	slab.c: Slab allocator.

	Copyright (C) 2013 Paul Barker
	
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

#include "list.h"
#include "log.h"
#include "slab.h"
#include "types.h"

const size_t page_size = 1<<12;		/* 4 KiB */
const size_t rounding = 1<<4;		/* 16 B */

struct page_header {
	struct list_entry	e;

	/* Number of elements on this page that are in use. */
	uint			allocated;
};

int slab_init(struct slab * s, size_t element_size, size_t list_entry_offset)
{
	assert(s);

	/* Currently don't support large items. */
	assert(element_size <= page_size / 2);

	s->sz = element_size;
	s->offset = list_entry_offset;

	list_init(&s->pages);
	list_init(&s->cache);

	return 0;
}

void slab_exit(struct slab * s)
{
	assert(s);

	list_exit(&s->pages);
	list_exit(&s->cache);
}

int slab_prealloc(struct slab * s, uint n_pages)
{
	struct page_header * h;
	struct list_entry * e;
	void * p;
	void * base;
	uint i, j;
	size_t first_offset;
	uint n_elements;

	assert(s);

	p = malloc(n_pages * page_size);
	if (!p)
		return -1;

	/*
		Calculate offset of the first element within a page and the
		number of elements per page. The first offset is the size of the
		page header struct rounded up to a multiple of rounding.
	*/
	first_offset = (sizeof(struct page_header) + (rounding-1)) & ~(rounding-1);
	n_elements = (page_size - first_offset) / s->sz;

	for (i = 0; i < n_pages; i++) {
		h = (struct page_header *)(p + i * page_size);
		list_append(&s->pages, &h->e);

		h->allocated = 0;

		base = p + i * page_size + first_offset;
		for (j = 0; j < n_elements; j++) {
			e = (struct list_entry *)(base + j * s->sz + s->offset);
			list_append(&s->cache, e);
		}
	}

	return 0;
}

void * slab_alloc(struct slab * s)
{
	struct list_entry * e;
	struct page_header * h;

	assert(s);

	if (list_is_empty(&s->cache)) {
		/* Need to allocate new elements */
		slab_prealloc(s, 1);

		if (list_is_empty(&s->cache)) {
			error("slab: Failed to allocate elements");
			return NULL;
		}
	}

	/* We ensured the list isn't empty above so list_pop() will not return
	 * NULL.
	 */
	e = list_pop(&s->cache);
	
	/* Find the header for the page containing e. */
	h = (struct page_header *)ptr_truncate(e, page_size);
	h->allocated++;

	return ((void *)e) - s->offset;
}

void slab_free(struct slab * s, void * p)
{
	struct list_entry * e;
	struct page_header * h;

	assert(s);

	e = (struct list_entry *)(p + s->offset);
	
	/* Find the header for the page containing e. */
	h = (struct page_header *)ptr_truncate(e, page_size);
	h->allocated--;

	list_prepend(&s->cache, e);
}
