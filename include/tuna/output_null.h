/*******************************************************************************
	output_null.h: Consumer equivalent of /dev/null.

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

#ifndef __TUNA_OUTPUT_NULL_H_INCLUDED__
#define __TUNA_OUTPUT_NULL_H_INCLUDED__

#include "consumer.h"

/**
 * \file <tuna/output_null.h>
 *
 * \brief Consumer equivalent of `/dev/null`.
 */

/**
 * Initialise null output consumer.
 *
 * \param consumer The consumer object to initialise. The call to
 * output_null_init() should immediately follow the creation of a consumer
 * object with consumer_new().
 *
 * \return 0. This function always succeeds.
 */
int output_null_init(struct consumer * consumer);

#endif /* !__TUNA_OUTPUT_NULL_H_INCLUDED__ */
