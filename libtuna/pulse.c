/*******************************************************************************
	pulse.c: Per *pulse* processing.

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
#include <string.h>

#include "bufhold.h"
#include "cbuf.h"
#include "consumer.h"
#include "csv.h"
#include "env_estimate.h"
#include "fft.h"
#include "log.h"
#include "onset_threshold.h"
#include "offset_threshold.h"
#include "pulse.h"
#include "tol.h"
#include "types.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

enum pulse_state {
	STATE_NONPULSE,
	STATE_PULSE
};

struct pulse_results {
	sample_t				peak_positive;
	sample_t				peak_negative;
	uint					peak_positive_offset;
	uint					peak_negative_offset;

	/* Offsets of 5% and 95% energy, giving the 90% energy duration. */
	uint					offset_5;
	uint					offset_95;

	/* attack_duration = peak_positive_offset - offset_5;
	 * decay_duration = offset_95 - peak_positive_offset;
	 * duration_90 = offset_95 - offset_5;
	 */

	float					tols[];
};

struct pulse_processor {
	/* Third-octave analysis. */
	struct tol *				tol;

	/* Current result set. */
	struct pulse_results *			results;

	/* List of held buffers. */
	struct bufhold *			held_buffers;

	/* Parameters passed during initialisation. */
	const struct pulse_params *	params;

	/* Output stream for writing results in csv format. */
	FILE *					csv;

	/* Filename of csv output stream. */
	char *					csv_name;

	/* FFT: This is provided during initialisation and is not exited when
	 * the pulse processor is exited.
	 */
	struct fft *				fft;

	/* Pointer to FFT data buffer, when open. */
	float *					fft_data;

	/* Onset threshold tracker. */
	struct onset_threshold *		onset;

	/* Offset threshold tracker. */
	struct offset_threshold *		offset;

	/* Envelope estimation. */
	struct env_estimate *			env;

	/* Current state: See enum above. */
	enum pulse_state			state;

	/* Length of the FFT window. Pulses are padded to this length before
	 * transformation so that only one set of third octave level
	 * coefficients are needed.
	 */
	uint					fft_length;

	/* Index within current pulse, once a pulse has been detected. */
	uint					index;

	/* Number of active third octave levels. */
	uint					n_tol;

	/* Maximum pulse duration in samples, calculated from
	 * params->pulse_max_duration.
	 */
	uint					pulse_max_duration_w;

	/* Minimum decay duration in samples, calculated from
	 * params->pulse_min_decay.
	 */
	uint					pulse_min_decay_w;

	/* Current detection threshold for pulse onset detection.
	 *
	 * TODO: Compute this only when the minimum changes, not every sample.
	 */
	sample_t				threshold;
};

static int write_results(struct pulse_processor * p)
{
	assert(p);

	int r;
	uint i;

	r = csv_write_sample(p->csv, p->results->peak_positive);
	if (r < 0)
		goto err;

	r = csv_write_sample(p->csv, p->results->peak_negative);
	if (r < 0)
		goto err;

	r = csv_write_uint(p->csv, p->results->peak_positive_offset);
	if (r < 0)
		goto err;

	r = csv_write_uint(p->csv, p->results->peak_negative_offset);
	if (r < 0)
		goto err;

	r = csv_write_uint(p->csv, p->results->offset_5);
	if (r < 0)
		goto err;

	r = csv_write_uint(p->csv, p->results->offset_95);
	if (r < 0)
		goto err;

	for (i = 0; i < p->n_tol; i++) {
		r = csv_write_float(p->csv, p->results->tols[i]);
		if (r < 0)
			goto err;
	}

	r = csv_next(p->csv);
	if (r < 0)
		goto err;

	return 0;

err:
	error("pulse: Failed to write to output file %s", p->csv_name);
	return r;
}

