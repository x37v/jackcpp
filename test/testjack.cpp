//an example JACKC++ program
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

#include "jackaudioio.hpp"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using std::cout;
using std::endl;
#define MIN(x,y) ((x) < (y) ? (x) : (y))

class TestJack: public JackCpp::AudioIO {
	public:
		// Your audio callback. All audio processing goes in this function.
		virtual int audioCallback(jack_nframes_t nframes, 
				// A vector of pointers to each input port.
				audioBufVector inBufs,
				// A vector of pointers to each output port.
				audioBufVector outBufs){
			for(unsigned int i = 0; i < MIN(inBufs.size(), outBufs.size()); i++){
				for(unsigned int j = 0; j < nframes; j++)
					outBufs[i][j] = inBufs[i][j];	// A simple example: copy the input to the output.
			}
			//return 0 on success
			return 0;
		}
		TestJack() :
			JackCpp::AudioIO("jackcpp-test", 2,2){
				//we have 16 total input and output ports that we could have
				reserveInPorts(16);
				reserveOutPorts(16);
			/* constructor:
				Names JACK client "jackcpp-test"
				creates 2 in ports named input0 and input1
				creates 2 out ports named output0 and output1
			*/
		}
};

int main(){
	TestJack * t = new TestJack;	// initial ports from constructor created here.
	t->addOutPort("blahout0");		// add new out port (2) named "blahout0"
	t->addOutPort("blahout1");		// add new out port (3) named "blahout1"
	t->addInPort("blahin0");		// add new in port (2) named "blahin0"
	t->addInPort("blahin1");		// add new in port (3) named "blahin1"
	t->start();	// activate the client

	// reporting some client info
	cout << endl << "my name: " << t->getName() << endl;

	// test to see if it is real time
	if (t->isRealTime())
		cout << "is realtime " << endl;
	else
		cout << "is not realtime " << endl;

	//count the number of physical source and destination ports
	cout << "num physical source ports " << t->numPhysicalSourcePorts() << endl;
	cout << "num physical destination ports " << t->numPhysicalDestinationPorts() << endl;

	//connect our ports to physical ports
	t->connectToPhysical(0,0);		// connects this client out port 0 to physical destination port 0
	t->connectToPhysical(1,1);		// connects this client out port 1 to physical destination port 1
	t->connectFromPhysical(0,0);	// connects this client in port 0 to physical source port 0
	t->connectFromPhysical(1,1);	// connects this client in port 1 to physical source port 1

	// exception test example
	cout << endl << "testing exceptions, trying to connect to a fake port" << endl;
	try {
		t->connectFrom(1,"fakeportname");
	} catch (std::runtime_error e){
		cout << "YES: " << e.what() << endl;
	}

	cout << "try to query a port we don't have" << endl;
	try {
		t->numConnectionsInPort(100);
	} catch (std::range_error e){
		cout << "YES: " << e.what() << endl;
	}

	// port connection status
	cout << endl;
	cout << "output 0 is connected to " << t->numConnectionsOutPort(0) << " ports" << endl;
	cout << "output 1 is connected to " << t->numConnectionsOutPort(1) << " ports" << endl;
	cout << "input 0 is connected to " << t->numConnectionsInPort(0) << " ports" << endl;
	cout << "input 1 is connected to " << t->numConnectionsInPort(1) << " ports" << endl;

	//print names
	cout << endl;
	cout << "inport names:" << endl;
	for(unsigned int i = 0; i < t->inPorts(); i++)
		cout << "\t" << t->getInputPortName(i) << endl;

	cout << "outport names:" << endl;
	for(unsigned int i = 0; i < t->outPorts(); i++)
		cout << "\t" << t->getOutputPortName(i) << endl;

	cout << "********ADDING PORTS WHILE THE CLIENT IS RUNNING!*******" << endl;
	cout << "adding a new output port!" << endl;
	t->addOutPort("newOut0");
	cout << "adding a new input port!" << endl;
	t->addInPort("newIn0");
	cout << "adding a new input port!" << endl;
	t->addInPort("newIn1");
	cout << "adding a new input port!" << endl;
	t->addInPort("newIn2");
	sleep(3);
	cout << "adding a new output port!" << endl;
	t->addOutPort("newOut1");

	sleep(50);		// sleep to allow the callback to run for 50 seconds.

	t->disconnectInPort(0);	// Disconnecting ports.
	t->disconnectOutPort(1);
	t->close();	// stop client.
	delete t;	// always clean up after yourself.
	exit(0);
}
