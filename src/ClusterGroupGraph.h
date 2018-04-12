#ifndef _CLUSTER_GROUP_GRAPH_H
#define _CLUSTER_GROUP_GRAPH_H


#include <iostream>
#include <map>
#include <vector>
#include "ClusterGroup.h"
#include "schedulerSSG.h"

GLOBAL extern SchedulerSSG *SSSG;

class ClusterGroupGraph
{//���ڵ�Ŀ�ľ��ǽ�ͼ���ֳ�K�ݣ�ִ�������ڶ��K·���֣�
private:
	std::map<ClusterGroup *,float>clusterGroup2Radio;//��¼��ǰ���е�group�Լ�ͨ�ļ���ͨ�ű�
	std::multimap<ClusterGroup*, ClusterGroup*> groupSrc2SnkChannel;//group��ı�(ÿһ��clustergroup ֻ�����һ��)
	std::map<FlatNode *,ClusterGroup *>flatNode2ClusterGroup;//flatNode��group���ӳ��(ֻ��¼���һ��group��ӳ���ϵ)
	std::vector<ClusterGroup *>clusterGroupVec;//��¼��ʼ���ֺ����е�group(����group���Ⱥ�˳����)
	int mngroups;//group����Ŀ
	std::vector< std::pair<ClusterGroup*,ClusterGroup*> > coarseGroupSort;//��¼�ڴֻ���group���ں�˳��
	std::vector<ClusterGroup*> coarseGroupResult;//��¼ÿһ��group�ϲ����γɵ��µ�group�ڵ㣬�� coarseGroupSort��Ӧ

public:
	ClusterGroupGraph(SchedulerSSG *SSSG);//����SSG���������groupͼ
	void CreateCoarseGraph(int k);//�ֻ������ڵ�group֮�������ۺϣ����ۺϳ�K�ݡ���Ҫ��֤���ֵ���ͨ�ԣ�
	void CreateCoarseGraph();//�ֻ�(����ʵ�ʵ���������վۺϵõ�K����Ŀ����ȷ��)
	std::vector< std::pair<ClusterGroup*,ClusterGroup*> > GetCoarseGroupSort();//ȡ�ںϵ�˳��
	std::vector<ClusterGroup *> GetCoarseGroupResult();
	//Ϊ����ɴֻ�Ҫ��������
	ClusterGroup* FussGroup(ClusterGroup *,ClusterGroup *);//�ϲ�һ�����ڵ�group,�γ��µ�group������ͨ�ű��������һ����
	float CompToCommRadio(ClusterGroup *);//����һgroup�ļ���ͨ�ű�
	Bool HasGroupTopologicalSort(std::vector<ClusterGroup *>original);//group�����������,���򷵻�ture��(��Ҫ���ڼ�⻷·��)
	//ȡ��Ӧ�����ݳ�Ա����Ϣ
	ClusterGroup *GetClusterGroup(FlatNode *);//ȡһ��flatNode���ڵ�group
	std::vector<ClusterGroup *> GetClusterGroupSet(); //ȡ���е�group
	std::multimap<ClusterGroup*, ClusterGroup*> GetGroupSrc2SnkChannel();//ȡgroup���еı�
	int GetGroupNum();//ȡ������Ŀ
	void SetGroupNum(int );//���û�����Ŀ
	void ModifyConnectionOfGroup(ClusterGroup *group1,ClusterGroup *group2,ClusterGroup *group);//�޸�groupͼ�е����ӹ�ϵ
	std::map<FlatNode *,ClusterGroup *> GetFlatNode2ClusterGroup();//����flatNode��group֮���map
};

#endif