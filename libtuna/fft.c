/*******************************************************************************
	fft.c: Fast Fourier Transform implementation using fftw.

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

#include <assert.h>
#include <complex.h>
#include <errno.h>
#include <fftw3.h>
#include <malloc.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

#include "fft.h"
#include "log.h"
#include "types.h"

/*******************************************************************************
	Public functions
*******************************************************************************/

struct fft * fft_init()
{
	int r;
	pthread_mutexattr_t attr;
	struct fft * fft;

	fft = (struct fft *) calloc(1, sizeof(struct fft));
	if (!fft) {
		error("fft: Failed to allocated memory");
		return NULL;
	}

	/* Create a recursive mutex. */
	r = pthread_mutexattr_init(&attr);
	if (r != 0) {
		error("fft: Failed to create mutexattr");
		return NULL;
	}

	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	r = pthread_mutex_init(&fft->mutex, &attr);
	pthread_mutexattr_destroy(&attr);
	if (r != 0) {
		error("fft: Failed to create mutex");
		return NULL;
	}
	
	return fft;
}

void fft_exit(struct fft * fft)
{
	assert(fft);

	pthread_mutex_destroy(&fft->mutex);
	free(fft);
}

int fft_set_length(struct fft * fft, uint length)
{
	assert(fft);

	/* This can be called multiple times if different uses are sharing the
	 * same fft buffer. We need to reallocate if the new length is longer
	 * than the old length.
	 */
	if (length > fft->length) {
		pthread_mutex_lock(&fft->mutex);

		fft->length = length;
		fft->data = (float *)realloc(fft->data, (length + 4) * sizeof(float));
		if (!fft->data) {
			error("fft: Failed to allocate memory");
			return -ENOMEM;
		}

		/* Ignore errors in reading or writing fftw wisdom as it is only
		 * a time saving mechanism.
		 */
		fftwf_import_wisdom_from_filename("fftw.wisdom");
		fft->plan = fftwf_plan_dft_r2c_1d(length, fft->data, fft->cdata, FFTW_MEASURE);
		fftwf_export_wisdom_to_filename("fftw.wisdom");

		pthread_mutex_unlock(&fft->mutex);
	}

	return 0;
}

float * fft_open(struct fft * fft)
{
	assert(fft);

	pthread_mutex_lock(&fft->mutex);
	return fft->data;
}

void fft_close(struct fft * fft)
{
	assert(fft);

	pthread_mutex_unlock(&fft->mutex);
}

int fft_transform(struct fft * fft)
{
	assert(fft);

	uint i;
	double re, im;

	pthread_mutex_lock(&fft->mutex);

	fftwf_execute(fft->plan);

	/* Find the magnitude of each complex frequency sample. */
	for (i = 0; i < fft->length / 2; i++) {
		/* Read in a complex double. */
		re = creal(fft->cdata[i]);
		im = cimag(fft->cdata[i]);

		/* Write out a single double. The output will take up half the
		 * space that the input did.
		 */
		fft->data[i] = sqrt(re * re + im * im) / fft->length;
	}

	pthread_mutex_unlock(&fft->mutex);

	return 0;
}
