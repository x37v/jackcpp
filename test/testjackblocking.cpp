//An example JACKC++ program
//Copyright 2007 Alex Norman
//
//This file is part of JACKC++.
//
//JACKC++ is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//JACKC++ is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with JACKC++.  If not, see <http://www.gnu.org/licenses/>.

#include <math.h>
#include <iostream>
using std::cout;
using std::endl;
#include "jackblockingaudioio.hpp"

int main(){
	unsigned int chans = 2;

	JackCpp::BlockingAudioIO b("testblocking", chans, chans);
	b.start();
	for(unsigned int i = 0; i < chans; i++){
		b.connectToPhysical(i,i);
		b.connectFromPhysical(i,i);
	}

	unsigned int x = 0;
	float mul = M_PI * 2.0 / b.getSampleRate();
	while(true){
		float val = sinf(x * 440.0 * mul) * fabs(sinf(x * 0.5 * mul)) * 0.9;
		for(unsigned int i = 0; i < chans; i++)
			b.write(i, val);
		x += 1;
	}

	while(true){
		for(unsigned int i = 0; i < chans; i++)
			b.write(i, b.read(i));
	}
}
