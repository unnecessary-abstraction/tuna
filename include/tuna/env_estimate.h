/*******************************************************************************
	env_estimate.h: Sample-based peak envelope estimation.

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

#ifndef __TUNA_ENV_ESTIMATE_H_INCLUDED__
#define __TUNA_ENV_ESTIMATE_H_INCLUDED__

#include "types.h"

struct env_estimate;

struct env_estimate * env_estimate_init(float Tc, uint sample_rate, float sample_limit);
void env_estimate_exit(struct env_estimate * e);
void env_estimate_reset(struct env_estimate * e);
sample_t env_estimate_next(struct env_estimate * e, sample_t x);

#endif /* !__TUNA_ENV_ESTIMATE_H_INCLUDED__ */
