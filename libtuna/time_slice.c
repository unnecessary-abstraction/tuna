/*******************************************************************************
	time_slice.c: Per time-slice processing.

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
#include <errno.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "buffer.h"
#include "bufhold.h"
#include "compiler.h"
#include "consumer.h"
#include "csv.h"
#include "dat.h"
#include "log.h"
#include "time_slice.h"
#include "timespec.h"
#include "tol.h"
#include "types.h"
#include "window.h"

#ifdef ENABLE_ARM_NEON
#include <arm_neon.h>
#endif

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

#define MAX_TIME_SLICE_RESULTS (6 + MAX_THIRD_OCTAVE_LEVELS)

struct time_slice {
#ifdef ENABLE_ARM_NEON
	float32x4x4_t			moments_vec;
#endif

	/* The following fields are initialised in time_slice_init(). */
	struct bufhold *		held_buffers;
	FILE *				out;
	char *				out_name;
	struct fft *			fft;
	float *				fft_data;
	int				out_mode;

	/* The following fields are initialised in time_slice_start(). */
	struct tol *			tol;
	struct time_slice_results *	results;
	float *				window;
	uint				sample_rate;
	uint				slice_length;
	uint				slice_period;
	uint				available;
	uint				n_tol;

	/* The following field is used within process_time_slice(). */
	uint				index;
};

struct time_slice_results {
	sample_t			peak_positive;
	sample_t			peak_negative;
	uint				peak_positive_offset;
	uint				peak_negative_offset;

	float				moments[4];

	float				tols[];
};

static inline uint min(uint a, uint b)
{
	if (a < b)
		return a;

	return b;
}

static int write_results_csv(struct time_slice * t)
{
	int r;
	uint i;

	assert(t);

	r = csv_write_sample(t->out, t->results->peak_positive);
	if (r < 0)
		goto error;

	r = csv_write_sample(t->out, t->results->peak_negative);
	if (r < 0)
		goto error;

	r = csv_write_uint(t->out, t->results->peak_positive_offset);
	if (r < 0)
		goto error;

	r = csv_write_uint(t->out, t->results->peak_negative_offset);
	if (r < 0)
		goto error;

	for (i = 0; i < 4; i++) {
		r = csv_write_float(t->out, t->results->moments[i]);
		if (r < 0)
			goto error;
	}

	for (i = 0; i < t->n_tol; i++) {
		r = csv_write_float(t->out, t->results->tols[i]);
		if (r < 0)
			goto error;
	}

	r = csv_next(t->out);
	if (r < 0)
		goto error;

	return 0;

error:
	error("time_slice: Failed to write to output file %s", t->out_name);
	return r;
}

static int write_results_dat(struct time_slice * t)
{
	assert(t);

	size_t sz = sizeof(struct time_slice_results) + t->n_tol * sizeof(float);

	return dat_write_record(t->out, TUNA_DAT_TIME_SLICE, t->results, sz);
}

static inline float process_common_sca(struct time_slice * t, int32_t * p_data)
{
	assert(t);
	assert(p_data);

	float x = (float) *p_data;

	t->fft_data[t->index] = x * t->window[t->index];

	return x;
}

static inline void update_stats_sca(struct time_slice * t, float x)
{
	assert(t);

	float * m = t->results->moments;
	float e = fabsf(x);
	float e2 = e * e;

	m[0] += e;
	m[1] += e2;
	m[2] += e2 * e;
	m[3] += e2 * e2;
}

static inline void detect_peaks_sca(struct time_slice * t, float v, uint offset)
{
	assert(t);

	if (v > t->results->peak_positive) {
		t->results->peak_positive = v;
		t->results->peak_positive_offset = offset;
	} else if (v < t->results->peak_negative) {
		t->results->peak_negative = v;
		t->results->peak_negative_offset = offset;
	}
}