void calc_offsets(struct pulse_processor * p)
{
	assert(p);

	uint i;
	float energy = 0.0f;
	float energy_5perc;

	/* This looks hideously inefficient - there must be a faster way to find
	 * these offsets.
	 */

	for (i = 0; i < p->index; i++)
		energy += p->fft_data[i] * p->fft_data[i];

	energy_5perc = energy / 20.0f;

	/* Find 5% offset. */
	p->results->offset_5 = 0;
	energy = p->fft_data[0] * p->fft_data[0];
	while (energy <= energy_5perc) {
		p->results->offset_5++;
		energy += p->fft_data[p->results->offset_5] *
			p->fft_data[p->results->offset_5];
	}

	/* Find 5% offset from end. */
	p->results->offset_95 = p->index - 1;
	energy = p->fft_data[p->results->offset_95] *
		p->fft_data[p->results->offset_95];
	while (energy < energy_5perc) {
		p->results->offset_95--;
		energy += p->fft_data[p->results->offset_95] *
			p->fft_data[p->results->offset_95];
	}
}

void process_start_pulse(struct pulse_processor * p)
{
	assert(p);

        memset(p->results, 0, sizeof(struct pulse_results) + p->n_tol * sizeof(float));

	p->index = 0;
}

static void process_end_pulse(struct pulse_processor * p)
{
	assert(p);

	calc_offsets(p);

	/* Add padding, perform FFT and third octave analysis. */
	memset(&p->fft_data[p->index], 0,
			(p->fft_length - p->index) * sizeof(float));
	fft_transform(p->fft);
	tol_calculate(p->tol, p->fft_data, p->results->tols);

	write_results(p);
}

/* Returns 1 if a new positive peak was found, 0 otherwise. This allows the
 * pulse end detector to be reset when necessary.
 */
static int process_sample(struct pulse_processor * p, sample_t x)
{
	assert(p);
	int r = 0;

	/* Copy into fft buffer. */
	p->fft_data[p->index] = (float)x;

	/* Detect Peaks */
	if (x > p->results->peak_positive) {
		p->results->peak_positive = x;
		p->results->peak_positive_offset = p->index;

		r = 1;
	} else if (x < p->results->peak_negative) {
		p->results->peak_negative = x;
		p->results->peak_negative_offset = p->index;
	}

	p->index++;
	return r;
}

static void process_data(struct pulse_processor * p, sample_t * data,
		uint count)
{
	assert(p);
	assert(data);

	uint i;

	for (i = 0; i < count; i++)
		process_sample(p, data[i]);
}

/* Discard data older than the given offset measured backwards from the end of
 * the latest held buffer. Return the buffer during which this offset occurs,
 * that is the oldest buffer which remains after discarding. This returned
 * buffer is truncated from the left to the given offset.
 */
static struct held_buffer * discard_leading_data(struct pulse_processor * p, uint offset)
{
	assert(p);

	/* Work backwards through the list of held buffers until we get the the
	 * starting sample.
	 */
	struct held_buffer * h;
	struct held_buffer * h_tmp;
	struct held_buffer * h_release;
	uint i;

	h = bufhold_newest(p->held_buffers);
	i = bufhold_count(h);

	while (i < offset) {
		h = bufhold_prev(h);
		if (!h) {
			/* This should never happen, make sure we catch it if it
			 * does though.
			 */
			error("pulse: Internal error - expected data is not present");
		}
		i += bufhold_count(h);
	}

	/* Line up the first buffer with the given offset and discard any
	 * previous buffers.
	 */
	bufhold_advance(p->held_buffers, h, i - offset);

	h_tmp = bufhold_oldest(p->held_buffers);
	while (h != h_tmp) {
		h_release = h_tmp;
		h_tmp = bufhold_next(h_tmp);
		bufhold_release(p->held_buffers, h_release);
	}

	return h;
}

/* Process and the discard all held data. */
static void process_leading_data(struct pulse_processor * p, uint offset)
{
	assert(p);

	struct held_buffer * h;
	struct held_buffer * h_release;

	/* Work backwards through the list of held buffers until we get the the
	 * starting sample.
	 */
	h = discard_leading_data(p, offset);

	/* Now process the remaining held data in order. */
	while (h) {
		process_data(p, bufhold_data(h), bufhold_count(h));
		h_release = h;
		h = bufhold_next(h);
		bufhold_release(p->held_buffers, h_release);
	}
}

