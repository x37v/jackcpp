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

#include "jackaudioio.hpp"
#include <iostream>
#include <errno.h>
#include <sstream>
#include <algorithm>
#include <unistd.h>

template <typename T>
std::string ToString(T aValue){
   std::stringstream ss;
   ss << aValue;
   return ss.str();
}

/* callback for jack's error messages */
static void error_callback (const char *msg) {
	std::cerr << "Jack:" << msg << std::endl;
	std::cerr.flush();
}

static void shutdown_callback (void *arg) {
	return ((JackCpp::AudioIO *)arg)->jackShutdownCallback();
}

void JackCpp::AudioIO::jackShutdownCallback(){
	std::cerr << std::endl << "jack has shutdown" << std::endl;
}

int JackCpp::AudioIO::jackProcessCallback(jack_nframes_t nframes, void *arg){
	JackCpp::AudioIO* callbackjackobject = (AudioIO * )arg;
	return callbackjackobject->jackToClassAudioCallback(nframes);
}

int JackCpp::AudioIO::jackToClassAudioCallback(jack_nframes_t nframes){
	//read in commands
	while(mCmdBuffer.getReadSpace() > 0){
		cmd_t cmd;
		mCmdBuffer.read(cmd);
		switch(cmd){
			case add_in_port:
				//we will have tested that we have this capacity, so we resize the buffer
				//to include the new port
				mJackInBuf.resize(mJackInBuf.size() + 1);
				mNumInputPorts++;
				break;
			case add_out_port:
				//we will have tested that we have this capacity, so we resize the buffer
				//to include the new port
				mJackOutBuf.resize(mJackOutBuf.size() + 1);
				mNumOutputPorts++;
				break;
		}
	}

	//get the input and output buffers
	for(unsigned int i = 0; i < mNumInputPorts; i++)
		mJackInBuf[i] = (jack_default_audio_sample_t *) jack_port_get_buffer ( mInputPorts[i], nframes);
	for(unsigned int i = 0; i < mNumOutputPorts; i++)
		mJackOutBuf[i] = (jack_default_audio_sample_t *) jack_port_get_buffer ( mOutputPorts[i], nframes);

	return audioCallback(nframes, mJackInBuf, mJackOutBuf);
}

jack_client_t * JackCpp::AudioIO::client(){
	return mJackClient;
}

JackCpp::AudioIO::AudioIO(std::string name, unsigned int inPorts, unsigned int outPorts, bool startServer) 
	throw(std::runtime_error) : mCmdBuffer(256,true)
{
  createClient(name, inPorts, outPorts, startServer);
}

JackCpp::AudioIO::AudioIO() : mCmdBuffer(256,true), mJackClient(NULL)
{
}

