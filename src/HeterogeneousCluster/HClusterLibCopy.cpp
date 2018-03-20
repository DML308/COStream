#include "HClusterLibCopy.h"

/*构造函数*/
HClusterLibCopy::HClusterLibCopy()
{
	this->libDir_ = ".\\HClusterLib\\";
	producer_file_name_ = "Producer.h";
	consumer_file_name_ = "Consumer.h";
	buffer_file_name_ = "Buffer.h";
	lock_free_h_ = "lock_free_barrier.h";
	lock_free_c_ = "lock_free_barrier.cpp";
	Math_name_ = "MathExtension.h";

	//makefile_name_ = "Makefile";

	time_file_name_ = "rdtsc.h";		//获取时钟周期函数
	setCpu_name_ = "setcpu.h";
	setCpu_cpp_name_ = "setcpu.cpp";
	clusterConsumer_name_ = "NetConsumer.h";
	clusterProducer_name_ = "NetProducer.h";
	basic_lib_h = "costreamlib.h";
	basic_lib_cpp = "costreamlib.cpp";


	tinystrh = "tinystr.h";
	tinyxmlh = "tinyxml.h";
	tinystrcpp = "tinystr.cpp";
	tinyxmlcpp = "tinyxml.cpp";
	tinyxmlerrorcpp = "tinyxmlerror.cpp";
	tinyxmlparsercpp = "tinyxmlparser.cpp";
	p_stable_lsh_h = "p_stable_lsh.h";
	p_stable_lsh_cpp = "p_stable_lsh.cpp";

	content_ = NULL;
}


/*文件拷贝入口函数*/
void HClusterLibCopy::run(const char * dir)
{
	char *tmp = new char[1000];

	string str = libDir_;

	str += producer_file_name_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, producer_file_name_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += consumer_file_name_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, consumer_file_name_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += buffer_file_name_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, buffer_file_name_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += lock_free_h_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, lock_free_h_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += lock_free_c_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, lock_free_c_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += Math_name_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, Math_name_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += time_file_name_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, time_file_name_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += setCpu_name_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, setCpu_name_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += setCpu_cpp_name_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, setCpu_cpp_name_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += clusterConsumer_name_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, clusterConsumer_name_);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += clusterProducer_name_;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, clusterProducer_name_);
	TextFileWrite(tmp, content_);


	str = libDir_;
	str += basic_lib_h;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, basic_lib_h);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += basic_lib_cpp;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, basic_lib_cpp);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += tinystrh;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, tinystrh);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += tinyxmlh;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, tinyxmlh);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += tinystrcpp;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, tinystrcpp);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += tinyxmlcpp;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, tinyxmlcpp);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += tinyxmlerrorcpp;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, tinyxmlerrorcpp);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += tinyxmlparsercpp;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, tinyxmlparsercpp);
	TextFileWrite(tmp, content_);

	/*str = libDir_;
	str += p_stable_lsh_h;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, p_stable_lsh_h);
	TextFileWrite(tmp, content_);

	str = libDir_;
	str += p_stable_lsh_cpp;
	content_ = (char *)TextFileRead(str.c_str());
	sprintf(tmp, "%s%s", dir, p_stable_lsh_cpp);
	TextFileWrite(tmp, content_);*/

	//删除临时变量
	delete tmp;
	tmp = NULL;
}


int HClusterLibCopy::TextFileWrite(char * file_name, char * content)
{
	assert(file_name && content);

	int status = 0;
	static int count = 0;
	int len = strlen(content);

	FILE *fp = fopen(file_name, "wt+");
	if (fp != NULL)
	{
		if (fwrite(content, sizeof(char), len, fp) == len)
			status = 1;
		fclose(fp);
	}
	else
	{
		printf("error:cannot write file :%s\n", file_name);
		system("pause");
		exit(1);
	}

	//printf("FileNumber = %d, status = %d\n",++count, status);

	delete content;
	content = NULL;
	return(status);
}

void * HClusterLibCopy::TextFileRead(const char * file_name)
{
	FILE *fp = NULL;
	char *content = NULL;
	int count = 0;

	assert(file_name);

	fp = fopen(file_name, "rt");

	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		count = ftell(fp);
		rewind(fp);

		if (count > 0)
		{
			content = new char[count + 1];
			count = fread(content, sizeof(char), count, fp);
			content[count] = '\0';
		}
		fclose(fp);
	}
	else
	{
		printf("error:cannot open file :%s\n", file_name);
		system("pause");
		exit(1);
	}

	return content;
}

