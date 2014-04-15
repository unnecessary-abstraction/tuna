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
	fft->data = (float *)fftwf_malloc((length + 4) * sizeof(float));
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

	fftwf_free(fft->data);
	free(fft);
}

float * fft_get_data(struct fft * fft)
{
	assert(fft);

	return fft->data;
}

float complex * fft_get_cdata(struct fft * fft)
{
	assert(fft);

	return fft->cdata;
}

uint fft_get_length(struct fft * fft)
{
	assert(fft);

	return fft->length;
}

int fft_transform(struct fft * fft)
{
	assert(fft);

	fftwf_execute(fft->plan);

	return 0;
}

void fft_power_spectrum(float complex * cdata, float * data, uint n)
{
	assert(cdata);
	assert(data);

	uint i;

	for (i = 0; i < n; i++) {
		float re = crealf(cdata[i]);
		float im = cimagf(cdata[i]);

		data[i] = (re * re + im * im) / (2 * n);
	}
}