void JackCpp::AudioIO::createClient(std::string name, unsigned int inPorts, unsigned int outPorts, bool startServer) 
	throw(std::runtime_error) 
{
  if (mJackClient) {
    //XXX close it
  }
  
	jack_options_t jack_open_options = JackNullOption;

	if (startServer == false)
		jack_open_options = JackNoStartServer;

	mJackState = notActive;

	//set the error callback
	jack_set_error_function (error_callback);

	/* try to become a client of the JACK server */
	if ((mJackClient = jack_client_open (name.c_str(), jack_open_options, NULL)) == 0) {
		throw std::runtime_error("cannot create client jack server not running?");
	} 
#ifdef __APPLE__
	else {
		// because the mac version of jack is being totally LAME
		sleep(2);
	}
#endif 

	//set the shutdown callback
	jack_on_shutdown (mJackClient, shutdown_callback, this);

	//allocate ports
	if (inPorts > 0){
		for(unsigned int i = 0; i < inPorts; i++){
			std::string portname = "input";
			portname.append(ToString(i));
			mInputPorts.push_back(
					jack_port_register (mJackClient, portname.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
			mPortNames.push_back(portname);
		}
		//reserve the data for the jack callback buffers
		for(unsigned int i = 0; i < mInputPorts.size(); i++)
			mJackInBuf.push_back(NULL);
	} 
	if (outPorts > 0){
		for(unsigned int i = 0; i < outPorts; i++){
			std::string portname = "output";
			portname.append(ToString(i));
			mOutputPorts.push_back(
					jack_port_register (mJackClient, portname.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
			mPortNames.push_back(portname);
		}
		//reserve the data for the jack callback buffers
		for(unsigned int i = 0; i < mOutputPorts.size(); i++)
			mJackOutBuf.push_back(NULL);
	} 

	//set up the callback
	if(0 != jack_set_process_callback (mJackClient, JackCpp::AudioIO::jackProcessCallback, this))
		throw std::runtime_error("cannot register process callback");
}

JackCpp::AudioIO::~AudioIO(){
	//make sure to deactiveate the client if we need to
	switch(mJackState){
		case active:
			stop();
			close();
			break;
		case notActive:
			close();
			break;
		default:
			break;
			//do nothing
	}
}

bool JackCpp::AudioIO::portExists(std::string name){
	//see if the port name exists
	std::vector<std::string>::iterator it;
	it = std::find(mPortNames.begin(),mPortNames.end(), name);
	if (it != mPortNames.end())
		return true;
	else
		return false;
}

void JackCpp::AudioIO::reserveOutPorts(unsigned int num)
	throw(std::runtime_error)
{
	if(getState() == active)
		throw std::runtime_error("reserving ports while the client is running is not supported yet.");
	mOutputPorts.reserve(num);
	mJackOutBuf.reserve(num);
}

void JackCpp::AudioIO::reserveInPorts(unsigned int num)
	throw(std::runtime_error)
{
	if(getState() == active)
		throw std::runtime_error("reserving ports while the client is running is not supported yet.");
	mInputPorts.reserve(num);
	mJackInBuf.reserve(num);
}

unsigned int JackCpp::AudioIO::inPorts(){
	return mInputPorts.size();
}

unsigned int JackCpp::AudioIO::outPorts(){
	return mOutputPorts.size();
}

unsigned int JackCpp::AudioIO::addInPort(std::string name)
	throw(std::runtime_error)
{
	if (mJackState == active && mInputPorts.size() == mInputPorts.capacity())
		throw std::runtime_error("trying to add input ports while the client is running and there are not reserved ports");

	if(portExists(name)){
		std::string ret_string("cannot register new inport: ");
		ret_string.append(name);
		ret_string.append(" port already exists with that name");
		throw std::runtime_error(ret_string);
	}

	//allocate the item in the vector
	jack_port_t * newPort = jack_port_register (mJackClient, name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	if(newPort == NULL){
		std::string ret_string("cannot register new inport: ");
		ret_string.append(name);
		throw std::runtime_error(ret_string);
	}
	mInputPorts.push_back(newPort);
	mPortNames.push_back(name);

	//if we're active then send a command indicating this change
	if (mJackState == active) {
		//loop while there isn't space to write
		while(mCmdBuffer.getWriteSpace() == 0);
		mCmdBuffer.write(add_in_port);
	} else 
		mJackInBuf.push_back(NULL);

	return mInputPorts.size() - 1;
}

//add an output port, if we are active then deactivate and reactivate after
//maybe we can do this more intelligently in the future?
unsigned int JackCpp::AudioIO::addOutPort(std::string name)
	throw(std::runtime_error)
{
	if (mJackState == active && mOutputPorts.size() == mOutputPorts.capacity())
		throw std::runtime_error("trying to add output ports while the client is running and there are not reserved ports");

	if(portExists(name)){
		std::string ret_string("cannot register new outport: ");
		ret_string.append(name);
		ret_string.append(" port already exists with that name");
		throw std::runtime_error(ret_string);
	}

	//allocate the item in the vector
	jack_port_t * newPort = jack_port_register (mJackClient, name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	if(newPort == NULL){
		std::string ret_string("cannot register new outport: ");
		ret_string.append(name);
		throw std::runtime_error(ret_string);
	}
	mOutputPorts.push_back(newPort);
	mPortNames.push_back(name);

	//if we're active then send a command indicating this change
	if (mJackState == active) {
		//loop while there isn't space to write
		while(mCmdBuffer.getWriteSpace() == 0);
		mCmdBuffer.write(add_out_port);
	} else
		mJackOutBuf.push_back(NULL);

	return mOutputPorts.size() - 1;
}

void JackCpp::AudioIO::connectTo(unsigned int index, std::string destPortName)
	throw(std::range_error, std::runtime_error)
{
	int connect_ret;
	if (mJackState != active)
		throw std::runtime_error("client must be active before connecting ports");
	if(index < mOutputPorts.size()){
		connect_ret = jack_connect(mJackClient, jack_port_name(mOutputPorts[index]), destPortName.c_str());
		if(connect_ret != 0 && connect_ret != EEXIST){
			std::string ret_string("cannot connect source: ");
			ret_string.append(jack_port_name(mOutputPorts[index]));
			ret_string.append(" to dest: ");
			ret_string.append(destPortName);
			ret_string.append(" does dest exist?");
			throw std::range_error(ret_string);
		}
	} else
		throw std::range_error("outport index out of range");
}

void JackCpp::AudioIO::connectFrom(unsigned int index, std::string sourcePortName)
	throw(std::range_error, std::runtime_error)
{
	int connect_ret;
	if (mJackState != active)
		throw std::runtime_error("client must be active before connecting ports");
	if(index < mInputPorts.size()){
		connect_ret = jack_connect(mJackClient, sourcePortName.c_str(), jack_port_name(mInputPorts[index]));
		if(connect_ret != 0 && connect_ret != EEXIST){
			std::string ret_string("cannot connect source: ");
			ret_string.append(sourcePortName);
			ret_string.append(" to dest: ");
			ret_string.append(jack_port_name(mInputPorts[index]));
			ret_string.append(" does source exist?");
			throw std::range_error(ret_string);
		}
	} else
		throw std::range_error("inport index out of range");
}

//XXX should the "free" free the names that these ports point too as well?
void JackCpp::AudioIO::connectToPhysical(unsigned int index, unsigned physical_index)
	throw(std::range_error, std::runtime_error)
{
	const char **ports;
	if (mJackState != active)
		throw std::runtime_error("client must be active before connecting ports");
	if (index > mOutputPorts.size())
		throw std::range_error("outport index out of range");
	ports = jack_get_ports (mJackClient, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
	if(ports == NULL){
		throw std::range_error("no physical inports to connect to");
	}
	//make sure the port exists
	for(unsigned int i = 0; i <= physical_index; i++){
		if(ports[i] == NULL){
			free(ports);
			throw std::range_error("physical inport index out of range");
		}
	}
	connectTo(index, ports[physical_index]);
	free(ports);
}

//XXX should the "free" free the names that these ports point too as well?
void JackCpp::AudioIO::connectFromPhysical(unsigned int index, unsigned physical_index)
	throw(std::range_error, std::runtime_error)
{
	const char **ports;
	if (mJackState != active)
		throw std::runtime_error("client must be active before connecting ports");
	if (index > mInputPorts.size())
		throw std::range_error("inport index out of range");
	ports = jack_get_ports (mJackClient, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
	if(ports == NULL){
		throw std::range_error("no physical outports to connect to");
	}
	//make sure the port exists
	for(unsigned int i = 0; i <= physical_index; i++){
		if(ports[i] == NULL){
			free(ports);
			throw std::range_error("physical outport index out of range");
		}
	}
	connectFrom(index, ports[physical_index]);
	free(ports);
}

void JackCpp::AudioIO::disconnectInPort(unsigned int index)
	throw(std::range_error, std::runtime_error)
{
	if (mJackState != active)
		throw std::runtime_error("client must be active before disconnecting ports");
	if(index < mInputPorts.size()){
		jack_port_disconnect(mJackClient, mInputPorts[index]);
	} else 
		throw std::range_error("inport index out of range");
}

void JackCpp::AudioIO::disconnectOutPort(unsigned int index)
	throw(std::range_error, std::runtime_error)
{
	if (mJackState != active)
		throw std::runtime_error("client must be active before disconnecting ports");
	if(index < mOutputPorts.size()){
		jack_port_disconnect(mJackClient, mOutputPorts[index]);
	} else 
		throw std::range_error("outport index out of range");
}

unsigned int JackCpp::AudioIO::numConnectionsInPort(unsigned int index)
	throw(std::range_error)
{
	if(index < mInputPorts.size())
		return jack_port_connected(mInputPorts[index]);
	else 
		throw std::range_error("inport index out of range");
}

unsigned int JackCpp::AudioIO::numConnectionsOutPort(unsigned int index)
	throw(std::range_error)
{
	if(index < mOutputPorts.size())
		return jack_port_connected(mOutputPorts[index]);
	else 
		throw std::range_error("outport index out of range");
}

unsigned int JackCpp::AudioIO::numPhysicalDestinationPorts(){
	const char **ports;
	unsigned int cnt = 0;
	ports = jack_get_ports (mJackClient, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
	if (ports != NULL){
		while(ports[cnt] != NULL)
			cnt++;
		free(ports);
		return cnt;
	} else
		return 0;
}

unsigned int JackCpp::AudioIO::numPhysicalSourcePorts(){
	const char **ports;
	unsigned int cnt = 0;
	//XXX is this really correct? we should get the naming right...
	ports = jack_get_ports (mJackClient, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
	if (ports != NULL){
		while(ports[cnt] != NULL)
			cnt++;
		free(ports);
		return cnt;
	} else
		return 0;
}

std::string JackCpp::AudioIO::getInputPortName(unsigned int index)
	throw(std::range_error)
{
	if(index < mInputPorts.size())
		return std::string(jack_port_name(mInputPorts[index]));
	else 
		throw std::range_error("inport index out of range");

}
std::string JackCpp::AudioIO::getOutputPortName(unsigned int index)
	throw(std::range_error)
{
	if(index < mOutputPorts.size())
		return std::string(jack_port_name(mOutputPorts[index]));
	else 
		throw std::range_error("outport index out of range");
}

void JackCpp::AudioIO::start()
	throw(std::runtime_error)
{
	//update these so that the callback can use them
	if(mJackState != active){
		mNumOutputPorts = mOutputPorts.size();
		mNumInputPorts = mInputPorts.size();
	}
	if (jack_activate(mJackClient) != 0)
		throw std::runtime_error("cannot activate the client");
	mJackState = active;
}

void JackCpp::AudioIO::stop()
	throw(std::runtime_error)
{
	if (jack_deactivate(mJackClient) != 0)
		throw std::runtime_error("cannot deactivate the client");
	mJackState = notActive;
}

void JackCpp::AudioIO::close()
	throw(std::runtime_error)
{
	if (jack_client_close(mJackClient) != 0)
		throw std::runtime_error("cannot close the client");
	mJackState = closed;
}

float JackCpp::AudioIO::getCpuLoad(){
	return jack_cpu_load(mJackClient);
}

jack_nframes_t JackCpp::AudioIO::getSampleRate(){
	return jack_get_sample_rate(mJackClient);
}

jack_nframes_t JackCpp::AudioIO::getBufferSize(){
	return jack_get_buffer_size(mJackClient);
}

