/*******************************************************************************
	bufhold.h: List of held buffers.

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

#ifndef __TUNA_BUFHOLD_H_INCLUDED__
#define __TUNA_BUFHOLD_H_INCLUDED__

#include "types.h"

/**
 * \file <tuna/bufhold.h>
 *
 * \brief Hold buffers in memory until they are no longer needed.
 *
 * This expands on the capabilities of <tuna/buffer.h> to provide more complete
 * management for holding onto buffers which are still needed. It is used by
 * consumer objects to preserve past data across write calls when this past data
 * is required for further processing.
 *
 * Whereas the basic buffer management routines only deal with the overall size
 * of the buffer, these routines track the number of valid samples in the buffer
 * and allow the start and length of the held buffers to be updated.
 */

struct bufhold;
struct held_buffer;

#ifdef DOXYGEN
/**
 * \brief A queue of held buffers.
 *
 * The ordering of the held buffers is preserved.
 */
struct bufhold {};

/**
 * \brief An individual held buffer.
 */
struct held_buffer {};
#endif

/**
 * \brief Get the oldest held buffer in a bufhold queue.
 *
 * \param bh The bufhold queue to operate on.
 *
 * \return A pointer to the oldest held buffer object stored in the given
 * bufhold queue.
 */
struct held_buffer * bufhold_oldest(struct bufhold * bh);

/**
 * \brief Get the newest held buffer in a bufhold queue.
 *
 * \param bh The bufhold queue to operate on.
 *
 * \return A pointer to the newest held buffer object stored in the given
 * bufhold queue.
 */
struct held_buffer * bufhold_newest(struct bufhold * bh);

/**
 * \brief Get the next held buffer in a bufhold queue.
 *
 * A pointer to the bufhold queue itself is not needed for this function.
 *
 * \param h The held buffer to operate on.
 *
 * \return A pointer to the next held buffer object after the given held buffer
 * object.
 */
struct held_buffer * bufhold_next(struct held_buffer * h);

/**
 * \brief Get the previous held buffer in a bufhold queue.
 *
 * A pointer to the bufhold queue itself is not needed for this function.
 *
 * \param h The held buffer to operate on.
 *
 * \return A pointer to the previous held buffer object before the given held
 * buffer object.
 */
struct held_buffer * bufhold_prev(struct held_buffer * h);

/**
 * \brief Get a pointer to the data stored in a held buffer.
 *
 * \param h The held buffer to operate on.
 *
 * \return A pointer to the data stored in the given held buffer.
 */
sample_t * bufhold_data(struct held_buffer * h);

/**
 * \brief Get the number of samples stored in a held buffer.
 *
 * \param h The held buffer to operate on.
 *
 * \return The number of samples stored in the given held buffer.
 */
uint bufhold_count(struct held_buffer * h);

/**
 * \brief Advance the start of a buffer held in a bufhold queue.
 *
 * This function allows data at the start of a buffer to be discarded from a
 * bufhold queue, leaving data at the end of that buffer still held. The actual
 * buffer release may be delayed until the whole buffer is freed but the return
 * values of bufhold_data() and bufhold_count() will be changed for the given
 * buffer.
 *
 * The given offset parameter determines how many samples to discard from the
 * given buffer. If this value is greater than or equal to the length of the
 * buffer in question then the buffer will be released from the bufhold queue as
 * if bufhold_release() had been called.
 *
 * \param h The buffer to advance.
 *
 * \param offset The number of samples by which to advance the start of the
 * given buffer.
 *
 * \return The number of samples remaining in the given held buffer. If this
 * value is zero, the given held buffer will have been released via a call to
 * bufhold_release().
 */
int bufhold_advance(struct held_buffer * h, uint offset);

/**
 * \brief Release a single buffer in a given bufhold queue.
 *
 * \param h The held buffer to be released.
 */
void bufhold_release(struct held_buffer * h);

/**
 * \brief Release all buffers held in a given bufhold queue.
 *
 * \param bh The bufhold queue to empty.
 */
void bufhold_release_all(struct bufhold * bh);

/**
 * \brief Add a buffer to the end of a bufhold queue.
 *
 * \param bh The bufhold to operate on.
 *
 * \param buf The start location of the buffer to add to the bufhold queue.
 *
 * \param count The length (in samples) of the buffer to add to the bufhold
 * queue.
 *
 * \return >=0 on success, <0 on failure.
 */
int bufhold_add(struct bufhold * bh, sample_t * buf, uint count);

/**
 * \brief Initialise a bufhold queue.
 *
 * \return A pointer to a new bufhold queue or NULL on error.
 */
struct bufhold * bufhold_init();

/**
 * \brief Destroy a bufhold queue which is no longer needed.
 */
void bufhold_exit(struct bufhold * bh);

#endif /* !__TUNA_BUFHOLD_H_INCLUDED__ */
