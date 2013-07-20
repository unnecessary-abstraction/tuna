/*******************************************************************************
	input_ads1672.c: Input from ads1672 driver.

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

#include <ads1672.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "compiler.h"
#include "consumer.h"
#include "input_ads1672.h"
#include "log.h"
#include "producer.h"
#include "types.h"

/*******************************************************************************
	Private declarations and functions
*******************************************************************************/

struct input_ads1672 {
	struct producer		producer;
	struct consumer *	consumer;

	volatile int		stop;

	int			fh;
	ads1672_sample_t *	raw_buf;

	/* Length of raw_buf in samples of type ads1672_sample_t. */
	uint			raw_buf_length;

	uint			sample_rate;
};

static int prep(struct input_ads1672 * a)
{
	int r;

	assert(a);

	r = open("/dev/ads1672", O_RDONLY);
	if (r < 0) {
		error("input_ads1672: Unable to open ads1672 device file");
		return r;
	}

	/* Store file handle */
	a->fh = r;

	/* Set chip select pin low now. */
	r = ads1672_ioctl_gpio_select_set(a->fh, 0);
	if (r < 0) {
		error("input_ads1672: Failed to set chip select pin low");
		close(a->fh);
		return r;
	}

	/* Set start pin low to ensure we're in a known safe state. */
	r = ads1672_ioctl_gpio_start_set(a->fh, 0);
	if (r < 0) {
		error("input_ads1672: Failed to set start pin low");
		close(a->fh);
		return r;
	}

	/* Allocate buffer for samples in ads1672 driver format. The buffer
	 * length is the same as the period length within the ads1672 driver to
	 * maximise efficiency by aligning read operations.
	 */
	a->raw_buf_length = ADS1672_PERIOD_LENGTH;
	a->raw_buf = (ads1672_sample_t *)
		malloc(ADS1672_PERIOD_LENGTH * sizeof(ads1672_sample_t));
	if (!a->raw_buf) {
		error("input_ads1672: Failed to allocate memory for incoming samples");
		close(a->fh);
		return -ENOMEM;
	}

	return 0;
}

static void cleanup(struct input_ads1672 * a)
{
	assert(a);

	free(a->raw_buf);
	close(a->fh);
}

static int stop(struct input_ads1672 * a)
{
	int r;

	assert(a);

	/* Ordering is take start pin low then stop driver. We don't care about
	 * the chip select pin here.
	 */
	r = ads1672_ioctl_gpio_start_set(a->fh, 0);
	if (r < 0)
		error("input_ads1672: Failed to set start pin low");
		/* We still want to attempt to stop the driver even if the start
		 * pin couldn't be taken low, so don't return here.
		 */

	r = ads1672_ioctl_stop(a->fh);
	if (r < 0) {
		error("input_ads1672: Failed to stop driver");
		return r;
	}

	return 0;
}

static int start(struct input_ads1672 * a)
{
	int r, r2;

	assert(a);

	/* Ordering is start driver then take start pin high. Chip select pin is
	 * handled in prep().
	 */
	r = ads1672_ioctl_start(a->fh);
	if (r < 0) {
		error("input_ads1672: Failed to start driver");
		return r;
	}

	r = ads1672_ioctl_gpio_start_set(a->fh, 1);
	if (r < 0) {
		error("input_ads1672: Failed to set start pin high");
		r2 = ads1672_ioctl_stop(a->fh);
		if (r2 < 0) {
			error("input_ads1672: Failed to stop driver in response to error");
			return r2;
		}

		return r;
	}

	return 0;
}

static int handle_condition(struct input_ads1672 * a, int cond)
{
	int r;
	struct timespec ts;

	assert(a);

	switch (cond) {
		/* Catch unnecessary calls */
		case ADS1672_COND_OK:
			return 0;

		/* Errors which require resync. */
		case ADS1672_COND_OVERRUN:
		case ADS1672_COND_DMA_ERROR:
			r = ads1672_ioctl_get_timespec(a->fh, &ts);
			if (r < 0) {
				error("input_ads1672: Failed to get timespec required for resync operation");
				return r;
			}

			r = ads1672_ioctl_clear_condition(a->fh);
			if (r < 0) {
				error("input_ads1672: Failed to clear error condition");
				return r;
			}

			return a->consumer->resync(a->consumer, &ts);

		/* Errors which just require reading again */
		case ADS1672_COND_IN_USE:
			r = ads1672_ioctl_clear_condition(a->fh);
			if (r < 0)
				error("input_ads1672: Failed to clear error condition");
			return r;

		/* Unrecoverable errors */
		case ADS1672_COND_STOP:
		case ADS1672_COND_INVALID:
		default:
			return -EIO;
	}
}

