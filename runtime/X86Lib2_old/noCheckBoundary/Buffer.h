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
	//��������~Buffer();
	~Buffer()
	{
		delete	[]buffer;
	}
	T* buffer;
};
//��ʱ����ͷ�ļ��У�ģ����ı���������
template<typename T>
Buffer<T>::Buffer(int size,int copySize,int copyStartPos){//constructor    ���仺������ַ
	bufferSize = size;
	this->copySize = copySize;
	this->copyStartPos = copyStartPos;
	buffer = (T*)malloc(bufferSize*sizeof(T));
}
template<typename T>
T& Buffer<T>::operator[](const size_t index){
	return buffer[index];					//�Ƿ���Ҫ��֤�±�����������ݲ��ᳬ��tail?
}

#endif
