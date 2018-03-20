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
	void SssgPartition(SchedulerSSG *sssg , int level);//����metis���л���
	
	void metisPartition(int nvtx,int mncon,int *mxadj,int *madjncy,int *mvwgt,int *mvsize,int *madjwgt,int mnparts,float *tpwgts,float *ubvec,int objval,int *mpart);//zww 20120924 ���
private:
	//�����ȣ�BS��metis����Ҫ�õ��ĳ�Ա����
	int *mxadj;//����ָ��ָ��xadj����
	int *madjncy;//����ָ��ָ��adjncy����
	int *mobjval;//����ָ��ָ��objval
	int *mpart;//����ָ��ָ��part����
	int *mvwgt;//����ָ��ָ��vwgt���飬���ߴ洢ÿ�������Ȩֵ
	int *madjwgt;//����ָ��ָ��adjwgt����
	int nvtxs;//���嶥�����
	float *tpwgts;
	int *mvsize;//����ָ��ָ��vsize����
	float *ubvec;
	int mncon; //ƽ��������������Ŀ������Ϊ1
	int objval;
	int options[40];//��������
};
extern "C"
{
	extern GLOBAL Bool X86Backend;
	extern GLOBAL Bool GPUBackend;
	extern GLOBAL Bool DynamicX86Backend;
};
#endif 