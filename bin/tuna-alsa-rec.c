/*******************************************************************************
	tuna-alsa-rec.c: A simple program to record ALSA input to a file.

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

#include <sndfile.h>
#include <stdlib.h>

#include "bufq.h"
#include "compiler.h"
#include "consumer.h"
#include "input_alsa.h"
#include "log.h"
#include "output_sndfile.h"
#include "producer.h"
#include "types.h"

struct producer * in;
struct consumer * bufq;
struct consumer * out;

void __noreturn rec_exit(int r)
{
	if (in)
		in->exit(in);
	if (bufq)
		bufq->exit(bufq);
	if (out)
		out->exit(out);

	log_exit();
	exit(r);
}

void rec_init()
{
	int r;

	const char * source = "hw:0";
	const uint sample_rate = 44100;
	const char * dest_prefix = "rec-";
	const char * dest_suffix = ".wav";
	const int dest_format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	const uint max_samples_per_file = 1000000;

	r = log_init("tuna-alsa-rec.log", "tuna-alsa-rec");
	if (r < 0) {
		/* Don't call rec_exit() without logging available. */
		abort();
	}

	out = output_sndfile_init(dest_prefix, dest_suffix, dest_format, max_samples_per_file);
	if (!out)
		rec_exit(-1);

	bufq = bufq_init(out);
	if (!bufq)
		rec_exit(-1);

	in = input_alsa_init(bufq, source, sample_rate);
	if (!in)
		rec_exit(-1);
}

int main(int argc, char * argv[])
{
	__unused argc;
	__unused argv;

	int r;

	rec_init();
	r = in->run(in);
	rec_exit(r);
}
