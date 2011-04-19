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

#ifndef JACK_RING_BUFFER_CLASS_H
#define JACK_RING_BUFFER_CLASS_H

#ifndef NULL
#define NULL 0
#endif

extern "C" {
#include <jack/ringbuffer.h>
}
#include <string.h>

namespace JackCpp {

template<typename Type>

/** 
@class RingBuffer

@brief This template class wraps the Jack lock-free ringbuffer.

This class creates a lock-free ring buffer that accepts a specific data type.

The key attribute of a ringbuffer is that it can be safely accessed by two
threads simultaneously -- one reading from the buffer and the other writing to
it -- without using any synchronization or mutual exclusion primitives. For
this to work correctly, there can only be a single reader and a single writer
thread. Their identities cannot be interchanged.

@author Alex Norman

*/
	class RingBuffer {
		private:
			jack_ringbuffer_t *mRingBufferPtr;
			size_t mLength;
		public:
			/**
			  @brief The Constructor
			  \param size the number of items that the ring buffer should be able to hold
			  \param mlock a boolean indicating whether or not the ring buffer should be locked in memory
			  */
			RingBuffer(size_t size, bool mlock = false){
				mLength = size;
				mRingBufferPtr = jack_ringbuffer_create(mLength * sizeof(Type));

				//should we lock the memory for the ring buffer?
				if(mlock)
					jack_ringbuffer_mlock(mRingBufferPtr);
			}
			///The Destructor
			~RingBuffer(){
				if(mRingBufferPtr != NULL)
					jack_ringbuffer_free(mRingBufferPtr);
			}

			///Get the total length of the ring buffer
			size_t length(){
				return mLength;
			}

			///Get the number of items that can be read at this time
			size_t getReadSpace(){ 
				return jack_ringbuffer_read_space(mRingBufferPtr) / sizeof(Type);
			}

			///Get the number of items that can be written at this time
			size_t getWriteSpace(){
				return jack_ringbuffer_write_space(mRingBufferPtr) / sizeof(Type);
			}
			
			/**
			  @brief Read into dest

			  Read from the buffer into a variable.

			  \param dest an item to be read into
			  */
			void read(Type &dest){
				if(getReadSpace() <= 0){
					//throw error!!!!
					return;
				}
				jack_ringbuffer_read(mRingBufferPtr, (char *)&dest, sizeof(Type));
			}

			/**
			  @brief Read into an array

			  Read from the buffer into an array.

			  \param dest an array to be read into
			  \param cnt the number of elements to read into this array
			  */
			void read(Type *dest, unsigned cnt){
				jack_ringbuffer_data_t readVec[2];
				unsigned int read_size = sizeof(Type) * cnt;
				if(getReadSpace() <= 0){
					//throw error!!!!
					return;
				}

				//get the readvector
				jack_ringbuffer_get_read_vector(mRingBufferPtr, readVec);

				//if the first vector has enough data then just read from there
				if(readVec[0].len >= read_size){
					memcpy(dest, readVec[0].buf, read_size);
				} else {
					//if the first vector is zero length then read from the second
					if(readVec[0].len == 0){
						memcpy(dest, readVec[1].buf, read_size);
					} else {
						//this gets tricky
						char * byterep = (char *)dest;
						//first read the data out of the first vector
						memcpy(byterep, readVec[0].buf, readVec[0].len);
						//then read the rest out of the second
						memcpy(byterep + readVec[0].len, readVec[1].buf, read_size - readVec[0].len);
					}
				}
				//advance the read pointer
				jack_ringbuffer_read_advance(mRingBufferPtr, read_size);
			}
			
			/**
			  @brief Write into the ring buffer.

			  \param src the value to write
			  */
			void write(Type src){
				if(getWriteSpace() <= 0){
					//throw error!!!!
					return;
				}
				jack_ringbuffer_write(mRingBufferPtr, (char *)&src, sizeof(Type));
			}

			/**
			  @brief Write an array of values into the ring buffer.

			  \param src an array of values to write
			  \param cnt the number of items from the array to write into our buffer
			  */
			void write(Type *src, unsigned int cnt){
				jack_ringbuffer_data_t writeVec[2];
				unsigned int write_size = sizeof(Type) * cnt;
				if(cnt > getWriteSpace()){
					//throw error!!!!
					return;
				}

				//get the write vector
				jack_ringbuffer_get_write_vector(mRingBufferPtr, writeVec);
				//if there is enough room in the first vector then just write there
				if(writeVec[0].len >= write_size){
					memcpy(writeVec[0].buf,src,write_size);
				} else {
					//if there is no room in the first vector then write into the second
					if(writeVec[0].len == 0){
						memcpy(writeVec[1].buf,src,write_size);
					} else {
						//this is more tricky, we have to split the data up
						char * byterep = (char *)src;
						//copy the first chunck
						memcpy(writeVec[0].buf, byterep, writeVec[0].len);
						//copy the second chunck
						memcpy(writeVec[1].buf, byterep + writeVec[0].len, write_size - writeVec[0].len);
					}
				}
				jack_ringbuffer_write_advance(mRingBufferPtr, write_size);
			}

			/**
			  @brief Reset

			  This is not threadsafe.  This resets the read and write pointers,
			  effectively making the ring buffer empty.
			  */
			void reset(){
				jack_ringbuffer_reset(mRingBufferPtr);
			}
	};

}

#endif
