#ifndef _GPU_LIB_COPY_H
#define _GPU_LIB_COPY_H

using namespace std;

class GPULibCopy
{
private:

	char *libDir_;
	char *producer_file_name_;
	char *consumer_file_name_;
	char *buffer_file_name_;
	char *lock_free_h_;
	char *lock_free_c_;
	char *Math_name_;
	char *makefile_name_;
	char *content_;
	char *time_file_name_;
	char *setCpu_name_;


public:
	int TextFileWrite(char *file_name, char *content);
	char *TextFileRead(const char *file_name);

	GPULibCopy();
	~GPULibCopy(){};
	void Run(const char *dir);
};
#endif