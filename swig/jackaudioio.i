%{
#include "jackaudioio.hpp"
using namespace JackCpp;
%}

typedef unsigned int jack_nframes_t;
class AudioIO {
   public:
      enum jack_state_t {notActive,active,closed};
   public:
      AudioIO(std::string name, 
            unsigned int inPorts = 0, 
            unsigned int outPorts = 2, 
#ifdef __APPLE__
            bool start_server = false)
#else
            bool start_server = true)
#endif
         throw(std::runtime_error);
      virtual ~AudioIO() = 0;

      bool portExists(std::string name);

      virtual void reserveOutPorts(unsigned int num)
         throw(std::runtime_error);
      virtual void reserveInPorts(unsigned int num)
         throw(std::runtime_error);

      void start()
         throw(std::runtime_error);
      void stop()
         throw(std::runtime_error);
      void close()
         throw(std::runtime_error);

      virtual unsigned int inPorts();
      virtual unsigned int outPorts();

      unsigned int addInPort(std::string name)
         throw(std::runtime_error);
      unsigned int addOutPort(std::string name)
         throw(std::runtime_error);

      void connectTo(unsigned int index, std::string sourcePortName) 
         throw(std::range_error, std::runtime_error);
      void connectFrom(unsigned int index, std::string destPortName)
         throw(std::range_error, std::runtime_error);
      //connect our outport to a physical destination
      void connectToPhysical(unsigned int index, unsigned physical_index)
         throw(std::range_error, std::runtime_error);
      //connect our inport to a physical source
      void connectFromPhysical(unsigned int index, unsigned physical_index)
         throw(std::range_error, std::runtime_error);
      void disconnectInPort(unsigned int index)
         throw(std::range_error, std::runtime_error);
      void disconnectOutPort(unsigned int index)
         throw(std::range_error, std::runtime_error);

      unsigned int numConnectionsInPort(unsigned int index)
         throw(std::range_error);
      unsigned int numConnectionsOutPort(unsigned int index)
         throw(std::range_error);

      //sources are ports that send audio to your application
      unsigned int numPhysicalSourcePorts();
      //destinations are ports that you send audio to
      unsigned int numPhysicalDestinationPorts();

      std::string getInputPortName(unsigned int index)
         throw(std::range_error);
      std::string getOutputPortName(unsigned int index)
         throw(std::range_error);

      virtual void jackShutdownCallback();
      float getCpuLoad();
      jack_nframes_t getSampleRate();
      jack_nframes_t getBufferSize();
      bool isRealTime();
      std::string getName();
      jack_state_t getState();
};
