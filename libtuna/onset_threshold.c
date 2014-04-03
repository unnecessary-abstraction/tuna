/*******************************************************************************
	onset_threshold.c: Onset detection threshold tracking.

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

#define __TUNA_ONSET_THRESHOLD_C__

#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stddef.h>

#include "log.h"
#include "minima.h"
#include "onset_threshold.h"
#include "types.h"

struct onset_threshold * onset_threshold_init(float Tw, uint sample_rate,
		env_t ratio)
{
	struct onset_threshold * onset = (struct onset_threshold *)
		malloc(sizeof(struct onset_threshold));
	if (!onset) {
		error("onset_threshold: Failed to allocate memory");
		return NULL;
	}

	uint Tw_w = (uint) floor(Tw * sample_rate);
	onset->min = minima_init(Tw_w);
	if (!onset->min) {
		error("onset_threshold: Failed to initialise minima tracker");
		free(onset);
		return NULL;
	}

	onset->ratio = ratio;

	return onset;
}

void onset_threshold_exit(struct onset_threshold * onset)
{
	assert(onset);

	if (onset->min)
		minima_exit(onset->min);
	free(onset);
}

void onset_threshold_reset(struct onset_threshold * onset)
{
	assert(onset);

	minima_reset(onset->min);
}

uint onset_threshold_age(struct onset_threshold * onset)
{
	assert(onset);

	return minima_current_age(onset->min);
}
