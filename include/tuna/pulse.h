/*******************************************************************************
	pulse.h: Per *pulse* processing.

	Copyright (C) 2013, 2014 Paul Barker, Loughborough University

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

#ifndef __TUNA_PULSE_H_INCLUDED__
#define __TUNA_PULSE_H_INCLUDED__

#include "consumer.h"
#include "fft.h"
#include "types.h"

/**
 * \file <tuna/pulse.h>
 *
 * \brief Pulse detection and processing module.
 *
 * TODO: Document more fully.
 */

/**
 * Parameters for pulse detection and processing.
 */
struct pulse_params {
	/**
	 * Width in seconds of the analysis window applied to the detection
	 * function to detect pulses.
	 */
	float					Tw;

	/**
	 * Decay time constant in seconds for envelope estimation.
	 */
	float					Tc;

	/**
	 * Length of delay line used to determine when a pulse has ended.
	 */
	float					Td;

	/**
	 * Highest absolute value taken by an input sample, cast to a floating
	 * point number.
	 */
	float					sample_limit;

	/**
	 * Maximum duration of a pulse in seconds. Pulses which exceed this
	 * duration will be cut short.
	 */
	float					pulse_max_duration;

	/**
	 * Minimum duration of the decay period of a pulse in seconds. This must
	 * be at least equal to Td so that the delay line is filled before we
	 * begin checking for pulse end.
	 */
	float					pulse_min_decay;

	/**
	 * Increase as a ratio which must be exceeded within the duration Tw to
	 * cause detection of a pulse.
	 */
	sample_t				threshold_ratio;

	/**
	 * Decrease as a ratio which must be exceeded over the duration Td to
	 * remain within a pulse once detected. A pulse is determined to end
	 * when the decrease drops below this ratio.
	 */
	sample_t				decay_threshold_ratio;
};

/**
 * Initialise an instance of the pulse processing module.
 *
 * \param consumer The consumer object to initialise. The call to pulse_init()
 * should immediately follow the creation of a consumer object with
 * consumer_new().
 *
 * \param csv_name The filename of the output file which will be created.
 * Analysis results will be written to this file in CSV format.
 *
 * \param params The various mathematical parameters to be used in the pulse
 * detection. The structure pointed to by this argument is used in-place by the
 * pulse processing module and therefore the data it points to should be valid
 * until the module is exited.  Changing values within the params structure
 * during processing could cause undefined behaviour and crashes.
 *
 * \return >=0 on success, <0 on failure.
 */
int pulse_init(struct consumer * consumer, const char * csv_name,
		const struct pulse_params * params);

#endif /* !__TUNA_PULSE_H_INCLUDED__ */
