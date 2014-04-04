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
#include "onset_threshold.h"
#include "types.h"

struct onset_threshold * onset_threshold_init(float Tw, uint sample_rate,
		env_t ratio)
{
	uint Tw_w = (uint) floor(Tw * sample_rate);

	struct onset_threshold * onset = (struct onset_threshold *)
		malloc(sizeof(struct onset_threshold) + Tw_w * sizeof(struct minima));
	if (!onset) {
		error("onset_threshold: Failed to allocate memory");
		return NULL;
	}

	onset->windowlen = Tw_w;
	onset->ratio = ratio;
	onset_threshold_reset(onset);

	return onset;
}

void onset_threshold_exit(struct onset_threshold * onset)
{
	assert(onset);

	free(onset);
}

void onset_threshold_reset(struct onset_threshold * onset)
{
	assert(onset);

	onset->left = 0;
	onset->right = 0;
	onset->ticker = 0;
	onset->len = 1;
	onset->mins[0].expiry = 0;
	onset->mins[0].value = 4294967296.0f;
}

uint onset_threshold_age(struct onset_threshold * onset)
{
	assert(onset);
	uint life;

	/* As age is by definition always positive, we can return -1 on error.
	 */
	if (onset->len == 0)
		return -1;

	/* Determine the remaining lifetime of the current minima and then
	 * subtract this from the window length.
	 */
	life = ((onset->windowlen + onset->mins[onset->left].expiry) - (onset->ticker + 1))
		% onset->windowlen;
	return onset->windowlen - (life + 1);
}

env_t onset_threshold_current_minimum(struct onset_threshold * onset)
{
	assert(onset);

	/* Check whether there is valid data in the minima tracker. As we're
	 * storing minima of energy levels and these are always positive, we can
	 * return a negative value to indicate an error.
	 */
	if (onset->len == 0)
		return -1;

	return onset->mins[onset->left].value;
}

env_t onset_threshold_current(struct onset_threshold * onset)
{
	assert(onset);

	/* Check whether there is valid data in the minima tracker. As we're
	 * storing minima of energy levels and these are always positive, we can
	 * return a negative value to indicate an error.
	 */
	if (onset->len == 0)
		return -1;

	return onset->mins[onset->left].value * onset->ratio;
}
