#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <string>

#include "X10LibCopy.h"

using namespace std;

X10LibCopy::X10LibCopy()
{
	libDir_ = ".\\X10Lib2\\";
	producer_file_name_ = "Producer.x10";
	consumer_file_name_ = "Consumer.x10";
	edge_file_name_ = "Edge.x10";
	content_ = NULL;
}


void X10LibCopy::Run(const char *dir)
{
	char *tmp = new char[1000];

	string str1 = libDir_;
	str1 += producer_file_name_;
	content_ = TextFileRead(str1.c_str());
	sprintf(tmp,"%slib\\%s",dir, producer_file_name_);
	TextFileWrite(tmp, content_);
	
	string str2 = libDir_;
	str2 += consumer_file_name_;
	content_ = TextFileRead(str2.c_str());
	sprintf(tmp,"%slib\\%s",dir,consumer_file_name_);
	TextFileWrite(tmp, content_);

	string str3 = libDir_;
	str3 += edge_file_name_;
	content_ = TextFileRead(str3.c_str());
	sprintf(tmp,"%slib\\%s",dir,edge_file_name_);
	TextFileWrite(tmp, content_);

	delete tmp;
	tmp = NULL;
}

int X10LibCopy::TextFileWrite(char *file_name, char *content)
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

char* X10LibCopy::TextFileRead(const char *file_name)
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