/*******************************************************************************
	env_estimate.h: Sample-based peak envelope estimation.

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

#ifndef __TUNA_ENV_ESTIMATE_H_INCLUDED__
#define __TUNA_ENV_ESTIMATE_H_INCLUDED__

#include "types.h"

/**
 * \file <tuna/env_estimate.h>
 *
 * \brief Signal envelope estimation.
 *
 * The peak envelope of a signal is estimated for each sample of that signal. At
 * any point in time, the envelope is either the current sample magnitude or the
 * previous envelope estimate multiplied by a decay factor, whichever is larger.
 * The decay factor is exponential and is calculated from a given time constant
 * and from the sampling rate of the signal to be processed.
 *
 * Sample magnitude is used rather than sample energy to simplify the necessary
 * calculations and to ensure that we don't have scaling issues when the sample
 * energy may require more bits than are used to store sample values.
 *
 * That is to say:
 * \f[
 * 	E [n] = \max \left(e^{-1/(T_c F_s)} E [n-1], \left|x[n]\right| \right)
 * \f]
 */

struct env_estimate;

#ifdef DOXYGEN
/**
 * \brief An envelope estimator.
 */
struct env_estimate {};
#endif

/**
 * \brief Initialise an envelope estimator.
 *
 * \param Tc The decay time constant of the envelope estimator.
 *
 * \param sample_rate The sampling frequency of the signal which will be
 * processed by this envelope estimator.
 *
 * \return A pointer to the new envelope estimator or NULL if an error occured.
 */
struct env_estimate * env_estimate_init(float Tc, uint sample_rate);

/**
 * \brief Destroy an envelope estimator that is no longer needed.
 *
 * \param e The envelope estimator to destroy.
 */
void env_estimate_exit(struct env_estimate * e);

/**
 * \brief Reset an envelope estimator.
 *
 * As the envelope estimator is effectively an IIR filter on the signal energy
 * (not the signal value), the output of the estimator is affected by previous
 * sample values. Thus if we are recovering after a resyncronisation of the
 * input signal we need to discard the internal state of the estimator and begin
 * estimation anew.
 *
 * \param e The envelope estimator to reset.
 */
void env_estimate_reset(struct env_estimate * e);

/**
 * \brief Estimate the envelope of the next sample in a sequence.
 *
 * \param e The envelope estimator to use.
 *
 * \param x The next value of the signal who's envelope we wish to estimate.
 *
 * \return The estimated envelope at the time position of the new sample.
 */
sample_t env_estimate_next(struct env_estimate * e, sample_t x);

#endif /* !__TUNA_ENV_ESTIMATE_H_INCLUDED__ */
