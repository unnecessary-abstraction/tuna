/*******************************************************************************
	offset_threshold.inl: Onset detection threshold calculation.

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

#ifndef __TUNA_OFFSET_THRESHOLD_INL_INCLUDED__
#define __TUNA_OFFSET_THRESHOLD_INL_INCLUDED__

#if defined(ENABLE_INLINE) || defined(__TUNA_OFFSET_THRESHOLD_C__)

#include <assert.h>

#include "cbuf.h"
#include "types.h"

struct offset_threshold {
	struct cbuf *			delay_line;

	env_t				current_min;
	env_t				delayed_min;
	env_t				ratio;
	env_t				threshold;
};

TUNA_INLINE env_t offset_threshold_next(struct offset_threshold * o, env_t env)
{
	assert(o);

	env_t old = cbuf_rotate(o->delay_line, env);

	/* Update delayed minimum. */
	if (old < o->delayed_min)
		o->delayed_min = old;

	/* Update current minimum and threshold. */
	if (env < o->current_min) {
		o->current_min = env;
		o->threshold = env * o->ratio;
	}

	return o->threshold;
}

#if defined(ENABLE_INLINE) && defined(__TUNA_OFFSET_THRESHOLD_C__)
TUNA_EXTERN_INLINE env_t offset_threshold_next(struct offset_threshold * o,
		env_t env);
#endif /* ENABLE_INLINE && __TUNA_OFFSET_THRESHOLD_C__ */
#endif /* ENABLE_INLINE || __TUNA_OFFSET_THRESHOLD_C__ */
#endif /* !__TUNA_OFFSET_THRESHOLD_INL_INCLUDED__ */
