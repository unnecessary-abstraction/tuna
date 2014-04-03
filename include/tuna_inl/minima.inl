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

	/* Pop from right while highest min > next. */
	while (t->len) {
		if (t->mins[t->right].value <= next) {
			/* Advance ticker and pop from left if expired. */
			if (++t->ticker == t->windowlen)
				t->ticker = 0;
			if (t->mins[t->left].expiry == t->ticker) {
				if (t->len == 1) {
					t->right = 0;
					t->left = 0;
					t->mins[0].value = next;
					t->mins[0].expiry = t->ticker;
					return next;
				}
				if (++t->left == t->windowlen)
					t->left = 0;
				if (++t->right == t->windowlen)
					t->right = 0;
			} else {
				t->len++;
				if (++t->right == t->windowlen)
					t->right = 0;
			}

			/* Add to the right of the deque. */
			t->mins[t->right].value = next;
			t->mins[t->right].expiry = t->ticker;

			/* Return new minimum. */
			return t->mins[t->left].value;
		}
		if (t->right == 0)
			t->right = t->windowlen;
		t->right--;
		t->len--;
	}

	t->right = 0;
	t->left = 0;

	/* Add to the right of the deque. */
	t->len++;
	t->mins[0].value = next;
	t->mins[0].expiry = t->ticker;

	/* Return new minimum. */
	return next;
}

#if defined(ENABLE_INLINE) && defined(__TUNA_MINIMA_C__)
TUNA_EXTERN_INLINE env_t minima_next(struct minima_tracker *t, env_t next);
#endif /* ENABLE_INLINE && __TUNA_MINIMA_C__ */
#endif /* ENABLE_INLINE || __TUNA_MINIMA_C__ */
#endif /* !__TUNA_MINIMA_INL_INCLUDED__ */
