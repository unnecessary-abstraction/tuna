/*******************************************************************************
	types.h: Basic types used by TUNA.

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

#ifndef __TUNA_TYPES_H_INCLUDED__
#define __TUNA_TYPES_H_INCLUDED__

#include <limits.h>

typedef unsigned int uint;
typedef unsigned long long uint64;

typedef int sample_t;
#define SAMPLE_MAX INT_MAX
#define SAMPLE_MIN INT_MIN

/* Envelope estimates have their own type incase it needs to be different from
 * the sample type.
 */
typedef float env_t;

#endif /* !__TUNA_TYPES_H_INCLUDED__ */
