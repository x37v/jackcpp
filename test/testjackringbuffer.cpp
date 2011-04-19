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

#include "jackringbuffer.hpp"

#include <iostream>
using std::cout;
using std::endl;

class X {
	public:
		int t;
		int x;
		float y;
};

int main(){
	JackCpp::RingBuffer<int> f(20);
	int r;
	int t[] = {3,4,5};

	cout << "the length of the ring buffer is: " << f.length() << endl;
	cout << "the free space to write is: " << f.getWriteSpace() << endl;

	f.write(t,3);
	f.write(10);
	f.write(1);
	f.write(1);
	f.write(4);
	f.write(5);
	f.write(6123);
	f.write(1234);
	f.write(112345);
	f.write(16);
	while(f.getReadSpace() != 0){
		f.read(r);
		cout << r << endl;
	}


	JackCpp::RingBuffer<X> jX(10);
	X x;
	x.t = 12;
	x.x = 1;
	x.y = 1234.023f;
	jX.write(x);
	X y;
	jX.read(y);
	cout << y.t << " " << y.x << " " << y.y << endl;
}
