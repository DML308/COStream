#ifndef DNBPARTITION_H
#define DNBPARTITION_H

#include "HAFLPartition.h"
#include "ActorEdgeInfo.h"
using namespace std;
class DNBPartition:public HAFLPartition{
private:
	double CPUTotalWork;  //cpu������actor��������
	double GPUTotalWork;  //GPU������actor�Ĺ�������
	double CpuGpuTotalComm;   //cpu��GPU������ݴ�����
	double Cpu2GpuVelocity;  //cpu��GPU��������
	double CpuFrequency;  //cpu����Ƶ��
	double GpuFrequency;  //gpu����Ƶ��
	int PENumber;    //GPU  ÿ��CU��PE����
	int CUNumber;   //GPU  CU����
	int MaxCputhreadNum; //������cputhread��������
public:
	//multimap<FlatNode*,int> Actor2Cputhread;  //actor����CPU�̵߳�ӳ��
	multimap<int,pair<FlatNode*,string> > Cputhread2Actor;
	map<string,pair<int,int> > actorName2count;
	//multimap<int,pair<FlatNode*,pair<int,int>>> Cputhread2Actor;
	vector<double> CputhreadWorkValue; //�洢ÿ��CPU�̵߳Ĺ�����
	vector<FlatNode*> statelessNodes; //cpu�е�stateless�ڵ�
	vector<FlatNode*> FissionNodes;
	multimap<FlatNode*,pair<int,int> > FissionNodes2count;
	map<FlatNode*,bool> actor2fission; //���ѹ��Ľڵ���1��ʾ��δ������0��ʾ 
	SchedulerSSG *Sssg;
	ActorEdgeInfo* pEdgeInfo;//��Ÿ����ߵ�������Ϣ
public:
	DNBPartition(HAFLPartition *,SchedulerSSG*);
	void DiscreteNodePartition();
	int minThreadnumofWorkvalue(); //workvalue��С��thread��

};
extern "C"
{
	extern GLOBAL int NCpuThreads;
	extern GLOBAL int GpuNum;
};
#endif
