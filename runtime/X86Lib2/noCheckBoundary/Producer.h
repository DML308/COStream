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
		return proBuffer[tail+index];
	}
	void updatetail(int offset){
		tail += offset;
	}
	void resetTail(){
		if(tail == proBuffer.bufferSize)
		{
			tail = proBuffer.copySize + proBuffer.copyStartPos;
		}
	}
private:
	Buffer<T>& proBuffer;
	int tail;
};
#endif
