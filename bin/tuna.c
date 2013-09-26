/*******************************************************************************
	tuna.c: A simple analysis pipeline.

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

#include <argp.h>
#include <assert.h>
#include <signal.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>

#include "bufq.h"
#include "compiler.h"
#include "consumer.h"
#include "fft.h"
#include "input_alsa.h"
#include "input_sndfile.h"
#include "input_zero.h"
#include "log.h"
#include "output_null.h"
#include "output_sndfile.h"
#include "producer.h"
#include "time_slice.h"

#ifdef ENABLE_ADS1672
#include "input_ads1672.h"
#endif

/* Globals. */
struct producer * in = NULL;
struct consumer * bufq = NULL;
struct consumer * out = NULL;
struct fft * fft = NULL;

/* Set program name, version and bug reporting address so that they can be seen
 * by argp.
 */
const char * argp_program_version = "tuna 0.1-pre1";
const char * argp_program_bug_address = "https://bitbucket.org/underwater-acoustics/tuna/issues";

static char docstring[] = "Toolkit for Underwater Noise Analysis (TUNA)";

static const struct argp_option options[] = {
	{"input", 'i', "SOURCE", 0, "Configure input module", 0},
	{"output", 'o', "SINK", 0, "Configure output module", 0},
	{"sample-rate", 'r', "RATE", 0, "Set sample rate for input module if supported", 0},
	{0, 0, 0, 0, 0, 0}
};

struct arguments {
	char * input;
	char * output;
	uint sample_rate;
};

char * safe_strdup(const char * p)
{
	char * r = strdup(p);
	if (!r) {
		error("tuna: Failed to allocate memory for internal string");
		exit(-ENOMEM);
	}
	return r;
}

void set_defaults(struct arguments * args)
{
	assert(args);

	args->input = safe_strdup("alsa:hw:0");
	args->output = safe_strdup("time_slice:results.csv");
	args->sample_rate = 44100;
}

/* Split a string of the form "a:b" so that param just contains "a" and "b" is
 * returned.
 */
char * split_param(char * param)
{
	char * split = strchr(param, ':');
	if (!split)
		return NULL;

	/* Terminate param at separator so it just contains "a". */
	*split = '\0';

	/* Return a string starting one character after the separator, which
	 * will just contain "b".
	 */
	return ++split;
}

static error_t parse(int key, char * param, struct argp_state * state)
{
	assert(state);

	struct arguments * args = (struct arguments *)state->input;

	switch (key) {
	    case 'i':
		args->input = param;
		break;

	    case 'o':
		args->output = param;
		break;

	    case 'r':
		args->sample_rate = (uint) strtoul(param, NULL, 10);
		break;

	    default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = {options, parse, NULL, docstring, NULL, NULL, NULL};

int init_output(struct arguments * args)
{
	assert(args);

	char * sink = split_param(args->output);
	int format;
	uint max_samples_per_file;
	int r;

	if (strcmp(args->output, "time_slice") == 0) {
		fft = (struct fft *)malloc(sizeof(struct fft));
		if (!fft) {
			error("tuna: Failed to allocate memory for fft module");
			return -ENOMEM;
		}

		r = fft_init(fft);
		if (r < 0) {
			error("tuna: Failed to initialize fft module");

			/* Manually free and zero here as exit_all assumes
			 * fft_init succeeded if fft is not NULL.
			 */
			free(fft);
			fft = NULL;
			return r;
		}

		out = time_slice_init(sink, fft);
	} else if (strcmp(args->output, "sndfile") == 0) {
		/* TODO: These should be configurable. */
		format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		max_samples_per_file = 60 * 60 * args->sample_rate; /* One hour. */
		out = output_sndfile_init(sink, ".wav", format, max_samples_per_file);
	} else if (strcmp(args->output, "null") == 0) {
		out = output_null_init();
	} else {
		error("tuna: Unknown output module %s", args->output);
		return -EINVAL;
	}

	if (!out) {
		error("tuna: Failed to initialise %s output", args->output);
		return -1;
	}

	return 0;
}

int init_input(struct arguments * args)
{
	assert(args);

	char * source = split_param(args->input);

	bufq = bufq_init(out);
	if (!bufq) {
		error("tuna: Failed to initialise bufq module");
		return -1;
	}

	if (strcmp(args->input, "sndfile") == 0) {
		in = input_sndfile_init(bufq, source);
	} else if (strcmp(args->input, "alsa") == 0) {
		in = input_alsa_init(bufq, source, args->sample_rate);
	} else if (strcmp(args->input, "zero") == 0) {
		in = input_zero_init(bufq, args->sample_rate);
#ifdef ENABLE_ADS1672
	} else if (strcmp(args->input, "ads1672") == 0) {
		in = input_ads1672_init(bufq);
#endif
	} else {
		error("tuna: Unknown input module %s", args->input);
		return -EINVAL;
	}

	if (!in) {
		error("tuna: Failed to initialise %s input", args->input);
		return -1;
	}

	return 0;
}

void exit_all()
{
	if (in)
		in->exit(in);
	if (bufq)
		bufq->exit(bufq);
	if (out)
		out->exit(out);
	if (fft) {
		fft_exit(fft);
		free(fft);
	}
}

void sigterm_handler(int sig)
{
	msg("tuna: Terminating due to SIGTERM");

	in->stop(in, sig);
}

int main(int argc, char * argv[])
{
	int r;
	const char * log_file = "tuna.log";
	const char * app_name = "tuna";

	r = log_init(log_file, app_name);
	if (r < 0)
		return r;

	/* Argument parsing. */
	struct arguments args;
	set_defaults(&args);
	argp_parse(&argp, argc, argv, 0, 0, &args);

	r = init_output(&args);
	if (r < 0)
		return r;

	r = init_input(&args);
	if (r < 0)
		return r;

	/* Setup signal handler now that 'in' is valid (as the handler calls
	 * in->stop() ).
	 */
	signal(SIGTERM, sigterm_handler);

	r = in->run(in);

	exit_all();
	log_exit();

	return r;
}
