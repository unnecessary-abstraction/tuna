/*******************************************************************************
	tol.c: Third octave level calculation.

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
#include <malloc.h>
#include <math.h>
#include <string.h>

#include "log.h"
#include "tol.h"
#include "types.h"

#ifdef ENABLE_ARM_NEON
#include <arm_neon.h>
#endif

struct tol_transition {
	uint				t_onset;
	uint				t_width;
	float *				coeffs;
};

struct tol {
	/* Number of third octave levels which are active. This is at most
	 * MAX_THIRD_OCTAVE_LEVELS when the sample rate is high enough.
	 * Otherwise this is the number of third octave levels which are
	 * completely contained below half the sampling rate.
	 */
	uint				n_tol;

	struct tol_transition		desc[MAX_THIRD_OCTAVE_LEVELS];
};

/*******************************************************************************
	Private declarations
*******************************************************************************/

static const float band_centres[MAX_THIRD_OCTAVE_LEVELS] = {
	10,
	12.5,
	16,
	20,
	25,
	31.5,
	40,
	50,
	63,
	80,
	100,
	125,
	160,
	200,
	250,
	315,
	400,
	500,
	630,
	800,
	1000,
	1250,
	1600,
	2000,
	2500,
	3150,
	4000,
	5000,
	6300,
	8000,
	10000,
	12500,
	16000,
	20000,
	25000,
	31500,
	40000,
	50000,
	63000,
	80000,
	100000,
	125000,
	160000
};

static const float band_edges[MAX_THIRD_OCTAVE_LEVELS + 1] = {
	11.22,
	14.13,
	17.78,
	22.39,
	28.18,
	35.48,
	44.67,
	56.23,
	70.79,
	89.13,
	112.2,
	141.3,
	177.8,
	223.9,
	281.8,
	354.8,
	446.7,
	562.3,
	707.9,
	891.3,
	1122,
	1413,
	1778,
	2239,
	2818,
	3548,
	4467,
	5623,
	7079,
	8913,
	11220,
	14130,
	17780,
	22390,
	28180,
	35480,
	44670,
	56230,
	70790,
	89130,
	112200,
	141300,
	177800,
	223900
};

/*******************************************************************************
	Private functions
*******************************************************************************/

/* Unweighted power sum. */
static inline float psum(float complex *x, uint N)
{
	assert(x);

#ifdef ENABLE_ARM_NEON
	uint i;
	float32_t * data = (float32_t *) x;
	float32x4_t sum_vec = {0, 0, 0, 0};
	uint n_floats = N * 2;

	for (i = 0; (i + 3) < n_floats; i += 4) {
		/* Preload next set of data. */
		__builtin_prefetch(data + i + 4);

		/* Load data, square and accumulate. */
		float32x4_t f = vld1q_f32(data + i);
		sum_vec = vmlaq_f32(sum_vec, f, f);
	}

	float32x2_t sum_lo = vget_low_f32(sum_vec);
	float32x2_t sum_hi = vget_high_f32(sum_vec);

	/* Add in remaining elements - we know than n_floats cannot be odd and
	 * the above loop can leave behind only zero or two floats.
	 */
	if (i < n_floats) {
		float32x2_t f = vld1_f32(data + i);
		sum_lo = vmla_f32(sum_lo, f, f);
	}

	/* Combine sums. */
	float32x2_t sum_pair = vpadd_f32(sum_lo, sum_hi);
	return sum_pair[0] + sum_pair[1];
#else
	float sum;
	uint i;

	sum = 0;

	for (i = 0; i < N; i++) {
		float re = crealf(x[i]);
		float im = cimagf(x[i]);
		sum += re * re + im * im;
	}

	return sum;
#endif
}

