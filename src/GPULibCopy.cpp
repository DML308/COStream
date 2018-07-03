#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <string>

#include "GPULibCopy.h"

using namespace std;

GPULibCopy::GPULibCopy()
{
	libDir_ = ".\\GPULib2\\";
	producer_file_name_ = "Producer.h";
	consumer_file_name_ = "Consumer.h";
	buffer_file_name_ = "Buffer.h";
	lock_free_h_="barrier_sync.h";
	lock_free_c_="barrier_sync.cpp";
	Math_name_="MathExtension.h";
	makefile_name_ = "Makefile";
	time_file_name_ = "rdtsc.h";		//获取时钟周期函数
	setCpu_name_ = "setCpu.h";
	content_ = NULL;
}


void GPULibCopy::Run(const char *dir)
{
	char *tmp = new char[1000];

	string str1 = libDir_;
	str1 += producer_file_name_;
	content_ = TextFileRead(str1.c_str());
	sprintf(tmp,"%s%s",dir, producer_file_name_);
	TextFileWrite(tmp, content_);

	string str2 = libDir_;
	str2 += consumer_file_name_;
	content_ = TextFileRead(str2.c_str());
	sprintf(tmp,"%s%s",dir,consumer_file_name_);
	TextFileWrite(tmp, content_);

	string str3 = libDir_;
	str3 += buffer_file_name_;
	content_ = TextFileRead(str3.c_str());
	sprintf(tmp,"%s%s",dir,buffer_file_name_);
	TextFileWrite(tmp, content_);

	string str4 = libDir_;
	str4 += lock_free_h_;
	content_ = TextFileRead(str4.c_str());
	sprintf(tmp,"%s%s",dir,lock_free_h_);
	TextFileWrite(tmp, content_);

	string str5 = libDir_;
	str5 += lock_free_c_;
	content_ = TextFileRead(str5.c_str());
	sprintf(tmp,"%s%s",dir,lock_free_c_);
	TextFileWrite(tmp, content_);

	string str6 = libDir_;
	str6 += Math_name_;
	content_ =  TextFileRead(str6.c_str());
	sprintf(tmp,"%s%s",dir,Math_name_);
	TextFileWrite(tmp, content_);

	string str7 = libDir_;
	str7 += time_file_name_;
	content_ = TextFileRead(str7.c_str());
	sprintf(tmp,"%s%s",dir, time_file_name_);
	TextFileWrite(tmp, content_);

	string str9 = libDir_;
	str9 += setCpu_name_;
	content_ = TextFileRead(str9.c_str());
	sprintf(tmp,"%s%s",dir, setCpu_name_);
	TextFileWrite(tmp, content_);

	//以下只为临时只用，以后需该为调用CGMAKEFILE
	string str8 = libDir_;
	str8 += makefile_name_;
	content_ = TextFileRead(str8.c_str());
	sprintf(tmp,"%s%s",dir, makefile_name_);
	TextFileWrite(tmp, content_);




	delete tmp;
	tmp = NULL;
}

int GPULibCopy::TextFileWrite(char *file_name, char *content)
{
	assert( file_name && content);

	int status = 0;
	static int count=0;
	int len = strlen(content);

	FILE *fp = fopen(file_name,"wt+");
	if (fp != NULL)
	{
		if (fwrite(content,sizeof(char),len,fp) == len)
			status = 1;
		fclose(fp);
	}
	else
	{
		printf("error:cannot write file :%s\n",file_name);
		system("pause");
		exit(1);
	}

	//printf("FileNumber = %d, status = %d\n",++count, status);

	delete content;
	content = NULL;
	return(status);
}

char* GPULibCopy::TextFileRead(const char *file_name)
{
	FILE *fp = NULL;
	char *content = NULL;
	int count = 0;

	assert(file_name);

	fp = fopen(file_name,"rt");

	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		count = ftell(fp);
		rewind(fp);

		if (count > 0)
		{
			content = new char[count+1];
			count = fread(content,sizeof(char),count,fp);
			content[count] = '\0';
		}
		fclose(fp);
	}
	else
	{
		printf("error:cannot open file :%s\n",file_name);
		system("pause");
		exit(1);
	}

	return content;
}