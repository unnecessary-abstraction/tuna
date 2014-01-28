/*******************************************************************************
	fft.h: Fast Fourier Transform.

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

#ifndef __TUNA_FFT_H_INCLUDED__
#define __TUNA_FFT_H_INCLUDED__

#include "types.h"

/**
 * \file <tuna/fft.h>
 *
 * \brief Fast Fourier Transform.
 *
 * The exact transformation performed by these interfaces is from a sequence of
 * N time domain samples to a sequence of N/2 frequency domain power spectral
 * density samples. Both the input and output data are arrays of float rather
 * than arrays of sample_t as are used elsewhere as the FFT implementation used
 * cannot operate on integers. It would be possible to use arrays of type double
 * but it would significantly slow down the transformation as well as further
 * analysis of the output data. So float is a good compromise.
 *
 * The transformation is performed in-place used an internal buffer allocated
 * during fft_init(). A pointer to this buffer is obtained using fft_get_data()
 * to enable the writing of time-domain samples prior to transformation and
 * frequency-domain samples after transformation. The buffer should not be used
 * after the call to fft_exit() as it will be freed. The transformation itself
 * is performed by calling fft_transform() once the buffer contains the
 * appropriate data.
 */

struct fft;

#ifdef DOXYGEN
/**
 * \brief An FFT context.
 */
struct fft {};
#endif

/**
 * Initialise an FFT context.
 *
 * \param length The length of the FFT analysis window in samples. Every
 * transform performed by this FFT context will operate on this number of time
 * domain samples and produce half this number of frequency domain samples.
 *
 * \return A pointer to a new FFT context or NULL on error.
 */
struct fft * fft_init(uint length);

/**
 * Destroy an FFT context that is no longer needed.
 *
 * \param fft The FFT context to destroy.
 */
void fft_exit(struct fft * fft);

/**
 * Get a pointer to the data buffer used by an FFT context. The caller is
 * assumed to keep track of what it is doing and to know whether the buffer
 * contains time domain samples, frequency domain samples or junk data at any
 * given time.
 *
 * \param fft The FFT context to get the data pointer from.
 *
 * \return A pointer to the data buffer used by the given FFT context.
 */
float * fft_get_data(struct fft * fft);

/**
 * Get the length of an FFT context, that is the number of time domain samples
 * which it will operate on and therefore the length of the data buffer in
 * samples (not in bytes). Each sample is of type float so the length of the
 * data buffer in byters is `length * sizeof(float)`.
 *
 * \param fft The FFT context to get the length of.
 *
 * \return The length of the given FFT context.
 */
uint fft_get_length(struct fft * fft);

/**
 * Perform an FFT transform on a given context. Before this function is called,
 * the data buffer of the FFT context should be filled with time domain samples.
 * After this function is called, if no error occurred, the data buffer of the
 * FFT context will contain N/2 frequency domain power spectral density samples
 * where N is the length of the FFT context. If an error occurs, the content of
 * the FFT data buffer is undefined and should be assumed to be junk data.
 *
 * \param fft The FFT context on which the transform is to be performed.
 *
 * \return >=0 on success, <0 on error.
 */
int fft_transform(struct fft * fft);

#endif /* !__TUNA_FFT_H_INCLUDED__ */
