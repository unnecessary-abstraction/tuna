/*******************************************************************************
	offset_threshold.h: Offset detection threshold calculation.

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

#ifndef __TUNA_OFFSET_THRESHOLD_H_INCLUDED__
#define __TUNA_OFFSET_THRESHOLD_H_INCLUDED__

#include "types.h"

struct offset_threshold;

struct offset_threshold * offset_threshold_init(float Td, uint sample_rate,
		sample_t ratio);
void offset_threshold_exit(struct offset_threshold * o);
sample_t offset_threshold_next(struct offset_threshold * o, sample_t env);
void offset_threshold_reset(struct offset_threshold * o, sample_t env);
sample_t offset_threshold_delayed_min(struct offset_threshold * o);

#endif /* !__TUNA_OFFSET_THRESHOLD_H_INCLUDED__ */
