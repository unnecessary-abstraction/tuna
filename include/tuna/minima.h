/*******************************************************************************
	minima.h: Moving minimum filtering.

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

#ifndef __TUNA_MINIMA_H_INCLUDED__
#define __TUNA_MINIMA_H_INCLUDED__

#include "types.h"

struct minima {
	sample_t		value;
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

	/* Incrementing counter used to determine when a minima expires.
	 */
	uint			ticker;

	struct minima		mins[];
};

struct minima_tracker * minima_init(uint windowlen);
void minima_exit(struct minima_tracker * t);
sample_t minima_current(struct minima_tracker * t);
int minima_current_age(struct minima_tracker * t);
uint minima_len(struct minima_tracker * t);
sample_t minima_next(struct minima_tracker * t, sample_t next);
void minima_reset(struct minima_tracker * t);

#endif /* !__TUNA_MINIMA_H_INCLUDED__ */