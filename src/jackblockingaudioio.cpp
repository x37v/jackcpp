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

#include "jackblockingaudioio.hpp"
#include <unistd.h>
#define MIN(x,y) ((x) < (y) ? (x) : (y))

#if 0
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
#endif

//allocate input and output buffers
JackCpp::BlockingAudioIO::BlockingAudioIO(std::string name,
		unsigned int inChans, unsigned int outChans,
		unsigned int inBufSize, unsigned int outBufSize,
		bool startServer) throw(std::runtime_error):
	AudioIO(name, inChans, outChans, startServer),
	mOutputBufferMaxSize((unsigned int)getSampleRate()),
	mInputBufferMaxSize((unsigned int)getSampleRate())
{
	if(inBufSize < 2 * getBufferSize())
		inBufSize = 2 * getBufferSize();
	else if (inBufSize > mInputBufferMaxSize)
		inBufSize = mInputBufferMaxSize;
	if(outBufSize < 2 * getBufferSize())
		outBufSize = 2 * getBufferSize();
	else if (outBufSize > mOutputBufferMaxSize)
		outBufSize = mOutputBufferMaxSize;

	//set the amount of the ring buffer that we leave free
	mOutputBufferFreeSize = mOutputBufferMaxSize - outBufSize;
	mInputBufferFreeSize = mInputBufferMaxSize - inBufSize;

	//create input and output buffers, give them extra space to work with and memory lock them
	for(unsigned int i = 0; i < outChans; i++)
		mUserOutBuff.push_back(new RingBuffer<jack_default_audio_sample_t>(mOutputBufferMaxSize, true));
	for(unsigned int i = 0; i < inChans; i++)
		mUserInBuff.push_back(new RingBuffer<jack_default_audio_sample_t>(mInputBufferMaxSize, true));
}

//clean up the buffers we allocated
JackCpp::BlockingAudioIO::~BlockingAudioIO(){
	stop();
	for(std::vector<RingBuffer<jack_default_audio_sample_t> *>::iterator it = mUserOutBuff.begin();
			it != mUserOutBuff.end(); it++)
		delete *it;
	for(std::vector<RingBuffer<jack_default_audio_sample_t> *>::iterator it = mUserInBuff.begin();
			it != mUserInBuff.end(); it++)
		delete *it;
}

//wait until we can write, then write
void JackCpp::BlockingAudioIO::write(unsigned int channel, jack_default_audio_sample_t val){
	if (channel >= outPorts())
		return;
	while(mUserOutBuff[channel]->getWriteSpace() <= mOutputBufferFreeSize)
		usleep(10);
	mUserOutBuff[channel]->write(val);
}

//we we can write then write, otherwise return false
bool JackCpp::BlockingAudioIO::tryWrite(unsigned int channel, jack_default_audio_sample_t val){
	if (channel < outPorts() && mUserOutBuff[channel]->getWriteSpace() > mOutputBufferFreeSize){
		mUserOutBuff[channel]->write(val);
		return true;
	}
	return false;
}

//wait until we can read, then return the value
jack_default_audio_sample_t JackCpp::BlockingAudioIO::read(unsigned int channel){
	jack_default_audio_sample_t val;
	if (channel >= inPorts())
		return 0;
	while(mUserInBuff[channel]->getReadSpace() == 0)
		usleep(10);
	mUserInBuff[channel]->read(val);
	return val;
}

//if we cannot read then return false, otherwise, read and return true
bool JackCpp::BlockingAudioIO::tryRead(unsigned int channel, jack_default_audio_sample_t &val){
	if (channel >= inPorts() || mUserInBuff[channel]->getReadSpace() == 0)
		return false;
	mUserInBuff[channel]->read(val);
	return true;
}

void JackCpp::BlockingAudioIO::reserveOutPorts(unsigned int num)
	throw(std::runtime_error)
{
	AudioIO::reserveOutPorts(num);
	mUserOutBuff.reserve(num);
}

void JackCpp::BlockingAudioIO::reserveInPorts(unsigned int num)
	throw(std::runtime_error)
{
	AudioIO::reserveInPorts(num);
	mUserInBuff.reserve(num);
}

unsigned int JackCpp::BlockingAudioIO::addInPort(std::string name)
	throw(std::runtime_error)
{
	unsigned int ret;
	if(getState() == AudioIO::active)
		throw std::runtime_error("JackCpp::BlockingAudioIO::addInPort not allowed while the client is active");
	ret = AudioIO::addInPort(name);
	mUserInBuff.push_back(new RingBuffer<jack_default_audio_sample_t>(mInputBufferMaxSize, true));
	return ret;
}

unsigned int JackCpp::BlockingAudioIO::addOutPort(std::string name)
	throw(std::runtime_error)
{
	unsigned int ret;
	if(getState() == AudioIO::active)
		throw std::runtime_error("JackCpp::BlockingAudioIO::addOutPort not allowed while the client is active");
	ret = AudioIO::addOutPort(name);
	mUserOutBuff.push_back(new RingBuffer<jack_default_audio_sample_t>(mOutputBufferMaxSize, true));
	return ret;
}

//read the jack input buffers into the user input buffers
//write the user output buffers into the jack output buffers
int JackCpp::BlockingAudioIO::audioCallback(jack_nframes_t nframes, 
		audioBufVector inBufs,
		audioBufVector outBufs){

	//only try to write as much as we have space to write
	unsigned int numToWrite = MIN(mUserOutBuff[0]->getReadSpace(), nframes);
	unsigned int numToRead = MIN(mUserInBuff[0]->getWriteSpace(), nframes);

	//make sure we leave the amount of free space we require
	if(mUserInBuff[0]->getWriteSpace() - numToRead < mInputBufferFreeSize)
		numToRead = mUserInBuff[0]->getWriteSpace() - mInputBufferFreeSize;

	//if (numToWrite < nframes)
		//cerr << "oops" << endl;

	//read get inputs
	for(unsigned int i = 0; i < inPorts(); i++){
		for(unsigned int j = 0; j < numToRead; j++)
			mUserInBuff[i]->write(inBufs[i][j]);
	}

	//write output
	for(unsigned int i = 0; i < outPorts(); i++){
		for(unsigned int j = 0; j < numToWrite; j++){
			jack_default_audio_sample_t val;
			mUserOutBuff[i]->read(val);
			outBufs[i][j] = val;
		}
		//write zeros for the rest
		for(unsigned int j = numToWrite; j < nframes; j++)
			outBufs[i][j] = 0.0;
		//if(numToWrite < nframes)
			//cerr << "oops" << endl;
	}
	return 0;
}

