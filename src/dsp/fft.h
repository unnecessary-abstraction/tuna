/*******************************************************************************
	fft.h: FFT implementation using fftw.

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

#ifndef __UARA_FFT_H_INCLUDED__
#define __UARA_FFT_H_INCLUDED__

#include "types.h"

int init_fft(uint length);
void fft_no_window(void);
void fft_with_window(void);
void fft_load(double * source, uint offset, uint length);
void fft_pad(uint offset);
double * fft_get_data(void);

#endif /* !__UARA_FFT_H_INCLUDED__ */
