/*******************************************************************************
	input_alsa.h: Input from ALSA.

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

#ifndef __TUNA_INPUT_ALSA_H_INCLUDED__
#define __TUNA_INPUT_ALSA_H_INCLUDED__

#include "consumer.h"
#include "producer.h"
#include "types.h"

/**
 * \file <tuna/input_alsa.h>
 *
 * \brief Input driver for ALSA sound cards.
 *
 * This producer captures data from an ALSA sound card.
 */

/**
 * \brief Initialise the alsa producer.
 *
 * \param producer The producer object to initialise. The call to
 * input_alsa_init() should immediately follow the creation of a producer object
 * with producer_new().
 *
 * \param consumer The consumer to which this producer will write data.
 *
 * \param device_name The name of the ALSA device from which to capture data.
 *
 * \param sample_rate The sampling rate at which to capture data from the
 * sepcified ALSA device. This sampling rate must be supported by the ALSA
 * device.
 *
 * \return >=0 on success, <0 on failure.
 */
int input_alsa_init(struct producer * producer, struct consumer * consumer,
		const char * device_name, uint sample_rate);

#endif /* !__TUNA_INPUT_ALSA_H_INCLUDED__ */