/* Dual band weighted power sum. */
/* Weighting coefficients are interleaved to speed up memory loads. */
static inline void wpsum2(float complex *x, float *w, uint N, float *e)
{
	assert(x);
	assert(w);
	assert(e);

#ifdef ENABLE_ARM_NEON
	uint i;
	float32_t * data = (float32_t *) x;
	float32x4_t sum_vec = {0, 0, 0, 0};
	uint n_floats = N * 2;

	for (i = 0; (i + 3) < n_floats; i += 4) {
		/* Preload next set of data and coefficients. */
		__builtin_prefetch(data + i + 4);
		__builtin_prefetch(w + i + 4);

		/* Equivalent operation of this sequence:
		 *
		 * sum_vec[0] += ((f[0] * f[0]) + (f[1] * f[1])) * w_vec[0];
		 * sum_vec[1] += ((f[0] * f[0]) + (f[1] * f[1])) * w_vec[1];
		 * sum_vec[2] += ((f[2] * f[2]) + (f[3] * f[3])) * w_vec[2];
		 * sum_vec[3] += ((f[2] * f[2]) + (f[3] * f[3])) * w_vec[3];
		 */

		/* Load data and coefficients. */
		float32x4_t f = vld1q_f32(data + i);
		float32x4_t w_vec = vld1q_f32(w + i);

		/* Squared vector.
		 * q = f^2.
		 */
		float32x4_t q = vmulq_f32(f, f);

		/* Squared vector with elements reversed within pairs.
		 * [q0, q1, q2, q3] -> [q1, q0, q3, q2]
		 */
		float32x4_t q_rev = vrev64q_f32(q);

		/* Intermediate vector laid out in preparation for
		 * multiplication by coefficients.
		 * i_vec = [q0 + q1, q0 + q1, q2 + q3, q2 + q3]
		 */
		float32x4_t i_vec = vaddq_f32(q, q_rev);

		/* sum_vec += i_vec * w_vec */
		sum_vec = vmlaq_f32(sum_vec, i_vec, w_vec);
	}

	/* We're going to need the previous values of the sums at the end of the
	 * function, so we preload it here and hope that gives it time to get
	 * into the cache before we need it.
	 */
	__builtin_prefetch(e);

	float32x2_t sum_lo = vget_low_f32(sum_vec);
	float32x2_t sum_hi = vget_high_f32(sum_vec);

	/* Add in remaining elements - we know than n_floats cannot be odd and
	 * the above loop can leave behind only zero or two floats.
	 */
	if (i < n_floats) {
		/* Equivalent operation of this sequence:
		 *
		 * sum_vec[0] += ((f[0] * f[0]) + (f[1] * f[1])) * w_vec[0];
		 * sum_vec[1] += ((f[0] * f[0]) + (f[1] * f[1])) * w_vec[1];
		 */

		/* Load data and coefficients. */
		float32x2_t f = vld1_f32(data + i);
		float32x2_t w_vec = vld1_f32(w + i);

		/* Squared vector. */
		float32x2_t q = vmul_f32(f, f);

		/* Duplicate and pairwise add
		 * i_vec = [q0 + q1, q0 + q1]
		 */
		float32x2_t i_vec = vpadd_f32(q, q);

		/* sum_lo += i_vec * w_vec */
		sum_lo = vmla_f32(sum_lo, i_vec, w_vec);
	}

	/* We need to output two sums, one for odd positions and one for even
	 * positions. To get this we do a non pairwise add.
	 */
	float32x2_t sum_pair = vadd_f32(sum_lo, sum_hi);

	/* Combine new sums with previous sums and write back. */
	float32x2_t sums = vld1_f32(e);
	sums = vadd_f32(sums, sum_pair);
	vst1_f32(e, sums);
#else
	float sum0, sum1;
	uint i;

	sum0 = sum1 = 0;

	for (i = 0; i < N; i++) {
		float re = crealf(x[i]);
		float im = cimagf(x[i]);
		float v = re * re + im * im;
		sum0 += v * w[2 * i];
		sum1 += v * w[(2 * i) + 1];
	}

	/* Write back results. */
	e[0] += sum0;
	e[1] += sum1;
#endif
}

