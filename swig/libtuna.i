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

/* For all bindings, ignore the log wrappers. The only work properly for native
 * C code.
 *
 * We need to do this rather than just skipping "log.h" as programs which use
 * libtuna need to be able to call log_init() and log_exit().
 */
%ignore log_printf;
%ignore msg;
%ignore warn;
%ignore error;
%ignore fatal;

%{
        #include "analysis.h"
        #include "buffer.h"
        #include "bufhold.h"
        #include "bufq.h"
        #include "cbuf.h"
        #include "consumer.h"
        #include "counter.h"
        #include "csv.h"
        #include "dat.h"
        #include "env_estimate.h"
        #include "fft.h"
#ifdef ENABLE_ADS1672
        #include "input_ads1672.h"
#endif
        #include "input_alsa.h"
        #include "input_sndfile.h"
        #include "input_zero.h"
        #include "log.h"
        #include "onset_threshold.h"
        #include "offset_threshold.h"
        #include "output_null.h"
        #include "output_sndfile.h"
        #include "pulse.h"
        #include "time_slice.h"
        #include "tol.h"
        #include "types.h"
        #include "window.h"
%}

#define __noreturn
#define TUNA_INLINE

%include "analysis.h"
%include "buffer.h"
%include "bufhold.h"
%include "bufq.h"
%include "cbuf.h"
%include "consumer.h"
%include "counter.h"
%include "csv.h"
%include "dat.h"
%include "env_estimate.h"
%include "fft.h"
#ifdef ENABLE_ADS1672
%include "input_ads1672.h"
#endif
%include "input_alsa.h"
%include "input_sndfile.h"
%include "input_zero.h"
%include "log.h"
%include "onset_threshold.h"
%include "offset_threshold.h"
%include "output_null.h"
%include "output_sndfile.h"
%include "pulse.h"
%include "time_slice.h"
%include "tol.h"
%include "types.h"
%include "window.h"

/* Incase anyone really needs it, we provide a simple logging wrapper. The
 * string formatting capabilities of the bound language can be used and so no
 * formatting is done by this method, the string given is simply logged as-is.
 */
%inline %{
int log_print(int level, const char * s)
{
        return log_printf(level, "%s", s);
}
%}
