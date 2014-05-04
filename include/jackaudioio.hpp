//C++ Classes that wrap JACK
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

#ifndef JACK_AUDIO_IO_H
#define JACK_AUDIO_IO_H

extern "C" {
#include <jack/jack.h>
#include <jack/types.h>
#include <jack/ringbuffer.h>
}
#include <string>
#include <vector>
#include <stdexcept>
#include "jackringbuffer.hpp"

namespace JackCpp {

/** 
@class AudioIO

@brief This is an abstract class that provides a jack callback method.

This abstract class wraps the Jack Audio IO functionality needed for basic
audio input and output.  In order to create your own jack client you simply
make your own class that inherits from this class and overloads audioCallback.
In that method you can get audio in from jack and write it out to jack.

@author Alex Norman

*/

	class AudioIO {
		public:
			///An enum indicating the state of our jack client
			enum jack_state_t {notActive,active,closed};
			///A typedef so so that we don't always have to write std::vector<jack_default_audio_sample_t *>
			typedef std::vector<jack_default_audio_sample_t *> audioBufVector;
		private:
			//commands
			enum cmd_t {add_in_port, add_out_port};
			RingBuffer<cmd_t> mCmdBuffer;
			/* the client */
			jack_client_t *mJackClient;
			// an vector of i/o ports
			std::vector<jack_port_t *> mOutputPorts;
			std::vector<jack_port_t *> mInputPorts;

			//these are only accessed by the callback [once it is activated]
			//they will usually be equal mOutputPorts.size() etc, except when
			//a new port is added, before the callback
			unsigned int mNumOutputPorts;
			unsigned int mNumInputPorts;

			//these items are used for grabbing data for the jack callback
			audioBufVector mJackInBuf;
			audioBufVector mJackOutBuf;
			//this stores the state of this jack process [active,notActive,closed]
			jack_state_t mJackState;
			//this prepares the input/output buffers to be passed 
			//to the callback function that a user writes
			//XXX should this be virtual?
			inline int jackToClassAudioCallback(jack_nframes_t nframes);
			std::vector<std::string> mPortNames;
		protected:
			/**
			  @brief The method that the user must overload in order to actually process jack data.
			  \param nframes the number frames to process
			  \param inBufs a vector of audio buffers
			  \param outBufs a vector of audio buffers
			  \return 0 on success, non zero on error, which will cause jack to remove the client from the process graph
			  */
			virtual int audioCallback(jack_nframes_t nframes, 
					audioBufVector inBufs,
					audioBufVector outBufs) = 0;
		public:
			/**
			  @brief Gives users a pointer to the client created and used by this class.
			  \return a pointer to the jack client used by this class.
			  */
			jack_client_t * client();

			/**
			  @brief The Constructor
			  \param name string indicating the name of the jack client to create
			  \param inPorts an unsigned integer indicating the number of default input ports to create
			  \param outPorts an unsigned integer indicating the number of default output ports to create
			  \param startServer a boolean indicating whether to start a jack server if one isn't already running
			  \sa audioCallback
			  */
			AudioIO(std::string name, 
					unsigned int inPorts = 0, 
					unsigned int outPorts = 2, 
#ifdef __APPLE__
					bool startServer = false)
#else
					bool startServer = true)
#endif
				throw(std::runtime_error);

      //create the object but don't actually create the client yet
			AudioIO();

			void createClient(std::string name, 
					unsigned int inPorts = 0, 
					unsigned int outPorts = 2, 
#ifdef __APPLE__
					bool startServer = false)
#else
					bool startServer = true)
