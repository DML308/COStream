#ifndef _CLUSTER_PARTITION_H
#define _CLUSTER_PARTITION_H

#include "ClusterGroupGraph.h"
#include "../metis.h"
#include "../schedulerSSG.h"

GLOBAL extern SchedulerSSG *SSSG;

class ClusterPartition
{//�����ֺõ�groupӳ�䵽��Ⱥ�ڵ���
public:
	std::map<FlatNode *,std::pair<int ,int> >flatNode2Cluster2Core;//flatNode����Ⱥ�ϻ����ı�ţ�core�ı�� ��map
	std::map<int, std::map<FlatNode *, int > > cluster2FlatNode2Core;//��Ⱥ�ϻ����ı��,flatNode,core֮���ӳ��
	std::map<int, std::multimap<int, FlatNode *> > cluster2Core2FlatNode;//��Ⱥ�ϻ����ı��,core,flatNode֮���ӳ��
	std::vector<std::map<FlatNode *, int> >cluster2flatNodeSteadyCount;//��¼ÿ��cluster�ϵ�flatNode���ȶ�״̬����
	std::map<FlatNode *,int> flatNode2Cluster;//flatNode�뼯Ⱥ�ڵ���ӳ��
	std::multimap<int,FlatNode*> cluster2FlatNode;//��Ⱥ�ڵ���FlatNode֮��ӳ��
private:
	std::multimap<int,ClusterGroup*> cluster2Group;//��Ⱥ�ڵ�ı����group��ŵ�ӳ��
	std::map<ClusterGroup*, int> group2Cluster;//group����뼯Ⱥ�ڵ�ı�ŵ�ӳ��
	std::map<int, int> group2Amortize;//group�����̯�����ӵ�ӳ��
	//��ʼ����ʱҪ�õ�	
	ClusterGroupGraph* groupGraph;
	int mnclusters;//��Ⱥ�ڵ����Ŀ
	int mnCores;//�����ڲ��ĺ˵���Ŀ���˴�ֻ����ͬ���ģ�
	std::vector<FlatNode*>flatNodeSet;//���ڼ�¼flatNode�ļ���(cluster���->flatNode)
	std::vector<ClusterGroup *> clusterGroupSet;//���ڼ�¼group�ļ���(cluster���->group)
	//Ϊ�˼�⼯Ⱥ����û�л�·����������map
	std::map<int, std::vector<int> > cluster2PrecedenceCluster;//��ǰcluster��ǰ��֮���map
	std::map<int, std::vector<int> > cluster2SuccessorCluster; //��ǰcluster����֮���map
public:
	ClusterPartition(){ };
	ClusterPartition(ClusterGroupGraph*);// ���group�ı��
	int GetClusters();//���ػ��ָ���mnclusters
	void SetClusters(int);//���ü�Ⱥ�ڵ���Ŀ����������Ŀ��
	int GetCores();//���ػ��ָ���mnCore
	void SetCores(int);
	void InitPartition(ClusterGroupGraph* groupGraph);//��ʼ���֣���K��ȷ������mnparts����mnclustersʱ��Ҫ���group��cluster�ڵ�֮��Ļ��֣�
	void RefinePartition(ClusterGroupGraph *groupGraph);//ϸ����������Ҫ��Ե��Ǳ߽��Ͻڵ㣩
	void MovingGroup(ClusterGroup *srcGroup,int destCluster,ClusterGroupGraph *graph );//��src����dest�ϲ������޸�graph�������Ϣ�����ص����ƶ��������
	int RefineMoveGroup(ClusterGroup *srcGroup, std::vector<ClusterGroup *>destClusterVec, ClusterGroupGraph *graph);//srcGroup��Ҫ�ƶ���group��destClusterVecĿ�Ļ���(��ѡ)���ϣ�graph��group��ͼ��gain���ƶ������棬���ص���src���ܱ��ƶ�����cluster���
	float ComputeCommDeltWeight(ClusterGroup *group);//����group����(��group��ͬһ��cluster��)�Ͷ���(��group����ͬһ��cluster��)ͨ�ŵ���������ֵ�������
	void MetisPartitionFlatNodeInCluster();//��cluster�ڲ���flatnode�ڵ㻮�֣���Ҫ��ӳ�䵽core��
	int GetClusterNum(ClusterGroup* group);//����group����cluster�ı��
	std::pair<int, int> GetClusterCoreNum(FlatNode *flatNode);//����flatNode����cluster��ź�core�ı��
	std::vector<FlatNode*> GetFlatNodes(int clusterNum);//����cluster�����flatNode
	std::vector<ClusterGroup *> GetGroups(int clusterNum);//����Cluster����Ҹýڵ��ϵ�����Group
	std::vector<FlatNode*> GetFlatNodes(int clusterNum,int coreNum);//����cluster��źͺ˵ı����flatNode
	std::map<FlatNode *,int > GetFlatNode2Core(int clusterNum);//���ݸ����ļ�Ⱥ�ڵ�ı�ţ�����ڸû����ϵĻ��֣�FlatNode��core֮���ӳ�䣩
	std::multimap<int,FlatNode *> GetCore2FlatNode(int clusterNum);//���ݸ����ļ�Ⱥ�ڵ�ı�ţ�����ڸû����ϵĻ��֣�core��FlatNode֮���ӳ�䣩
	int GetSteadyCount(FlatNode *flatNode);//ȡһ��FlatNode�ϵľֲ���̬����
	ClusterPartition* RevisionClusterPartition(ClusterPartition* _tmpcp, SchedulerSSG* sssg,std::map<int, std::map<FlatNode *, int > >& _tmpcluster2FlatNode2Core); //����cluster2FlatNode2Core��sssg�������ֵĽ��
	//ȡ������ɺ����Ӧ�������121106 zww ���
	inline std::map<FlatNode *,std::pair<int ,int> > GetflatNode2Cluster2Core()
	{
		return flatNode2Cluster2Core;
	}
	inline std::map<int, std::map<FlatNode *, int > > Getcluster2FlatNode2Core()
	{
		return cluster2FlatNode2Core;
	}
	inline std::map<int, std::multimap<int, FlatNode *> > Getcluster2Core2FlatNode()
	{
		return cluster2Core2FlatNode;
	}
	~ClusterPartition(){};
private:
	void MetisPartitionGroupGraph(ClusterGroupGraph* groupGraph);//ʹ��Metis����groupͼ�ģ������group��cluster�ڵ���ӳ��
	int findID(FlatNode *flatnode,std::vector<FlatNode *> original);//����flatnode�ҵ����±�� ��source_0�е�0
	//����cluster֮���������ϵ
	void CreateClusterGraph();
	void CreateClusterSteadyCount();//�����ȶ�״̬
	Bool HasClusterTopologicalSort(); 
	int ComputeWorkloadOfCluster(int cluster);//���㼯Ⱥ�ڵ��ϵĹ�����
	std::vector<FlatNode*> GetFlatNodesInGroups(int clusterNum);
	inline std::map<int, std::vector<int> > Getcluster2PrecedenceCluster()
	{
		return cluster2PrecedenceCluster;
	}
	inline std::map<int, std::vector<int> > Getcluster2SuccessorCluster()
	{
		return cluster2SuccessorCluster;
	}

};

extern ClusterPartition* CCPartition;//��Ⱥ���� ��������
GLOBAL ClusterPartition* SSGPartitionCluster(int nclusters, int nplaces);//���м�Ⱥ��Ķ�������


#endif