//an example JACKC++ program
//Copyright 2012 Alex Norman
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
#include "jackmidiport.hpp"

#include <iostream>
#include <stdlib.h>
using std::cout;
using std::endl;

#define MIN(x,y) ((x) < (y) ? (x) : (y))

class TestJackMIDI : public JackCpp::AudioIO {
   public:
      // Your audio callback. All audio processing goes in this function.
      virtual int audioCallback(jack_nframes_t nframes, 
            audioBufVector inBufs,
            audioBufVector outBufs){

         //clear the output port
         mMidiOutput.clear();

         //return 0 on success
         return 0;
      }
      TestJackMIDI() :
         JackCpp::AudioIO("jackcpp-miditest", 0,0) {
            mMidiOutput.init(this, "testoutput");
            mMidiInput.init(this, "testinput");
         }
   private:
      JackCpp::MIDIOutPort mMidiOutput;
      JackCpp::MIDIInPort mMidiInput;
};

int main(){
   TestJackMIDI * t = new TestJackMIDI;  // initial ports from constructor created here.
	t->start();	// activate the client

   sleep(50);  // sleep to allow the callback to run
   t->close();   // stop client.

   delete t;     // always clean up after yourself.
   exit(0);
}
