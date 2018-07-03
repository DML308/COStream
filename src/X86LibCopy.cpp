#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <string>

#include "X86LibCopy.h"

using namespace std;
X86LibCopy::X86LibCopy()
{
	libDir_ = ".\\X86Lib2\\";
	producer_file_name_ = "Producer.h";
	consumer_file_name_ = "Consumer.h";
	buffer_file_name_ = "Buffer.h";
	lock_free_h_="lock_free_barrier.h";
	lock_free_c_="lock_free_barrier.cpp";
	Math_name_="MathExtension.h";
	makefile_name_ = "Makefile";
	time_file_name_ = "rdtsc.h";		//获取时钟周期函数
	setCpu_name_ = "setCpu.h";
	lsh_header_ = "p-stable_lsh.h";
	lsh_source_ = "p-stable_lsh.cpp";
	content_ = NULL;

	tinystrh = "tinystr.h";
	tinyxmlh = "tinyxml.h";
	tinystrcpp = "tinystr.cpp";
	tinyxmlcpp = "tinyxml.cpp";
	tinyxmlerrorcpp = "tinyxmlerror.cpp";
	tinyxmlparsercpp = "tinyxmlparser.cpp";
}

#ifndef WIN32
void X86LibCopy::copyfile(const char *dir)
{	
	char *tmp = new char[4096];
	string comLibPath = "";
	string path = getenv("COSTREAM_LIB");

	printf("path = %s\n" , path.c_str());
	printf("comLibPath = %s\n" , comLibPath.c_str());

	if(NoCheckBuffer)	//无边界检查
	{
		comLibPath = "noCheckBoundary/";	
	}
	else
	{
		comLibPath = "CheckBoundary/";	
	}
	printf("comLibPath = %s\n" , comLibPath.c_str());

	string str1 = path;
	str1 += comLibPath;
	str1 += producer_file_name_;
	content_ = TextFileRead(str1.c_str());
	sprintf(tmp,"%s%s",dir,producer_file_name_);
	TextFileWrite(tmp, content_);

	string str2 = path;
	str2 += comLibPath;
	str2 += consumer_file_name_;
	content_ = TextFileRead(str2.c_str());
	sprintf(tmp,"%s%s",dir,consumer_file_name_);
	TextFileWrite(tmp, content_);

	string str3 = path;
	str3 += comLibPath;
	str3 += buffer_file_name_;
	content_ = TextFileRead(str3.c_str());
	sprintf(tmp,"%s%s",dir,buffer_file_name_);
	TextFileWrite(tmp, content_);

	string str4 = path;
	str4 += lock_free_h_;
	content_ = TextFileRead(str4.c_str());
	sprintf(tmp,"%s%s",dir,lock_free_h_);
	TextFileWrite(tmp, content_);

	string str5 = path;
	str5 += lock_free_c_;
	content_ = TextFileRead(str5.c_str());
	sprintf(tmp,"%s%s",dir,lock_free_c_);
	TextFileWrite(tmp, content_);

	string str6 = path;
	str6 += Math_name_;
	content_ =  TextFileRead(str6.c_str());
	sprintf(tmp,"%s%s",dir,Math_name_);
	TextFileWrite(tmp, content_);
	
	string str7 = path;
	str7 += time_file_name_;
	content_ = TextFileRead(str7.c_str());
	sprintf(tmp,"%s%s",dir, time_file_name_);
	TextFileWrite(tmp, content_);

	string str9 = path;
	str9 += setCpu_name_;
	content_ = TextFileRead(str9.c_str());
	sprintf(tmp,"%s%s",dir, setCpu_name_);
	TextFileWrite(tmp, content_);

	string str10 = path;
	str10 += lsh_header_;
	content_ = TextFileRead(str10.c_str());
	sprintf(tmp,"%s%s",dir, lsh_header_);
	TextFileWrite(tmp, content_);

	string str11 = path;
	str11 += lsh_source_;
	content_ = TextFileRead(str11.c_str());
	sprintf(tmp,"%s%s",dir, lsh_source_);
	TextFileWrite(tmp, content_);
	
	string str12 = path;
	str12 += tinystrh;
	content_ = TextFileRead(str12.c_str());
	sprintf(tmp,"%s%s",dir, tinystrh);
	TextFileWrite(tmp, content_);

	string str13 = path;
	str13 += tinyxmlh;
	content_ = TextFileRead(str13.c_str());
	sprintf(tmp,"%s%s",dir, tinyxmlh);
	TextFileWrite(tmp, content_);

	string str14 = path;
	str14 += tinystrcpp;
	content_ = TextFileRead(str14.c_str());
	sprintf(tmp,"%s%s",dir, tinystrcpp);
	TextFileWrite(tmp, content_);

	string str15 = path;
	str15 += tinyxmlcpp;
	content_ = TextFileRead(str15.c_str());
	sprintf(tmp,"%s%s",dir, tinyxmlcpp);
	TextFileWrite(tmp, content_);

	string str16 = path;
	str16 += tinyxmlerrorcpp;
	content_ = TextFileRead(str16.c_str());
	sprintf(tmp,"%s%s",dir, tinyxmlerrorcpp);
	TextFileWrite(tmp, content_);

	string str17 = path;
	str17 += tinyxmlparsercpp;
	content_ = TextFileRead(str17.c_str());
	sprintf(tmp,"%s%s",dir, tinyxmlparsercpp);
	TextFileWrite(tmp, content_);
	
	string str18 = path;
	str18 += makefile_name_;
	content_ = TextFileRead(str18.c_str());
	sprintf(tmp,"%s%s",dir, makefile_name_);
	TextFileWrite(tmp, content_);

	delete tmp;
	tmp = NULL;

}
#endif

