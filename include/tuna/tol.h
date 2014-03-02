/*******************************************************************************
	tol.h: Third octave level calculation.

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

#ifndef __TUNA_TOL_H_INCLUDED__
#define __TUNA_TOL_H_INCLUDED__

#include "types.h"

/**
 * \file <tuna/tol.h>
 *
 * \brief Third octave level calculation.
 *
 * The transform provided by this header operates on an array of
 * frequency-domain samples obtained by use of the FFT module (see <tuna/fft.h>
 * and fft_transform()). This array is divided into third octave bands in
 * accordance with the following paper:
 *
 * Antoni, J. (2010). Orthogonal-like fractional-octave-band filters. The
 * Journal of the Acoustical Society of America, 127(2), 884–95.
 * doi:10.1121/1.3273888
 */
struct tol;

#ifdef DOXYGEN
/**
 * \brief A third octave level calculation context.
 */
struct tol {};
#endif

/**
 * The maximum number of third-octave levels which are supported.
 */
#define MAX_THIRD_OCTAVE_LEVELS 43

/**
 * \brief Initialise a third octave level calculation context.
 *
 * \param sample_rate The sampling frequency of the data which will be analysed.
 *
 * \param analysis_length The length of the array of frequency samples which
 * will be analysed. Every third octave analysis performed with this context
 * will operate on the same number of frequency samples.
 *
 * \param overlap The ratio by which each third octave band overlaps with the
 * next one. This value must be less than 0.5 to ensure that a band only
 * overlaps with its direct neighbours.
 *
 * \param phi_L The \f$\phi\f$ parameter for the initialising the third octave
 * bands.
 *
 * \return A pointer to a new third octave level calculation context or NULL if
 * an error occurs.
 */
struct tol * tol_init(uint sample_rate, uint analysis_length, float overlap, uint phi_L);

/**
 * Destroy a third octave level calculation context when it is no longer needed.
 *
 * \param t The third octave level calculation context to destroy.
 */
void tol_exit(struct tol * t);

/**
 * Get the number of third octave levels which a given context will calculate.
 * This value is dependent on the sampling rate passed to tol_init().
 *
 * \param t The third octave level calculation context to act upon.
 *
 * \return The number of third octave levels calculated by the given context.
 */
uint tol_get_num_levels(struct tol * t);

/**
 * Get the centre frequency in Hz of a given third-octave band.
 *
 * \param band The index of the third-octave band to retrieve. This is
 * zero-based where band zero has a centre frequency of 10 Hz. This parameter
 * must be less than MAX_THIRD_OCTAVE_LEVELS.
 *
 * \return The centre frequency of the requested band.
 */
float tol_get_band_centre(uint band);

/**
 * Get the upper band edge frequency in Hz of a given third-octave band.
 *
 * \param band The index of the third-octave band to retrieve. This is
 * zero-based where band zero has a centre frequency of 10 Hz. This parameter
 * must be less than MAX_THIRD_OCTAVE_LEVELS + 1.
 *
 * \return The upper band-edge frequency of the requested band.
 */
float tol_get_band_edge(uint band);

/**
 * Extract the frequency domain coefficients for a particular third-octave band.
 *
 * \param t The third octave level calculation context to extract coefficients
 * from.
 *
 * \param level The index of the third octave level for which to extract
 * coefficients.
 *
 * \param dest A buffer into which coefficients will be written in float format.
 *
 * \param length The length of the given dest buffer. This should correspond to
 * the analysis_length parameter passed to tol_init() in order to ensure
 * coefficients are extracted accurately.
 *
 * \return >=0 on success, <0 on failure.
 */
int tol_get_coeffs(struct tol * t, uint level, float * dest, uint length);

/**
 * Perform a third octave level calculation.
 *
 * \param t The third octave level calculation context to use.
 *
 * \param data A pointer to an array of frequency domain samples in floating
 * point format on which to perform the calculation. This array must be of the
 * length given to tol_init().
 *
 * \param results A pointer to which an array of third octave levels will be
 * written. The length of this array can be obtained by calling
 * tol_get_num_levels() and the given buffer must be large enough to store this
 * number of floating point values.
 */
void tol_calculate(struct tol * t, float * data, float * results);

#endif /* !__TUNA_TOL_H_INCLUDED__ */
