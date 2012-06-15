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
