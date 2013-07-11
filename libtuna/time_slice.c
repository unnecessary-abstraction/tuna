/*******************************************************************************
	time_slice.c: Per time-slice processing.

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

#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>

#include "buffer.h"
#include "compiler.h"
#include "consumer.h"
#include "list.h"
#include "log.h"
#include "slab.h"
#include "time_slice.h"
#include "tol.h"
#include "types.h"
#include "window.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

#define MAX_TIME_SLICE_RESULTS (6 + MAX_THIRD_OCTAVE_LEVELS)

struct held_buffer {
	/* We may be starting at an offset into the buffer due to previous
	 * processing or other reasons. If so, we will have a data pointer which
	 * is different to the base pointer which references the base of the
	 * allocated memory for the buffer. This is because we need to pass the
	 * base address of the allocated buffer to free(), not a data pointer
	 * which may have been subject to an offset.
	 *
	 * Note that count refers to the number of samples actually in the
	 * buffer, beginning at the data pointer, not the total length of the
	 * whole buffer itself.
	 */
	sample_t *		base;
	sample_t *		data;
	uint			count;

	struct list_entry	e;
};

struct time_slice {
	struct consumer		consumer;

	/* The following fields are initialised in time_slice_init(). */
	struct list		held_buffers;
	struct slab		held_buffer_allocator;
	struct consumer *	out;
	struct fft *		fft;

	/* The following fields are initialised in time_slice_start(). */
	struct tol		tol;
	double *		window;
	uint			sample_rate;
	uint			slice_period;
	uint			available;
	uint			n_tol;

	/* The following field is used within process_time_slice(). */
	uint			index;
};

struct time_slice_results {
	double			peak_positive;
	double			peak_negative;
	uint			peak_positive_offset;
	uint			peak_negative_offset;

	double			m_2;
	double			m_4;

	struct tol_results	tol;
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

	return 6 + t->n_tol;
}

static void write_results(struct time_slice * t, struct time_slice_results * r)
{
	assert(t);
	assert(r);

	uint i;
	uint frames;

	/* We need to convert our results back into the original sample type and
	 * put them in a contiguous array.
	 *
	 * TODO: We may need to do something more intelligent than just type
	 * casting.
	 */
	frames = MAX_TIME_SLICE_RESULTS;
	sample_t * buf = (sample_t *)buffer_acquire(&frames);
	if (!buf)
		fatal("time_slice: Failed to allocate memory for results");
	
	buf[0] = (sample_t)r->peak_positive;
	buf[1] = (sample_t)r->peak_negative;
	buf[2] = (sample_t)r->peak_positive_offset;
	buf[3] = (sample_t)r->peak_negative_offset;
	buf[4] = (sample_t)r->m_2;
	buf[5] = (sample_t)r->m_4;

	for (i = 0; i < t->n_tol; i++)
		buf[6 + i] = (sample_t)r->tol.values[i];

	t->out->write(t->out, buf, values_per_slice(t));

	buffer_release(buf);
}

static void process_buffer(struct time_slice * t, struct held_buffer * h, double * fft_data, struct time_slice_results * r)
{
	assert(t);
	assert(h);
	assert(fft_data);
	assert(r);

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
	double x, x_2;
	uint offset = 0;
	
	avail = h->count;
	if (avail && t->index < len/4) {
		c = min(len/4 - t->index, avail);
		for (i = 0; i < c; i++) {
			x = (double)h->data[i];
			fft_data[t->index] = x * t->window[t->index];
			t->index++;
		}
		avail -= c;
		offset = i;
	}
	if (avail && t->index < len*3/4) {
		c = min(len*3/4 - t->index, avail);
		for (i = 0; i < c; i++) {
			x = (double)h->data[offset + i];

			/* Calculate x^2 and x^4 sums for kurtosis. */
			x_2 = x * x;

			r->m_2 += x_2;
			r->m_4 += x_2 * x_2;

			/* Detect Peaks */
			if (x > r->peak_positive) {
				r->peak_positive = x;
				r->peak_positive_offset = t->index - len/4;
			} else if (x < r->peak_negative) {
				r->peak_negative = x;
				r->peak_negative_offset = t->index - len/4;
			}
			
			fft_data[t->index] = x * t->window[t->index];
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
			x = (double)h->data[offset + i];
			fft_data[t->index] = x * t->window[t->index];
			t->index++;
		}
		avail -= c;
		offset += i;
	}
}

