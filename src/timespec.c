/*******************************************************************************
	timespec.c: Timespec manipulation functions.

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

#include <time.h>

#include "timespec.h"
#include "types.h"
#include "uara.h"

#define NS 1000000000

void timespec_add_ns(struct timespec * ts, uint ns)
{
	ts->tv_nsec += ns;
	if (ts->tv_nsec >= NS) {
		ts->tv_sec += ts->tv_nsec / NS;
		ts->tv_nsec = ts->tv_nsec % NS;
	}
}

void timespec_add_ticks(struct timespec * ts, uint ticks, uint sample_rate)
{
	uint64 tmp = ticks * NS;
	uint ns = tmp / sample_rate;
	timespec_add_ns(ts, ns);
}

