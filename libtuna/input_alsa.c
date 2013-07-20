/*******************************************************************************
	input_alsa.c: Input from ALSA.

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

#include <alsa/asoundlib.h>
#include <assert.h>
#include <malloc.h>

#include "buffer.h"
#include "consumer.h"
#include "input_alsa.h"
#include "log.h"
#include "producer.h"

/*******************************************************************************
	Private declarations
*******************************************************************************/

struct input_alsa {
	struct producer		producer;
	struct consumer *	consumer;

	snd_pcm_t *		capture;
	int16_t *		alsa_buf;
	volatile int		stop;

	/* ALSA parameters */
	const char *		device_name;
	uint			sample_rate;
	uint			channels;
	snd_pcm_format_t	format;
	snd_pcm_uframes_t	period_size;
};

/* Maximum number of frames to read at once. */
#define MAX_FRAMES (1<<16)

/*******************************************************************************
	Private functions
*******************************************************************************/

static int prep(struct input_alsa * a)
{
	assert(a);

	int r, r2;
	snd_pcm_hw_params_t *hw_params = NULL;
	snd_pcm_sw_params_t *sw_params = NULL;

	a->capture = NULL;

	r = snd_pcm_open(&a->capture, a->device_name, SND_PCM_STREAM_CAPTURE, 0);
	if (r < 0) {
		error("input_alsa: Failed to open device %s: %s", a->device_name, snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_hw_params_malloc(&hw_params);
	if (r < 0) {
		error("input_alsa: Failed to allocate memory for hardware parameters: %s", snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_hw_params_any(a->capture, hw_params);
	if (r < 0) {
		error("input_alsa: Failed to initialize hardware parameters: %s", snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_hw_params_set_access(a->capture, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (r < 0) {
		error("input_alsa: Failed to set access mode to RW_INTERLEAVED: %s", snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_hw_params_set_format(a->capture, hw_params, a->format);
	if (r < 0) {
		error("input_alsa: Failed to set format to %s: %s", snd_pcm_format_name(a->format), snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_hw_params_set_rate_near(a->capture, hw_params, &a->sample_rate, 0);
	if (r < 0) {
		error("input_alsa: Failed to set sampling frequency near %d: %s", a->sample_rate, snd_strerror(r));
		goto handle_err;
	}
	msg("input_alsa: Sample rate set at %u Hz", a->sample_rate);

	r = snd_pcm_hw_params_set_channels(a->capture, hw_params, a->channels);
	if (r < 0) {
		error("input_alsa: Failed to set number of channels to %d: %s", a->channels, snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_hw_params_set_period_size_near(a->capture, hw_params, &a->period_size, 0);
	if (r < 0) {
		error("input_alsa: Failed to set period size near %d: %s", a->period_size, snd_strerror(r));
		goto handle_err;
	}
	msg("input_alsa: Period size set at %u", a->period_size);

	r = snd_pcm_hw_params(a->capture, hw_params);
	if (r < 0) {
		error("input_alsa: Failed to commit hardware parameters: %s", snd_strerror(r));
		goto handle_err;
	}

	snd_pcm_hw_params_free(hw_params);
	hw_params = NULL;

	r = snd_pcm_sw_params_malloc(&sw_params);
	if (r < 0) {
		error("input_alsa: Failed to allocate memory for software parameters: %s", snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_sw_params_current(a->capture, sw_params);
	if (r < 0) {
		error("input_alsa: Failed to read current software parameters: %s", snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_sw_params_set_tstamp_mode(a->capture, sw_params, SND_PCM_TSTAMP_ENABLE);
	if (r < 0) {
		error("input_alsa: Failed to set timestamp mode: %s", snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_sw_params_set_avail_min(a->capture, sw_params, 512);
	if (r < 0) {
		error("input_alsa: Failed to set minimum available threshold: %s", snd_strerror(r));
		goto handle_err;
	}

	r = snd_pcm_sw_params(a->capture, sw_params);
	if (r < 0) {
		error("input_alsa: Failed to commit software parameters: %s", snd_strerror(r));
		goto handle_err;
	}

	snd_pcm_sw_params_free(sw_params);
	sw_params = NULL;

	/* Allocate memory for samples in ALSA format. */
	a->alsa_buf = (int16_t *)malloc(MAX_FRAMES * a->channels * sizeof(int16_t));
	if (!a->alsa_buf) {
		error("input_alsa: Failed to allocate memory for incoming samples");
		r = -ENOMEM;
		goto handle_err;
	}

	r = snd_pcm_prepare(a->capture);
	if (r < 0) {
		error("input_alsa: Failed to prepare for reading: %s", snd_strerror(r));
		goto handle_err;
	}

	return 0;

handle_err:
	/* Error cleanup. */
	if (a->alsa_buf)
		free(a->alsa_buf);

	if (hw_params)
		snd_pcm_hw_params_free(hw_params);

	if (sw_params)
		snd_pcm_sw_params_free(sw_params);

	if (a->capture) {
		r2 = snd_pcm_close(a->capture);
		if (r2 < 0)
			error("input_alsa: Error closing device: %s", snd_strerror(r2));
	}

	return r;
}

static int start(struct input_alsa * a)
{
	int r;
	struct timespec		ts;

	/* Unused but snd_pcm_htimestamp doesn't say whether the second argument
	 * can be null or not.
	 */
	snd_pcm_uframes_t	avail;

	assert(a);

	/* Attempt to get the timespec before calling snd_pcm_start so that we
	 * can start the consumer before we start capturing acoustic data. As
	 * starting the consumer may do things like creating a new output file
	 * or calculating tables of data based on the given sampling rate, it
	 * may take a while to complete. If the sound capture is running during
	 * this time then we may miss data before the consumer is started and we
	 * are able collect acoustic data.
	 *
	 * TODO: Check this works.
	 *
	 * TODO: Split the chained consumer start into the consumer thread in
	 *	 the bufq consumer to alleviate this when bufq is used.
	 */
	r = snd_pcm_htimestamp(a->capture, &avail, (snd_htimestamp_t *) &ts);
	if (r < 0) {
		error("input_alsa: Unable to get initial timestamp: %s", snd_strerror(r));
		return r;
	}

	r = a->consumer->start(a->consumer, a->sample_rate, &ts);
	if (r < 0) {
		error("input_alsa: consumer->start failed");
		return r;
	}

	r = snd_pcm_start(a->capture);
	if (r < 0) {
		error("input_alsa: Could not start capture: %s", snd_strerror(r));
		return r;
	}

	return 0;
}

static int handle_error(struct input_alsa * a, int err)
{
	assert(a);

	int r;
	struct timespec ts;

	/* Unused but snd_pcm_htimestamp doesn't say whether the second argument
	 * can be null or not.
	 */
	snd_pcm_uframes_t	avail;

	msg("input_alsa: Attempting to recover from error");

	/* See if ALSA layer can deal with the error. */
	r = snd_pcm_recover(a->capture, err, 0);

	if (r < 0) {
		/* Deep recovery: Close and re-open the ALSA input device. */
		error("input_alsa: Standard recovery failed: %s", snd_strerror(r));
		msg("input_alsa: Attempting deep recovery");

		r = snd_pcm_close(a->capture);
		if (r < 0) {
			error("input_alsa: Error closing device: %s", snd_strerror(r));
			return r;
		}

		r = prep(a);
		if (r < 0) {
			error("input_alsa: Failed to re-initialise input");
			return r;
		}

		r = snd_pcm_start(a->capture);
		if (r < 0) {
			error("input_alsa: Could not re-start capture: %s", snd_strerror(r));
			return r;
		}
	}

	/* Resync the consumer. */
	r = snd_pcm_htimestamp(a->capture, &avail, (snd_htimestamp_t *) &ts);
	if (r < 0) {
		error("input_alsa: Unable to get timestamp for resync: %s", snd_strerror(r));
		return r;
	}
	
	r = a->consumer->resync(a->consumer, &ts);
	if (r < 0) {
		error("input_alsa: consumer->resync failed");
		return r;
	}

	return r;
}

/* Convert 16-bit samples from ALSA into our sample_t type and if multiple
 * channels are present the select only the first one. The samples are copied
 * from the ALSA buffer to a new buffer (given as an argument) as they are 
 * converted.
 */
void convert_buffer(struct input_alsa * a, sample_t * buf, uint frames)
{
	uint i;

	for (i = 0; i < frames; i++) {
		buf[i] = (sample_t)a->alsa_buf[i * a->channels];
	}
}

int input_alsa_run(struct producer * producer)
{
	assert(producer);

	struct input_alsa * a = container_of(producer, struct input_alsa, producer);

	int			r;
	snd_pcm_sframes_t	sf;
	snd_pcm_sframes_t	avail;
	uint			frames;
	sample_t *		buf;

	r = start(a);
	if (r < 0)
		/* Start failed, error message has already been printed */
		return r;

	while (1) {
		/* Check for termination signal. */
		if (a->stop) {
			msg("input_alsa: Stop");
			return a->stop;
		}

		r = snd_pcm_wait(a->capture, -1);
		if (r < 0) {
			/* TODO: Handle errors like xrun and suspend. */
			fatal("input_alsa: snd_pcm_wait failed: %s", snd_strerror(r));
		}

		avail = snd_pcm_avail_update(a->capture);
		frames = (avail > MAX_FRAMES) ? MAX_FRAMES : (uint)avail;
		
		sf = snd_pcm_readi(a->capture, a->alsa_buf, (snd_pcm_uframes_t) frames);
		r = (int)sf;

		if (r < 0) {
			error("input_alsa: Read error: %s", snd_strerror(r));
			
			r = handle_error(a, r);
			if (r < 0) {
				/* failed to handle error, message has already been printed */
				return r;
			}
		} else {
			/* Got sf frames. */
			frames = (uint)sf;

			buf = buffer_acquire(&frames);
			if (!buf) {
				error("input_alsa: Failed to acquire buffer");
				return -1;
			}

			/* Convert samples from ALSA into our sample_t type. */
			convert_buffer(a, buf, frames);
			
			r = a->consumer->write(a->consumer, buf, frames);
			if (r < 0) {
				error("input_alsa: consumer->write failed");
				buffer_release(buf);
				return r;
			}

			buffer_release(buf);
		}
	}

	return 0;
}

void input_alsa_exit(struct producer * producer)
{
	assert(producer);

	int r;

	struct input_alsa * a = container_of(producer, struct input_alsa, producer);

	r = snd_pcm_close(a->capture);
	if (r < 0) {
		error("input_alsa: Error closing device: %s", snd_strerror(r));
	}

	free(a);
}

void input_alsa_stop(struct producer * producer, int condition)
{
	assert(producer);

	struct input_alsa * a = container_of(producer, struct input_alsa, producer);
	a->stop = condition;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct producer * input_alsa_init(struct consumer * c, const char * device_name,
		uint sample_rate)
{
	assert(c);

	int r;

	struct input_alsa * a = (struct input_alsa *)malloc(sizeof(struct input_alsa));
	if (!a) {
		error("input_alsa: Failed to allocate memory");
		return NULL;
	}

	a->consumer = c;
	a->device_name = device_name;
	a->sample_rate = sample_rate;
	a->channels = 2;
	a->format = SND_PCM_FORMAT_S16_LE;
	a->period_size = 4096;			/* ~93 ms at 44.1kHz */
	a->stop = 0;

	a->capture = NULL;
	a->alsa_buf = NULL;

	r = prep(a);
	if (r < 0) {
		/* Prep failed, error message has already been printed */
		free(a);
		return NULL;
	}

	a->producer.run = input_alsa_run;
	a->producer.exit = input_alsa_exit;
	a->producer.stop = input_alsa_stop;

	return &a->producer;
}
