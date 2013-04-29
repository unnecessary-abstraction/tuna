/*******************************************************************************
	impulse.c: Impulse detection and processing.

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

#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "fft.h"
#include "impulse.h"
#include "output.h"
#include "output_csv.h"
#include "third_octave_levels.h"
#include "timespec.h"
#include "types.h"
#include "uara.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

struct impulse_state {
	double *			buffer;
	uint				buffer_length;
	struct timespec			time_base;
	
	double				y[2];
	
	uint				short_width;
	uint				long_width;
	double				threshold;
	
	bool				in_impulse;
	uint				start;
	uint				end;
	double				peak_positive;
	double				peak_negative;
	uint				peak_positive_offset;
	uint				peak_negative_offset;	
};

static struct impulse_state state;

static struct record_type impulse_type;

struct impulse {
	struct timespec			start;
	uint				center;
	uint				length;
	
	double				peak_positive;
	double				peak_negative;
	uint				peak_positive_offset;
	uint				peak_negative_offset;
	
	struct third_octave_levels	third_octave_levels;
};

static double impulse_parameter(uint offset);
static uint rebase(uint abs_offset);
static void copy_with_padding(void);
static void process_attack(void);
static void process_impulse(void);
static void write_impulse(struct impulse * i);

/*******************************************************************************
	Private functions
*******************************************************************************/

/*
	Calculate the impulse parameter for the sample at the given offset
	within the given buffer. This function must be called on consecutive
	samples.
	
	We assume the buffer starts full of zeros and we assume that sample
	values are stored in the buffer rather than sample energies.
	
	TODO: Initialise filters on first sample.
*/
static double impulse_parameter(uint offset)
{
	uint old_offset, new_offset;
	double old_energy, new_energy;
	
	/* Update short filter. */
	new_offset = offset + state.short_width/2;
	if (new_offset > state.buffer_length)
		new_offset -= state.buffer_length;
	new_energy = state.buffer[new_offset] * state.buffer[new_offset];
	
	old_offset = offset - state.short_width/2;
	if (old_offset > state.buffer_length)
		old_offset += state.buffer_length;
	old_energy = state.buffer[old_offset] * state.buffer[old_offset];
	
	state.y[0] += new_energy - old_energy;
	
	/* Update long filter. */
	new_offset = offset + state.long_width/2;
	if (new_offset > state.buffer_length)
		new_offset -= state.buffer_length;
	new_energy = state.buffer[new_offset] * state.buffer[new_offset];
	
	old_offset = offset - state.long_width/2;
	if (old_offset > state.buffer_length)
		old_offset += state.buffer_length;
	old_energy = state.buffer[old_offset] * state.buffer[old_offset];
	
	state.y[1] += new_energy - old_energy;
	
	/* Return impulse parameter for this sample. */
	return state.y[0] / state.y[1];
}

static uint rebase(uint abs_offset)
{
	uint rel_offset = abs_offset - state.start;
	
	/* Account for wrap around on the buffer. */
	if (rel_offset > state.buffer_length)
		rel_offset += state.buffer_length;
	
	return rel_offset;
}

static void copy_with_padding(void)
{
	uint len;
	if (state.end >= state.start) {
		len = state.end - state.start;
		fft_load(&state.buffer[state.start], 0, len);
		fft_pad(len);
	} else {
		len = state.buffer_length - state.start;
		fft_load(&state.buffer[state.start], 0, len);
		fft_load(state.buffer, len, state.end);
		fft_pad(len + state.end);
	}
}

/* Impulse was too long so we just want to record how quick the attack was and what peak levels were reached. */
static void process_attack(void)
{
	struct impulse output;
	
	output.start = state.time_base;
	timespec_add_ticks(&output.start, state.start);
	output.center = rebase(state.peak_positive_offset);
	output.length = 0;
	output.peak_positive = state.peak_positive;
	output.peak_positive_offset = rebase(state.peak_positive_offset);
	output.peak_negative = state.peak_negative;
	output.peak_negative_offset = rebase(state.peak_negative_offset);
	
	memset(&output.third_octave_levels, 0, sizeof(output.third_octave_levels));
	
	write_impulse(&output);
}

/* Handle a complete impulse. */
static void process_impulse(void)
{
	struct impulse output;
	
	output.start = state.time_base;
	timespec_add_ticks(&output.start, state.start);
	output.center = rebase(state.peak_positive_offset);
	output.length = rebase(state.end);
	output.peak_positive = state.peak_positive;
	output.peak_positive_offset = rebase(state.peak_positive_offset);
	output.peak_negative = state.peak_negative;
	output.peak_negative_offset = rebase(state.peak_negative_offset);
	
	copy_with_padding();
	fft_no_window();
	third_octave_levels(&output.third_octave_levels);
	
	write_impulse(&output);
}

static void write_impulse(struct impulse * i)
{
	struct record rec;
	
	record_init(&rec, &impulse_type);
	output_uint(&rec, i->start.tv_sec);
	output_uint(&rec, i->start.tv_nsec);
	output_uint(&rec, i->center);
	output_uint(&rec, i->length);
	
	output_double(&rec, i->peak_positive);
	output_double(&rec, i->peak_negative);
	output_uint(&rec, i->peak_positive_offset);
	output_uint(&rec, i->peak_negative_offset);
	
	write_third_octave_levels(&rec, &i->third_octave_levels);
	
	record_exit(&rec);
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int impulse_detector_init(double * buffer, uint buffer_length)
{
	state.y[0] = 0;
	state.y[1] = 0;
	
	state.buffer = buffer;
	state.buffer_length = buffer_length;
	state.short_width = (uint)(0.001 * sample_rate);
	state.long_width = sample_rate;
	state.threshold = 2;
	
	record_type_init(&impulse_type, "impulse", &csv_driver);
	
	return 0;
}

void impulse_detector_exit(void)
{
	record_type_exit(&impulse_type);
}

void impulse_detector_sample(uint offset)
{
	double param = impulse_parameter(offset);
	double value = state.buffer[offset];
	
	if (param > state.threshold) {
		if (state.in_impulse) {
			if (offset == state.start) {
				/* We've gone too far and looped round the whole buffer. */
				process_attack();
				state.in_impulse = false;
			}
			/* Continue accumulating samples for impulse. */
			if (value > state.peak_positive) {
				state.peak_positive = value;
				state.peak_positive_offset = offset;
			} else if (value < state.peak_negative) {
				state.peak_negative = value;
				state.peak_negative_offset = offset;
			}
		} else {
			/* Start impulse. */
			state.in_impulse = true;
			state.start = offset;
			state.peak_positive = value;
			state.peak_positive_offset = offset;
			state.peak_negative = value;
			state.peak_negative_offset = offset;
		}
	} else if (state.in_impulse) {
		state.end = offset;
		process_impulse();
		state.in_impulse = false;
	}
}
