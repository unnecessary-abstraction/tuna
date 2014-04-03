/*******************************************************************************
	minima.c: Moving minimum filtering.

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

/* Uses ascending minima tracking algorithm from Richard Harter, 2001.
 *
 * http://richardhartersworld.com/cri/2001/slidingmin.html
 */

#define __TUNA_MINIMA_C__

#include <assert.h>
#include <malloc.h>
#include <stddef.h>

#include "minima.h"
#include "log.h"
#include "types.h"

struct minima_tracker * minima_init(uint windowlen)
{
	struct minima_tracker * t = (struct minima_tracker *)
		malloc(sizeof(struct minima_tracker) + windowlen *
				sizeof(struct minima));

	if (!t) {
		error("minima_init: Failed to allocate memory");
		return NULL;
	}

	minima_reset(t);
	t->windowlen = windowlen;

	return t;
}

void minima_exit(struct minima_tracker * t)
{
	assert(t);

	free(t);
}

env_t minima_current(struct minima_tracker * t)
{
	assert(t);

	/* Check whether there is valid data in the minima tracker. As we're
	 * storing minima of energy levels and these are always positive, we can
	 * return a negative value to indicate an error.
	 */
	if (t->len == 0)
		return -1;

	return t->mins[t->left].value;
}

/* Distance from current minimum to current sample. */
int minima_current_age(struct minima_tracker * t)
{
	assert(t);
	uint life;

	/* As age is by definition always positive, we can return -1 on error.
	 */
	if (t->len == 0)
		return -1;

	/* Determine the remaining lifetime of the current minima and then
	 * subtract this from the window length.
	 */
	life = ((t->windowlen + t->mins[t->left].expiry) - (t->ticker + 1))
		% t->windowlen;
	return t->windowlen - (life + 1);
}

void minima_reset(struct minima_tracker * t)
{
	assert(t);

	t->left = 0;
	t->right = 0;
	t->ticker = 0;
	t->len = 0;
	t->mins[t->left].expiry = 0;
}
