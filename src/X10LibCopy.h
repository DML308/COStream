#ifndef _X10_LIB_COPY_H
#define _X10_LIB_COPY_H

using namespace std;

class X10LibCopy
{
private:

	char *libDir_;
	char *producer_file_name_;
	char *consumer_file_name_;
	char *edge_file_name_;
	char *content_;


public:
	int TextFileWrite(char *file_name, char *content);
	char *TextFileRead(const char *file_name);

	X10LibCopy();
	~X10LibCopy(){};
	void Run(const char *dir);
};
#endif