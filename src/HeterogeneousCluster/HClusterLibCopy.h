/*
 * 异构集群后端文件拷贝
 * 2016/10/24 yrr
 */

#ifndef _HCLUSTER_LIBCOPY_H_
#define _HCLUSTER_LIBCOPY_H_
#include <iostream>
#include "HClusterBackend.h"

using namespace std;

class HClusterLibCopy {
public:
	int TextFileWrite(char *file_name, char *content);
	void *TextFileRead(const char * file_name);

	HClusterLibCopy();
	~HClusterLibCopy(){}

	void run(const char *dir);
protected:

private:
	char *libDir_;
	char *producer_file_name_;
	char *consumer_file_name_;
	char *buffer_file_name_;
	char *lock_free_h_;
	char *lock_free_c_;
	char *Math_name_;
	//char *makefile_name_;
	char *content_;
	char *time_file_name_;
	char *setCpu_name_;
	char *setCpu_cpp_name_;

	char *clusterConsumer_name_;
	char *clusterProducer_name_;
	char *basic_lib_h;
	char *basic_lib_cpp;

	char *tinystrh;
	char *tinyxmlh;
	char *tinystrcpp;
	char *tinyxmlcpp;
	char *tinyxmlerrorcpp;
	char *tinyxmlparsercpp;

	char *p_stable_lsh_h;
	char *p_stable_lsh_cpp;
};



#endif