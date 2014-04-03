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

#include "minima.h"
#include "types.h"

struct onset_threshold {
	struct minima_tracker *		min;

	env_t				ratio;
};

TUNA_INLINE env_t onset_threshold_next(struct onset_threshold * onset,
		env_t env)
{
	assert(onset);

	env_t min;

	/* Track minima and calculate new threshold. */
	min = minima_next(onset->min, env);
	return min * onset->ratio;
}

#if defined(ENABLE_INLINE) && defined(__TUNA_ONSET_THRESHOLD_C__)
TUNA_EXTERN_INLINE env_t onset_threshold_next(struct onset_threshold * onset,
		env_t env);
#endif /* ENABLE_INLINE && __TUNA_ONSET_THRESHOLD_C__ */
#endif /* ENABLE_INLINE || __TUNA_ONSET_THRESHOLD_C__ */
#endif /* !__TUNA_ONSET_THRESHOLD_INL_INCLUDED__ */
