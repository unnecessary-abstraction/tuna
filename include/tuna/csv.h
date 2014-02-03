/*******************************************************************************
	csv.h: CSV file output.

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

#ifndef __TUNA_CSV_H_INCLUDED__
#define __TUNA_CSV_H_INCLUDED__

#include <stdio.h>
#include <time.h>

#include "types.h"

/**
 * \file <tuna/csv.h>
 *
 * \brief CSV file output
 *
 * This is a simple set of functions to output data to CSV files in a consistent
 * way. It doesn't implement the consumer interface that is provided by other
 * output modules as it isn't possible to directly tie it to a provider module.
 * Instead it is intended to be used internally by analysis consumers.
 *
 * We also don't declare our own struct type. To keep things really simple,
 * these functions just interact with a standard FILE *.
 */

/**
 * Open a file for writing data in CSV format. The file will be created if it
 * does not exist and will be truncated if it does exist.
 *
 * \return A new FILE * for the opened CSV file or NULL on error.
 */
FILE * csv_open(const char * filename);

/**
 * Close an open CSV file.
 *
 * \param csv The CSV file to close.
 */
void csv_close(FILE * csv);

/**
 * Write a field containing a given floating-point value to an open CSV file.
 *
 * \param csv The CSV file to write to.
 *
 * \param f The floating-point value to be written as a field in the given CSV
 * file.
 *
 * \return >=0 on success, <0 on failure.
 */
int csv_write_float(FILE * csv, float f);

/**
 * Write a field containing a given sample value to an open CSV file.
 *
 * \param csv The CSV file to write to.
 *
 * \param s The sample value to be written as a field in the given CSV file.
 *
 * \return >=0 on success, <0 on failure.
 */
int csv_write_sample(FILE * csv, sample_t s);

/**
 * Write a field containing a given unsigned integer value to an open CSV file.
 *
 * \param csv The CSV file to write to.
 *
 * \param u The unsigned integer value to be written as a field in the given CSV
 * file.
 *
 * \return >=0 on success, <0 on failure.
 */
int csv_write_uint(FILE * csv, uint u);

/**
 * Finish the current record in a given CSV file and start a new record. This
 * essentially just means write a newline to the file.
 *
 * \param csv The CSV file to advance to a new record.
 *
 * \return >=0 on success, <0 on failure.
 */
int csv_next(FILE * csv);

/**
 * Write a start marker and a given time stamp to an open CSV file. This creates
 * a line (or record) in the CSV file which contains the word "START" followed
 * by the given time stamp.
 *
 * \param csv The CSV file to write to.
 *
 * \param ts The time stamp to place in the CSV file in timespec format.
 *
 * \return >=0 on success, <0 on failure.
 */
int csv_write_start(FILE * csv, struct timespec * ts);

/**
 * Write a resynchronisation marker and a given time stamp to an open CSV file.
 * This creates a line (or record) in the CSV file which contains the word
 * "RESYNC" followed by the given time stamp.
 *
 * \param csv The CSV file to write to.
 *
 * \param ts The time stamp to place in the CSV file in timespec format.
 *
 * \return >=0 on success, <0 on failure.
 */
int csv_write_resync(FILE * csv, struct timespec * ts);

#endif /* !__TUNA_CSV_H_INCLUDED__ */
