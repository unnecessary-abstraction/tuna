/*******************************************************************************
	output_csv_test.c: Test program for CSV output driver.

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

#include <math.h>
#include "compiler.h"
#include "output.h"
#include "output_csv.h"

int main(int argc, char * argv[])
{
	__unused argc;
	__unused argv;

	struct record_type type;
	struct record rec;

	output_init();
	csv_init();
	record_type_init(&type, "test", &csv_driver);
	
	record_init(&rec, &type);
	output_uint(&rec, 1);
	output_uint(&rec, 2);
	output_double(&rec, M_PI);
	record_exit(&rec);
	
	record_init(&rec, &type);
	output_double(&rec, 3);
	output_double(&rec, 3.1);
	record_exit(&rec);
	
	record_type_exit(&type);
	csv_exit();
	output_exit();

	return 0;
}
