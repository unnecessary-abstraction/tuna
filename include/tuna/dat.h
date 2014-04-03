/*******************************************************************************
	dat.h: Raw data file output.

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

#ifndef __TUNA_DAT_H_INCLUDED__
#define __TUNA_DAT_H_INCLUDED__

#include <stdio.h>
#include <time.h>

/**
 * \file <tuna/dat.h>
 *
 * \brief Raw data file output
 */

/* Magic numbers. */
enum tuna_dat_magic {
	TUNA_DAT_MAGIC = 0xBADDBEEF,
	TUNA_DAT_ENDIAN_INDICATOR = 0xAABBCCDD
};

/* Record types. */
enum tuna_dat_record_types {
	TUNA_DAT_NULL,

	TUNA_DAT_RECORD_BASE = 0x01000000,
	TUNA_DAT_START,
	TUNA_DAT_RESYNC,

	TUNA_DAT_MISC_DATA = 0x02000000,
	TUNA_DAT_SIGNAL,
	TUNA_DAT_TIME_SLICE,
	TUNA_DAT_PULSE
};

FILE * dat_open(const char * filename);
void dat_close(FILE * dat);
int dat_write_record(FILE * dat, int record_type, void * data, size_t count);
int dat_write_null(FILE * dat, size_t count);
int dat_write_start(FILE * dat, struct timespec * ts);
int dat_write_resync(FILE * dat, struct timespec * ts);

#endif /* !__TUNA_DAT_H_INCLUDED__ */
