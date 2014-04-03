/*******************************************************************************
	env_estimate.inl: Sample-based peak envelope estimation.

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

#ifndef __TUNA_ENV_ESTIMATE_INL_INCLUDED__
#define __TUNA_ENV_ESTIMATE_INL_INCLUDED__

#if defined(ENABLE_INLINE) || defined(__TUNA_ENV_ESTIMATE_C__)

#include <assert.h>
#include <math.h>

#include "types.h"

struct env_estimate {
	float		decay;
	float		cur;
};

TUNA_INLINE env_t env_estimate_next(struct env_estimate * e, sample_t x)
{
	assert(e);

	float next = fabs(x);
	float decayed = e->decay * e->cur;

	e->cur = (decayed > next) ? decayed : next;
	return (env_t)e->cur;
}

#if defined(ENABLE_INLINE) && defined(__TUNA_ENV_ESTIMATE_C__)
TUNA_EXTERN_INLINE env_t env_estimate_next(struct env_estimate * e, sample_t x);
#endif /* ENABLE_INLINE && __TUNA_ENV_ESTIMATE_C__ */
#endif /* ENABLE_INLINE || __TUNA_ENV_ESTIMATE_C__ */
#endif /* !__TUNA_ENV_ESTIMATE_INL_INCLUDED__ */
