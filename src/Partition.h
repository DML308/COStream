#ifndef _PARTITION_H_
#define _PARTITION_H_

#include "schedulerSSG.h"
#include "ClusterPartition.h"

extern "C"
{
	extern Bool X10DistributedBackEnd;/*��������ѡ�������ݡ�ͨ��ģ��x10�ĺ��*/
	extern int CpuCoreNum;
	extern int NThreads;/*��ʾѡ���threads����*/
};

class Partition
{
public:
	//���е��ȷ�������ʹ�õ��ĳ�Ա����
	std::map<FlatNode *,int>FlatNode2PartitionNum;//�ڵ㵽���ֱ�ŵ�ӳ��
	std::multimap<int,FlatNode *>PartitonNum2FlatNode;//���ֱ�ŵ��ڵ��ӳ��
	std::vector<FlatNode*>PartitonNumSet;//���ڼ�¼���صĽڵ㼯�� 
	int mnparts;//���ֵķ�����������ָ����places����


public:
	Partition();
	int getParts();//���ػ��ָ���mnparts
	void setCpuCoreNum(int,SchedulerSSG*);//����place��Ŀ����������Ŀ��
	int findID(SchedulerSSG *sssg,FlatNode *flatnode);//����flatnode�ҵ����±�� ��source_0�е�0
	std::vector<FlatNode *>findNodeSetInPartition(int partitionNum);//���ݱ��num�������еĽڵ㣬���ڵ㼯�Ϸ��ظ�PartitonNumSet(���->�ڵ�)
	int findPartitionNumForFlatNode(FlatNode *flatnode);//���ݽڵ㷵�������ڻ������ı��(�ڵ�->���)
	int Adjust(SchedulerSSG *sssg, int mode);//�������ֽ����������
	void FinalAdjust(SchedulerSSG *sssg);//ÿ��asyncֻ��������һ����ͨ��ͼ
	bool IsDeadLock(const std::vector<FlatNode *> &,const std::vector<FlatNode *> &, const int);//������ڵ㼯���Ƿ�����໥�ȴ�
	void AddDownFlatnodes(std::vector<FlatNode *> &,const std::vector<FlatNode *> &);//�ҵ�ͬһ�����нڵ���������νڵ�
	void AddUpFlatnodes(std::vector<FlatNode *> &,const std::vector<FlatNode *> &);//�ҵ�ͬһ�����нڵ���������νڵ�
	FlatNode *FindTopFlatnode(const std::vector<FlatNode *> &);//�ҵ���������������Ķ��˽ڵ�
	void AddFlatnodes(std::vector<FlatNode *> &,const std::vector<FlatNode *> &);//��vector��β���flatnodes
	int FindFlatnode(const std::vector<FlatNode *> &, const FlatNode *);
	double SumOfWork(const std::vector<FlatNode *> &);//����ڵ�Ĺ�����֮��
	int UnLock(const std::vector<FlatNode *> &, const std::vector<FlatNode *> &, const int);//������ֱ������
	//�����ǻ��ֳ�Ա����������ʵ��������ʵ��
	virtual void SssgPartition(SchedulerSSG *sssg ,int level)=0;//���ֳ�Ա����������ʵ��������ʵ��
	virtual ~Partition(){}
	inline std::map<FlatNode *,int>GetFlatNode2PartitionNum(void)
	{
		return FlatNode2PartitionNum;
	}


protected:
	std::multimap<FlatNode *, FlatNode *> currentSrc2Dest, currentDest2Src;
	std::vector<FlatNode *> currentSrcUp, currentSrcDown, currentDestUp, currentDestDown; 

};
GLOBAL void DumpStreamGraph(SchedulerSSG *ssg,Partition *,const char *fileName, ClusterPartition *);
GLOBAL void ComputeSpeedup(SchedulerSSG *sssg,Partition *mp,std::string,const char *fileName,std::string);
#endif
