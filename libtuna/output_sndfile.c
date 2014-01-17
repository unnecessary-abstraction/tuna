/*******************************************************************************
	output_sndfile.c: Output via libsndfile.

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

#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <sndfile.h>
#include <stdio.h>
#include <string.h>

#include "consumer.h"
#include "log.h"
#include "output_sndfile.h"
#include "timespec.h"
#include "types.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

struct output_sndfile {
	SNDFILE *		sf;
	SF_INFO			sf_info;
	char *			sf_name;
	size_t			sf_name_len;

	/* Samples written to the current output file - this is reset to zero
	 * when a new file is opened.
	 */
	uint			samples_written;
	uint			samples_max;

	/* The output filename is formed by putting a three digit number between
	 * prefix and suffix. For example, with prefix="REC-" and suffix=".wav",
	 * output files will be "REC-000.wav", "REC-001.wav", etc. The index
	 * variable is used to keep track of the next number to be used.
	 */
	const char *		prefix;
	const char *		suffix;
	uint			index;
};

static int open_sndfile(struct output_sndfile * snd)
{
	int r;

	assert(snd);

	snprintf(snd->sf_name, snd->sf_name_len, "%s%03d%s", snd->prefix, snd->index, snd->suffix);
	snd->index++;

	snd->sf = sf_open(snd->sf_name, SFM_WRITE, &snd->sf_info);
	if (!snd->sf) {
		r = sf_error(NULL);
		error("libsndfile: Error %d: %s", r, sf_strerror(NULL));
		error("output_sndfile: Failed to create file %s", snd->sf_name);
		return -r;	/* libsndfile error values are positive. */
	}

	msg("output_sndfile: Created new file %s", snd->sf_name);
	snd->samples_written = 0;

	return 0;
}

static void close_sndfile(struct output_sndfile * snd)
{
	assert(snd);

	int r;

	if (!snd->sf) {
		warn("output_sndfile: Skipping close_sndfile()");
		return;
	}

	r = sf_close(snd->sf);
	if (r != 0) {
		error("libsndfile: Error %d: %s", r, sf_strerror(snd->sf));
		fatal("output_sndfile: Could not close file %s", snd->sf_name);
	}

	msg("output_sndfile: Closed file %s", snd->sf_name);

	snd->sf = NULL;
}

void output_sndfile_exit(struct consumer * consumer)
{
	assert(consumer);

	struct output_sndfile * snd = (struct output_sndfile *)
		consumer_get_data(consumer);

	close_sndfile(snd);

	free(snd->sf_name);
	free(snd);
}

int output_sndfile_write(struct consumer * consumer, sample_t * buf, uint count)
{
	assert(consumer);
	assert(buf);

	int r;
	uint w = 0;

	struct output_sndfile * snd = (struct output_sndfile *)
		consumer_get_data(consumer);

	if (snd->samples_written + count > snd->samples_max) {
		/* We need to start a new wavefile before we can write all the
		 * samples we have been given. We may still have space for more
		 * samples in the current file so check that first.
		 */
		uint remaining = snd->samples_max - snd->samples_written;
		if (remaining) {
			r = output_sndfile_write(consumer, buf, remaining);
			if (r <= 0) {
				/* Something went wrong. */
				error("output_sndfile: Nested write call failed");
				return r;
			}
			/* As this function doesn't exit unless it hits an error
			 * or it has written all the samples with was asked to
			 * we know that r=remaining here.
			 */
		} else {
			r = 0;
		}

		close_sndfile(snd);
		r = open_sndfile(snd);
		if (r < 0)
			/* Error message already printed. */
			return r;
		msg("output_sndfile: Old file was full");

		w += r;
		buf += r;
	}

	while (w < count) {
		r = sf_writef_int(snd->sf, buf, count - w);
		if (r <= 0) {
			r = sf_error(snd->sf);
			error("libsndfile: Error %d: %s", r, sf_strerror(snd->sf));
			error("output_sndfile: Failed to write samples");
			return -r;	/* libsndfile error values are positive. */
		}

		/* Skip over the data that has been written incase we need to
		 * try again to write the rest.
		 */
		w += r;
		buf += r;
	}
	
	/* If we get to here we have written all the samples we were asked to.
	 */
	return w;
}

int output_sndfile_start(struct consumer * consumer, uint sample_rate, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	int r;

	struct output_sndfile * snd = (struct output_sndfile *)
		consumer_get_data(consumer);

	snd->sf_info.samplerate = sample_rate;

	r = open_sndfile(snd);
	if (r < 0)
		/* Error message already printed. */
		return r;

	char s[64];
	timespec_snprint(ts, s, 64);
	msg("output_sndfile: START at %s", s);

	return 0;
}

int output_sndfile_resync(struct consumer * consumer, struct timespec * ts)
{
	assert(consumer);
	assert(ts);

	int r;

	struct output_sndfile * snd = (struct output_sndfile *)
		consumer_get_data(consumer);

	/* Create a new output file. */
	close_sndfile(snd);
	r = open_sndfile(snd);
	if (r < 0)
		/* Error message already printed. */
		return r;

	char s[64];
	timespec_snprint(ts, s, 64);
	msg("output_sndfile: RESYNC at %s", s);
	
	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int output_sndfile_init(struct consumer * consumer, const char * prefix,
		const char * suffix, int format, uint max_samples_per_file)
{
	assert(consumer);
	assert(prefix);
	assert(suffix);

	struct output_sndfile * snd = (struct output_sndfile *)
		malloc(sizeof(struct output_sndfile));
	if (!snd) {
		error("output_sndfile: Failed to allocate memory for internal data");
		return -ENOMEM;
	}

	snd->sf = NULL;
	snd->prefix = prefix;
	snd->suffix = suffix;
	snd->index = 0;
	snd->samples_max = max_samples_per_file;

	snd->sf_name_len = strlen(prefix) + strlen(suffix) + 10;
	snd->sf_name = (char *)malloc(snd->sf_name_len);
	if (!snd->sf_name) {
		error("output_sndfile: Failed to allocate memory for filename");
		return -ENOMEM;
	}

	snd->sf_info.format = format;
	snd->sf_info.channels = 1;
	
	consumer_set_module(consumer, output_sndfile_write,
			output_sndfile_start, output_sndfile_resync,
			output_sndfile_exit, snd);

	return 0;
}
