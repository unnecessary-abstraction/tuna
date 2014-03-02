/*******************************************************************************
	window.c: Window function.

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

#include <assert.h>
#include <math.h>

#include "types.h"
#include "window.h"

void window_init_sine(float * window, uint length)
{
	uint i;

	/* We need to preserve the total energy content of the window - that is,
	 * for some constant input x[i] and our window w[i], the sum of
	 * (x[i] * w[i])^2 should be equal to the sum of (x[i])^2. The scale
	 * value is chosen to meet this condition.
	 */
	float scale = sqrtf(2.0f);

	assert(window);

	for (i = 0; i < length; i++) {
		window[i] = scale * sin(M_PI * i / length);
	}
}
