/*******************************************************************************
	tol.h: Third octave level calculation.

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

#ifndef __TUNA_TOL_H_INCLUDED__
#define __TUNA_TOL_H_INCLUDED__

#include "types.h"

#define MAX_THIRD_OCTAVE_LEVELS 43

struct tol_transition {
	uint				t_onset;
	uint				t_width;
	float *			coeffs;
};

struct tol {
	/* Number of third octave levels which are active. This is at most
	 * MAX_THIRD_OCTAVE_LEVELS when the sample rate is high enough.
	 * Otherwise this is the number of third octave levels which are
	 * completely contained below half the sampling rate.
	 */
	uint				n_tol;

	struct tol_transition		desc[MAX_THIRD_OCTAVE_LEVELS];
};

struct tol_results {
	float				values[MAX_THIRD_OCTAVE_LEVELS + 1];
};

void tol_calculate(struct tol * t, float * data, struct tol_results * r);
int tol_init(struct tol * t, uint sample_rate, uint analysis_length, float overlap, uint phi_L);
void tol_exit(struct tol * t);

#endif /* !__TUNA_TOL_H_INCLUDED__ */
