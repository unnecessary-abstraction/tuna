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

sample_t minima_current(struct minima_tracker * t)
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

	/* As age is by definition always positive, we can return -1 on error.
	 */
	if (t->len == 0)
		return -1;

	return t->windowlen - (t->mins[t->left].expiry - t->ticker);
}

uint minima_len(struct minima_tracker * t)
{
	assert(t);

	return t->len;
}

sample_t minima_next(struct minima_tracker * t, sample_t next)
{
	assert(t);

	/* Advance ticker and pop from left if expired. */
	t->ticker++;
	if (t->len && (t->mins[t->left].expiry == t->ticker)) {
		t->left = (t->left + 1) % t->windowlen;
		t->len--;
	}

	/* Pop from right while highest min > next. */
	while (t->len && (t->mins[t->right].value > next)) {
		t->right = (t->windowlen + t->right - 1) % t->windowlen;
		t->len--;
	}

	/* Add to right. */
	if (t->len)
		t->right = (t->right + 1) % t->windowlen;
	t->len++;
	t->mins[t->right].value = next;
	t->mins[t->right].expiry = t->ticker + t->windowlen;

	/* Return new minimum. */
	return t->mins[t->left].value;
}

void minima_reset(struct minima_tracker * t)
{
	assert(t);

	t->left = 0;
	t->right = 0;
	t->ticker = 0;
	t->len = 0;
}