/*******************************************************************************
	time_slice.h: Per time-slice processing.

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

#ifndef __TUNA_TIME_SLICE_H_INCLUDED__
#define __TUNA_TIME_SLICE_H_INCLUDED__

#include "consumer.h"
#include "fft.h"

/**
 * \file <tuna/time_slice.h>
 *
 * \brief Per-time slice processing module.
 *
 * This module splits the input signal into slices of length 0.5 s. For each
 * slice we calculate:
 *
 * - The peak positive and peak negative signal levels followed by the offset of
 *   each of these peaks from the start of the slice.
 *
 * - The total energy within the slice, that is the sum of \f$x[n]^2\f$ for each
 *   sample \f$x[n]\f$.
 *
 * - The 2nd to 4th order energy sums within the slice. These are the sums of
 *   \f$x[n]^4\f$, \f$x[n]^6\f$ and \f$x[n]^8\f$ respectively for each sample
 *   \f$x[n]\f$. These intermediate sums may be used to find the standard
 *   deviation, skewness and kurtosis of the energy distribution within any
 *   number of consecutive slices.
 *
 * - The third octave energy levels within the slice. These are computed over a
 *   time window of length 1 s, centered on the middle of the time slice under
 *   analysis, giving a 50% overlap with adjacent time slices. A sine windowing
 *   function is used to preserve energy values across overlapping time slices.
 *
 * The analysis results are written to a CSV file with one line per time slice.
 * Within each line the results are stored in the order described above, with
 * third octave levels stored in the order of low frequency to high frequency. A
 * START line is written at the beginning of the file (see csv_write_start())
 * and a RESYNC line is written each time analysis is recovered following a loss
 * of synchronisation (see csv_write_resync()).
 */

/**
 * Initialise per-time slice analysis.
 *
 * \param consumer The consumer object to initialise. The call to
 * time_slice_init() should immediately follow the creation of a consumer object
 * with consumer_new().
 *
 * \param csv_name The filename of the output file which will be created.
 * Analysis results will be written to this file in CSV format.
 *
 * \return >=0 on success, <0 on failure.
 */
int time_slice_init(struct consumer * consumer, const char * csv_name);

#endif /* !__TUNA_TIME_SLICE_H_INCLUDED__ */