static inline float phi(float p, uint l)
{
	uint i;

	for (i = 0; i < l; i++) {
		p = sinf(p * M_PI / 2);
	}

	return p;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

/* We assume the the results array has already been zero'd by the caller. This
 * allows slightly optimised performance.
 */
void tol_calculate(struct tol * t, float complex * cdata, float * results)
{
	assert(t);
	assert(cdata);

	uint i, j;
	
	j = 0;

	for (i = 0; i < t->n_tol; i++) {
		/* Unweighted sum from current position to start of transition, this is added to band i. */
		results[i] += psum(&cdata[j], (t->desc[i].t_onset - j));

		/* Weighted sum over the transition, added to bands i and i+1. */
		wpsum2(&cdata[t->desc[i].t_onset], t->desc[i].coeffs, t->desc[i].t_width, &results[i]);

		/* Update j to end of transition. */
		j = t->desc[i].t_onset + t->desc[i].t_width;
	}
}

uint tol_get_num_levels(struct tol * t)
{
	assert(t);

	return t->n_tol;
}

float tol_get_band_centre(uint band)
{
	if (band >= MAX_THIRD_OCTAVE_LEVELS)
		return NAN;

	return band_centres[band];
}

float tol_get_band_edge(uint band)
{
	if (band >= (MAX_THIRD_OCTAVE_LEVELS + 1))
		return NAN;

	return band_edges[band];
}

int tol_get_coeffs(struct tol * t, uint level, float * dest, uint length)
{
	assert(t);
	assert(dest);

	if (level >= t->n_tol)
		return -1;

	struct tol_transition * tr_upper = &t->desc[level];
	uint dest_offset = 0;
	uint i;

	if (level > 0) {
		struct tol_transition * tr_lower = &t->desc[level - 1];

		/* All coeffs are zero before the onset of the lower transition. */
		memset(dest, 0, tr_lower->t_onset * sizeof(float));
		dest_offset = tr_lower->t_onset;

		/* Copy coeffs from the lower transition. */
		for (i = 0; i < tr_lower->t_width; i++)
			dest[dest_offset + i] = tr_lower->coeffs[1 + 2 * i];
		dest_offset += tr_lower->t_width;
	}

	/* All coeffs between the end of the lower transition and the start of
	 * the upper transition are unity.
	 */
	for (i = 0; i < (tr_upper->t_onset - dest_offset); i++)
		dest[dest_offset + i] = 1.0f;
	dest_offset = tr_upper->t_onset;;

	/* Copy coeffs from the upper transition. */
	for (i = 0; i < tr_upper->t_width; i++)
		dest[dest_offset + i] = tr_upper->coeffs[2 * i];
	dest_offset += tr_upper->t_width;

	/* All remaining coeffs are zero. */
	memset(&dest[dest_offset], 0, (length - dest_offset) * sizeof(float));
	return 0;
}

struct tol * tol_init(uint sample_rate, uint analysis_length, float overlap, uint phi_L)
{
	struct tol * t;
	uint i, j;
	float p, tmp, sin_tmp, cos_tmp;
	float delta, cur_freq, offset;
	float step;
	uint t_end;

	assert(overlap < 0.5);

	t = (struct tol *) malloc(sizeof(struct tol));
	if (!t) {
		error("tol: Failed to allocate memory");
		return NULL;
	}

	step = (float)sample_rate / (float)analysis_length;

	/* Prepare each transition region. */
	for (i = 0; i < MAX_THIRD_OCTAVE_LEVELS; i++) {
		/* Calculate exact transition width. */
		delta = 2 * overlap * (sqrtf(band_edges[i] * band_edges[i + 1]) - band_edges[i]);

		t->desc[i].t_onset = (uint) ceilf((band_edges[i] - delta) / step);
		t_end = (uint) floorf((band_edges[i] + delta) / step);
		t->desc[i].t_width = 1 + t_end - t->desc[i].t_onset;

		if (t_end > analysis_length / 2) {
			/* The current third octave band does not fit within half the sampling rate. */
			t->n_tol = i - 1;

			return t;
		}

		/* Allocate enough memory. */
		t->desc[i].coeffs = (float *)malloc(sizeof(float) * 2 * t->desc[i].t_width);
		if (!t->desc[i].coeffs) {
			error("tol: Failed to allocate memory for tol coefficients");

			/* Free previously allocated coefficient arrays. */
			for (j = 0; j < i; j++)
				free(t->desc[j].coeffs);
			free(t);
			return NULL;
		}

		/* Fill memory with correct coefficients. */
		for (j = 0; j < t->desc[i].t_width; j++) {
			if (delta) {
				cur_freq = t->desc[i].t_onset + j * step;
				offset = cur_freq - band_edges[i];
				p = offset / delta;
			} else {
				p = 0;
			}
			tmp = (1 + phi(p, phi_L)) * M_PI / 4;
			sin_tmp = sinf(tmp);
			cos_tmp = cosf(tmp);
			t->desc[i].coeffs[2 * j] = cos_tmp * cos_tmp;
			t->desc[i].coeffs[1 + 2 * j] = sin_tmp * sin_tmp;
		}
	}

	t->n_tol = MAX_THIRD_OCTAVE_LEVELS;
	return t;
}

void tol_exit(struct tol * t)
{
	assert(t);

	uint i;

	for (i = 0; i < t->n_tol; i++) {
		free(t->desc[i].coeffs);
	}

	free(t);
}
