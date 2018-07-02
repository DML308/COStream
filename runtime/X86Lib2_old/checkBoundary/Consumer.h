#ifndef _CONSUMER_H
#define _CONSUMER_H
#include "Buffer.h"
template<typename T>
class Consumer{
public:
	Consumer(Buffer<T>& conBuffer):conBuffer(conBuffer){
		head = 0;
	}
	T& operator[](int index){
		if(head+index<conBuffer.bufferSize)
			return conBuffer[head+index];
		return conBuffer[head+index-conBuffer.bufferSize];
	}
	void updatehead(int offset){
		head = head+offset>=conBuffer.bufferSize?head+offset-conBuffer.bufferSize:head+offset;
	}
private:
	Buffer<T>& conBuffer;
	int head;
};
#endif
