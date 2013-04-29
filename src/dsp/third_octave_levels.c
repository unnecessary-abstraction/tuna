/*******************************************************************************
	third_octave_levels.c: Third octave level calculation.

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

#include <errno.h>
#include <malloc.h>
#include <math.h>
#include "fft.h"
#include "output.h"
#include "third_octave_levels.h"
#include "types.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

struct transition_desc {
	uint		t_onset;
	uint		t_width;
	double *	coeffs;
};

const double overlap = 0.4;     /* Must be <= 0.5 */
#define phi_L 3

struct transition_desc desc[N_THIRD_OCTAVE_LEVELS];

const double freq[N_THIRD_OCTAVE_LEVELS + 1] = {
	11.2,
	14.1,
	17.8,
	22.4,
	28.2,
	35.5,
	44.7,
	56.2,
	70.8,
	89.1,
	112.0,
	141.0,
	178.0,
	224.0,
	282.0,
	355.0,
	447.0,
	562.0,
	708.0,
	891.0,
	1120.0,
	1410.0,
	1780.0,
	2240.0,
	2820.0,
	3550.0,
	4470.0,
	5620.0,
	7080.0,
	8910.0,
	11200.0,
	14100.0,
	17800.0,
	22400.0,
	28200.0,
	35500.0,
	44700.0,
	56200.0,
	70800.0,
	89100.0,
	112000.0,
	141000.0,
	178000.0,
	224000.0
};

/*
	Number of third octave levels which are active. This is at most
	N_THIRD_OCTAVE_LEVELS when the sample rate is high enough. Otherwise this is
	the number of third octave levels which are completely contained below half
	the sampling rate.
*/
static unsigned int active_third_octave_levels;

double psum(double *x, uint N);
void wpsum2(double *x, double *w, uint N, double *e);
static inline double phi(double p, uint l);

/*******************************************************************************
	Private functions
*******************************************************************************/

/* Unweighted power sum. */
double psum(double *x, uint N)
{
	double sum;
	uint i;

	sum = 0;

	for (i = 0; i < N; i++) {
		sum += x[i] * x[i];
	}

	return sum;
}

/* Dual band weighted power sum. */
/* Weighting coefficients are interleaved to speed up memory loads. */
void wpsum2(double *x, double *w, uint N, double *e)
{
	double sum0, sum1;
	double x_sq;
	uint i;

	sum0 = sum1 = 0;

	for (i = 0; i < N; i++) {
		x_sq = x[i] * x[i];

		sum0 += x_sq * w[2 * i];
		sum1 += x_sq * w[(2 * i) + 1];
	}

	/* Write back results. */
	e[0] += sum0;
	e[1] += sum1;
}

static inline double phi(double p, uint l)
{
	uint i;

	for (i = 0; i < l; i++) {
		p = sin(p * M_PI / 2);
	}

	return p;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

/*
	Writes active_third_octave_levels values to out, plus an additional
	junk value due to the calculation method.
*/
void third_octave_levels(struct third_octave_levels * output)
{
	uint i, j;
	double * x = fft_get_data();
	
	j = 0;

	for (i = 0; i < active_third_octave_levels; i++) {
		/* Unweighted sum from current position to start of transition, this is added to band i. */
		output->levels[i] += psum(&x[j], (desc[i].t_onset - j));

		/* Weighted sum over the transition, added to bands i and i+1. */
		wpsum2(&x[desc[i].t_onset], desc[i].coeffs, desc[i].t_width, &output->levels[i]);

		/* Update j to end of transition. */
		j = desc[i].t_onset + desc[i].t_width;
	}
}

int init_third_octave_levels(unsigned long sample_rate)
{
	uint i, j;
	double p, tmp, sin_tmp, cos_tmp;
	double delta, cur_freq, offset;
	uint t_end;

	/* Prepare each transition region. */
	for (i = 0; i < N_THIRD_OCTAVE_LEVELS; i++) {
		/* Calculate exact transition width. */
		delta = 2 * overlap * (sqrt(freq[i] * freq[i + 1]) - freq[i]);

		desc[i].t_onset = (uint) ceil(freq[i] - delta);
		t_end = (uint) floor(freq[i] + delta);
		desc[i].t_width = 1 + t_end - desc[i].t_onset;

		if (t_end > sample_rate / 2) {
			/* The current third octave band does not fit within half the sampling rate. */
			active_third_octave_levels = i - 1;

			return active_third_octave_levels;
		}

		/* Allocate enough memory. */
		desc[i].coeffs = (double *)malloc(sizeof(double) * 2 * desc[i].t_width);
		if (desc[i].coeffs == NULL)
			return -ENOMEM;

		/* Fill memory with correct coefficients. */
		for (j = 0; j < desc[i].t_width; j++) {
			if (delta) {
				cur_freq = desc[i].t_onset + j;
				offset = cur_freq - freq[i];
				p = offset / delta;
			} else {
				p = 0;
			}
			tmp = (1 + phi(p, phi_L)) * M_PI / 4;
			sin_tmp = sin(tmp);
			cos_tmp = cos(tmp);
			desc[i].coeffs[2 * j] = cos_tmp * cos_tmp;
			desc[i].coeffs[1 + 2 * j] = sin_tmp * sin_tmp;
		}
	}

	active_third_octave_levels = N_THIRD_OCTAVE_LEVELS;

	return active_third_octave_levels;
}

void write_third_octave_levels(struct record * rec, struct third_octave_levels * tol)
{
	uint i;
	for (i = 0; i < active_third_octave_levels; i++)
		output_double(rec, tol->levels[i]);
}
