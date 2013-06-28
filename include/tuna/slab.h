/*******************************************************************************
	slab.h: Slab allocator.

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

#ifndef __TUNA_SLAB_H_INCLUDED__
#define __TUNA_SLAB_H_INCLUDED__

#include <stddef.h>

#include "list.h"
#include "types.h"

struct slab {
	size_t			sz;
	size_t			offset;
	struct list		pages;
	struct list		cache;
};

int slab_init(struct slab * s, size_t element_size, size_t list_entry_offset);
void slab_exit(struct slab * s);
int slab_prealloc(struct slab * s, uint n_pages);
void * slab_alloc(struct slab * s);
void slab_free(struct slab * s, void * p);

#endif /* !__TUNA_SLAB_H_INCLUDED__ */