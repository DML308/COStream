#ifndef _METIS_PARTITION_H_
#define _METIS_PARTITION_H_


#include "Partition.h"
#include "ActorStateDetector.h"
#include "metis.h"

class MetisPartiton:public Partition
{
public:
	MetisPartiton(int objtype=-1,int contig=1);
	~MetisPartiton();
	void SssgPartition(SchedulerSSG *sssg , int level);//利用metis进行划分
	void metisPartition(int nvtx,int mncon,int *mxadj,int *madjncy,int *mvwgt,int *mvsize,int *madjwgt,int mnparts,float *tpwgts,float *ubvec,int objval,int *mpart);//zww 20120924 添加
private:
	//束调度（BS）metis会需要用到的成员变量
	int *mxadj;//定义指针指向xadj数组
	int *madjncy;//定义指针指向adjncy数组
	int *mobjval;//定义指针指向objval
	int *mpart;//定义指针指向part数组
	int *mvwgt;//定义指针指向vwgt数组，后者存储每个顶点的权值
	int *madjwgt;//定义指针指向adjwgt数组
	int nvtxs;//定义顶点个数
	float *tpwgts;
	int *mvsize;//定义指针指向vsize数组
	float *ubvec;
	int mncon; //平衡限制条件的数目，至少为1
	int objval;
	int options[40];//参数数组
};
extern "C"
{
	extern GLOBAL Bool X86Backend;
	extern GLOBAL Bool GPUBackend;
};
#endif 