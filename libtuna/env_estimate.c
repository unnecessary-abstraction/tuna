/*******************************************************************************
	env_estimate.c: Sample-based peak envelope estimation.

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

#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stddef.h>

#include "env_estimate.h"
#include "log.h"
#include "types.h"

struct env_estimate {
	float		decay;
	float		cur;
};

static inline float maxf(float a, float b)
{
	return (a > b) ? a : b;
}

struct env_estimate * env_estimate_init(float Tc, uint sample_rate)
{
	struct env_estimate * e;

	e = (struct env_estimate *) malloc(sizeof(struct env_estimate));
	if (!e) {
		error("env_estimate: Failed to allocate memory");
		return NULL;
	}

	e->decay = expf(-1.0f / (Tc * sample_rate));
	e->cur = 0;

	return e;
}

void env_estimate_exit(struct env_estimate * e)
{
	assert(e);

	free(e);
}

void env_estimate_reset(struct env_estimate * e)
{
	assert(e);

	e->cur = 0;
}

sample_t env_estimate_next(struct env_estimate * e, sample_t x)
{
	assert(e);

	float f = fabs(x);
	e->cur = maxf(e->decay * e->cur, f);
	return (sample_t)e->cur;
}
