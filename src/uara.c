/*******************************************************************************
	uara.c: Main program entry point.

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

#include "uara.h"
#include "compiler.h"

uint input_sample_rate;
uint output_sample_rate;

int main(int argc, char * argv[])
{
	__unused argc;
	__unused argv;

	input_sample_rate = 625000;
	output_sample_rate = input_sample_rate;
	return 0;
}
