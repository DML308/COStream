#ifndef _X86_LIB_COPY_H
#define _X86_LIB_COPY_H
#include "schedulerSSG.h"

using namespace std;

class X86LibCopy
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
	char *lsh_header_;
	char *lsh_source_;
	char *tinystrh;
	char *tinyxmlh;
	char *tinystrcpp;
	char *tinyxmlcpp;
	char *tinyxmlerrorcpp;
	char *tinyxmlparsercpp;

public:
	int TextFileWrite(char *file_name, char *content);
	char *TextFileRead(const char *file_name);

	X86LibCopy();
	~X86LibCopy(){};
#ifndef WIN32
	void copyfile(const char *dir);
#endif
	void Run(const char *dir);
};
extern "C"{
	extern Bool Win;
	extern Bool Linux;
	extern Bool TRACE;
	extern Bool CHECKBARRIERTIME;
	extern Bool CHECKEACHACTORTIME;
	extern Bool MakeProfile;
 	extern GLOBAL const char *output_file;
 	extern Bool CallModelEmbed; 
 	extern GLOBAL const char *infileName;
 	extern GLOBAL const char *outfileName ;
 	extern GLOBAL Bool NoCheckBuffer;
 	extern GLOBAL Bool AmplifySchedule;
 	extern GLOBAL int AmplifyFactor;
 	extern GLOBAL Bool CALRATIO;
	extern GLOBAL Bool PrintResult;
 };
#endif
