/*******************************************************************************
	input_ads1672.c: Input from ads1672 driver.

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
	struct consumer *	consumer;

	volatile int		stop;
	int			stop_condition;

	int			fh;

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

	return 0;
}

static void cleanup(struct input_ads1672 * a)
{
	assert(a);

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

			return consumer_resync(a->consumer, &ts);

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

int input_ads1672_run(struct producer * producer)
{
	assert(producer);

	int		r;
	uint		frames;
	struct timespec ts;
	sample_t *	buf;
	int		cond;

	struct input_ads1672 * a = (struct input_ads1672 *)
		producer_get_data(producer);

	/* TODO: This will likely return nonsense before the start() has been
	 * called. However calling start() before consumer->start() may cause a
	 * delay, leading to loss of samples and necessitating a resync.
	 */
	r = ads1672_ioctl_get_timespec(a->fh, &ts);
	if (r < 0) {
		error("input_ads1672: Failed to get timespec required for start operation");
		return r;
	}

	r = consumer_start(a->consumer, a->sample_rate, &ts);
	if (r < 0) {
		error("input_ads1672: Failed to start consumer");
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
			return a->stop_condition;
		}

		/* Life is easiest if we try to read blocks of length 1 s. */
		frames = a->sample_rate;
		buf = buffer_acquire(&frames);
		if (!buf) {
			error("input_ads1672: Failed to acquire buffer");
			stop(a);
			return -ENOMEM;
		}

		r = read(a->fh, buf, frames * sizeof(ads1672_sample_t));
		if (r < 0) {
			error("input_ads1672: Failed to read samples");
			buffer_release(buf);

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
			r = consumer_write(a->consumer, buf, frames);
			if (r < 0) {
				error("input_ads1672: Failed to write to consumer");
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

	struct input_ads1672 * a = (struct input_ads1672 *)
		producer_get_data(producer);

	cleanup(a);
	free(a);
}

int input_ads1672_stop(struct producer * producer, int condition)
{
	assert(producer);

	struct input_ads1672 * a = (struct input_ads1672 *)
		producer_get_data(producer);
	a->stop = 1;
	a->stop_condition = condition;

	return 0;
}

/*******************************************************************************
	Public functions
*******************************************************************************/

int input_ads1672_init(struct producer * producer, struct consumer * consumer,
	uint sample_rate)
{
	int r;

	assert(producer);
	assert(consumer);

	/* We assume that sample_t and ads1672_sample_t are the same type. */
	assert(sizeof(sample_t) == sizeof(ads1672_sample_t));

	struct input_ads1672 * a = (struct input_ads1672 *)
		malloc(sizeof(struct input_ads1672));
	if (!a) {
		error("input_ads1672: Failed to allocate memory");
		return -ENOMEM;
	}

	a->stop = 0;
	a->consumer = consumer;

	/* Sadly we can't check if this matches the sample rate configured via
	 * the DIP switches on the ADS1672 EVM. We could check that the value is
	 * at least one of the possible sample rates which the ADS1672 supports,
	 * but that probably isn't worth the effort.
	 */
	a->sample_rate = sample_rate;

	r = prep(a);
	if (r < 0) {
		/* Error already logged */
		free(a);
		return r;
	}

	producer_set_module(producer, input_ads1672_run, input_ads1672_stop,
			input_ads1672_exit, a);

	return 0;
}
