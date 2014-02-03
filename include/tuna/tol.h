/*******************************************************************************
	tol.h: Third octave level calculation.

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

#ifndef __TUNA_TOL_H_INCLUDED__
#define __TUNA_TOL_H_INCLUDED__

#include "types.h"

struct tol;

struct tol * tol_init(uint sample_rate, uint analysis_length, float overlap, uint phi_L);
void tol_exit(struct tol * t);
uint tol_get_num_levels(struct tol * t);
void tol_calculate(struct tol * t, float * data, float * results);

#endif /* !__TUNA_TOL_H_INCLUDED__ */
