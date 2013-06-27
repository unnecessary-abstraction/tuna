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
#include "output_csv.h"

int main(int argc, char * argv[])
{
	uint len;
	sample_t buf[4];
	int r;

	__unused argc;
	__unused argv;

	struct consumer * csv = output_csv_init("test.csv");

	/* Get current time. */
	struct timespec ts;
	r = clock_gettime(CLOCK_REALTIME, &ts);
	if (r < 0)
		return -1;
	
	/* Set 3 samples per line. */
	csv->start(csv, 3, &ts);

	/* Write 3 values */
	len = 3;
	buf[0] = 1;
	buf[1] = 2;
	buf[2] = 3;
	buf[3] = 4;
	csv->write(csv, buf, len);
	
	/* Write 2 values. */
	len = 2;
	buf[0] = 11;
	buf[1] = 12;
	buf[2] = 13;
	buf[3] = 14;
	csv->write(csv, buf, len);

	/* Write 4 values - first should finish a line, next 3 should fill their own line. */
	len = 4;
	buf[0] = 21;
	buf[1] = 22;
	buf[2] = 23;
	buf[3] = 24;
	csv->write(csv, buf, len);

	csv->exit(csv);
	return 0;
}
