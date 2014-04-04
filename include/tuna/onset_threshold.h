/*******************************************************************************
	onset_threshold.h: Onset detection threshold calculation.

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

#ifndef __TUNA_ONSET_THRESHOLD_H_INCLUDED__
#define __TUNA_ONSET_THRESHOLD_H_INCLUDED__

#include "compiler.h"
#include "types.h"

/**
 * \file <tuna/onset_threshold.h>
 *
 * \brief Pulse onset threshold tracking.
 *
 * An onset detection threshold is calculated from a series of estimates of the
 * envelope of a signal. The current envelope may then be compared to the
 * detection threshold and if it exceeds this threshold then an onset detection
 * occurs.
 *
 * The aim is to detect increases in the envelope estimate above a given ratio
 * within a given time period. Therefore a moving minimum filter is applied to
 * the envelope estimates and the output of this filter is multiplied by the
 * desired ratio to give the detection threshold.
 *
 * See <tuna/minima.h> for the moving minimum filter interface.
 */
struct onset_threshold;

#ifdef DOXYGEN
/**
 * \brief A pulse onset threshold tracker.
 */
struct onset_threshold {};
#endif

/**
 * \brief Initialise a pulse onset threshold tracker.
 *
 * \param Tw The duration of the time window over which increases in the
 * envelope estimate will be considered.
 *
 * \param sample_rate The sampling frequency of the signal which is to be
 * processed through this onset detector.
 *
 * \param ratio The ratio by which the envelope estimate must increase within
 * a time window of the given duration in order to be detected as an onset.
 *
 * \return The new offset threshold tracker or NULL on error.
 */
struct onset_threshold * onset_threshold_init(float Tw, uint sample_rate,
		env_t ratio);

/**
 * \brief Destroy a pulse onset threshold tracker which is no longer needed.
 *
 * \param onset The pulse onset threshold tracker to destroy.
 */
void onset_threshold_exit(struct onset_threshold * onset);

/**
 * \brief Obtain the next onset threshold value from a tracker.
 *
 * \param onset The onset threshold tracker to use.
 *
 * \param env The next envelope estimate to be provessed by the given onset
 * threshold tracker.
 *
 * \param threshold The location in which to store the next onset threshold
 * value. This value will only be updated if a new threshold is calculated.
 */
TUNA_INLINE void onset_threshold_next(struct onset_threshold * onset,
		env_t next, env_t * threshold);

/**
 * \brief Reset a pulse onset threshold tracker.
 *
 * As the onset threshold is dependent on previous envelope estimates, the
 * tracker must be reset when there is a gap in the data being processed (such
 * as after a resynchronisation with the input).
 *
 * \param onset The pulse onset threshold tracker to reset.
 */
void onset_threshold_reset(struct onset_threshold * onset);

/**
 * \brief Determine the age of the current onset threshold.
 *
 * As the onset threshold is determined by multiplying the minimum sample energy
 * within an analysis window by a ratio, the age of the current threshold is
 * equal to the age of the current minimum.
 *
 * \param onset The pulse onset threshold tracker to use.
 *
 * \return The age of the current output of the given pulse onset threshold
 * tracker.
 */
uint onset_threshold_age(struct onset_threshold * onset);

env_t onset_threshold_current_minimum(struct onset_threshold * onset);

env_t onset_threshold_current(struct onset_threshold * onset);

#include "tuna_inl/onset_threshold.inl"

#endif /* !__TUNA_ONSET_THRESHOLD_H_INCLUDED__ */
