/*******************************************************************************
	tuna-analyse.c: A simple analysis pipeline.

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

#include "bufq.h"
#include "compiler.h"
#include "consumer.h"
#include "fft.h"
#include "input_sndfile.h"
#include "log.h"
#include "output_csv.h"
#include "producer.h"
#include "time_slice.h"

int main(int argc, char * argv[])
{
	int r;

	/* Pipeline:
	 *
	 * input_sndfile -> bufq -> time_slice -> output_csv
	 */
	struct producer * in = NULL;
	struct consumer * bufq = NULL;
	struct consumer * time_slice = NULL;
	struct consumer * out = NULL;
	struct fft fft;

	/* These need to be configurable. */
	const char * source = "input.wav";
	const char * sink = "time_slice.csv";
	const char * log_file = "tuna-analyse.log";
	const char * app_name = "tuna-analyse";

	__unused argc;
	__unused argv;
	
	r = log_init(log_file, app_name);
	if (r < 0)
		return r;

	r = fft_init(&fft);
	if (r < 0) {
		error("tuna-analyse: Failed to initialize fft module");
		return r;
	}

	out = output_csv_init(sink);
	if (!out) {
		error("tuna-analyse: Failed to initialise output_csv module");
		r = -1;
		goto cleanup;
	}

	time_slice = time_slice_init(out, &fft);
	if (!time_slice) {
		error("tuna-analyse: Failed to initialise time_slice module");
		r = -1;
		goto cleanup;
	}

	bufq = bufq_init(time_slice);
	if (!bufq) {
		error("tuna-analyse: Failed to initialise bufq module");
		r = -1;
		goto cleanup;
	}

	in = input_sndfile_init(source, bufq);
	if (!in) {
		error("tuna-analyse: Failed to initialise input_sndfile module");
		r = -1;
		goto cleanup;
	}

	r = in->run(in);

cleanup:
	if (in)
		in->exit(in);
	if (bufq)
		bufq->exit(bufq);
	if (time_slice)
		time_slice->exit(time_slice);
	if (out)
		out->exit(out);
	fft_exit(&fft);
	log_exit();
	return r;
}
