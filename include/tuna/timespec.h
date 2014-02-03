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

/**
 * \file <tuna/timespec.h>
 *
 * \brief Timespec manipulation functions.
 */

/**
 * Add a number of nanoseconds to a timespec in place.
 *
 * \param ts Timespec to modify.
 *
 * \param ns Number of nanoseconds to add to the given timespec.
 */
void timespec_add_ns(struct timespec * ts, uint ns);

/**
 * Add a number of ticks at a given sample rate to a timespec in place.
 *
 * \param ts Timespec to modify.
 *
 * \param ticks Number of clock ticks to add the the given timespec.
 *
 * \param sample_rate The clock frequency to use.
 */
void timespec_add_ticks(struct timespec * ts, uint ticks, uint sample_rate);

/**
 * Print a timespec to a string.
 *
 * \param ts Timespec to print.
 *
 * \param s String buffer for output.
 *
 * \param n Maximum number of characters to write to the string buffer,
 * including the null terminator.
 *
 * \return Number of characters that would be written to the string buffer if
 * there was adequate space, or <0 on error. Thus if the return value is greater
 * than or equal to n, truncation has occurred.
 */
int timespec_snprint(struct timespec * ts, char * s, size_t n);

/**
 * Print a timespec to a file.
 *
 * \param ts Timespec to print.
 *
 * \param f File to print timespec to.
 *
 * \return Number of characters written to the file, or <0 on error.
 */
int timespec_fprint(struct timespec * ts, FILE * f);

#endif /* !__TUNA_TIMESPEC_H_INCLUDED__ */
