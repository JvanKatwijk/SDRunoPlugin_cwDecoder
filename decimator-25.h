#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the cw plugin
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

#ifndef __DECIMATOR_25_H
#define __DECIMATOR_25_H

#include	<stdint.h>
#include	<complex>
#include	<vector>
#include	"decimator.h"

//
class	decimator_25 {
public:
	                 decimator_25	(int32_t, int32_t);
			~decimator_25	();
	bool		Pass	(std::complex<float>, std::complex<float> *);
private:
	int		counter;
	int		amount;
	std::complex<float>	iValue;
};

#endif
