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

#include "compiler.h"
#include "types.h"

/**
 * \file <tuna/minima.h>
 *
 * \brief Moving minimum filter.
 *
 * This filter selects the minimum value within a window of a given length
 * ending at the current sample of a signal.
 */

struct minima_tracker;

#ifdef DOXYGEN
/**
 * \brief A moving minimum tracker.
 */
struct minima_tracker {};
#endif

/**
 * \brief Initialise a moving minimum filter.
 *
 * \param windowlen The length of the moving minimum window.
 *
 * \return A pointer to the new moving minimum filter or NULL on error.
 */
struct minima_tracker * minima_init(uint windowlen);

/**
 * \brief Destroy a moving minimum filter which is no longer needed.
 *
 * \param t The moving minimum filter to destroy.
 */
void minima_exit(struct minima_tracker * t);

/**
 * \brief Get the current output of a moving minimum filter.
 *
 * \param t The moving minimum filter to get the current output from.
 *
 * \return The current output of the given moving minimum filter.
 */
env_t minima_current(struct minima_tracker * t);

/**
 * \brief Get the age of the current output of a moving minimum filter.
 *
 * The output of a moving minimum filter will be a particular value within the
 * window on which the filter operates. This function returns the age of that
 * particular value as an offset from the current sample value.
 *
 * \param t The moving minimum filter to get the age of the current output from.
 *
 * \return The age of the current output of the given moving minimum filter.
 */
int minima_current_age(struct minima_tracker * t);

/**
 * \brief Obtain the output of the moving minimum filter for the next sample in
 * a signal.
 *
 * \param t The moving minimum filter to use.
 *
 * \param next The next value of the signal which the moving minimum filter is
 * to operate on.
 *
 * \return The output of the moving minimum filter after the given sample.
 */
TUNA_INLINE env_t minima_next(struct minima_tracker * t, env_t next);

/**
 * \brief Reset a moving minimum filter.
 *
 * As a moving minimum filter retains a number of past samples equal to the
 * length of the filter window, these samples must be discarded after a
 * resynchronisation occurs. This filter discards those samples and resets the
 * internal state of the moving minimum filer.
 *
 * \param t The moving minimum filter to reset.
 */
void minima_reset(struct minima_tracker * t);

#include "tuna_inl/minima.inl"

#endif /* !__TUNA_MINIMA_H_INCLUDED__ */
