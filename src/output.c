/*******************************************************************************
	output.c: Wrappers for record output.

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

#include "output.h"
#include "types.h"

/*******************************************************************************
	Public functions
*******************************************************************************/

int output_init(void)
{
	return 0;
}

void output_exit(void)
{
	/* Do nothing. */
}

int record_type_init(struct record_type * type, const char * name, struct output_driver * driver)
{
	type->name = name;
	type->driver = driver;
	driver->record_type_init(type);
	
	return 0;
}

void record_type_exit(struct record_type * type)
{
	type->driver->record_type_exit(type);
}

int record_init(struct record * rec, struct record_type * type)
{
	rec->type = type;
	type->driver->record_init(rec);
	
	return 0;
}

void record_exit(struct record * rec)
{
	rec->type->driver->record_exit(rec);
}

int output_uint(struct record * rec, uint value)
{
	return rec->type->driver->output_uint(rec, value);
}

int output_double(struct record * rec, double value)
{
	return rec->type->driver->output_double(rec, value);
}
