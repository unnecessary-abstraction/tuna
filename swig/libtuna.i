/*******************************************************************************
	libtuna.i: libtuna SWIG interface file.

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

%module libtuna

%{
        #include "analysis.h"
        #include "buffer.h"
        #include "bufhold.h"
        #include "bufq.h"
        #include "cbuf.h"
        #include "consumer.h"
        #include "csv.h"
        #include "env_estimate.h"
        #include "fft.h"
#ifdef ENABLE_ADS1672
        #include "input_ads1672.h"
#endif
        #include "input_alsa.h"
        #include "input_sndfile.h"
        #include "input_zero.h"
        #include "log.h"
        #include "minima.h"
        #include "onset_threshold.h"
        #include "offset_threshold.h"
        #include "output_null.h"
        #include "output_sndfile.h"
        #include "pulse.h"
        #include "slab.h"
        #include "time_slice.h"
        #include "tol.h"
        #include "window.h"
%}

#define __noreturn

%include "analysis.h"
%include "buffer.h"
%include "bufhold.h"
%include "bufq.h"
%include "cbuf.h"
%include "consumer.h"
%include "csv.h"
%include "env_estimate.h"
%include "fft.h"
#ifdef ENABLE_ADS1672
%include "input_ads1672.h"
#endif
%include "input_alsa.h"
%include "input_sndfile.h"
%include "input_zero.h"
%include "log.h"
%include "minima.h"
%include "onset_threshold.h"
%include "offset_threshold.h"
%include "output_null.h"
%include "output_sndfile.h"
%include "pulse.h"
%include "slab.h"
%include "time_slice.h"
%include "tol.h"
%include "window.h"
