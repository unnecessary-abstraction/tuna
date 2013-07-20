/*******************************************************************************
	timespec.h: Timespec manipulation functions.

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

#ifndef __TUNA_TIMESPEC_H_INCLUDED__
#define __TUNA_TIMESPEC_H_INCLUDED__

#include <stdio.h>
#include <time.h>

#include "types.h"

void timespec_add_ns(struct timespec * ts, uint ns);
void timespec_add_ticks(struct timespec * ts, uint ticks, uint sample_rate);
int timespec_snprint(struct timespec * ts, char * s, size_t n);
int timespec_fprint(struct timespec * ts, FILE * f);

#endif /* !__TUNA_TIMESPEC_H_INCLUDED__ */
