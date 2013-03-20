/*******************************************************************************
	output.h: Wrappers for record output.

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

#ifndef __UARA_OUTPUT_H_INCLUDED__
#define __UARA_OUTPUT_H_INCLUDED__

#include "types.h"

struct output_driver;

struct record_type {
	const char *		name;
	struct output_driver *	driver;
	void *			driver_private;
};

struct record {
	struct record_type *	type;
	void *			driver_private;
};

typedef int (*output_driver_record_type_init_fn)(struct record_type *);
typedef void (*output_driver_record_type_exit_fn)(struct record_type *);
typedef int (*output_driver_record_init_fn)(struct record *);
typedef void (*output_driver_record_exit_fn)(struct record *);
typedef int (*output_driver_output_uint_fn)(struct record *, uint);
typedef int (*output_driver_output_double_fn)(struct record *, double);

struct output_driver {
	output_driver_record_type_init_fn	record_type_init;
	output_driver_record_type_exit_fn	record_type_exit;
	output_driver_record_init_fn		record_init;
	output_driver_record_exit_fn		record_exit;
	output_driver_output_uint_fn		output_uint;
	output_driver_output_double_fn		output_double;
};

int output_init(void);
void output_exit(void);
int record_type_init(struct record_type * type, const char * name, struct output_driver * driver);
void record_type_exit(struct record_type * type);
int record_init(struct record * rec, struct record_type * type);
void record_exit(struct record * rec);
int output_uint(struct record * rec, uint value);
int output_double(struct record * rec, double value);

#endif /* !__UARA_OUTPUT_H_INCLUDED__ */
