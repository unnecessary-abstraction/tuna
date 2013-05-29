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

#include <time.h>
#include "fft.h"
#include "output.h"
#include "output_csv.h"
#include "third_octave_levels.h"
#include "time_slice.h"
#include "timespec.h"
#include "types.h"
#include "uara.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

struct time_slice_state {
	double *			buffer;
	uint				buffer_length;
	struct timespec			time_base;
	
	uint				start;
	uint				count;
	uint				slice_length;
};

static struct time_slice_state state;

static struct record_type time_slice_type;

struct time_slice {
	struct timespec			start;
	
	double				peak_positive;
	double				peak_negative;
	uint				peak_positive_offset;
	uint				peak_negative_offset;
	
	double				m_2;
	double				m_4;
	
	struct third_octave_levels	third_octave_levels;
};

void process_time_array(double * x, struct time_slice * output);
static void copy_data(void);
static void write_time_slice(struct time_slice * output);
void process_time_slice(void);

/*******************************************************************************
	Private functions
*******************************************************************************/

void process_time_array(double * x, struct time_slice * output)
{
	double x_2;
	uint i;

	for (i = 0; i < state.slice_length/2; i++) {
		/* Calculate x^2 and x^4 sums for kurtosis. */
		x_2 = x[i] * x[i];
		output->m_2 += x_2;

		output->m_4 += x_2 * x_2;

		/* Detect Peaks */
		if (x[i] > output->peak_positive) {
			output->peak_positive = x[i];
			output->peak_positive_offset = i;
		} else if (x[i] < output->peak_negative) {
			output->peak_negative = x[i];
			output->peak_negative_offset = i;
		}
	}
}

static void copy_data(void)
{
	uint len;
	uint half_length;
	double * x;
	
	half_length = state.slice_length/2;
	
	/* Move last half time slice to the front of the FFT buffer. */
	x = fft_get_data();
	fft_load(&x[half_length], 0, half_length);
	
	/* Load new half time slice. */
	if (state.start + half_length >= state.buffer_length) {
		len = state.buffer_length - state.start;
		fft_load(&state.buffer[state.start], half_length, len);
		fft_load(state.buffer, half_length + len, half_length - len);
	} else {
		fft_load(&state.buffer[state.start], half_length, half_length);
	}
}

static void write_time_slice(struct time_slice * output)
{
	struct record rec;
	
	record_init(&rec, &time_slice_type);
	output_uint(&rec, output->start.tv_sec);
	output_uint(&rec, output->start.tv_nsec);
	
	output_double(&rec, output->peak_positive);
	output_double(&rec, output->peak_negative);
	output_uint(&rec, output->peak_positive_offset);
	output_uint(&rec, output->peak_negative_offset);
	
	output_double(&rec, output->m_2);
	output_double(&rec, output->m_4);
	
	write_third_octave_levels(&rec, &output->third_octave_levels);
	
	record_exit(&rec);
}

void process_time_slice(void)
{
	double * x;
	struct time_slice output;
	
	output.start = state.time_base;
	timespec_add_ticks(&output.start, state.start, output_sample_rate);
	copy_data();
	x = fft_get_data();
	process_time_array(&x[state.slice_length/2], &output);
	fft_with_window();
	third_octave_levels(&output.third_octave_levels);
	write_time_slice(&output);
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int time_slice_init(double * buffer, uint buffer_length)
{
	state.buffer = buffer;
	state.buffer_length = buffer_length;
	state.count = 0;
	state.start = 0;
	
	state.slice_length = output_sample_rate;
	
	record_type_init(&time_slice_type, "time_slice", &csv_driver);
	
	return 0;
}

void time_slice_exit(void)
{
	record_type_exit(&time_slice_type);
}

void time_slice_sample(uint offset)
{
	state.count++;
	if (state.count == state.slice_length/2) {
		process_time_slice();
		state.count = 0;
		state.start = offset;
	}
}
