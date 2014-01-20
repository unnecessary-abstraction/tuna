/*******************************************************************************
	output_sndfile.h: Output via libsndfile.

	Copyright (C) 2013, 2014 Paul Barker, Loughborough University
	
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

#ifndef __TUNA_OUTPUT_SNDFILE_H_INCLUDED__
#define __TUNA_OUTPUT_SNDFILE_H_INCLUDED__

#include "consumer.h"
#include "types.h"

/**
 * \file <tuna/output_sndfile.h>
 *
 * \brief Consumer module to write sample data to any format supported by
 * libsndfile.
 *
 * This consumer module is useful for both testing and basic recording of
 * captured acoustic data without analysis. It uses libsndfile to write data to
 * a WAVE file or other appropriate sound file format.
 */

/**
 * Initialise sndfile output consumer.
 *
 * Output filenames are constructed from the given prefix, an index number and
 * the given suffix. For example, if the prefix is "output-" and the suffix is
 * ".wav", output files will be named "output-000.wav", "output-001.wav", etc.
 * The size of each output file will be limited by max_samples_per_file.
 *
 * \param consumer The consumer object to initialise. The call to
 * output_sndfile_init() should immediately follow the creation of a consumer
 * object with consumer_new().
 *
 * \param prefix The first part of the path for the output file that is to be
 * written.
 *
 * \param suffix The last part of the path for the output file that is to be
 * written.
 *
 * \param format The output file format. This value should be constructed from
 * the format flags specified in <sndfile.h>.
 *
 * \param max_samples_per_file Maximum number of samples to be written to a
 * single output file until it is closed and a new output file is started.
 *
 * \return >=0 on success, <0 on failure.
 */
int output_sndfile_init(struct consumer * consumer, const char * prefix,
		const char * suffix, int format, uint max_samples_per_file);

#endif /* !__TUNA_OUTPUT_SNDFILE_H_INCLUDED__ */
