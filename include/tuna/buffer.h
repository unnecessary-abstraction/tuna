/*******************************************************************************
	buffer.h: Sample buffering.

	Copyright (C) 2013 Paul Barker, Loughborough University
	
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

#ifndef __TUNA_BUFFER_H_INCLUDED__
#define __TUNA_BUFFER_H_INCLUDED__

#include "types.h"

/**
 * \file <tuna/buffer.h>
 *
 * \brief Buffer management.
 */

/**
 * \brief Acquire a buffer with space for at least a given number of samples.
 *
 * This function may allocate a buffer with more storage space than is required
 * if that would be more efficient. If additional space is allocated, the given
 * number of frames will be updated.
 *
 * Note that buffer_addref() doesn't need to be called after this function - the
 * reference count for the newly allocated buffer is initialised to 1.
 *
 * \param frames A pointer to the number of frames for which space is required.
 * If additional space is allocated, the value that this parameter points to
 * will be updated.
 *
 * \return A pointer to a new buffer or NULL on error.
 */
sample_t * buffer_acquire(uint * frames);

/**
 * \brief Add a reference to an existing buffer.
 *
 * Buffer objects are reference counted so that they can have multiple users and
 * can be freed when the last user releases them. Any code which duplicates a
 * pointer to a buffer should call this function and then call buffer_release()
 * when it is finished with the duplicate.
 *
 * \param p The buffer to which a reference will be added.
 */
void buffer_addref(sample_t * p);

/**
 * \brief Release a reference to an existing buffer.
 *
 * This function decrements the reference count of the given buffer. It should
 * be called whenever a pointer to a buffer is no longer needed. If this was the
 * last reference to the given buffer the associated memory may be freed or
 * recycled for use in future buffers.
 *
 * \param p The buffer to which a reference will be released.
 *
 * \return 0 if the buffer remains referenced elsewhere, 1 if the buffer was
 * free'd back to the system.
 */
int buffer_release(sample_t * p);

/**
 * \brief Get a count of the number of refs held on a buffer.
 *
 * This function is mainly intended for use in testing libtuna.
 *
 * \param p The buffer of which to get the reference count.
 *
 * \return The number of references held on the given buffer.
 */
uint buffer_refcount(sample_t * p);

#endif /* !__TUNA_BUFFER_H_INCLUDED__ */
