//class Buffer definition
#include <stdlib.h>
#ifndef _BUFFER_H
#define _BUFFER_H
template<typename T>
class Buffer{
public:
	Buffer(int size,int copySize,int copyStartPos);
	int bufferSize;
	int copySize;
	int copyStartPos;
	//T peek(int index);			//peak seems useless
	T& operator[](const size_t);
	//析构函数~Buffer();
	~Buffer()
	{
		delete	[]buffer;
	}
	T* buffer;
};
//暂时放在头文件中，模板类的编译有问题
template<typename T>
Buffer<T>::Buffer(int size,int copySize,int copyStartPos){//constructor    分配缓冲区地址
	bufferSize = size;
	this->copySize = copySize;
	this->copyStartPos = copyStartPos;
	buffer = (T*)malloc(bufferSize*sizeof(T));
}
template<typename T>
T& Buffer<T>::operator[](const size_t index){
	return buffer[index];					//是否需要保证下标操作读的数据不会超出tail?
}

#endif