static sample_t calc_envelope(struct pulse_processor * p, sample_t x)
{
	assert(p);

	sample_t e;

	/* Calculate envelope estimate and detection threshold. */
	e = env_estimate_next(p->env, x);
	p->threshold = onset_threshold_next(p->onset, e);

	return e;
}

/* Returns 0 to remain in pulse, 1 to exit pulse. */
static int check_pulse_end(struct pulse_processor * p, sample_t env)
{
	assert(p);

	sample_t decay_threshold, delayed_min;

	decay_threshold = offset_threshold_next(p->offset, env);
	delayed_min = offset_threshold_delayed_min(p->offset);

	/* Check for min/max pulse duration. */
	if (p->index > p->pulse_max_duration_w)
		return 1;
	else if ((p->index - p->results->peak_positive_offset) < p->pulse_min_decay_w)
		/* TODO: Do we still need to update delayed_min in this case? */
		return 0;

	/* The delayed minimum should be greater than or equal to the calculated
	 * threshold if the signal envelope is decaying faster than the desired
	 * rate.
	 */
	if (delayed_min < decay_threshold)
		return 1;

	return 0;
}

static void detect_data(struct pulse_processor * p, sample_t * data,
		uint count)
{
	assert(p);
	assert(data);

	uint i;
	int start_offset;
	sample_t e;

	for (i = 0; i < count; i++) {
		e = calc_envelope(p, data[i]);

		/* Check against detection threshold if we're not already in a
		 * pulse.
		 */
		if ((p->state == STATE_NONPULSE) && (e > p->threshold)) {
			p->state = STATE_PULSE;
			process_start_pulse(p);

			/* Mark the pulse as beginning from the minimum point.
			 *
			 * We want start_offset to be the signed offset from the
			 * start of the current data buffer so we subtract the
			 * age of the current minimum from our current offset
			 * into the buffer.
			 */
			start_offset = i - onset_threshold_age(p->onset);

			/* Process the data between the minimum point and the
			 * start of the buffer passed to this function.
			 */
			if (start_offset < 0) {
				process_leading_data(p, -start_offset);
				start_offset = 0;
			}

			/* Process the data in the buffer passed to this
			 * function between the minimum point and the current
			 * sample.
			 */
			process_data(p, &data[start_offset], i - start_offset);

			/* Setup the pulse end detector. */
			offset_threshold_reset(p->offset, e);
		} else if (p->state == STATE_PULSE) {
			/* We're in a pulse but this isn't the first sample of
			 * it.
			 */

			if (process_sample(p, data[i])) {
				/* A new peak was found. */
				offset_threshold_reset(p->offset, e);
			}

			if (check_pulse_end(p, e)) {
				p->state = STATE_NONPULSE;
				process_end_pulse(p);
			}
		}
	}
}

void pulse_exit(struct consumer * consumer)
{
	assert(consumer);

	struct pulse_processor * p;
	
	p = (struct pulse_processor *)consumer_get_data(consumer);

	if (p->onset)
		onset_threshold_exit(p->onset);

	if (p->offset)
		offset_threshold_exit(p->offset);

	if (p->results)
		free(p->results);

	if (p->tol)
		tol_exit(p->tol);

	if (p->env)
		env_estimate_exit(p->env);

	if (p->fft)
		fft_exit(p->fft);

	bufhold_release_all(p->held_buffers);
	bufhold_exit(p->held_buffers);
	csv_close(p->csv);
	free(p->csv_name);
	free(p);
}

int pulse_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);
	assert(buf);

	int r;
	struct pulse_processor * p;
	
	p = (struct pulse_processor *)consumer_get_data(consumer);

	detect_data(p, buf, count);

	/* Discard all data before the current minimum if we are not currently
	 * in a pulse as it will not be needed.
	 */
	int start_offset = count - onset_threshold_age(p->onset);
	if (start_offset < 0)
		discard_leading_data(p, -start_offset);

	r = bufhold_add(p->held_buffers, buf, count);
	if (r < 0) {
		error("pulse: Failed to hold buffer");
		return r;
	}

	return 0;
}

