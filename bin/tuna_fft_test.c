/*******************************************************************************
	tuna_fft_test.c: Test program for FFT.

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

/* As I'm unsure whether I can build ffts with -fPIC, I'm sticking to static
 * linking for now. That means I can't generate python bindings and use the test
 * suite which I previously developed. Thus to validate the FFT module when it
 * is using ffts I need a separate program which can be statically linked
 * against libtuna and ffts.
 *
 * This will also work with FFTW but it won't strictly be necessary in that
 * case.
 */

#include <math.h>

#include "fft.h"
#include "log.h"

int test_init()
{
	struct fft * fft;
	uint fft_length = 8192;

	fft = fft_init(fft_length);
	if (!fft) {
		error("test_init: fft_init failed");
		return -1;
	}

	if (fft_get_length(fft) != fft_length) {
		error("test_init: fft_get_length wrong");
		return -1;
	}

	if (!fft_get_data(fft)) {
		error("test_init: fft_get_data returned NULL");
		return -1;
	}

	if (!fft_get_cdata(fft)) {
		error("test_init: fft_get_cdata returned NULL");
		return -1;
	}

	fft_exit(fft);
	return 0;
}

int test_zeros()
{
	uint i;
	int r;
	struct fft * fft;
	float * data;
	complex float * cdata;
	uint fft_length = 8192;

	fft = fft_init(fft_length);
	if (!fft) {
		error("test_zeros: fft_init failed");
		return -1;
	}

	data = fft_get_data(fft);
	if (!data) {
		error("test_zeros: fft_get_data returned NULL");
		return -1;
	}

	cdata = fft_get_cdata(fft);
	if (!cdata) {
		error("test_zeros: fft_get_cdata returned NULL");
		return -1;
	}

	/* Fill input buffer with zeros. */
	for (i = 0; i < fft_length; i++)
		data[i] = 0.0f;

	r = fft_transform(fft);
	if (r < 0) {
		error("test_zeros: fft_transform failed");
		return -1;
	}

	fft_power_spectrum(cdata, data, fft_length/2);

	/* Ensure that the output data is all zeros. */
	for (i = 0; i < fft_length/2; i++) {
		if (data[i] != 0.0f) {
			error("test_zeros: non-zero output at index %u", i);
			return -1;
		}
	}

	fft_exit(fft);
	return 0;
}

int test_dc()
{
	uint i;
	int r;
	struct fft * fft;
	float * data;
	complex float * cdata;
	uint fft_length = 8192;

	fft = fft_init(fft_length);
	if (!fft) {
		error("test_dc: fft_init failed");
		return -1;
	}

	data = fft_get_data(fft);
	if (!data) {
		error("test_dc: fft_get_data returned NULL");
		return -1;
	}

	cdata = fft_get_cdata(fft);
	if (!cdata) {
		error("test_dc: fft_get_cdata returned NULL");
		return -1;
	}

	/* Fill input buffer with a fixed value. */
	for (i = 0; i < fft_length; i++)
		data[i] = 1.0f;

	r = fft_transform(fft);
	if (r < 0) {
		error("test_dc: fft_transform failed");
		return -1;
	}

	fft_power_spectrum(cdata, data, fft_length/2);

	/* Ensure that the output data is all zeros except for the dc term. */
	if (data[0] != 8192.0f) {
		error("test_dc: dc output term is %f, should be 8192", data[0]);
		return -1;
	}

	for (i = 1; i < fft_length/2; i++) {
		if (data[i] != 0.0f) {
			error("test_dc: non-zero output at index %u", i);
			return -1;
		}
	}

	fft_exit(fft);
	return 0;
}

int almost_equal(float a, float b, float e)
{
	float d = a - b;
	return (d < e);
}

