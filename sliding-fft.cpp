#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the cwPlugin
 *
 *    cwPplugin is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    cwPlugin is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with cwPlugin; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#define  _USE_MATH_DEFINES
#include	<stdio.h>
#include <math.h>
#include	"sliding-fft.h"
/*
 *	we slide with a goertzel derived algorithm, while
 *	the whole spectrum takes len bins, we are only
 *	interested in the first .. last bins.
 *	Very elegant algorithm, obviously not invented by me
 */
	slidingFFT::slidingFFT	(int32_t len, int32_t first, int32_t last) {
int32_t	i;

	fftBase. resize (len);
	Past.    resize (len);
	Bins.    resize (len);

	for (i = 0; i < len; i ++) 
	   Past [i] = Bins [i] = std::complex<float> (0, 0);
	FFTlen	= len;
	First	= first;
	Amount	= last - first + 1;
	Last	= last;
	Ptr	= 0;

	for (i = 0; i < len; i ++) {
	   fftBase [i] =
	           std::complex<float> (cos (2.0 * i * M_PI / len),
				        sin (2.0 * i * M_PI / len)
	                               ); 
	}

	Corrector = pow (STABILIZER, len);
}

	slidingFFT::~slidingFFT () {
}

void	slidingFFT::reset	(){
	for (int i = 0; i < FFTlen; i ++) {
	   Bins [i] = std::complex<float> (0, 0);
	   Past [i] = std::complex<float> (0, 0);
	}
}

int32_t	slidingFFT::first	() {
	return First;
}

void	slidingFFT::do_FFT (std::complex<float> nnew,
	                        std::complex<float> *out) {
std::complex<float>	old;
std::complex<float>	z;
int32_t	i;

	old = std::complex<float> (real (Past [Ptr]) * Corrector,
	                           imag (Past [Ptr]) * Corrector);
/*
 *	save the new sample
 */
	Past [Ptr] = nnew;
	Ptr = (Ptr + 1) % FFTlen;
/*
 *	calculate the wanted bins
 */
	for (i = 0; i < FFTlen; i ++) {
	   z = Bins [i] - old + nnew;
	   Bins [i] = z * fftBase [i];
	}

	for (i = 0; i < Amount; i ++) 
	   out [i] = Bins [First + i];
}

std::complex<float>	slidingFFT::getBin (int32_t ind) {
	return Bins [ind];
}