int pulse_start(struct consumer * consumer, uint sample_rate,
		struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	int r;
	struct pulse_processor * p;
	
	p = (struct pulse_processor *)consumer_get_data(consumer);

	/* Reset detector. */
	p->state = STATE_NONPULSE;

	/* Convert parameters. */
	p->pulse_min_decay_w = (uint) floor(p->params->pulse_min_decay * sample_rate);
	p->pulse_max_duration_w = (uint) floor(p->params->pulse_max_duration * sample_rate);

	p->env = env_estimate_init(p->params->Tc, sample_rate, p->params->sample_limit);
	if (!p->env) {
		error("pulse: Failed to initialise envelope estimator");
		return -1;
	}

	p->onset = onset_threshold_init(p->params->Tw, sample_rate, p->params->threshold_ratio);
	if (!p->onset) {
		error("pulse: Failed to initialise onset threshold tracking");
		return -1;
	}

	p->offset = offset_threshold_init(p->params->Td, sample_rate, p->params->decay_threshold_ratio);
	if (!p->offset) {
		error("pulse: Failed to initialise offset threshold tracking");
		return -1;
	}

	p->fft_length = p->pulse_max_duration_w; /* 1 s long FFT. */
	p->fft = fft_init(p->fft_length);
	if (!p->fft) {
		error("pulse: Failed to initialise FFT");
		return -1;
	}
	p->fft_data = fft_get_data(p->fft);

	p->tol = tol_init(sample_rate, p->fft_length/2, 0.4, 3);
	if (!p->tol) {
		error("pulse: Failed to initialise third octave level calculation");
		return -1;
	}

	p->n_tol = tol_get_num_levels(p->tol);

	p->results = (struct pulse_results *)
		malloc(sizeof(struct pulse_results) + (p->n_tol + 1) *
				sizeof(float));
	if (!p->results) {
		error("pulse: Failed to allocate memory for results");
		return -ENOMEM;
	}

	r = csv_write_start(p->csv, ts);
	if (r < 0) {
		error("pulse: Failed to write to output file %s", p->csv_name);
		return r;
	}

	return 0;
}

int pulse_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	int r;
	struct pulse_processor * p;
	
	p = (struct pulse_processor *)consumer_get_data(consumer);

	bufhold_release_all(p->held_buffers);

	/* Reset detector. */
	p->state = STATE_NONPULSE;
	env_estimate_reset(p->env);
	onset_threshold_reset(p->onset);

	r = csv_write_resync(p->csv, ts);
	if (r < 0) {
		error("pulse: Failed to write to output file %s", p->csv_name);
		return r;
	}

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int pulse_init(struct consumer * consumer, const char * csv_name,
		const struct pulse_params * params)
{
	assert(consumer);
	assert(csv_name);
	assert(params);

	int r;
	struct pulse_processor * p;
	p = (struct pulse_processor *) malloc(sizeof(struct pulse_processor));
	if (!p) {
		error("pulse: Failed to allocate memory");
		r = -ENOMEM;
		goto err;
	}

	p->held_buffers = bufhold_init();
	if (!p->held_buffers) {
		error("pulse: Failed to initialise bufhold");
		r = -1;
		goto err;
	}

	/* Initialize csv file. */
	p->csv_name = strdup(csv_name);
	if (!p->csv_name) {
		error("pulse: Failed to allocate memory for csv file name");
		r = -1;
		goto err;
	}

	p->csv = csv_open(p->csv_name);
	if (!p->csv) {
		error("pulse: Failed to open file %s", p->csv_name);
		r = -1;
		goto err;
	}

	p->params = params;
	p->env = NULL;
	p->onset = NULL;
	p->offset = NULL;
	p->fft = NULL;
	p->fft_data = NULL;

	consumer_set_module(consumer, pulse_write, pulse_start, pulse_resync,
			pulse_exit, p);

	return 0;

err:
	if (p) {
		if (p->csv)
			fclose(p->csv);
		if (p->csv_name)
			free(p->csv_name);
		if (p->held_buffers)
			bufhold_exit(p->held_buffers);
		free(p);
	}

	return r;
}
