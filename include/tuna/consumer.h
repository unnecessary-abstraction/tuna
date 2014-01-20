/*******************************************************************************
	consumer.h: Consumer type.

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

#ifndef __TUNA_CONSUMER_H_INCLUDED__
#define __TUNA_CONSUMER_H_INCLUDED__

#include <time.h>

#include "types.h"

/**
 * \file <tuna/consumer.h>
 *
 * \brief Generic consumer interface.
 */

struct consumer;

#ifdef DOXYGEN
/**
 * \brief A standardised consumer interface.
 *
 * This is just a thin wrapper around a particular consumer module,
 * standardising the interface so that it can easily be used from C or any other
 * language.
 */
struct consumer {};
#endif

/**
 * Prototype for a callback function which implements consumer_write(). See the
 * documentation for that function for the meaning of parameters and return
 * value.
 */
typedef int (*consumer_write_fn)(struct consumer * consumer, sample_t * buf,
		uint count);

/**
 * Prototype for a callback function which implements consumer_start(). See the
 * documentation for that function for the meaning of parameters and return
 * value.
 */
typedef int (*consumer_start_fn)(struct consumer * consumer, uint sample_rate,
		struct timespec * ts);

/**
 * Prototype for a callback function which implements consumer_resync(). See the
 * documentation for that function for the meaning of parameters and return
 * value.
 */
typedef int (*consumer_resync_fn)(struct consumer * consumer,
		struct timespec * ts);

/**
 * Prototype for a callback function which implements consumer_exit(). See the
 * documentation for that function for the meaning of parameters and return
 * value.
 */
typedef void (*consumer_exit_fn)(struct consumer * consumer);

/**
 * Create a new consumer object and perform generic initialisation. The call to
 * this function should be followed to a call to an appropriate init function
 * such as:
 *
 * - time_slice_init()
 * - pulse_init()
 * - bufq_init()
 * - output_sndfile_init()
 * - output_null_init()
 *
 * These functions in turn call consumer_set_module() to register their
 * implementations of the consumer_write(), consumer_start(), consumer_resync()
 * and consumer_exit() functions.
 *
 * \return Pointer to a new consumer object on success, NULL on failure.
 */
struct consumer * consumer_new();

/**
 * Destroy a consumer object once it is finished with. If an exit callback was
 * provided to consumer_set_module() it will be executed. Following this the
 * memory associated with the consumer object will be freed.
 *
 * \param consumer The consumer object to destroy.
 */
void consumer_exit(struct consumer * consumer);

/**
 * Setup the function callbacks and internal data pointer used to implement
 * the consumer interface.
 *
 * \param consumer The consumer object to setup, which should have just been
 * allocated by consumer_new() and should not have had any non-initialisation
 * functions ran on it.
 *
 * \param write The callback function to implement consumer_write().
 *
 * \param start The callback function to implement consumer_start().
 *
 * \param resync The callback function to implement consumer_resync().
 *
 * \param exit The callback function to implement consumer_exit().
 *
 * \param data An internal pointer which may be used to pass state data to the
 * various callback functions.
 */
void consumer_set_module(struct consumer * consumer, consumer_write_fn write,
		consumer_start_fn start, consumer_resync_fn resync,
		consumer_exit_fn exit, void * data);

/**
 * Get the internal data pointer passed to consumer_set_module().
 *
 * \param consumer The consumer object to act on.
 *
 * \return The required data pointer or NULL if one was not set.
 */
void * consumer_get_data(struct consumer * consumer);

/**
 * Write a block of data to a consumer module, using the write callback function
 * passed to consumer_set_module().
 *
 * \param consumer The consumer object to write data to.
 *
 * \param buf A pointer to the array of sample data to write to the consumer.
 *
 * \param count The number of samples contained in the array.
 *
 * \return >=0 on success, <0 on failure.
 */
int consumer_write(struct consumer * consumer, sample_t * buf, uint count);

/**
 * Start a consumer module running. This function executes any setup
 * which depends on knowledge of the sample rate at which data will be written
 * to the consumer, which might not be known at the time that the initialisation
 * function was called.
 *
 * \param consumer The consumer object to start.
 *
 * \param sample_rate The sample rate at which data will be written to the
 * consumer module.
 *
 * \param ts The start time of the data which will be written to the consumer
 * module. This does not need to relate to the current system clock, especially
 * in the case of a producer reading stored data from disk. It may be written to
 * the output of the consumer module if appropriate.
 *
 * \return >=0 on success, <0 on failure.
 */
int consumer_start(struct consumer * consumer, uint sample_rate,
		struct timespec * ts);

/**
 * Resynchronise a consumer object in the event that data has been lost by the
 * module which writes to this consumer. It may cause the data gap to be
 * recorded to the output of the consumer object if appropriate. It may also
 * cause internal buffers and states to be flushed to enable correct processing
 * of further data.
 *
 * \param consumer The consumer object to resynchronise.
 *
 * \param ts The start time of new data which will follow the period of lost
 * data.
 *
 * \return >=0 on success, <0 on failure.
 */
int consumer_resync(struct consumer * consumer, struct timespec * ts);

#endif /* !__TUNA_CONSUMER_H_INCLUDED__ */