int test_sine()
{
	uint i;
	int r;
	struct fft * fft;
	float * data;
	complex float * cdata;
	uint fft_length = 8192;

	fft = fft_init(fft_length);
	if (!fft) {
		error("test_sine: fft_init failed");
		return -1;
	}

	data = fft_get_data(fft);
	if (!data) {
		error("test_sine: fft_get_data returned NULL");
		return -1;
	}

	cdata = fft_get_cdata(fft);
	if (!cdata) {
		error("test_sine: fft_get_cdata returned NULL");
		return -1;
	}

	/* Fill input buffer with 1024 Hz sine wave. */
	for (i = 0; i < fft_length; i++)
		data[i] = sinf(M_PI * i / 4);

	r = fft_transform(fft);
	if (r < 0) {
		error("test_sine: fft_transform failed");
		return -1;
	}

	fft_power_spectrum(cdata, data, fft_length/2);

	/* Ensure that the output data is all zeros except for the 1 kHz term.
	 */
	for (i = 0; i < 1024; i++) {
		if (!almost_equal(data[i], 0.0f, 0.000002)) {
			error("test_sine: non-zero output %f at index %u", data[i], i);
			return -1;
		}
	}

	if (!almost_equal(data[1024], 2048.0f, 0.000002)) {
		error("test_sine: 1024 Hz output term is %f, should be 2048", data[1024]);
		return -1;
	}

	for (i = 1025; i < fft_length/2; i++) {
		if (!almost_equal(data[i], 0.0f, 0.000002)) {
			error("test_sine: non-zero output %f at index %u", data[i], i);
			return -1;
		}
	}

	fft_exit(fft);
	return 0;
}

int test_3_sine()
{
	uint i;
	int r;
	struct fft * fft;
	float * data;
	complex float * cdata;
	uint fft_length = 8192;

	fft = fft_init(fft_length);
	if (!fft) {
		error("test_3_sine: fft_init failed");
		return -1;
	}

	data = fft_get_data(fft);
	if (!data) {
		error("test_3_sine: fft_get_data returned NULL");
		return -1;
	}

	cdata = fft_get_cdata(fft);
	if (!cdata) {
		error("test_3_sine: fft_get_cdata returned NULL");
		return -1;
	}

	/* Fill input buffer with:
	 *	512 Hz sine wave, peak amplitude = 5
	 *	1024 Hz sine wave, peak amplitude = 1
	 *	1600 Hz sine wave, peak amplitude = 3
	 */
	for (i = 0; i < fft_length; i++)
		data[i] = sinf(M_PI * i / 4) + 5 * sinf(M_PI * i / 8) +
			3 * sinf(M_PI * i * 1600.0f / 4096.0f);

	r = fft_transform(fft);
	if (r < 0) {
		error("test_3_sine: fft_transform failed");
		return -1;
	}

	fft_power_spectrum(cdata, data, fft_length/2);

	/* Ensure that the output data is all zeros except for the 1 kHz term.
	 */
	for (i = 0; i < 512; i++) {
		if (!almost_equal(data[i], 0.0f, 0.00002f)) {
			error("test_3_sine: non-zero output %f at index %u", data[i], i);
			return -1;
		}
	}

	if (!almost_equal(data[512], 5 * 5 * 2048.0f, 0.1f)) {
		error("test_3_sine: 512 Hz output term is %f, should be (5^2 * 2048)", data[512]);
		return -1;
	}

	for (i = 513; i < 1024; i++) {
		if (!almost_equal(data[i], 0.0, 0.00002f)) {
			error("test_3_sine: non-zero output %f at index %u", data[i], i);
			return -1;
		}
	}

	if (!almost_equal(data[1024], 2048.0f, 0.01f)) {
		error("test_3_sine: 1024 Hz output term is %f, should be 2048", data[1024]);
		return -1;
	}

	for (i = 1025; i < 1600; i++) {
		if (!almost_equal(data[i], 0.0f, 0.00002f)) {
			error("test_3_sine: non-zero output %f at index %u", data[i], i);
			return -1;
		}
	}

	if (!almost_equal(data[1600], 3 * 3 * 2048.0f, 0.01f)) {
		error("test_3_sine: 1024 Hz output term is %f, should be (3^2 * 2048)", data[1600]);
		return -1;
	}

	for (i = 1601; i < fft_length/2; i++) {
		if (!almost_equal(data[i], 0.0f, 0.0001f)) {
			error("test_3_sine: non-zero output %f at index %u", data[i], i);
			return -1;
		}
	}

	fft_exit(fft);
	return 0;
}

int run_tests()
{
	int r;

	r = test_init();
	if (r < 0)
		return r;

	r = test_zeros();
	if (r < 0)
		return r;

	r = test_dc();
	if (r < 0)
		return r;

	r = test_sine();
	if (r < 0)
		return r;

	r = test_3_sine();
	if (r < 0)
		return r;

	return 0;
}

int main(int argc, char * argv[])
{
	(void) argc;
	(void) argv;

	int r;
	const char * app_name = "ffts_test";

	r = log_init(NULL, app_name);
	if (r < 0)
		return r;

	r = run_tests();

	log_exit();

	return r;
}
