%{
#include "jackblockingaudioio.hpp"
using namespace JackCpp;
%}

typedef float jack_default_audio_sample_t;

class BlockingAudioIO : public AudioIO {
   public:
   BlockingAudioIO(std::string name,
      unsigned int inChans = 2, unsigned int outChans = 2,
      unsigned int inBufSize = 0, unsigned int outBufSize = 0,
#ifdef __APPLE__
            bool startServer = false)
#else
            bool startServer = true)
#endif
            throw(std::runtime_error);
   
   void write(unsigned int channel, jack_default_audio_sample_t val);
   bool tryWrite(unsigned int channel, jack_default_audio_sample_t val);
   jack_default_audio_sample_t read(unsigned int channel);
   bool tryRead(unsigned int channel, jack_default_audio_sample_t &val);
};

