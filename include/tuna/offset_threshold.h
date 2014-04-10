/*******************************************************************************
	offset_threshold.h: Offset detection threshold calculation.

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

#ifndef __TUNA_OFFSET_THRESHOLD_H_INCLUDED__
#define __TUNA_OFFSET_THRESHOLD_H_INCLUDED__

#include "compiler.h"
#include "types.h"

/**
 * \file <tuna/offset_threshold.h>
 *
 * \brief Pulse offset threshold tracking.
 *
 * An offset detection threshold is calculated from a series of estimates of the
 * envelope of a signal. A delayed minimum envelope estimate may then be
 * compared to the detection threshold and if it is below this threshold then an
 * offset detection occurs.
 *
 * After a pulse has been detected and the peak envelope estimate within the
 * pulse has been achieved, the aim is to track the decay of the minimum
 * envelope estimate observed since that peak. In order to remain within the
 * current pulse, the minimum envelope estimate should decrease by a given
 * ratio within a given time period. If over the specified time period the
 * minimum envelope estimate does not decrease by at least the given ratio then
 * the pulse is determined to have ended.
 */

struct offset_threshold;

#ifdef DOXYGEN
/**
 * \brief A pulse offset threshold tracker.
 */
struct offset_threshold {};
#endif

/**
 * \brief Initialise a pulse offset threshold tracker.
 *
 * \param Td The time duration over which decreases in the envelope estimate
 * will be considered.
 *
 * \param sample_rate The sampling frequency of the signal which is to be
 * processed through this offset detector.
 *
 * \param ratio The ratio by which the envelope estimate must decrease within
 * the given time duration in order to prevent the end of a pulse from being
 * detected.
 *
 * \return The new offset threshold tracker or NULL on error.
 */
struct offset_threshold * offset_threshold_init(float Td, uint sample_rate,
		env_t ratio);

/**
 * \brief Destroy a pulse offset threshold tracker which is no longer needed.
 *
 * \param o The pulse offset threshold tracker to destroy.
 */
void offset_threshold_exit(struct offset_threshold * o);

/**
 * \brief Obtain the next offset threshold value from a tracker.
 *
 * \param o The offset threshold tracker to use.
 *
 * \param env The next envelope estimate to be provessed by the given offset
 * threshold tracker.
 *
 * \return The next offset threshold value.
 */
TUNA_INLINE env_t offset_threshold_next(struct offset_threshold * o, env_t env);

/**
 * \brief Reset a pulse offset threshold tracker.
 *
 * As the offset threshold is dependent on previous envelope estimates, the
 * tracker must be reset when there is a gap in the data being processed (such
 * as after a resynchronisation with the input).
 *
 * \param o The pulse offset threshold tracker to reset.
 *
 * \param env The envelope estimate of the signal to be processed by the given
 * offset threshold tracker at the time of reset.
 */
void offset_threshold_reset(struct offset_threshold * o, env_t env);

/**
 * \brief Obtain the delayed minimum value tracked by an offset threshold
 * tracker.
 *
 * It is necessary to compare the delayed minimum to the offset detection
 * threshold and this delayed minimum can only be calculated within the offset
 * threshold tracker itself.
 *
 * \param o The pulse offset threshold tracker to use.
 *
 * \return The delayed minimum value calculated by the given pulse offset
 * threshold tracker.
 */
env_t offset_threshold_delayed_min(struct offset_threshold * o);

#include "tuna_inl/offset_threshold.inl"

#endif /* !__TUNA_OFFSET_THRESHOLD_H_INCLUDED__ */
