/*******************************************************************************
	tuna.c: A simple analysis pipeline.

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

/* Defaults. */
const char * default_input = "alsa:hw:0";
const char * default_output = "time_slice:results.csv";

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
	{"bufq", 'q', "BOOL", OPTION_ARG_OPTIONAL, "Enable (BOOL=1) or disable (BOOL=0) buffer queueing", 0},
	{0, 0, 0, 0, 0, 0}
};

struct arguments {
	char * input;
	char * output;
	uint sample_rate;
	int use_bufq;
};

struct arguments * args_init()
{
	struct arguments * args;

	args = (struct arguments *) malloc(sizeof(struct arguments));
	if (!args) {
		error("tuna: Failed to allocate memory for arguments");
		return NULL;
	}

	/* We must copy the default strings as they will be modified during
	 * parsing.
	 */
	args->input = strdup(default_input);
	if (!args->input) {
		error("tuna: Failed to allocate memory for default input specifier");
		return NULL;
	}

	args->output = strdup(default_output);
	if (!args->output) {
		error("tuna: Failed to allocate memory for default output specifier");
		return NULL;
	}

	args->sample_rate = 44100;
	args->use_bufq = 0;

	return args;
}

void args_exit(struct arguments * args)
{
	free(args->input);
	free(args->output);
	free(args);
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
		/* Replace default/previous input specifier. */
		free(args->input);
		args->input = param;
		break;

	    case 'o':
		/* Replace default/previous output specifier. */
		free(args->output);
		args->output = param;
		break;

	    case 'r':
		args->sample_rate = (uint) strtoul(param, NULL, 10);
		break;

	    case 'q':
		if (param)
			args->use_bufq = (int) strtol(param, NULL, 10);
		else
			args->use_bufq = 1;

	    default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = {options, parse, NULL, docstring, NULL, NULL, NULL};

int output_init(struct arguments * args)
{
	assert(args);

	char * sink = split_param(args->output);
	int format;
	uint max_samples_per_file;
	int r;

	out = consumer_new();
	if (!out) {
		error("tune: Failed to create consumer object");
		return -1;
	}

	if (strcmp(args->output, "time_slice") == 0) {
		fft = fft_init();
		if (!fft) {
			error("tuna: Failed to initialize fft module");

			return -ENOMEM;
		}

		r = time_slice_init(out, sink, fft);
	} else if (strcmp(args->output, "sndfile") == 0) {
		/* TODO: These should be configurable. */
		format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		max_samples_per_file = 60 * 60 * args->sample_rate; /* One hour. */

		r = output_sndfile_init(out, sink, ".wav", format,
				max_samples_per_file);
	} else if (strcmp(args->output, "null") == 0) {
		r = output_null_init(out);
	} else {
		error("tuna: Unknown output module %s", args->output);
		return -EINVAL;
	}

	if (r < 0) {
		error("tuna: Failed to initialise %s output", args->output);
		return r;
	}

	return 0;
}

int input_init(struct arguments * args)
{
	assert(args);

	char * source = split_param(args->input);
	int r;
	struct consumer * target;

	if (args->use_bufq) {
		bufq = consumer_new();
		if (!bufq) {
			error("tuna: Failed to create consumer object for bufq");
			return -1;
		}

		r = bufq_init(bufq, out);
		if (r < 0) {
			error("tuna: Failed to initialise bufq module");
			return r;
		}

		target = bufq;
	} else
		target = out;

	in = producer_new();
	if (!in) {
		error("tuna: Failed to create producer object");
		return -1;
	}

	if (strcmp(args->input, "sndfile") == 0)
		r = input_sndfile_init(in, target, source);
	else if (strcmp(args->input, "alsa") == 0)
		r = input_alsa_init(in, target, source, args->sample_rate);
	else if (strcmp(args->input, "zero") == 0)
		r = input_zero_init(in, target, args->sample_rate);
#ifdef ENABLE_ADS1672
	else if (strcmp(args->input, "ads1672") == 0)
		r = input_ads1672_init(in, target);
#endif
	else {
		error("tuna: Unknown input module %s", args->input);
		return -EINVAL;
	}

	if (r < 0) {
		error("tuna: Could not initialise input module %s", args->input);
		return r;
	}

	return 0;
}

void input_exit()
{
	if (in)
		producer_exit(in);
	if (bufq)
		consumer_exit(bufq);
}

void output_exit()
{
	if (out)
		consumer_exit(out);
	if (fft)
		fft_exit(fft);
}

void sigterm_handler(int sig)
{
	int r;

	msg("tuna: Terminating due to SIGTERM");

	r = producer_stop(in, sig);
	if (r < 0)
		fatal("tuna: Failed to stop input module");
}

int main(int argc, char * argv[])
{
	int r;
	struct arguments * args;
	const char * log_file = "tuna.log";
	const char * app_name = "tuna";

	r = log_init(log_file, app_name);
	if (r < 0)
		return r;

	/* Argument parsing. */
	args = args_init();
	if (!args)
		return -ENOMEM;

	argp_parse(&argp, argc, argv, 0, 0, args);

	r = output_init(args);
	if (r < 0)
		return r;

	r = input_init(args);
	if (r < 0)
		return r;

	/* Setup signal handler now that 'in' is valid (as the handler calls
	 * in->stop() ).
	 */
	signal(SIGTERM, sigterm_handler);

	r = producer_run(in);

	input_exit();
	output_exit();
	log_exit();
	args_exit(args);

	return r;
}
