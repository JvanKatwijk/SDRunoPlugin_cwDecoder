#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDRuno cw decoder
 *
 *    cw decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    cw decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with cw decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef	__UTILITY_FUNCTIONS__
#define	__UTILITY_FUNCTIONS__

#include	<stdint.h>
#include	<vector>

float	decayingAverage (float, float, float);
float	clamp (float, float, float);
class	cwAverage {
public:
			cwAverage	(int16_t);
			~cwAverage	();
	float		filter		(float);
	void		clear		(float);
private:
        int16_t		size;
        int16_t		filp;
	std::vector<float> vec;
};

#endif


