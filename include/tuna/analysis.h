/*******************************************************************************
	analysis.h: Run data through both analysis modules.

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

#ifndef __TUNA_ANALYSIS_H_INCLUDED__
#define __TUNA_ANALYSIS_H_INCLUDED__

#include "consumer.h"
#include "fft.h"
#include "pulse.h"
#include "time_slice.h"

/**
 * \file <tuna/analysis.h>
 *
 * \brief Process data with both pulse and time_slice consumers.
 *
 * This consumer hands data in turn to the pulse and time_slice consumers. See
 * the documentation for <tuna/pulse.h> and <tuna/time_slice.h> for detils of
 * these consumers.
 */

/**
 * Initialise dual analysis consumer.
 *
 * \param consumer The consumer object to initialise. The call to
 * analysis_init() should immediately follow the creation of a consumer object
 * with consumer_new().
 *
 * \param pulse_csv_name The filename of the output file which will be created
 * by the pulse consumer. Analysis results will be written to this file in CSV
 * format.
 *
 * \param time_slice_csv_name The filename of the output file which will be
 * created by the time_slice consumer. Analysis results will be written to this
 * file in CSV format.
 *
 * \param pulse_params The various mathematical parameters to be used in the
 * pulse consumer. See the documentation for pulse_init() for how this argument
 * is used.
 *
 * \return >=0 on success, <0 on failure.
 */
int analysis_init(struct consumer * consumer, const char * pulse_csv_name,
		const char * time_slice_csv_name,
		const struct pulse_params * pulse_params);

#endif /* !__TUNA_ANALYSIS_H_INCLUDED__ */
