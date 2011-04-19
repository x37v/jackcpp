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

#ifndef JACK_BLOCKING_AUDIO_IO_HPP
#define JACK_BLOCKING_AUDIO_IO_HPP

#include "jackaudioio.hpp"
#include "jackringbuffer.hpp"

namespace JackCpp {

/** 
@class BlockingAudioIO

@brief This is an class that provides a blocking read/write interface for Jack.

This class has read/write methods that allow users to write audio to and read
audio from a Jack client.

@author Alex Norman

*/
	class BlockingAudioIO : public AudioIO {
		public:
			/**
			  @brief The Constructor
			  \param name string indicating the name of the jack client to create
			  \param inChans an unsigned integer indicating the number of default input ports to create
			  \param outChans an unsigned integer indicating the number of default output ports to create
			  \param inBufSize the size of the buffer that the jack callback fills for us to read with read
			  \param outBufSize the size of the buffer that we write to and the jack callback reads from
			  \param startServer a boolean indicating whether to start a jack server if one isn't already running
			  \sa AudioIO::AudioIO
			  */
			BlockingAudioIO(std::string name, 
					unsigned int inChans = 2, unsigned int outChans = 2,
					unsigned int inBufSize = 0, unsigned int outBufSize = 0,
#ifdef __APPLE__
					bool startServer = false)
#else
					bool startServer = true)
#endif
					throw(std::runtime_error);
			virtual ~BlockingAudioIO();

			/**
			   @brief Write to an output buffer

				Writes val to the output[channel] (if it exists).  If
				output[channel] does not exist is silently fails otherwise it
				sleeps until it can write output[channel]

			  \param channel the output chanel to write to
			  \param val the value to write to the channel
			  \sa tryWrite(unsigned int channel, jack_default_audio_sample_t val)
			*/
			void write(unsigned int channel, jack_default_audio_sample_t val);

			/**
			   @brief Try to write to an output buffer

				Trys to write to output[channel] (if it exists).  If it succeeds it
				returns true.  If it fails, because the buffer is full, it returns
				false.

			  \param channel the output chanel to write to
			  \param val the value to write to the channel
			  \return true if it can write, false if it cannot
			  \sa write(unsigned int channel, jack_default_audio_sample_t val)
			*/
			bool tryWrite(unsigned int channel, jack_default_audio_sample_t val);

			/**
			   @brief Read from an input buffer.

				Reads from input[channel] if it exists.  If there is no input to 
				read it sleeps until there is.

			  \param channel the input chanel to read from
			  \return the value read from the input channel [will be zero if the channel does not exist]
			  \sa tryRead(unsigned int channel, jack_default_audio_sample_t &val)
			*/
			jack_default_audio_sample_t read(unsigned int channel);

			/**
			   @brief Try to read from to an input buffer

				Trys to read from input[channel] (if it exists).  If it succeeds it
				returns true.  If it fails, because the buffer is empty, it returns
				false.

			  \param channel the output chanel to read from
			  \param val the value to read into (basically a return value)
			  \return true if it can read, false if it cannot
			  \sa read(unsigned int channel)
			*/
			bool tryRead(unsigned int channel, jack_default_audio_sample_t &val);

			//XXX reserve exists but is basically useless as you cannot
			//add ports while the client is active
			///This method is useless at the moment.
			virtual void reserveOutPorts(unsigned int num)
				throw(std::runtime_error);
			///This method is useless at the moment.
			virtual void reserveInPorts(unsigned int num)
				throw(std::runtime_error);

			/**
			   @brief Add an input port to our client

				Unlike AudioIO, this currently cannot be called while the client is running.

			  \param name string the name of the port to add
			  \return the number of total input ports
			  \sa AudioIO::addInPort(std::string name)
			*/
			virtual unsigned int addInPort(std::string name)
				throw(std::runtime_error);
			/**
			   @brief Add an output port to our client

				Unlike AudioIO, this currently cannot be called while the client is running.

			  \param name string the name of the port to add
			  \return the number of total output ports
			  \sa AudioIO::addOutPort(std::string name)
			*/
			virtual unsigned int addOutPort(std::string name)
				throw(std::runtime_error);

		protected:
			/**
			   @brief This is the callback that processes our buffers.

				This method takes the buffers we write to with "write" and writes
				them out to the Jack bus.  It also takes audio from the Jack bus
				and uses that to fill the input buffers that we read from.

			  \param nframes the number frames to process
			  \param inBufs a vector of audio buffers
			  \param outBufs a vector of audio buffers
			  \return the actual number of frames processed
			*/
			virtual int audioCallback(jack_nframes_t nframes, 
					std::vector<jack_default_audio_sample_t *> inBufs,
					std::vector<jack_default_audio_sample_t *> outBufs);
		private:
			std::vector<RingBuffer<jack_default_audio_sample_t> *> mUserOutBuff;
			std::vector<RingBuffer<jack_default_audio_sample_t> *> mUserInBuff;

			//this is the size of the ring buffers that we alloc
			const unsigned int mOutputBufferMaxSize;
			const unsigned int mInputBufferMaxSize;
			//this is the amount of free space we leave in the ring buffers
			//this can decrease so that we'll have more latency but fewer glitches
			unsigned int mOutputBufferFreeSize;
			unsigned int mInputBufferFreeSize;
	};
}
#endif