#ifdef ENABLE_ARM_NEON
static inline float32x4_t process_common_vec(struct time_slice * t, int32_t * p_data)
{
	assert(t);
	assert(p_data);

	/* Prefetch next element. */
	__builtin_prefetch(p_data + 4);

	float32_t * p_coeffs = (float32_t *) &t->window[t->index];
	float32_t * p_dest = (float32_t *) &t->fft_data[t->index];

	int32x4_t data_i32 = vld1q_s32(p_data);
	float32x4_t coeffs = vld1q_f32(p_coeffs);

	float32x4_t data_f32 = vcvtq_f32_s32(data_i32);

	float32x4_t dest = vmulq_f32(data_f32, coeffs);

	vst1q_f32(p_dest, dest);

	return data_f32;
}

static inline void update_stats_vec(struct time_slice * t, float32x4_t x)
{
	assert(t);

	float32x4x4_t m = t->moments_vec;

	float32x4_t e = vabsq_f32(x);
	m.val[0] = vaddq_f32(m.val[0], e);

	float32x4_t e2 = vmulq_f32(x, x);
	m.val[1] = vaddq_f32(m.val[1], e2);

	float32x4_t e3 = vmulq_f32(e2, x);
	m.val[2] = vaddq_f32(m.val[2], e3);

	float32x4_t e4 = vmulq_f32(e2, e2);
	m.val[3] = vaddq_f32(m.val[3], e4);

	t->moments_vec = m;
}

static inline void detect_peaks_vec(struct time_slice * t, float32x4_t vec, uint offset)
{
	/* We can't vectorise this */
	assert(t);

	detect_peaks_sca(t, vec[0], offset);
	detect_peaks_sca(t, vec[1], offset + 1);
	detect_peaks_sca(t, vec[2], offset + 2);
	detect_peaks_sca(t, vec[3], offset + 3);
}

static inline void update_stats_finish(struct time_slice * t)
{
	assert(t);

	float32x4x4_t m_vec = t->moments_vec;
	float32x4_t m = vld1q_f32(t->results->moments);

	float32x2_t m0_lo = vget_low_f32(m_vec.val[0]);
	float32x2_t m0_hi = vget_high_f32(m_vec.val[0]);
	float32x2_t m0_pair = vpadd_f32(m0_lo, m0_hi);

	float32x2_t m1_lo = vget_low_f32(m_vec.val[1]);
	float32x2_t m1_hi = vget_high_f32(m_vec.val[1]);
	float32x2_t m1_pair = vpadd_f32(m1_lo, m1_hi);

	float32x2_t m2_lo = vget_low_f32(m_vec.val[2]);
	float32x2_t m2_hi = vget_high_f32(m_vec.val[2]);
	float32x2_t m2_pair = vpadd_f32(m2_lo, m2_hi);

	float32x2_t m3_lo = vget_low_f32(m_vec.val[3]);
	float32x2_t m3_hi = vget_high_f32(m_vec.val[3]);
	float32x2_t m3_pair = vpadd_f32(m3_lo, m3_hi);

	float32x2_t m0m1_pair = vpadd_f32(m0_pair, m1_pair);
	float32x2_t m2m3_pair = vpadd_f32(m2_pair, m3_pair);

	float32x4_t m_summed = vcombine_f32(m0m1_pair, m2m3_pair);
	m = vaddq_f32(m, m_summed);
	vst1q_f32(t->results->moments, m);
}
#endif

