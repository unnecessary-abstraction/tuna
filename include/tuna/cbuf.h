/*******************************************************************************
	cbuf.h: Circular buffering.

	Copyright (C) 2013,2014 Paul Barker, Loughborough University

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

#ifndef __TUNA_CBUF_H_INCLUDED__
#define __TUNA_CBUF_H_INCLUDED__

#include "types.h"

/**
 * \file <tuna/cbuf.h>
 *
 * \brief Circular sample buffering.
 */

struct cbuf;

#ifdef DOXYGEN
/**
 * \brief A circular buffer of sample data.
 */
struct cbuf {};
#endif

/**
 * \brief Initiliase a circular buffer.
 *
 * \param len The length of the circular buffer to create.
 *
 * \return A pointer to a new circular buffer or NULL on error.
 */
struct cbuf * cbuf_init(uint len);

/**
 * \brief Reset a given circular buffer.
 *
 * The circular buffer is emptied and reset, the data in the buffer is
 * discarded.
 */
void cbuf_reset(struct cbuf * c);

/**
 * \brief Access a given time delay index in a circular buffer.
 *
 * \param c The circular buffer to access.
 *
 * \param i The index of the desired sample in the buffer. This index is counted
 * backwards with the latest value added to the buffer being at position 0.
 *
 * \return The desired sample value.
 */
env_t cbuf_index(struct cbuf * c, uint i);

/**
 * \brief Access the latest value added to a circular buffer.
 *
 * This call is equivalent to cbuf_index(c, 0).
 *
 * \param c The circular buffer to access.
 *
 * \return The desired sample value.
 */
env_t cbuf_get(struct cbuf * c);

/**
 * \brief Add a value to the end of the circular buffer.
 *
 * If the circular buffer has been filled, the oldest value in the buffer will
 * be overwritten by the new value.
 *
 * \param c The circular buffer to place data in.
 *
 * \param s The sample to append to the end of the circular buffer.
 */
void cbuf_put(struct cbuf * c, env_t s);

/**
 * \brief Rotate a circluar buffer.
 *
 * A new value is written to the end of the circular buffer and the oldest value
 * is returned. This oldest value is overwritten as the circular buffer is
 * advanced.
 *
 * \param c The circular buffer to rotate.
 *
 * \param s The sample to append to the end of the circular buffer.
 *
 * \return The oldest sample in the buffer.
 */
env_t cbuf_rotate(struct cbuf *c, env_t s);

/**
 * \brief Destroy a circular buffer that is no longer needed.
 *
 * All memory associated with the buffer is freed.
 *
 * \param c The circular buffer to destroy.
 */
void cbuf_exit(struct cbuf * c);

#endif /* !__TUNA_CBUF_H_INCLUDED__ */
