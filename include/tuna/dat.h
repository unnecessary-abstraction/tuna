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

/**
 * \brief Magic numbers used within DAT file output.
 */
enum tuna_dat_magic {
	/**
	 * \brief Magic number used to identify DAT file format.
	 *
	 * This magic number is written to the first 4 bytes of a DAT file in
	 * big endian byte order.
	 */
	TUNA_DAT_MAGIC = 0x0BADBEEF,

	/**
	 * \brief Magic number used to identify the byte order of a DAT file.
	 *
	 * This magic number is written to bytes 4-8 of a DAT file in the host
	 * byte order. When the DAT file is read, the ordering of the bytes in
	 * this number can be used to identify whether the byte order of the
	 * system which wrote the data file is the same as the byte order of the
	 * current system or not. If the byte orders differ, the bytes within
	 * the data values written to the file will be reversed when they are
	 * read. The program reading the DAT file can fix the byte order itself.
	 *
	 * By taking this approach, the program writing a DAT file doesn't have
	 * to reverse the bytes in every single value it writes, thus
	 * potentially speeding up DAT file writes. As speed matters to writers
	 * more than readers in this application, this is preferable.
	 */
	TUNA_DAT_ENDIAN_INDICATOR = 0x11223344
};

/**
 * \brief Record type identifiers.
 *
 * These type identifiers are written to DAT files at the start of each new
 * record. They are written in big endian byte order to ensure the semantics of
 * NULL records are simple. Correcting the byte order of a single 4 byte value
 * for each record that is written shouldn't put too high an overhead on the
 * program writing DAT files.
 */
enum tuna_dat_record_types {
	/**
	 * \brief NULL record identifier.
	 *
	 * NULL records are 1 byte in length. That is, a single '\0' is written
	 * rather than 4 bytes of zeros. This allows NULL records to be used as
	 * padding. Because of this decision, all other record types must not
	 * contain '\0' in their most-significant byte.
	 */
	TUNA_DAT_NULL,

	/**
	 * \brief Unused identifier: Marks the start of true (non-NULL) record
	 * types.
	 */
	TUNA_DAT_RECORD_BASE = 0x01000000,

	/**
	 * \brief START record identifier.
	 *
	 * This record indicates that a START event occurred and contains the
	 * timespec at which it occurred.
	 */
	TUNA_DAT_START,

	/**
	 * \brief RESYNC record identifier.
	 *
	 * This record indicates that a START event occurred and contains the
	 * timespec at which it occurred.
	 */
	TUNA_DAT_RESYNC,

	/**
	 * \brief Miscallaneous data record identifier.
	 */
	TUNA_DAT_MISC_DATA = 0x02000000,

	/**
	 * \brief Raw signal data record identifier.
	 *
	 * Records of this type contain raw signal data. Further details are not
	 * currently specified.
	 */
	TUNA_DAT_SIGNAL,

	/**
	 * \brief Time slice record identifier.
	 *
	 * Records of this type contain time slice IRPs.
	 */
	TUNA_DAT_TIME_SLICE,

	/**
	 * \brief Pulse record identifier.
	 *
	 * Records of this type contain pulse IRPs.
	 */
	TUNA_DAT_PULSE
};

/**
 * \brief Open a file for output in DAT format.
 *
 * \param filename The name and path of the file to open.
 *
 * \return A new FILE * object or NULL on failure.
 */
FILE * dat_open(const char * filename);

/**
 * \brief Close an output file in DAT format.
 *
 * \param dat The file to close.
 */
void dat_close(FILE * dat);

/**
 * \brief Write a record to an output file in DAT format.
 *
 * \param dat The file to write to.
 *
 * \param record_type The record type selected from enum tuna_dat_record_types.
 *
 * \return >=0 on success, <0 on failure.
 */
int dat_write_record(FILE * dat, int record_type, void * data, size_t count);

/**
 * \brief Write a number of NULL records to an output file in DAT format.
 *
 * This is a convenience wrapper equivalent to calling
 *  dat_write_record(dat, TUNA_DAT_NULL, NULL, 0)
 * a number of times equal to count. It is expected to be significantly faster
 * than directly calling dat_write_record() repeatedly though.
 *
 * \param dat The file to write to.
 *
 * \param count The number of NULL records to write.
 *
 * \return >=0 on success, <0 on failure.
 */
int dat_write_null(FILE * dat, size_t count);

/**
 * \brief Write a START record to an output file in DAT format.
 *
 * This is a convenience wrapper equivalent to calling
 *  dat_write_record(dat, TUNA_DAT_START, ts, sizeof(struct timespec))
 *
 * \param dat The file to write to.
 *
 * \param ts The timespec at which the START event occurred.
 *
 * \return >=0 on success, <0 on failure.
 */
int dat_write_start(FILE * dat, struct timespec * ts);

/**
 * \brief Write a RESYNC record to an output file in DAT format.
 *
 * This is a convenience wrapper equivalent to calling
 *  dat_write_record(dat, TUNA_DAT_RESYNC, ts, sizeof(struct timespec))
 *
 * \param dat The file to write to.
 *
 * \param ts The timespec at which the RESYNC event occurred.
 *
 * \return >=0 on success, <0 on failure.
 */
int dat_write_resync(FILE * dat, struct timespec * ts);

#endif /* !__TUNA_DAT_H_INCLUDED__ */
