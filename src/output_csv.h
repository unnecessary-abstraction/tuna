/*******************************************************************************
	output_csv.h: Output to CSV files.

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

#ifndef __UARA_OUTPUT_CSV_H_INCLUDED__
#define __UARA_OUTPUT_CSV_H_INCLUDED__

#include "output.h"
#include "types.h"

int csv_init(void);
void csv_exit(void);
int csv_record_type_init(struct record_type * type);
void csv_record_type_exit(struct record_type * type);
int csv_record_init(struct record * rec);
void csv_record_exit(struct record * rec);
int csv_output_uint(struct record * rec, uint value);
int csv_output_double(struct record * rec, double value);

extern struct output_driver csv_driver;

#endif /* !__UARA_OUTPUT_CSV_H_INCLUDED__ */
