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
 * The end of a pulse is considered to be the point in time where the envelope
 * estimate of the signal drops below a given fraction of the highest peak level
 * during that pulse. Typically, the fraction used is -10 dB (1/10th power).
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
 * \param ratio The ratio by which the envelope estimate must decrease from the
 * highest observed peak during a pulse to signal the end of that pulse.
 *
 * \return The new offset threshold tracker or NULL on error.
 */
struct offset_threshold * offset_threshold_init(env_t ratio);

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
 * \return 1 if the end of a pulse has been detected, 0 otherwise.
 */
TUNA_INLINE int offset_threshold_next(struct offset_threshold * o, env_t env);

/**
 * \brief Reset a pulse offset threshold tracker.
 *
 * As the offset threshold is dependent on previous envelope estimates, the
 * tracker must be reset when there is a gap in the data being processed (such
 * as after a resynchronisation with the input).
 *
 * The tracker must also be reset when a new highest peak is observed during a
 * pulse as the internal threshold will need to be updated.
 *
 * \param o The pulse offset threshold tracker to reset.
 *
 * \param env The envelope estimate of the signal to be processed by the given
 * offset threshold tracker at the time of reset.
 */
void offset_threshold_reset(struct offset_threshold * o, env_t env);

#include "tuna_inl/offset_threshold.inl"

#endif /* !__TUNA_OFFSET_THRESHOLD_H_INCLUDED__ */
