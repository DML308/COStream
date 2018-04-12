#ifndef _ACTORSTAGEASS_H_
#define _ACTORSTAGEASS_H_

#include "schedulerSSG.h"
#include "ActorStateDetector.h"
#include "set"
using namespace std;

class StageAssignment
{
public:
	void actorStageMap(map<FlatNode *,int>processor2actor);//������FlatNode��core֮���ӳ��
	void actorStageMapForGPU(map<FlatNode *,int>processor2actor);
	void actorTopologicalorder(vector<FlatNode *>original);
	int FindStage(FlatNode*);//���ݽ��Ѱ�������ڵĽ׶κ�
	vector<FlatNode*> FindActor(int);//���ݽ׶κ�Ѱ�ҽ��
	vector<FlatNode*> FindDataOfActor(int);//���ݽ׶κ�Ѱ����Ҫ�������ݴ���Ľ��
	//map<FlatNode *,int> ReturnDataOfActor2Stage();
	int MaxStageNum();//�������Ľ׶α��
	int MaxStageNumForGPU();//�������Ľ׶α��
	multimap<int,map<FlatNode*,int> >datastage;//���ڱ�����ݴ����Ƿ��ͻ���ȡ���ݣ�����thread.cpp������,�����ݽṹ�е�һ��int���ڼ�¼�׶κţ��ڶ���int���ڼ�¼��־λ,1:ȡ����  2��������
protected:
	vector<FlatNode *>actortopo;//���ڴ洢actor����������
	map<FlatNode *,int>Actor2Stage;//���ڴ洢�׶θ�ֵ�Ľ��
	multimap<int,FlatNode*>Stage2Actor;//���ڴ洢�׶θ�ֵ�Ľ��
	vector<FlatNode *>ActorSet;//
	/***������������ΪGPU��������ר�ã����ڼ�¼���ݴ������ڵĽ׶�***/
	map<FlatNode *,int>DataOfActor2Stage;
	multimap<int,FlatNode*>Stage2DataOfActor;
private:
};
extern "C"
{
	extern GLOBAL int GpuNum;
};
#endif
