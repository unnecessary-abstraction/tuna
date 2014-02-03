/*******************************************************************************
	input_zero.h: /dev/zero as a producer.

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

#ifndef __TUNA_INPUT_ZERO_H_INCLUDED__
#define __TUNA_INPUT_ZERO_H_INCLUDED__

#include "consumer.h"
#include "producer.h"
#include "types.h"

/**
 * \file <tuna/input_zero.h>
 *
 * \brief Producer equivalent of `/dev/zero`.
 *
 * This is a really simple test producer where every sample produced has the
 * value zero.
 */

/**
 * \brief Initialise a zero-input producer.
 *
 * \param producer The producer object to initialise. The call to
 * input_zero_init() should immediately follow the creation of a producer object
 * with producer_new().
 *
 * \param consumer The consumer to which this producer will write data.
 *
 * \param sample_rate The sampling frequency of the data which will be produced.
 *
 * \return >=0 on success, <0 on failure.
 */
int input_zero_init(struct producer * producer, struct consumer * consumer,
		uint sample_rate);

#endif /* !__TUNA_INPUT_ZERO_H_INCLUDED__ */
