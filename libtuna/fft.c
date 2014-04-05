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
#include <fftw3.h>
#include <malloc.h>
#include <string.h>

#include "fft.h"
#include "log.h"
#include "types.h"

struct fft {
	fftwf_plan			plan;
	uint				length;
	union {
		float *			data;
		float complex *		cdata;
	};
};

/*******************************************************************************
	Public functions
*******************************************************************************/

struct fft * fft_init(uint length)
{
	struct fft * fft;

	fft = (struct fft *) malloc(sizeof(struct fft));
	if (!fft) {
		error("fft: Failed to allocate memory");
		return NULL;
	}

	fft->length = length;
	fft->data = (float *)malloc((length + 4) * sizeof(float));
	if (!fft->data) {
		error("fft: Failed to allocate memory");
		free(fft);
		return NULL;
	}

	/* Ignore errors in reading or writing fftw wisdom as it is only a time
	 * saving mechanism.
	 */
	fftwf_import_wisdom_from_filename("fftw.wisdom");
	fft->plan = fftwf_plan_dft_r2c_1d(length, fft->data, fft->cdata, FFTW_MEASURE);
	if (fft->plan == NULL) {
		error("fft: Failed to plan FFT");
		free(fft->data);
		free(fft);
		return NULL;
	}
	fftwf_export_wisdom_to_filename("fftw.wisdom");

	return fft;
}

void fft_exit(struct fft * fft)
{
	assert(fft);

	free(fft->data);
	free(fft);
}

float * fft_get_data(struct fft * fft)
{
	assert(fft);

	return fft->data;
}

uint fft_get_length(struct fft * fft)
{
	assert(fft);

	return fft->length;
}

int fft_transform(struct fft * fft)
{
	assert(fft);

	uint i;
	double re, im;

	fftwf_execute(fft->plan);

	/* Find the magnitude of each complex frequency sample. */
	for (i = 0; i < fft->length / 2; i++) {
		/* Read in a complex value. */
		re = crealf(fft->cdata[i]);
		im = cimagf(fft->cdata[i]);

		/* Write out a single float. The output will take up half the
		 * space that the input did.
		 */
		fft->data[i] = (re * re + im * im) / fft->length;
	}

	return 0;
}
