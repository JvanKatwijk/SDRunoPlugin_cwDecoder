#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the cs plugin
 *
 *    cw plugin is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    cw plugin is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with cw plugin; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"cw-bandfilter.h"

#define  _USE_MATH_DEFINES
#include <math.h>

	bandpassFilter::bandpassFilter (int16_t filterSize,
	                                int32_t low, int32_t high,
	                                int32_t sampleRate):
	                                  filterKernel (filterSize),
		filterBase (filterSize),
	                                  Buffer (filterSize) {
float	lo	= (float)((high - low) / 2) / sampleRate;
float	shift	= (float) ((high + low) / 2) / sampleRate;
float	sum	= 0.0;

	this	-> sampleRate	= sampleRate;
	this	-> ip		= 0;
	this	-> filterSize	= filterSize;

	for (int i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      filterBase [i] =  (float)(2 * M_PI * lo);
	   else 
	      filterBase [i] = (float)sin (2 * M_PI * lo * (i - filterSize /2)) / (i - filterSize/2);
//
//	windowing, according to Blackman
	   filterBase [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += filterBase [i];
	}

	for (int i = 0; i < filterSize; i ++)
	   filterBase [i] /= sum;
//
//	and the modulation:
	for (int i = 0; i < filterSize; i ++) {	// shifting
	   float v = (i - filterSize / 2) * (2 * M_PI * shift);
	   filterKernel [i] = std::complex<float> (filterBase [i] * cos (v), 
	                                           filterBase [i] * sin (v));
	}

	for (int i = 0; i < filterSize; i ++)
	   Buffer [i] = std::complex<float> (0, 0);

}

	bandpassFilter::~bandpassFilter () {
}

//
//      we process the samples backwards rather than reversing
//      the kernel
std::complex<float>	bandpassFilter::Pass (std::complex<float> z) {
std::complex<float>	tmp     = 0;

        Buffer [ip]     = z;
        for (int i = 0; i < filterSize; i ++) {
           int16_t index = ip - i;
           if (index < 0)
              index += filterSize;
           tmp          += Buffer [index] * filterKernel [i];
        }

        ip = (ip + 1) % filterSize;
        return tmp;
}

void	bandpassFilter::update (int shift, int width) {
	(void)width;
        float rshift = (float)shift / sampleRate;
        for (int i = 0; i < filterSize; i ++) { // shifting
           float v = (float) (i - filterSize / 2) * (2 * M_PI * rshift);
           filterKernel [i] = std::complex<float> (filterBase [i] * cos (v),
                                                   filterBase [i] * sin (v));
        }
}

