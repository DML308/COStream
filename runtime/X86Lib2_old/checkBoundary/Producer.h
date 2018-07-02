#ifndef _PRODUCER_H_
#define _PRODUCER_H_
#include "Buffer.h"
template<typename T>
class Producer{
public:
	Producer(Buffer<T>& proBuffer):proBuffer(proBuffer){
		tail = 0;
	}
	T& operator[](int index){
		if(tail+index<proBuffer.bufferSize)
			return proBuffer[tail+index];
		return proBuffer[tail+index-proBuffer.bufferSize];
		
	}
	void updatetail(int offset){
		tail = ((tail+offset)>=proBuffer.bufferSize)?(tail+offset-proBuffer.bufferSize):(tail+offset);
	}
private:
	Buffer<T>& proBuffer;
	int tail;
};
#endif
