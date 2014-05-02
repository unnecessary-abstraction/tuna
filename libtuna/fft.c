/*******************************************************************************
	fft.c: Fast Fourier Transform implementation using fftw or ffts.

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
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "fft.h"
#include "log.h"
#include "types.h"

#ifdef ENABLE_FFTS
#include <ffts/ffts.h>
#else
#include <fftw3.h>
#endif

struct fft {
#ifdef ENABLE_FFTS
	ffts_plan_t *			plan;
#else
	fftwf_plan			plan;
#endif
	uint				length;
	float *				data;
	float complex *			cdata;
};

/*******************************************************************************
	Public functions
*******************************************************************************/

struct fft * fft_init(uint length)
{
	struct fft * fft;
	int r;
	size_t align;

	fft = (struct fft *) malloc(sizeof(struct fft));
	if (!fft) {
		error("fft: Failed to allocate memory");
		return NULL;
	}

	fft->length = length;

	/* Align memory to 16-byte boundary */
	align = 16;

	r = posix_memalign((void**)&fft->data, align, (length + 4) * sizeof(float));
	if (r != 0) {
		error("fft: Failed to allocate memory for input data");
		free(fft);
		return NULL;
	}
	r = posix_memalign((void**)&fft->cdata, align, (length + 4) * sizeof(complex float) / 2);
	if (!fft->cdata) {
		error("fft: Failed to allocate memory for output data");
		free(fft->data);
		free(fft);
		return NULL;
	}

#ifdef ENABLE_FFTS
	fft->plan = ffts_init_1d_real(length, -1);
#else
	fftwf_import_wisdom_from_filename("fftw.wisdom");
	fft->plan = fftwf_plan_dft_r2c_1d(length, fft->data, fft->cdata, FFTW_PATIENT);
#endif
	if (fft->plan == NULL) {
		error("fft: Failed to plan FFT");
		free(fft->cdata);
		free(fft->data);
		free(fft);
		return NULL;
	}
#ifndef ENABLE_FFTS
	fftwf_export_wisdom_to_filename("fftw.wisdom");
#endif

	return fft;
}

void fft_exit(struct fft * fft)
{
	assert(fft);

	if ((void*)fft->cdata != (void*)fft->data)
		free(fft->cdata);
	free(fft->data);
#ifdef ENABLE_FFTS
	ffts_free(fft->plan);
#endif
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

#ifdef ENABLE_FFTS
	ffts_execute(fft->plan, fft->data, fft->cdata);
#else
	fftwf_execute(fft->plan);
#endif

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
