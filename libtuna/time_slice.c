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
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "buffer.h"
#include "bufhold.h"
#include "compiler.h"
#include "consumer.h"
#include "csv.h"
#include "log.h"
#include "time_slice.h"
#include "timespec.h"
#include "tol.h"
#include "types.h"
#include "window.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

#define MAX_TIME_SLICE_RESULTS (6 + MAX_THIRD_OCTAVE_LEVELS)

struct time_slice {
	/* The following fields are initialised in time_slice_init(). */
	struct bufhold *		held_buffers;
	FILE *				csv;
	char *				csv_name;
	struct fft *			fft;
	float *				fft_data;

	/* The following fields are initialised in time_slice_start(). */
	struct tol *			tol;
	struct time_slice_results *	results;
	float *				window;
	uint				sample_rate;
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

	float				sum_1;
	float				sum_2;
	float				sum_3;
	float				sum_4;

	float				tols[];
};

static inline uint min(uint a, uint b)
{
	if (a < b)
		return a;

	return b;
}

static inline uint values_per_slice(struct time_slice * t)
{
	assert(t);

	return 8 + t->n_tol;
}

static int write_results(struct time_slice * t)
{
	int r;
	uint i;

	assert(t);

	r = csv_write_sample(t->csv, t->results->peak_positive);
	if (r < 0)
		goto error;

	r = csv_write_sample(t->csv, t->results->peak_negative);
	if (r < 0)
		goto error;

	r = csv_write_uint(t->csv, t->results->peak_positive_offset);
	if (r < 0)
		goto error;

	r = csv_write_uint(t->csv, t->results->peak_negative_offset);
	if (r < 0)
		goto error;

	r = csv_write_float(t->csv, t->results->sum_1);
	if (r < 0)
		goto error;

	r = csv_write_float(t->csv, t->results->sum_2);
	if (r < 0)
		goto error;

	r = csv_write_float(t->csv, t->results->sum_3);
	if (r < 0)
		goto error;

	r = csv_write_float(t->csv, t->results->sum_4);
	if (r < 0)
		goto error;

	for (i = 0; i < t->n_tol; i++) {
		r = csv_write_float(t->csv, t->results->tols[i]);
		if (r < 0)
			goto error;
	}

	r = csv_next(t->csv);
	if (r < 0)
		goto error;

	return 0;

error:
	error("time_slice: Failed to write to output file %s", t->csv_name);
	return r;
}

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
	float x, e, e_2;
	sample_t v;
	uint offset = 0;
	sample_t * data;
	
	avail = bufhold_count(h);
	data = bufhold_data(h);
	if (avail && t->index < len/4) {
		c = min(len/4 - t->index, avail);
		for (i = 0; i < c; i++) {
			v = data[i];
			x = (float)v;

			t->fft_data[t->index] = x * t->window[t->index];
			t->index++;
		}
		avail -= c;
		offset = i;
	}
	if (avail && t->index < len*3/4) {
		c = min(len*3/4 - t->index, avail);
		for (i = 0; i < c; i++) {
			v = data[offset + i];
			x = (float)v;

			/* Calculate intermediate sums for kurtosis. */
			e = x * x;
			e_2 = e * e;
			t->results->sum_1 += e;
			t->results->sum_2 += e_2;
			t->results->sum_3 += e_2 * e;
			t->results->sum_4 += e_2 * e_2;

			/* Detect Peaks */
			if (v > t->results->peak_positive) {
				t->results->peak_positive = v;
				t->results->peak_positive_offset = t->index - len/4;
			} else if (v < t->results->peak_negative) {
				t->results->peak_negative = v;
				t->results->peak_negative_offset = t->index - len/4;
			}
			
			t->fft_data[t->index] = x * t->window[t->index];
			t->index++;
		}
		avail -= c;
		offset += i;
	}

	/* We know (t->index < len) as we wouldn't have been called otherwise
	 * and the previous code will not advance t->index that far.
	 */
	if (avail) {
		c = min(len - t->index, avail);
		for (i = 0; i < c; i++) {
			v = data[offset + i];
			x = (float)v;

			t->fft_data[t->index] = x * t->window[t->index];
			t->index++;
		}
		avail -= c;
		offset += i;
	}
}

static int process_time_slice(struct time_slice * t)
{
	assert(t);

	struct held_buffer * h;
	uint start, offset;

	memset(t->results, 0,
		sizeof(struct time_slice_results) + t->n_tol * sizeof(float));

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
			bufhold_release(t->held_buffers, h);
		} else {
			/* Adjust start of buffer if this is the first buffer
			 * that we need to keep for the next time slice.
			 */
			if (start < t->slice_period) {
				offset = t->slice_period - start;
				bufhold_advance(t->held_buffers, h, offset);
			}
		}
		h = next;
	}

	fft_transform(t->fft);
	tol_calculate(t->tol, t->fft_data, t->results->tols);

	return write_results(t);
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
	csv_close(t->csv);

	free(t->csv_name);
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
	bufhold_add(t->held_buffers, buf, count);

	while (t->available >= t->slice_period) {
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

	struct time_slice * t = (struct time_slice *)
		consumer_get_data(consumer);

	t->sample_rate = sample_rate;
	t->slice_period = sample_rate / 2;
	t->available = 0;

	/* Create window function. */
	t->window = (float *)malloc(sizeof(float) * sample_rate);
	if (!t->window) {
		error("time_slice: Failed to allocate memory for window function");
		return -ENOMEM;
	}

	window_init_sine(t->window, t->slice_period * 2);

	t->fft = fft_init(sample_rate);
	if (!t->fft) {
		error("time_slice: Failed to initialise FFT");
		return -1;
	}
	t->fft_data = fft_get_data(t->fft);

	t->tol = tol_init(sample_rate, sample_rate, 0.4, 3);
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

	r = csv_write_start(t->csv, ts);
	if (r < 0) {
		error("time_slice: Failed to write to output file %s", t->csv_name);
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

	r = csv_write_resync(t->csv, ts);
	if (r < 0) {
		error("time_slice: Failed to write to output file %s", t->csv_name);
		return r;
	}

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int time_slice_init(struct consumer * consumer, const char * csv_name)
{
	assert(csv_name);
	int r;

	struct time_slice * t = (struct time_slice *)
		calloc(1, sizeof(struct time_slice));
	if (!t) {
		error("time_slice: Failed to allocate memory");
		r = -ENOMEM;
		goto err;
	}

	t->held_buffers = bufhold_init();
	if (!t->held_buffers) {
		error("time_slice: Failed to allocate memory for held buffers");
		r = -1;
		goto err;
	}

	/* Initialize csv file. */
	t->csv_name = strdup(csv_name);
	if (!t->csv_name) {
		error("time_slice: Failed to allocate memory for csv file name");
		r = -ENOMEM;
		goto err;
	}

	t->csv = csv_open(t->csv_name);
	if (!t->csv) {
		error("time_slice: Failed to open file %s", t->csv_name);
		r = -1;
		goto err;
	}

	consumer_set_module(consumer, time_slice_write, time_slice_start,
			time_slice_resync, time_slice_exit, t);

	return 0;

err:
	if (t->csv)
		csv_close(t->csv);
	if (t->csv_name)
		free(t->csv_name);
	if (t->held_buffers)
		bufhold_exit(t->held_buffers);
	if (t)
		free(t);

	return r;
}
