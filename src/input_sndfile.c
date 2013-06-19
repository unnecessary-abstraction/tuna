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

#include <errno.h>
#include <malloc.h>
#include <sndfile.h>
#include "buffer.h"
#include "input_sndfile.h"
#include "log.h"
#include "string.h"
#include "uara.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

struct input_sndfile {
	struct producer		producer;
	struct consumer *	consumer;

	const char *		source;
	SNDFILE *		sf;
	SF_INFO			sf_info;
	const char *		sf_name;

	uint			buf_size;
};

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

static int open_sndfile(struct input_sndfile * snd, const char * sf_name)
{
	int r;

	/* Store filename to be opened. */
	snd->sf_name = sf_name;

	memset(&snd->sf_info, 0, sizeof(snd->sf_info));
	snd->sf = sf_open(sf_name, SFM_READ, &snd->sf_info);
	if (!snd->sf) {
		r = sf_error(NULL);
		error("libsndfile: Error %d: %s", r, sf_strerror(NULL));
		error("input_sndfile: Failed to open file %s", sf_name);
		return -r;	/* libsndfile error values are positive. */
	}

	/* Print some info to the log. */
	msg("input_sndfile: Opened file %s, %u channels sampled at %u Hz, sample type %s",
		sf_name, snd->sf_info.channels, snd->sf_info.samplerate,
		sample_type(snd->sf_info.format));

	return 0;
}

static void close_sndfile(struct input_sndfile * snd)
{
	int r;

	r = sf_close(snd->sf);
	if (r != 0) {
		error("libsndfile: Error %d: %s", r, sf_error_number(r));
		fatal("input_sndfile: Could not close file %s", snd->sf_name);
	}

	snd->sf = NULL;
}

int run_single_channel(struct input_sndfile * snd)
{
	int		r;
	uint		frames;
	struct timespec ts;
	sample_t *	buf;

	memset(&ts, 0, sizeof(struct timespec));
	r = snd->consumer->start(snd->consumer, snd->sf_info.samplerate, &ts);
	if (r < 0) {
		error("input_sndfile: consumer->start failed");
		return r;
	}

	while (1) {
		frames = 1<<16;
		buf = buffer_acquire(&frames);
		if (!buf) {
			error("input_sndfile: Failed to acquire buffer");
			return -ENOMEM;
		}

		r = sf_readf_double(snd->sf, buf, frames);
		if (r <= 0) {
			error("libsndfile: Error %d: %s", r, sf_error_number(r));
			error("input_sndfile: Failed to read samples");
			return r;
		}

		/* Got r frames. */
		frames = (uint)r;
		
		r = snd->consumer->write(snd->consumer, buf, frames);
		if (r < 0) {
			error("input_sndfile: consumer->write failed");
			return r;
		}
	}
}

int run_multi_channel(struct input_sndfile * snd)
{
	int		r;
	uint		frames;
	uint		i;
	uint		channels;
	uint		selected_channel;
	struct timespec ts;
	sample_t *	buf;

	channels = snd->sf_info.channels;
	selected_channel = 0;	/* zero-based. TODO: Make configurable. */

	memset(&ts, 0, sizeof(struct timespec));
	r = snd->consumer->start(snd->consumer, snd->sf_info.samplerate, &ts);
	if (r < 0) {
		error("input_sndfile: consumer->start failed");
		return r;
	}
	
	/*
		Read into multi-channel buffer and strip out just the channel we
		want into the front of the buffer.
	*/
	while (1) {
		frames = 1<<16;
		buf = buffer_acquire(&frames);
		if (!buf) {
			error("input_sndfile: Failed to acquire buffer");
			return -ENOMEM;
		}

		/* Divide frames down by the number of channels. */
		frames /= channels;

		r = sf_readf_double(snd->sf, buf, frames);
		if (r <= 0) {
			error("libsndfile: Error %d: %s", r, sf_error_number(r));
			error("input_sndfile: Failed to read samples");
			return r;
		}

		/* Got r frames. */
		frames = (uint)r;

		for (i = 0; i < frames; i++)
			buf[i] = buf[i*channels + selected_channel];

		r = snd->consumer->write(snd->consumer, buf, frames);
		if (r < 0) {
			error("input_sndfile: consumer->write failed");
			return r;
		}
	}
}

int input_sndfile_run(struct producer * producer)
{
	int			r;
	struct input_sndfile *	snd = container_of(producer, struct input_sndfile, producer);

	if (snd->sf_info.channels > 1)
		r = run_multi_channel(snd);
	else
		r = run_single_channel(snd);

	/* Error or EOF. */
	if (r < 0)
		error("input_sndfile: Unrecoverable error reading frames");
	else
		msg("input_sndfile: EOF");
	
	return r;
}

void input_sndfile_exit(struct producer * producer)
{
	struct input_sndfile * snd = container_of(producer, struct input_sndfile, producer);

	/* Close input wavefile if open. */
	if (snd->sf)
		close_sndfile(snd);

	free(snd);
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct producer * input_sndfile_init(const char * source, struct consumer * c)
{
	int r;

	struct input_sndfile * snd = (struct input_sndfile *)malloc(sizeof(struct input_sndfile));
	if (!snd) {
		error("input_sndfile: Failed to allocate memory");
		return NULL;
	}

	/*
		Currently, source must be one filename. Open this file and set
		variables.
	*/
	snd->source = source;
	snd->consumer = c;

	r = open_sndfile(snd, source);
	if (r)
		/* Error message already printed. */
		return NULL;

	snd->producer.run = input_sndfile_run;
	snd->producer.exit = input_sndfile_exit;

	return &snd->producer;
}
