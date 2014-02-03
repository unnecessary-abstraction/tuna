/*******************************************************************************
	bufq.h: Buffer queue to decouple input from output.

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

#ifndef __TUNA_BUFQ_H_INCLUDED__
#define __TUNA_BUFQ_H_INCLUDED__

#include "consumer.h"

/**
 * \file <tuna/bufq.h>
 *
 * \brief A buffer queueing consumer.
 *
 * If a producer is operating on real-time data it may be unable to cope with
 * abnormal delays in the processing of this data by a consumer but the average
 * delay may be low enough for the producer to handle. In this case a buffer
 * queue may be used to provide isolation between the producer and the consumer.
 * The queue is allowed to grow when there is a delay in processing at the
 * consumer end. The consumer must be able to process data fast enough to reduce
 * the length of the queue after such a delay to ensure that the system memory
 * is not filled.
 *
 * The isolation is obtained by spawning a separate thread which reads entries
 * from the queue and writes the data to the next consumer in the chain. Thus,
 * if the thread handling the consumer is blocked, the original thread may
 * continue adding data to the queue.
 */

/**
 * \brief Initialise a buffer queue.
 *
 * \param consumer The consumer object to initialise. The call to bufq_init()
 * should immediately follow the creation of a consumer object with
 * consumer_new().
 *
 * \param target The consumer to which this buffer queue will write data.
 *
 * \return >=0 on success, <0 on failure.
 */
int bufq_init(struct consumer * consumer, struct consumer * target);

#endif /* !__TUNA_BUFQ_H_INCLUDED__ */
