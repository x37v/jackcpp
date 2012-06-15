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

extern "C" {
#include <jack/jack.h>
#include <jack/types.h>
}

namespace JackCpp {
   class MIDIPort {
      public:
         enum port_t {INPUT, OUTPUT};
         MIDIPort();
         void init(AudioIO * audio_client, std::string name, port_t type);
         std::string name() const;
      private:
         port_t mPortType;
         jack_port_t * mPort;
   };
}

#endif
