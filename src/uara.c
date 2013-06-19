/*******************************************************************************
	uara.c: Main program entry point.

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

#include "analysis.h"
#include "bufq.h"
#include "compiler.h"
#include "input_sndfile.h"
#include "log.h"
#include "output_csv.h"
#include "uara.h"

int main(int argc, char * argv[])
{
	int r;
	struct producer * in;
	struct consumer * bufq;
	struct consumer * analysis;
	struct consumer * out_impulse;
	struct consumer * out_time_slice;

	/* These need to be configurable. */
	const char * source = "input.wav";
	const char * sink_time_slice = "time_slice.csv";
	const char * sink_impulse = "impulse.csv";
	const char * log_file = "uara.log";
	const char * app_name = "uara";

	__unused argc;
	__unused argv;
	
	r = log_init(log_file, app_name);
	if (r < 0)
		goto err_log;

	out_time_slice = output_csv_init(sink_time_slice);
	if (!out_time_slice) {
		error("uara: Failed to initialise output_csv module for time slice results");
		r = -1;
		goto err_time_slice;
	}

	out_impulse = output_csv_init(sink_impulse);
	if (!out_impulse) {
		error("uara: Failed to initialise output_csv module for impulse results");
		r = -1;
		goto err_impulse;
	}

	analysis = analysis_init(out_time_slice, out_impulse);
	if (!analysis) {
		error("uara: Failed to initialise analysis module");
		r = -1;
		goto err_analysis;
	}

	bufq = bufq_init(analysis);
	if (!bufq) {
		error("uara: Failed to initialise bufq module");
		r = -1;
		goto err_bufq;
	}

	in = input_sndfile_init(source, bufq);
	if (!in) {
		error("uara: Failed to initialise input_sndfile module");
		r = -1;
		goto err_in;
	}

	r = in->run(in);

	in->exit(in);
err_in:
	bufq->exit(bufq);
err_bufq:
	analysis->exit(analysis);
err_analysis:
	out_impulse->exit(out_impulse);
err_impulse:
	out_time_slice->exit(out_time_slice);
err_time_slice:
	log_exit();
err_log:
	return r;
}