#endif
				throw(std::runtime_error);

			///The Destructor
			virtual ~AudioIO();

			/**
			   @brief The callback that jack actually gets [static].

				This is a static method that the C jack callback calls, users
				should not need to override this method.

			  \param nframes the number frames to process
			  \param arg a pointer to our AudioIO object
			  \return the actual number of frames processed
			  \sa audioCallback
			*/
			static int jackProcessCallback(jack_nframes_t nframes, void *arg);

			///See if a port with the name "name" exists for our client
			bool portExists(std::string name);

			/**
			   @brief Reserve output ports

				This method must be called before the client is started.  This
				reserves a number of ports so that they can be safely created while
				the client is running.  The number indicates the maximum number of ports
				that can be created while the client is running.  This number includes
				those ports that a have already been created, so if you've already created
				x ports and you reserve y ports total, you can only create y - x ports while
				the client is running.
			  \param num an integer indicating the number of output ports to reserve
			*/
			virtual void reserveOutPorts(unsigned int num)
				throw(std::runtime_error);
			/**
			   @brief Reserve input ports

				This method must be called before the client is started.  This
				reserves a number of ports so that they can be safely created while
				the client is running.  The number indicates the maximum number of ports
				that can be created while the client is running.  This number includes
				those ports that a have already been created, so if you've already created
				x ports and you reserve y ports total, you can only create y - x ports while
				the client is running.
			  \param num an integer indicating the number of input ports to reserve
			*/
			virtual void reserveInPorts(unsigned int num)
				throw(std::runtime_error);

			///Start the jack client.
			void start()
				throw(std::runtime_error);
			///Stop the jack client.
			void stop()
				throw(std::runtime_error);
			///Close the jack client.
			void close()
				throw(std::runtime_error);

			///Get the number of jack input ports
			unsigned int inPorts();
			///Get the number of jack output ports
			unsigned int outPorts();

			/**
			   @brief Add a jack input port to our client
			  \param name string the name of the port to add
			  \return the number of total input ports
			*/
			virtual unsigned int addInPort(std::string name)
				throw(std::runtime_error);
			/**
			   @brief Add a jack output port to our client
			  \param name string the name of the port to add
			  \return the number of total output ports
			*/
			virtual unsigned int addOutPort(std::string name)
				throw(std::runtime_error);

			/**
			   @brief Connect our output to a jack client's source port.
			  \param index the index of our output port to connect from.
			  \param sourcePortName the client:port name to connect to
			*/
			void connectTo(unsigned int index, std::string sourcePortName) 
				throw(std::range_error, std::runtime_error);
			/**
			   @brief Connect our input to a jack client's destination port.
			  \param index the index of our input port to connect to
			  \param destPortName the client:port name to connect from
			*/
			void connectFrom(unsigned int index, std::string destPortName)
				throw(std::range_error, std::runtime_error);
			/**
			   @brief Connect our output port to a physical output port
			  \param index the index of our output port to connect from
			  \param physical_index the physical output port index to connect to
			*/
			void connectToPhysical(unsigned int index, unsigned physical_index)
				throw(std::range_error, std::runtime_error);
			/**
			   @brief Connect our input port to a physical input port
			  \param index the index of our input port to connect to
			  \param physical_index the physical input port index to connect from
			*/
			void connectFromPhysical(unsigned int index, unsigned physical_index)
				throw(std::range_error, std::runtime_error);
			///Disconnect input port from all connections
			void disconnectInPort(unsigned int index)
				throw(std::range_error, std::runtime_error);
			///Disconnect output port from all connections
			void disconnectOutPort(unsigned int index)
				throw(std::range_error, std::runtime_error);

			///Get the number of connections to our input port
			unsigned int numConnectionsInPort(unsigned int index)
				throw(std::range_error);
			///Get the number of connections to our output port
			unsigned int numConnectionsOutPort(unsigned int index)
				throw(std::range_error);

			/**
			   @brief Get the number of physical audio input ports
				These are ports that can send audio to your client
			*/
			unsigned int numPhysicalSourcePorts();
			/**
			   @brief Get the number of physical audio output ports
				These are ports that your client can send audio to
			*/
			unsigned int numPhysicalDestinationPorts();

			///Get the name of our client's input port
			std::string getInputPortName(unsigned int index)
				throw(std::range_error);
			///Get the name of our client's output port
			std::string getOutputPortName(unsigned int index)
				throw(std::range_error);

			/**
			 	@brief This method is called when Jack shuts down.
				Override if you want to do something when jack shuts down.
			*/
			virtual void jackShutdownCallback();
			/**
			 	@brief The current CPU load estimated by JACK
				
				This is a running average of the time it takes to execute a full
				process cycle for all clients as a percentage of the real time
				available per cycle determined by the buffer size and sample rate.
			*/
			float getCpuLoad();
			///Get the sample rate
			jack_nframes_t getSampleRate();
			///Get the jack buffer size
			jack_nframes_t getBufferSize();
			///Check to see if the client is running in real time mode
			bool isRealTime(){return jack_is_realtime(mJackClient);}
			/**
			 	@brief Get the name of our client

				This might not be exactly the same as the name we provided to the
				constructor

			  \return a string indicating the name of our client.
			*/
			std::string getName(){return std::string(jack_get_client_name(mJackClient));}
			///Get the state of our Jack client.
			jack_state_t getState(){return mJackState;}

			/**
			 	@brief Get an estimate of the current time in frames

				This is a running counter, no significance should be attached to
				its value, but it can be compared to a previously returned value.
				\return an estimate of the current time in frames.
			*/
			jack_nframes_t getFrameTime(){return jack_frame_time(mJackClient);}

			/**
			 	@brief Get the time in frames since the JACK server began the current process cycle

				\return the time in frames that has passed since the JACK server began the current process cycle
			*/
			jack_nframes_t getFramesSinceCycleStart(){return jack_frames_since_cycle_start(mJackClient);}
	};

}

#endif
