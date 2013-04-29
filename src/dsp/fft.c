/*******************************************************************************
	fft.c: FFT implementation using fftw.

	Copyright (C) 2013 Paul Barker, Loughborough University
	
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

#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include "fft.h"
#include "types.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

struct fft {
	fftw_plan			plan;
	uint				length;
	double *			window;
	union {
		double *		data;
		double complex *	cdata;
	};
};

static struct fft fft;

static void create_sine_window(void);
static void apply_window(void);

/*******************************************************************************
	Private functions
*******************************************************************************/

static void create_sine_window(void)
{
	uint i;
	double w_sin;

	for (i = 0; i < fft.length; i++) {
		w_sin = sin(M_PI * i / fft.length);
		fft.window[i] = w_sin * 2;
	}
}

static void apply_window(void)
{
	uint i;

	for (i = 0; i < fft.length; i++) {
		fft.data[i] *= fft.window[i];
	}
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int init_fft(uint length)
{
	fft.length = length;
	fft.data = (double *)malloc((length + 4) * sizeof(double));
	if (!fft.data)
		return -1;

	fft.window = (double *)malloc(length * sizeof(double));
	if (!fft.window) {
		free(fft.data);
		return -1;
	}

	fft.plan = fftw_plan_dft_r2c_1d(length, fft.data, fft.cdata, FFTW_MEASURE);

	create_sine_window();

	return 0;
}

void fft_no_window(void)
{
	uint i;
	double re, im;

	fftw_execute(fft.plan);

	/* Find the magnitude of each complex frequency sample. */
	for (i = 0; i < fft.length / 2; i++) {
		/* Read in a complex double. */
		re = creal(fft.cdata[i]);
		im = cimag(fft.cdata[i]);

		/*
		   Write out a single double. The output will take up half the space
		   that the input did.
		 */
		fft.data[i] = sqrt(re * re + im * im) / fft.length;
	}
}

void fft_with_window(void)
{
	apply_window();
	fft_no_window();
}

void fft_load(double * source, uint offset, uint length)
{
	memcpy(&fft.data[offset], source, length);
}

void fft_pad(uint offset)
{
	memset(&fft.data[offset], 0, fft.length - offset);
}

double * fft_get_data(void)
{
	return fft.data;
}
