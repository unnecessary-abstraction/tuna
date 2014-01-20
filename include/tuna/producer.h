/*******************************************************************************
	producer.h: Producer type.

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

#ifndef __TUNA_PRODUCER_H_INCLUDED__
#define __TUNA_PRODUCER_H_INCLUDED__

/**
 * \file <tuna/producer.h>
 *
 * \brief Generic producer interface.
 */

struct producer;

#ifdef DOXYGEN
/**
 * \brief A standardised producer interface
 *
 * This is just a thin wrapper around a particular producer module,
 * standardising the interface so that it can easily be used from C or any other
 * language.
 */
struct producer {};
#endif

/**
 * Prototype for a callback function which implements producer_run(). See the
 * documentation for that function for the meaning of parameters and return
 * value.
 */
typedef int (*producer_run_fn)(struct producer * producer);

/**
 * Prototype for a callback function which implements producer_stop(). See the
 * documentation for that function for the meaning of parameters and return
 * value.
 */
typedef int (*producer_stop_fn)(struct producer * producer, int condition);

/**
 * Prototype for a callback function which implements producer_exit(). See the
 * documentation for that function for the meaning of parameters and return
 * value.
 */
typedef void (*producer_exit_fn)(struct producer * producer);

/**
 * Create a new producer object and perform generic initialisation. The call to
 * this function should be followed to a call to an appropriate init function
 * such as:
 *
 * - input_alsa_init()
 * - input_sndfile_init()
 * - input_zero_init()
 * - input_ads1672_init() if configured with "--enable-ads1672"
 *
 * These functions in turn call producer_set_module() to register their
 * implementations of the producer_run(), producer_stop() and producer_exit()
 * functions.
 *
 * \return Pointer to a new producer object on success, NULL on failure.
 */
struct producer * producer_new();

/**
 * Destroy a producer object once it is finished with. If an exit callback was
 * provided to producer_set_module() it will be executed. Following this the
 * memory associated with the producer object will be freed.
 *
 * \param producer The producer object to destroy.
 */
void producer_exit(struct producer * producer);

/**
 * Setup the function callbacks and internal data pointer used to implement
 * the producer interface.
 *
 * \param producer The producer object to setup, which should have just been
 * allocated by producer_new() and should not have had any non-initialisation
 * functions ran on it.
 *
 * \param run The callback function to implement producer_run().
 *
 * \param stop The callback function to implement pruducer_stop().
 *
 * \param exit The callback function to implement producer_exit().
 *
 * \param data An internal pointer which may be used to pass state data to the
 * various callback functions.
 */
void producer_set_module(struct producer * producer, producer_run_fn run,
		producer_stop_fn stop, producer_exit_fn exit, void * data);

/**
 * Get the internal data pointer passed to producer_set_module().
 *
 * \param producer The producer object to act on.
 *
 * \return The required data pointer or NULL if one was not set.
 */
void * producer_get_data(struct producer * producer);

/**
 * Run a producer module's main loop using the callback passed to
 * producer_set_module(). The function will typically continue running until
 * there is no more data to produce or until an error occurs.
 *
 * \param producer The producer object to act on.
 *
 * \return >=0 for successful completion of the main loop, <0 for termination
 * due to error.
 */
int producer_run(struct producer * producer);

/**
 * Terminate a producer module's main loop using the callback passed to
 * producer_set_module(). It is intended that this function be called from
 * another thread or from a signal handler while the producer_run() function is
 * executing. It should cause the producer to stop producing data and cleanly
 * return from the producer_run() function. The return from the producer_run()
 * function may not happen immediately though, and is not guaranteed to have
 * happened before this function returns.
 *
 * \param producer The producer object to act on.
 *
 * \param condition The condition code to return from producer_run().
 *
 * \return >=0 on success, <0 on failure.
 */
int producer_stop(struct producer * producer, int condition);

#endif /* !__TUNA_PRODUCER_H_INCLUDED__ */
