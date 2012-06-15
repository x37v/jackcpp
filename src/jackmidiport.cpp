//C++ Classes that wrap JACK
//Copyright 2007, 2012 Alex Norman
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

#include "jackmidiport.hpp"

JackCpp::MIDIPort::MIDIPort() : mPort(NULL) {
}

void JackCpp::MIDIPort::init(AudioIO * audio_client, std::string name, port_t type) {
   mPortType = type;
   mPort = jack_port_register(audio_client->client(),
         name.c_str(),
         JACK_DEFAULT_MIDI_TYPE,
         (mPortType == INPUT ? JackPortIsInput : JackPortIsOutput),
         0);
}

std::string JackCpp::MIDIPort::name() const {
   return std::string(jack_port_name(mPort));
}

