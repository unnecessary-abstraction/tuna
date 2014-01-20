/*******************************************************************************
	onset_threshold.h: Onset detection threshold calculation.

	Copyright (C) 2014 Paul Barker, Loughborough University

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

#ifndef __TUNA_ONSET_THRESHOLD_H_INCLUDED__
#define __TUNA_ONSET_THRESHOLD_H_INCLUDED__

#include "types.h"

struct onset_threshold;

struct onset_threshold * onset_threshold_init(float Tw, uint sample_rate,
		sample_t ratio);
void onset_threshold_exit(struct onset_threshold * onset);
sample_t onset_threshold_next(struct onset_threshold * onset, sample_t env);
void onset_threshold_reset(struct onset_threshold * onset);
uint onset_threshold_age(struct onset_threshold * onset);

#endif /* !__TUNA_ONSET_THRESHOLD_H_INCLUDED__ */
