/*******************************************************************************
	window.h: Window functions.

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

#ifndef __TUNA_WINDOW_H_INCLUDED__
#define __TUNA_WINDOW_H_INCLUDED__

#include "types.h"

/**
 * \file <tuna/window.h>
 *
 * \brief Windowing functions for FFT analysis.
 */

/**
 * Create a sine function window in a given buffer.
 *
 * \param window A pointer to a buffer which will be filled with window
 * coefficients.
 *
 * \param length The length of the window function which will be initialised.
 * This is therefore the length of the provided buffer in floats (not in bytes).
 */
void window_init_sine(float * window, uint length);

#endif /* !__TUNA_WINDOW_H_INCLUDED__ */
