/*******************************************************************************
	third_octave_levels.h: Third octave level calculation.

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

#ifndef __UARA_THIRD_OCTAVE_LEVELS_H_INCLUDED__
#define __UARA_THIRD_OCTAVE_LEVELS_H_INCLUDED__

#include "types.h"

#define N_THIRD_OCTAVE_LEVELS	43

struct third_octave_levels {
	double		levels[N_THIRD_OCTAVE_LEVELS + 1];
};

void third_octave_levels(struct third_octave_levels * output);
int init_third_octave_levels(unsigned long sample_rate);
void write_third_octave_levels(struct record * rec, struct third_octave_levels * tol);

#endif /* !__UARA_THIRD_OCTAVE_LEVELS_H_INCLUDED__ */