void X86LibCopy::Run(const char *dir)
{
	char *tmp = new char[1000];
	string comLibPath;
	if(NoCheckBuffer)	//无边界检查
	{
		comLibPath = "noCheckBoundary\\";	
	}
	else
	{
		comLibPath = "CheckBoundary\\";	
	}
	string str1 = libDir_;
	str1 += comLibPath;
	str1 += producer_file_name_;
	content_ = TextFileRead(str1.c_str());
	sprintf(tmp,"%s%s",dir,producer_file_name_);
	TextFileWrite(tmp, content_);

	string str2 = libDir_;
	str2 += comLibPath;
	str2 += consumer_file_name_;
	content_ = TextFileRead(str2.c_str());
	sprintf(tmp,"%s%s",dir,consumer_file_name_);
	TextFileWrite(tmp, content_);

	string str3 = libDir_;
	str3 += comLibPath;
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
	
	string str10 = libDir_;
	str10 += tinystrh;
	content_ = TextFileRead(str10.c_str());
	sprintf(tmp,"%s%s",dir, tinystrh);
	TextFileWrite(tmp, content_);

	string str11 = libDir_;
	str11 += tinyxmlh;
	content_ = TextFileRead(str11.c_str());
	sprintf(tmp,"%s%s",dir, tinyxmlh);
	TextFileWrite(tmp, content_);

	string str12 = libDir_;
	str12 += tinystrcpp;
	content_ = TextFileRead(str12.c_str());
	sprintf(tmp,"%s%s",dir, tinystrcpp);
	TextFileWrite(tmp, content_);

	string str13 = libDir_;
	str13 += tinyxmlcpp;
	content_ = TextFileRead(str13.c_str());
	sprintf(tmp,"%s%s",dir, tinyxmlcpp);
	TextFileWrite(tmp, content_);

	string str14 = libDir_;
	str14 += tinyxmlerrorcpp;
	content_ = TextFileRead(str14.c_str());
	sprintf(tmp,"%s%s",dir, tinyxmlerrorcpp);
	TextFileWrite(tmp, content_);

	string str15 = libDir_;
	str15 += tinyxmlparsercpp;
	content_ = TextFileRead(str15.c_str());
	sprintf(tmp,"%s%s",dir, tinyxmlparsercpp);
	TextFileWrite(tmp, content_);
	
	if(1){
		string str16 = libDir_;
		str16 += lsh_header_;
		content_ = TextFileRead(str16.c_str());
		sprintf(tmp,"%s%s",dir, lsh_header_);
		TextFileWrite(tmp, content_);

		string str17 = libDir_;
		str17 += lsh_source_;
		content_ = TextFileRead(str17.c_str());
		sprintf(tmp,"%s%s",dir, lsh_source_);
		TextFileWrite(tmp, content_);
	}
	delete tmp;
	tmp = NULL;
}

int X86LibCopy::TextFileWrite(char *file_name, char *content)
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

char* X86LibCopy::TextFileRead(const char *file_name)
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