/* Convert samples from the format used by the ads1672 driver into our sample_t
 * type. The samples are copied from the raw (unconverted) buffer to a new
 * buffer as they are converted.
 */
static void convert_buffer(struct input_ads1672 * a, sample_t * buf, uint frames)
{
	uint i;

	assert(a);
	assert(buf);

	for (i = 0; i < frames; i++) {
		buf[i] = (sample_t)a->raw_buf[i];
	}
}

int input_ads1672_run(struct producer * producer)
{
	assert(producer);

	int		r;
	uint		frames;
	struct timespec ts;
	sample_t *	buf;
	int		cond;

	struct input_ads1672 * a = container_of(producer, struct input_ads1672, producer);

	/* TODO: This will likely return nonsense before the start() has been
	 * called. However calling start() before consumer->start() may cause a
	 * delay, leading to loss of samples and necessitating a resync.
	 */
	r = ads1672_ioctl_get_timespec(a->fh, &ts);
	if (r < 0) {
		error("input_ads1672: Failed to get timespec required for start operation");
		return r;
	}

	r = a->consumer->start(a->consumer, a->sample_rate, &ts);
	if (r < 0) {
		error("input_ads1672: consumer->start failed");
		return r;
	}

	r = start(a);
	if (r < 0)
		/* Error already logged */
		return r;

	while (1) {
		/* Check for termination signal. */
		if (a->stop) {
			msg("input_ads1672: Stop");
			stop(a);
			return a->stop;
		}

		frames = a->raw_buf_length;

		r = read(a->fh, a->raw_buf, frames * sizeof(ads1672_sample_t));
		if (r < 0) {
			error("input_ads1672: Failed to read samples");

			r = ads1672_ioctl_get_condition(a->fh, &cond);
			if (r < 0) {
				error("input_ads1672: Failed to read condition code, cannot handle error");
				stop(a);
				return r;
			}
			
			r = handle_condition(a, cond);
			if (r < 0) {
				/* Error already logged. */
				stop(a);
				return r;
			}
		} else {
			/* Got r bytes. */
			frames = r / sizeof(ads1672_sample_t);
			buf = buffer_acquire(&frames);
			if (!buf) {
				error("input_ads1672: Failed to acquire buffer");
				stop(a);
				return -ENOMEM;
			}
			
			convert_buffer(a, buf, frames);
			
			r = a->consumer->write(a->consumer, buf, frames);
			if (r < 0) {
				error("input_ads1672: consumer->write failed");
				stop(a);
				return r;
			}

			buffer_release(buf);
		}
	}
}

void input_ads1672_exit(struct producer * producer)
{
	assert(producer);

	struct input_ads1672 * a = container_of(producer, struct input_ads1672, producer);

	cleanup(a);
	free(a);
}

void input_ads1672_stop(struct producer * producer, int condition)
{
	assert(producer);

	struct input_ads1672 * a = container_of(producer, struct input_ads1672, producer);
	a->stop = condition;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

struct producer * input_ads1672_init(struct consumer * c)
{
	int r;

	assert(c);

	struct input_ads1672 * a = (struct input_ads1672 *)malloc(sizeof(struct input_ads1672));
	if (!a) {
		error("input_ads1672: Failed to allocate memory");
		return NULL;
	}

	a->stop = 0;
	a->consumer = c;

	/* For now, the sample rate is always 625 kHz. */
	a->sample_rate = 625000;

	a->producer.run = input_ads1672_run;
	a->producer.exit = input_ads1672_exit;
	a->producer.stop = input_ads1672_stop;

	r = prep(a);
	if (r < 0) {
		/* Error already logged */
		free(a);
		return NULL;
	}

	return &a->producer;
}
