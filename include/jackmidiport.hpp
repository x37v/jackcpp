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

#ifndef JACKMIDIPORT_HPP
#define JACKMIDIPORT_HPP

#include "jackaudioio.hpp"
#include <string>
#include <jack/jack.h>
#include <jack/types.h>
#include <jack/midiport.h>

namespace JackCpp {
   class MIDIPort {
      public:
         MIDIPort();
         std::string name() const;
         jack_port_t * jack_port() const;
         virtual void init(AudioIO * audio_client, std::string name) = 0;
         void * port_buffer(jack_nframes_t frames);

         //if you and this with a byte and you get anything non-zero
         //it is a status message
         static const uint8_t status_mask;

         //returns zero if it isn't a status byte
         //otherwise, returns the byte with everything but status masked off
         static uint8_t status(const jack_midi_event_t& midi_event);

         //if you and this with a status message that contains channel info,
         //you'll get the channel
         static const uint8_t channel_mask;
         static uint8_t channel(const jack_midi_event_t& midi_event);


         //status types
         enum status_t {
            SYSEX_BEGIN = 0xF0,
            SYSEX_END = 0xF7,

            CC = 0xB0,
            NOTEON = 0x90,
            NOTEOFF = 0x80,
            AFTERTOUCH = 0xA0,
            PITCHBEND = 0xE0,
            PROGCHANGE = 0xC0,
            CHANPRESSURE = 0xD0,

            //midi = realtime,
            CLOCK = 0xF8,
            TICK = 0xF9,
            START = 0xFA,
            CONTINUE = 0xFB,
            STOP = 0xFC,
            ACTIVESENSE = 0xFE,
            RESET = 0xFF,

            TC_QUARTERFRAME = 0xF1,
            SONGPOSITION = 0xF2,
            SONGSELECT = 0xF3,
            TUNEREQUEST = 0xF6,
         };

         enum rpn_ccs {
            rpn_param_num_msb = 101,
            rpn_param_num_lsb = 100,
            rpn_param_value_1st = 6,
            rpn_param_value_2nd = 38, //optional
         };

         enum nrpn_ccs {
            nrpn_param_num_msb = 99,
            nrpn_param_num_lsb = 98,
            nrpn_param_value_msb = 6,
            nrpn_param_value_lsb = 38, //optional
         };

      protected:
         enum port_t {INPUT, OUTPUT};
         void init(AudioIO * audio_client, std::string name, port_t type);
      private:
         port_t mPortType;
         jack_port_t * mPort;
   };

   class MIDIInPort : public MIDIPort {
      public:
         virtual void init(AudioIO * audio_client, std::string name);
         jack_nframes_t event_count(void * port_buffer);
         bool get(jack_midi_event_t& event, void * port_buffer, uint32_t index);
   };

   class MIDIOutPort : public MIDIPort {
      public:
         virtual void init(AudioIO * audio_client, std::string name);
         //must be called before each use in the audioCallback
         void clear(void * port_buffer);
         size_t write_space(void * port_buffer);
   };
}

#endif
