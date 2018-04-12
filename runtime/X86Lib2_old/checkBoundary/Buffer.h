//class Buffer definition
#include <stdlib.h>
#ifndef _BUFFER_H
#define _BUFFER_H
template<typename T>
class Buffer{
public:
	Buffer(int size);
	int bufferSize;
	//T peek(int index);			//peak seems useless
	T& operator[](const size_t); 
	//��������~Buffer();
	~Buffer()
	{
		delete	[]buffer;
	}
private:
	T* buffer;
};
template<typename T>
Buffer<T>::Buffer(int size){//constructor    ���仺������ַ
	bufferSize = size;
	buffer = (T*)malloc(bufferSize*sizeof(T));
}
template<typename T>
T& Buffer<T>::operator[](const size_t index){
	return buffer[index];					
}
#endif
