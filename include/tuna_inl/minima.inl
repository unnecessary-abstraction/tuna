/*******************************************************************************
	minima.inl: Moving minimum filtering.

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

#ifndef __TUNA_MINIMA_INL_INCLUDED__
#define __TUNA_MINIMA_INL_INCLUDED__

#if defined(ENABLE_INLINE) || defined(__TUNA_MINIMA_C__)

#include <assert.h>

#include "minima.h"
#include "types.h"

struct minima {
	env_t			value;
	uint			expiry;
};

struct minima_tracker {
	/* Indices of left and right ends of the queue. Data at mins[left] and
	 * mins[right] is valid as long as len > 0.
	 */
	uint			left;
	uint			right;

	/* Number of valid minima in the deque. */
	uint			len;

	/* Length of the analysis window and therefore length of the circular
	 * buffer allocated.
	 */
	uint			windowlen;

	/* Incrementing counter used to determine when a minima expires. This
	 * counts modulo $windowlen.
	 */
	uint			ticker;

	struct minima		mins[];
};

TUNA_INLINE env_t minima_next(struct minima_tracker * t, env_t next)
{
	assert(t);

	/* Advance ticker and pop from left if expired. */
	t->ticker = (t->ticker + 1) % t->windowlen;
	if (t->len && (t->mins[t->left].expiry == t->ticker)) {
		t->left = (t->left + 1) % t->windowlen;
		t->len--;
	}

	/* Pop from right while highest min > next. */
	while (t->len && (t->mins[t->right].value > next)) {
		t->right = (t->windowlen + t->right - 1) % t->windowlen;
		t->len--;
	}

	/* If the deque of ascending minima has been emptied, reset right and
	 * left to be equal. Otherwise, we need to advance the right index.
	 */
	if (!t->len)
		t->right = t->left;
	else if (t->len)
		t->right = (t->right + 1) % t->windowlen;

	/* Add to the right of the deque. */
	t->len++;
	t->mins[t->right].value = next;
	t->mins[t->right].expiry = t->ticker;

	/* Return new minimum. */
	return t->mins[t->left].value;
}

#if defined(ENABLE_INLINE) && defined(__TUNA_MINIMA_C__)
TUNA_EXTERN_INLINE env_t minima_next(struct minima_tracker *t, env_t next);
#endif /* ENABLE_INLINE && __TUNA_MINIMA_C__ */
#endif /* ENABLE_INLINE || __TUNA_MINIMA_C__ */
#endif /* !__TUNA_MINIMA_INL_INCLUDED__ */
