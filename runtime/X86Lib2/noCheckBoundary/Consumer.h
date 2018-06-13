#ifndef _CONSUMER_H
#define _CONSUMER_H
#include "Buffer.h"
#include <memory.h>
template<typename T>
class Consumer{
public:
	Consumer(Buffer<T>& conBuffer):conBuffer(conBuffer){
		head = 0;
	}
	T& operator[](int index){
		return conBuffer.buffer[head+index];
	}
	void updatehead(int offset){
		head += offset;
	}
	void resetHead()
	{
		if(head >= conBuffer.bufferSize-conBuffer.copySize)
		{
			memcpy(conBuffer.buffer+conBuffer.copyStartPos,conBuffer.buffer+(conBuffer.bufferSize-conBuffer.copySize),sizeof(T)*conBuffer.copySize);
			head = conBuffer.copyStartPos;
		}
	}
private:
	Buffer<T>& conBuffer;
	int head;
};
#endif
