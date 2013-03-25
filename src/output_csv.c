/*******************************************************************************
	output_csv.c: Output to CSV files.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "output.h"
#include "output_csv.h"
#include "types.h"

/* TODO: Error handling. */

/*******************************************************************************
	Public data
*******************************************************************************/

struct output_driver csv_driver = {
	csv_record_type_init,
	csv_record_type_exit,
	csv_record_init,
	csv_record_exit,
	csv_output_uint,
	csv_output_double
};

/*******************************************************************************
	Public functions
*******************************************************************************/

int csv_init(void)
{
	return 0;
}

void csv_exit(void)
{
	/* Do nothing. */
}

int csv_record_type_init(struct record_type * type)
{
	char * fname;
	const char * suffix = ".csv";
	FILE * f;
	
	/* Inefficient, but only happens during startup. */
	fname = malloc(strlen(type->name) + strlen(suffix));
	strcpy(fname, type->name);
	strcat(fname, suffix);
	f = fopen(fname, "w");
	
	type->driver_private = (void *)f;
	
	return 0;
}

void csv_record_type_exit(struct record_type * type)
{
	__unused type;
	/* Do nothing. */
}

int csv_record_init(struct record * rec)
{
	__unused rec;
	return 0;
}

void csv_record_exit(struct record * rec)
{
	/* Just write a newline. */
	FILE * f = (FILE *)rec->type->driver_private;
	fputc('\n', f);
}

int csv_output_uint(struct record * rec, uint value)
{
	FILE * f = (FILE *)rec->type->driver_private;
	fprintf(f, "%u, ", value);
	
	return 0;
}

int csv_output_double(struct record * rec, double value)
{
	FILE * f = (FILE *)rec->type->driver_private;
	fprintf(f, "%f, ", value);
	
	return 0;
}
