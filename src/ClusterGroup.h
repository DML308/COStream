#ifndef _CLUSTER_GROUP_H
#define _CLUSTER_GROUP_H


#include <iostream>
#include <map>
#include <vector>
#include <list>

#include "ClusterBasics.h"
#include "schedulerSSG.h"

GLOBAL extern SchedulerSSG *SSSG;

class ClusterGroup
{
private:
	std::vector<FlatNode *> srcFlatNode;//group ��������ʼ�ڵ㣨����groupָ��ýڵ㣬һ���ڵ���ܼ�����ʼ��������ֹ�㣩
	std::vector<FlatNode *> snkFlatNode;//group ��������ֹ��㣨�ýڵ���ָ������group�ıߣ�
	std::vector<FlatNode *> flatNodes;//group �������������е�flatNode�ڵ�

	std::vector<ClusterGroup *> subClusterGroups;//��ǰgroup�ڲ����е���group��������Щgroup������Щ��С����group��ɣ�
	//ClusterGroup *parentClusterGroup;//ָ�������ںϺ��γɵ��½ڵ�

	std::map<FlatNode *,int> flatNode2SteadyCount;//group �и���flatNode�ľֲ���̬����
	std::vector<FlatNode *> precedenceFlatNode;//group ��ǰ��flatNode�ڵ�
	std::vector<FlatNode *> successorFlatNode;//group �ĺ��flatNode�ڵ�
	std::vector<ClusterGroup *> precedenceClusterGroup;//group��ǰ��group(��¼�뵱ǰgroupͬһ�����)
	std::vector<ClusterGroup *> successorClusterGroup;//group�ĺ��group(��¼�뵱ǰgroupͬһ�����)

	std::vector<ClusterGroup *> boundaryGroupSet;//���ڼ�¼һ��group�߽�group

	int groupSize;//��¼�ڵ���flatNode����Ŀ
	float commCost;//group����ĵ���ͨ�ſ���
	int workload;//group�Ĺ�����

public:
	Bool lock;//��ϸ��������ʹ�ã����һ��group���ƶ�һ�ξͲ����ڱ��ƶ���

	ClusterGroup(){};//Ĭ�Ϲ��캯��
	ClusterGroup(ClusterGroup* group1,ClusterGroup* group2);//��������group����һ���µ�group
	ClusterGroup(FlatNode *);//����һ��flatNode����һ��group
	
	/*��group��Դ�ڵ�Ĳ���*/
	std::vector<FlatNode *> GetSrcFlatNode();//ȡsrc

	/*��group��Դ�ڵ�Ĳ���*/
	std::vector<FlatNode *> GetSnkFlatNode();//ȡsrc

	/*ȡgroup�����еĽڵ�*/
	std::vector<FlatNode *> GetFlatNodes();

	/*ȡͼ��ǰ���ͺ��flatNode�ڵ�*/
	std::vector<FlatNode *> GetPrecedenceFlatNode();//ǰ��
	void AddPrecedenceFlatNode(FlatNode *);
	void DeletePrecedenceFlatNode(FlatNode *);
	std::vector<FlatNode *> GetSuccessorFlatNode();//���
	void AddSuccessorFlatNode(FlatNode *);
	void DeleteSuccessorFlatNode(FlatNode *);


	/*ȡgroup�ڵĽڵ���̬���ȵĽ��*/
	std::map<FlatNode *,int> GetSteadyCountMap();

	/*ȡͼ��ǰ���ͺ��ClusterGroup�ڵ�*/
	std::vector<ClusterGroup *> GetClusterGroups();//ȡ��ǰgroup�е�������group
	
	std::vector<ClusterGroup *> GetPrecedenceClusterGroup();//ǰ��
	void AddPrecedenceClusterGroup(ClusterGroup *);
	Bool DeletePrecedenceClusterGroup(ClusterGroup *);
	void SetPrecedenceClusterGroup(std::vector<ClusterGroup *>);
	
	std::vector<ClusterGroup *> GetSuccessorClusterGroup();//���
	void AddSuccessorClusterGroup(ClusterGroup *);
	Bool DeleteSuccessorClusterGroup(ClusterGroup *);
	void SetSuccessorClusterGroup(std::vector<ClusterGroup *>);

	std::vector<ClusterGroup *> GetBoundaryClusterGroup();//ȡ�߽�group
	float GetCommunicationCost();//ȡgroup��ͨ�ſ���
	void  SetCommunicationCost(float cost);

	int GetWorkload();//ȡgroup�Ĺ�����
	void SetWorkload(int weight);

	float GetCompCommRadio();//��ȡ����ͨ�ű�
	~ClusterGroup(){};

};
GLOBAL std::map<FlatNode *, int> SteadySchedulingGroup(std::vector<FlatNode *> );//��vector�еĵ㼯���оֲ���̬����

int lcm(int a, int b); // ��a,b����С������
int gcd(int a, int b); // ��a,b�����Լ��	



#endif