static void process_buffer(struct time_slice * t, struct held_buffer * h)
{
	assert(t);
	assert(h);

	/* We split processing into quarters as we need overlapped windowed
	 * analysis in the frequency domain and non-overlapped non-windowed
	 * analysis in the time domain.
	 *
	 * - The first quarter is copied with windowing into the fft buffer and
	 *   discarded.
	 *
	 * - The second quarter is copied with windowing into the fft buffer, is
	 *   processed in the time domain to check for peaks and is then
	 *   discarded.
	 *
	 * - The third quarter is copied with windowing into the fft buffer, is
	 *   processed in the time domain to check for peaks and is then kept as
	 *   it will form the first quarter of the next time slice.
	 *
	 * - The fourth is copied with windowing into the fft buffer and is kept
	 *   as it will form the first quarter of the next time slice.
	 */

	uint avail;	/* Number of available samples remaining. */
	uint len = t->slice_period * 2;
	uint i, c;
	uint offset = 0;
	sample_t * data;
	
	avail = bufhold_count(h);
	data = bufhold_data(h);
	if (avail && t->index < len/4) {
		c = min(len/4 - t->index, avail);
		i = 0;
#ifdef ENABLE_ARM_NEON
		do {
			process_common_vec(t, (int32_t *) &data[i]);
			t->index += 4;
			i += 4;
		} while (i < c - 3);
#endif
		while (i < c) {
			process_common_sca(t, (int32_t *) &data[i]);
			t->index++;
			i++;
		}
		avail -= c;
		offset = c;
	}
	if (avail && t->index < len*3/4) {
		c = min(len*3/4 - t->index, avail);
		i = 0;
#ifdef ENABLE_ARM_NEON
		do {
			float32x4_t vec;
			vec = process_common_vec(t, (int32_t *) &data[offset + i]);
			update_stats_vec(t, vec);
			detect_peaks_vec(t, vec, t->index - len/4);
			t->index += 4;
			i += 4;
		} while (i < c - 3);
#endif
		while (i < c) {
			float v;
			v = process_common_sca(t, (int32_t *) &data[offset + i]);
			update_stats_sca(t, v);
			detect_peaks_sca(t, v, t->index - len/4);
			t->index++;
			i++;
		}
		avail -= c;
		offset += c;
	}

	/* We know (t->index < len) as we wouldn't have been called otherwise
	 * and the previous code will not advance t->index that far.
	 */
	if (avail) {
		c = min(len - t->index, avail);
		i = 0;
#ifdef ENABLE_ARM_NEON
		do {
			process_common_vec(t, (int32_t *) &data[i]);
			t->index += 4;
			i += 4;
		} while (i < c - 3);
#endif
		while (i < c) {
			process_common_sca(t, (int32_t *) &data[i]);
			t->index++;
			i++;
		}
	}
}

static int process_time_slice(struct time_slice * t)
{
	assert(t);

	struct held_buffer * h;
	uint start, offset;

	memset(t->results, 0,
		sizeof(struct time_slice_results) + t->n_tol * sizeof(float));

#ifdef ENABLE_ARM_NEON
	memset(&t->moments_vec, 0, sizeof(t->moments_vec));
#endif

	t->index = 0;

	h = bufhold_oldest(t->held_buffers);
	while (h) {
		struct held_buffer * next = bufhold_next(h);
		start = t->index;
		process_buffer(t, h);

		/* Check whether this buffer extends into the second half of
		 * this time slice, if not then we can discard it.
		 */
		if (t->index <= t->slice_period) {
			bufhold_release(h);
		} else {
			/* Adjust start of buffer if this is the first buffer
			 * that we need to keep for the next time slice.
			 */
			if (start < t->slice_period) {
				offset = t->slice_period - start;
				bufhold_advance(h, offset);
			}
		}
		h = next;
	}

	fft_transform(t->fft);
	tol_calculate(t->tol, fft_get_cdata(t->fft), t->results->tols);

#ifdef ENABLE_ARM_NEON
	update_stats_finish(t);
#endif

	if (t->out_mode == TUNA_OUT_MODE_CSV)
		return write_results_csv(t);
	else
		return write_results_dat(t);
}

void time_slice_exit(struct consumer * consumer)
{
	assert(consumer);

	struct time_slice * t = (struct time_slice *)
		consumer_get_data(consumer);

	if (t->window)
		free(t->window);

	if (t->results)
		free(t->results);

        if (t->tol)
                tol_exit(t->tol);

	bufhold_release_all(t->held_buffers);
	bufhold_exit(t->held_buffers);
	if (t->out_mode == TUNA_OUT_MODE_CSV)
		csv_close(t->out);
	else
		dat_close(t->out);

	free(t->out_name);
	free(t);
}

int time_slice_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);
	assert(buf);

	int r;

	struct time_slice * t = (struct time_slice *)
		consumer_get_data(consumer);
	
	t->available += count;
	r = bufhold_add(t->held_buffers, buf, count);
	if (r < 0) {
		error("time_slice: Failed to hold buffer");
		return r;
	}

	while (t->available >= t->slice_length) {
		r = process_time_slice(t);
		if (r < 0) {
			error("time_slice: Failed to process time slice");
			return r;
		}
		t->available -= t->slice_period;
	}

	return 0;
}

