/*******************************************************************************
	buffer.c: Sample buffering.

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

#include <malloc.h>
#include <string.h>
#include "buffer.h"
#include "dsp/impulse.h"
#include "dsp/time_slice.h"
#include "types.h"
#include "uara.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

struct buffer_state {
	double *	buffer;
	uint		buffer_length;
	uint		offset;

	uint		quantum;
	uint		delay;
	uint		samples_processed;
};

static struct buffer_state state;

void process_samples(uint count);
void dispatch_samples(uint start, uint count);

/*******************************************************************************
	Private functions
*******************************************************************************/

/*
	This function assumes that <state.offset> + <count> is not greater than
	<state.buffer_length>.
*/
void process_samples(uint count)
{
	uint process_offset;

	/*
		As the consumers of this buffered data are not strictly causal
		functions, we process data with a delay. Therefore nothing is
		processed until at least <state.delay> samples are read. As this
		delay must be less than the buffer size we know we won't loop
		round the buffer until after we've began processing samples so
		we can simply use <state.offset> as the number of unprocessed
		samples available during this initial buffer filling.
	*/
	if (!state.samples_processed) {
		if (state.offset > state.delay)
			/*
				We have some samples to process but it may not
				be the full count.
			*/
			dispatch_samples(0, state.offset - state.delay);
	} else {
		process_offset = state.offset - state.delay;
		if (process_offset >= state.buffer_length) {
			/* We have wrap-around. */
			process_offset += state.buffer_length;
			dispatch_samples(process_offset,
					 state.buffer_length - process_offset);
			dispatch_samples(0, state.offset - process_offset);
		} else
			dispatch_samples(process_offset, count);
	}

	state.offset += count;
	if (state.offset == state.buffer_length)
		state.offset = 0;
}

/*
	This function assumes that <start> + <count> is not greater than
	<state.buffer_length>.
*/
void dispatch_samples(uint start, uint count)
{
	/* TODO: Pass onwards as a block rather than one sample at a time. */
	uint i;
	for (i = 0; i < count; i++) {
		impulse_detector_sample(start + i);
		time_slice_sample(start + i);
	}
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int buffer_init(uint buffer_length, uint delay)
{
	state.buffer = (double *)malloc(buffer_length * sizeof(double));
	if (!state.buffer)
		return -1;
	
	memset(state.buffer, 0, buffer_length * sizeof(double));
	
	state.offset = 0;
	state.buffer_length = buffer_length;
	state.delay = delay;
	state.samples_processed = 0;

	if (output_sample_rate > buffer_length/2)
		state.quantum = buffer_length/2;
	else
		state.quantum = output_sample_rate;
	
	/* Initialise impulse detection and time slice handling. */
	impulse_detector_init(state.buffer, buffer_length);
	time_slice_init(state.buffer, buffer_length);
	
	return 0;
}

void buffer_exit(void)
{
	impulse_detector_exit();
	time_slice_exit();
}

void buffer_samples(double * samples, uint count)
{
	memcpy(&state.buffer[state.offset], samples, count * sizeof(double));

	process_samples(count);
}

/* TODO: Currently assumes single channel operation. */
double * buffer_advise(uint * count)
{
	process_samples(*count);

	/*
		Request state.quantum samples unless this would run past the end
		of the buffer.
	*/
	if (state.offset + state.quantum >= state.buffer_length)
		*count = state.buffer_length - state.offset;
	else
		*count = state.quantum;

	return &state.buffer[state.offset];
}
