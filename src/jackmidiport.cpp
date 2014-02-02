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

const uint8_t MIDIPort::status_mask = 0x80;
const uint8_t MIDIPort::channel_mask = 0x0F;

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

void * MIDIPort::port_buffer(jack_nframes_t frames) {
   return jack_port_get_buffer(mPort, frames);
}

uint8_t MIDIPort::status(const jack_midi_event_t& midi_event) {
   if (midi_event.size < 1 || !(midi_event.buffer[0] & status_mask))
      return 0;
   return midi_event.buffer[0] & 0xF0;
}

uint8_t MIDIPort::channel(const jack_midi_event_t& midi_event) {
   if (midi_event.size < 1)
      return 0;
   return midi_event.buffer[0] & channel_mask;
}

//******** MIDIInPort

void MIDIInPort::init(AudioIO * audio_client, std::string name) {
   MIDIPort::init(audio_client, name, MIDIPort::INPUT);
}

jack_nframes_t MIDIInPort::event_count(void * port_buffer) {
   return jack_midi_get_event_count(port_buffer);
}

bool MIDIInPort::get(jack_midi_event_t& event, void * port_buffer, uint32_t index) {
   return jack_midi_event_get(&event, port_buffer, index) == 0;
}


//******** MIDIOutPort

void MIDIOutPort::init(AudioIO * audio_client, std::string name) {
   MIDIPort::init(audio_client, name, MIDIPort::OUTPUT);
}

void MIDIOutPort::clear(void * port_buffer) {
   jack_midi_clear_buffer(port_buffer);
}

size_t MIDIOutPort::write_space(void * port_buffer) {
   return jack_midi_max_event_size(port_buffer);
}
