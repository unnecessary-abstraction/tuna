/*******************************************************************************
	onset_threshold.inl: Onset detection threshold calculation.

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

#ifndef __TUNA_ONSET_THRESHOLD_INL_INCLUDED__
#define __TUNA_ONSET_THRESHOLD_INL_INCLUDED__

#if defined(ENABLE_INLINE) || defined(__TUNA_ONSET_THRESHOLD_C__)

#include <assert.h>

#include "types.h"

struct minima {
	env_t			value;
	uint			expiry;
};

struct onset_threshold {
	/* Ratio to multiply the output of the moving minimum filter by to get
	 * the onset threshold.
	 */
	env_t			ratio;

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

TUNA_INLINE void onset_threshold_next(struct onset_threshold * onset,
		env_t next, env_t * threshold)
{
	assert(onset);
	assert(threshold);

	/* Pop from right while highest min > next. */
	while (onset->len > 1) {
		if (onset->mins[onset->right].value <= next) {
			/* Advance ticker and pop from left if expired. */
			if (onset->mins[onset->left].expiry == ++onset->ticker) {
				if (++onset->left == onset->windowlen)
					onset->left = 0;
				*threshold = onset->mins[onset->left].value * onset->ratio;
			} else
				onset->len++;

			if (++onset->right == onset->windowlen)
				onset->right = 0;

			/* Add to the right of the deque. */
			onset->mins[onset->right].value = next;
			onset->mins[onset->right].expiry = onset->ticker + onset->windowlen;

			return;
		}
		if (onset->right == 0)
			onset->right = onset->windowlen;
		onset->right--;
		onset->len--;
	}

	if (onset->mins[onset->right].value <= next) {
		/* Advance ticker and pop from left if expired. */
		if (onset->mins[onset->left].expiry == ++onset->ticker) {
			onset->right = 0;
			onset->left = 0;
			onset->mins[0].value = next;
			onset->mins[0].expiry = onset->ticker + onset->windowlen;
			*threshold = next * onset->ratio;
			return;
		}

		onset->len = 2;
		if (++onset->right == onset->windowlen)
			onset->right = 0;

		/* Add to the right of the deque. */
		onset->mins[onset->right].value = next;
		onset->mins[onset->right].expiry = onset->ticker + onset->windowlen;
		return;
	}

	onset->right = 0;
	onset->left = 0;

	/* Add to the right of the deque. */
	onset->mins[0].value = next;
	onset->mins[0].expiry = onset->ticker + onset->windowlen;

	*threshold = next * onset->ratio;
}

#if defined(ENABLE_INLINE) && defined(__TUNA_ONSET_THRESHOLD_C__)
TUNA_EXTERN_INLINE void onset_threshold_next(struct onset_threshold * onset,
		env_t env, env_t * threshold);
#endif /* ENABLE_INLINE && __TUNA_ONSET_THRESHOLD_C__ */
#endif /* ENABLE_INLINE || __TUNA_ONSET_THRESHOLD_C__ */
#endif /* !__TUNA_ONSET_THRESHOLD_INL_INCLUDED__ */