int time_slice_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	int r;
	int rate_pow2;

	struct time_slice * t = (struct time_slice *)
		consumer_get_data(consumer);

	t->sample_rate = sample_rate;

	/* We can assume sample_rate > 0. */
	rate_pow2 = 31 - __builtin_clz(sample_rate);
	t->slice_length = 1<<rate_pow2;

	t->slice_period = t->slice_length / 2;
	t->available = 0;

	/* Create window function. */
	t->window = (float *)malloc(sizeof(float) * t->slice_length);
	if (!t->window) {
		error("time_slice: Failed to allocate memory for window function");
		return -ENOMEM;
	}

	window_init_sine(t->window, t->slice_length);

	t->fft = fft_init(t->slice_length);
	if (!t->fft) {
		error("time_slice: Failed to initialise FFT");
		return -1;
	}
	t->fft_data = fft_get_data(t->fft);

	t->tol = tol_init(sample_rate, t->slice_length, 0.4, 3);
	if (!t->tol) {
		error("time_slice: Failed to initialise third octave level calculation");
		return -1;
	}

	t->n_tol = tol_get_num_levels(t->tol);

	t->results = (struct time_slice_results *)
		malloc(sizeof(struct time_slice_results) + (t->n_tol + 1) *
				sizeof(float));
	if (!t->results) {
		error("time_slice: Failed to allocate memory for results");
		return -ENOMEM;
	}

	if (t->out_mode == TUNA_OUT_MODE_CSV)
		r = csv_write_start(t->out, ts);
	else
		r = dat_write_start(t->out, ts);

	if (r < 0) {
		error("time_slice: Failed to write to output file %s", t->out_name);
		return r;
	}

	return 0;
}

int time_slice_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	int r;

	struct time_slice * t = (struct time_slice *)
		consumer_get_data(consumer);

	/* We're going to have to dump old data. */
	bufhold_release_all(t->held_buffers);
	t->available = 0;

	if (t->out_mode == TUNA_OUT_MODE_CSV)
		r = csv_write_resync(t->out, ts);
	else
		r = dat_write_resync(t->out, ts);

	if (r < 0) {
		error("time_slice: Failed to write to output file %s", t->out_name);
		return r;
	}

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int time_slice_init(struct consumer * consumer, const char * out_name,
		int out_mode)
{
	assert(out_name);
	int r;
	struct time_slice * t;

	/* If we are using neon vectorisation, we want the first element of
	 * struct time_slice ('moments_vec') to be correctly aligned.
	 */
	r = posix_memalign((void **)&t, 16, sizeof(struct time_slice));
	if (r) {
		error("time_slice: Failed to allocate memory");
		r = -ENOMEM;
		goto err;
	}
	memset(t, 0, sizeof(struct time_slice));

	t->held_buffers = bufhold_init();
	if (!t->held_buffers) {
		error("time_slice: Failed to allocate memory for held buffers");
		r = -1;
		goto err;
	}

	/* Initialize csv file. */
	t->out_name = strdup(out_name);
	if (!t->out_name) {
		error("time_slice: Failed to allocate memory for output file name");
		r = -ENOMEM;
		goto err;
	}

	t->out_mode = out_mode;
	if (t->out_mode == TUNA_OUT_MODE_CSV)
		t->out = csv_open(t->out_name);
	else
		t->out = dat_open(t->out_name);

	if (!t->out) {
		error("time_slice: Failed to open file %s", t->out_name);
		r = -1;
		goto err;
	}

	consumer_set_module(consumer, time_slice_write, time_slice_start,
			time_slice_resync, time_slice_exit, t);

	return 0;

err:
	if (t->out) {
		if (t->out_mode == TUNA_OUT_MODE_CSV)
			csv_close(t->out);
		else
			dat_close(t->out);
	}
	if (t->out_name)
		free(t->out_name);
	if (t->held_buffers)
		bufhold_exit(t->held_buffers);
	if (t)
		free(t);

	return r;
}
