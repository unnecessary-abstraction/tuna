/*******************************************************************************
	offset_threshold.c: Offset detection threshold tracking.

	Copyright (C) 2014 Paul Barker, Loughborough University

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

#define __TUNA_OFFSET_THRESHOLD_C__

#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stddef.h>

#include "cbuf.h"
#include "log.h"
#include "offset_threshold.h"
#include "types.h"

struct offset_threshold * offset_threshold_init(env_t ratio)
{
	struct offset_threshold * o = (struct offset_threshold *)
		malloc(sizeof(struct offset_threshold));
	if (!o) {
		error("offset_threshold: Failed to allocate memory");
		return NULL;
	}

	o->ratio = ratio;

	return o;
}

void offset_threshold_exit(struct offset_threshold * o)
{
	assert(o);

	free(o);
}

void offset_threshold_reset(struct offset_threshold * o, env_t env)
{
	assert(o);

	o->threshold = env * o->ratio;
}
