/*******************************************************************************
	input_sndfile.c: Input from sound files of various formats.

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
#include "input_sndfile.h"
#include "log.h"
#include "string.h"
#include "uara.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

struct input_sndfile_state {
	const char *	source;
	SNDFILE *	sf;
	SF_INFO		sf_info;
	const char *	sf_name;
};

static struct input_sndfile_state state;

static const char * sample_type(int format);
static int open_sndfile(const char * sf_name);
static void close_sndfile(void);

/*******************************************************************************
	Private functions
*******************************************************************************/

/* Decode a libsndfile format to determine the sample type. */
static const char * sample_type(int format)
{
	switch (format & 0x7) {
	case 0:
		return "(unknown)";
	case 1:
		return "int8";
	case 2:
		return "int16";
	case 3:
		return "int24";
	case 4:
		return "int32";
	case 5:
		return "uint8";
	case 6:
		return "float";
	case 7:
		return "double";
	default:
		fatal("input_sndfile: Unknown sample type");
	}
}

static int open_sndfile(const char * sf_name)
{
	int r;

	/* Store filename to be opened. */
	state.sf_name = sf_name;

	memset(&state.sf_info, 0, sizeof(state.sf_info));
	state.sf = sf_open(sf_name, SFM_READ, &state.sf_info);
	if (!state.sf) {
		r = sf_error(NULL);
		error("libsndfile: Error %d: %s", r, sf_strerror(NULL));
		error("input_sndfile: Failed to open file %s", sf_name);
		return -r;	/* libsndfile error values are positive. */
	}

	/* Store sample rate. */
	input_sample_rate = state.sf_info.samplerate;
	output_sample_rate = input_sample_rate;		/* Currently no resampling. */

	/* Print some info to the log. */
	msg("input_sndfile: Opened file %s, %u channels sampled at %u Hz, sample type %s",
		sf_name, state.sf_info.channels, input_sample_rate,
		sample_type(state.sf_info.format));

	return 0;
}

static void close_sndfile(void)
{
	int r;

	r = sf_close(state.sf);
	if (r != 0) {
		error("libsndfile: Error %d: %s", r, sf_error_number(r));
		fatal("input_sndfile: Could not close file %s", state.sf_name);
	}

	state.sf = NULL;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int input_sndfile_init(const char * source)
{
	/*
		Currently, source must be one filename. Open this file and set
		variables.
	*/
	state.source = source;

	return open_sndfile(source);
}

int input_sndfile_run(void)
{
	/* TODO. */
	/* Return 0 if finished, 1 if running asynchronously. */
	return 0;
}

void init_sndfile_exit(void)
{
	/* Close input wavefile if open. */
	if (state.sf)
		close_sndfile();
}
