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

#include "compiler.h"
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
TUNA_INLINE env_t cbuf_rotate(struct cbuf *c, env_t s);

/**
 * \brief Destroy a circular buffer that is no longer needed.
 *
 * All memory associated with the buffer is freed.
 *
 * \param c The circular buffer to destroy.
 */
void cbuf_exit(struct cbuf * c);

#include "tuna_inl/cbuf.inl"

#endif /* !__TUNA_CBUF_H_INCLUDED__ */