static void process_time_slice(struct time_slice * t)
{
	assert(t);

	struct held_buffer * h;
	struct list_entry * e;
	struct time_slice_results r;
	double * fft_data;
	uint start, offset;

	fft_data = fft_open(t->fft);

	memset(&r, 0, sizeof(r));

	t->index = 0;

	e = list_head(&t->held_buffers);
	while (e) {
		h = container_of(e, struct held_buffer, e);
		start = t->index;
		process_buffer(t, h, fft_data, &r);

		/* Check whether this buffer extends into the second half of
		 * this time slice, if not then we can discard it.
		 */
		if (t->index <= t->slice_period) {
			e = list_next(&h->e);
			list_remove(&h->e);
			buffer_release(h->base);
			slab_free(&t->held_buffer_allocator, h);
		} else {
			/* Adjust start of buffer if this is the first buffer
			 * that we need to keep for the next time slice.
			 */
			if (start < t->slice_period) {
				offset = t->slice_period - start;
				h->data += offset;
				h->count -= offset;
			}

			e = list_next(&h->e);
		}
	}

	fft_transform(t->fft);
	tol_calculate(&t->tol, fft_data, &r.tol);

	fft_close(t->fft);
	write_results(t, &r);
}

static void release_all(struct time_slice * t)
{
	assert(t);

	struct held_buffer * h;
	struct list_entry * e;

	while ((e = list_pop(&t->held_buffers))) {
		h = container_of(e, struct held_buffer, e);
		buffer_release(h->base);
		slab_free(&t->held_buffer_allocator, h);
	}
}

static void hold(struct time_slice * t, sample_t * buf, uint count)
{
	assert(t);
	assert(buf);

	struct held_buffer * h = (struct held_buffer *)slab_alloc(&t->held_buffer_allocator);

	h->base = buf;
	h->data = buf;
	h->count = count;
	buffer_addref(buf);
	list_enqueue(&t->held_buffers, &h->e);
}

void time_slice_exit(struct consumer * consumer)
{
	assert(consumer);

	struct time_slice * t = container_of(consumer, struct time_slice, consumer);

	release_all(t);
	list_exit(&t->held_buffers);
	slab_exit(&t->held_buffer_allocator);
	tol_exit(&t->tol);
	t->out->exit(t->out);
}

int time_slice_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);
	assert(buf);

	struct time_slice * t = container_of(consumer, struct time_slice, consumer);
	
	t->available += count;
	hold(t, buf, count);

	while (t->available >= t->slice_period) {
		process_time_slice(t);
		t->available -= t->slice_period;
	}

	return 0;
}

int time_slice_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	int r;
	struct time_slice * t = container_of(consumer, struct time_slice, consumer);

	t->sample_rate = sample_rate;
	t->slice_period = sample_rate / 2;
	t->available = 0;

	/* Create window function. */
	t->window = (double *)malloc(sizeof(double) * t->slice_period * 2);
	if (!t->window) {
		error("time_slice: Failed to allocate memory for window function");
		return -ENOMEM;
	}

	window_init_sine(t->window, t->slice_period * 2);

	fft_set_length(t->fft, sample_rate);
	r = tol_init(&t->tol, t->sample_rate, t->slice_period * 2, 0.4, 3);
	if (r < 0) {
		error("time_slice: Failed to initialise third octave level calculation");
		free(t->window);
		return r;
	}

	t->n_tol = (uint)r;

	/* Pass the start on but give the number of analysis values per time
	 * slice as the sample rate.
	 */
	return t->out->start(t->out, values_per_slice(t), ts);
}

int time_slice_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	struct time_slice * t = container_of(consumer, struct time_slice, consumer);

	/* We're going to have to dump old data. */
	release_all(t);
	t->available = 0;

	/* Pass the resync on. */
	return t->out->resync(t->out, ts);
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct consumer * time_slice_init(struct consumer * out, struct fft * f)
{
	assert(out);
	assert(f);

	struct time_slice * t = (struct time_slice *)malloc(sizeof(struct time_slice));
	if (!t) {
		error("time_slice: Failed to allocate memory");
		return NULL;
	}

	memset(t, 0, sizeof(*t));

	list_init(&t->held_buffers);
	slab_init(&t->held_buffer_allocator, sizeof(struct held_buffer), offsetof(struct held_buffer, e));
	t->out = out;
	t->fft = f;

	/* Setup consumer and return. */
	t->consumer.write = time_slice_write;
	t->consumer.start = time_slice_start;
	t->consumer.resync = time_slice_resync;
	t->consumer.exit = time_slice_exit;

	return &t->consumer;
}
