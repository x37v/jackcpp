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

using namespace JackCpp;

MIDIPort::MIDIPort() : mPort(NULL) {
}

jack_port_t * MIDIPort::jack_port() const { return mPort; }

void MIDIPort::init(AudioIO * audio_client, std::string name, port_t type) {
   mPortType = type;
   mPort = jack_port_register(audio_client->client(),
         name.c_str(),
         JACK_DEFAULT_MIDI_TYPE,
         (mPortType == INPUT ? JackPortIsInput : JackPortIsOutput),
         0);
}

std::string MIDIPort::name() const {
   return std::string(jack_port_name(mPort));
}

//******** MIDIInPort

void MIDIInPort::init(AudioIO * audio_client, std::string name) {
   MIDIPort::init(audio_client, name, MIDIPort::INPUT);
}

jack_nframes_t MIDIInPort::event_count() {
   return jack_midi_get_event_count(jack_port());
}

jack_midi_event_t * MIDIInPort::get(uint32_t index) {
   jack_midi_event_t * event = NULL;
   if (jack_midi_event_get(event, jack_port(), index) == 0)
      return event;
   return NULL;
}


//******** MIDIOutPort

void MIDIOutPort::init(AudioIO * audio_client, std::string name) {
   MIDIPort::init(audio_client, name, MIDIPort::OUTPUT);
}

void MIDIOutPort::clear() {
   jack_midi_clear_buffer(jack_port());
}

size_t MIDIOutPort::write_space() {
   return jack_midi_max_event_size(jack_port());
}
