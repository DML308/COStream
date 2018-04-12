#include <sched.h>
#include <sys/types.h>
#include <iostream>
#include <linux/unistd.h>
#include <sys/syscall.h>
using namespace std;
#define gettid() syscall(__NR_gettid)   
int set_cpu(int i)
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(i,&mask);
	if(-1 == sched_setaffinity(gettid(),sizeof(&mask),&mask)){
		cout<<"error\t"<<i<<endl;
		return -1;
	}
	return 0;
}

