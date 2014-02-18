/*******************************************************************************
	counter.h: Passthrough and count samples.

	Copyright (C) 2014 Paul Barker, Loughborough University

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

#ifndef __TUNA_COUNTER_H_INCLUDED__
#define __TUNA_COUNTER_H_INCLUDED__

#include "consumer.h"

/**
 * \file <tuna/counter.h>
 *
 * \brief Pass data through to another consumer and count samples.
 *
 * This consumer passes unmodified data through to a taget consumer while
 * keeping a count of the number of samples it has seen. A limit and callback
 * function may be given when initialising this consumer, in this case the given
 * callback function will be called when the given limit is reached by the
 * sample counter.
 */

/**
 * \brief A callback function which will run when the counter consumer reaches
 * its limit.
 *
 * \param arg An arbitrary argument which takes the value given to
 * counter_init().
 *
 * \return <0 on failure, 0 on success where the counter module should continue
 * passing data through to its target consumer or >0 on success where the
 * counter module should stop passing data through to its target consumer.
 */
typedef int (*counter_limit_callback_fn)(void * arg);

/**
 * \brief Initialise counter consumer.
 *
 * \param consumer The consumer object to initialise. The call to
 * counter_init() should immediately follow the creation of a consumer object
 * with consumer_new().
 *
 * \param target The consumer to which this buffer queue will write data.
 *
 * \param limit The number of samples after which to call the limit_callback
 * function.
 *
 * \param limit_callback The function to call after the given limit has been
 * reached by the sample counter. Set this parameter to NULL to disable the
 * limit callback.
 *
 * \param arg The argument to pass to limit_callback when it is called.
 *
 * \return >=0 on success, <0 on failure.
 */
int counter_init(struct consumer * consumer, struct consumer * target,
		uint limit, counter_limit_callback_fn limit_callback,
		void * arg);

#endif /* !__TUNA_COUNTER_H_INCLUDED__ */
