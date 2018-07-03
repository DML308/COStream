#ifndef _CLUSTER_PARTITION_H
#define _CLUSTER_PARTITION_H

#include "ClusterGroupGraph.h"
#include "metis.h"
#include "schedulerSSG.h"

GLOBAL extern SchedulerSSG *SSSG;

class ClusterPartition
{//将划分好的group映射到集群节点中
public:
	std::map<FlatNode *,std::pair<int ,int> >flatNode2Cluster2Core;//flatNode，集群上机器的编号，core的编号 的map
	std::map<int, std::map<FlatNode *, int > > cluster2FlatNode2Core;//集群上机器的编号,flatNode,core之间的映射
	std::map<int, std::multimap<int, FlatNode *> > cluster2Core2FlatNode;//集群上机器的编号,core,flatNode之间的映射
	std::vector<std::map<FlatNode *, int> >cluster2flatNodeSteadyCount;//记录每个cluster上的flatNode的稳定状态次数
	std::map<FlatNode *,int> flatNode2Cluster;//flatNode与集群节点间的映射
	std::multimap<int,FlatNode*> cluster2FlatNode;//集群节点与FlatNode之间映射
private:
	std::multimap<int,ClusterGroup*> cluster2Group;//集群节点的编号与group编号的映射
	std::map<ClusterGroup*, int> group2Cluster;//group编号与集群节点的编号的映射
	std::map<int, int> group2Amortize;//group编号与摊销因子的映射
	//初始化分时要用到	
	ClusterGroupGraph* groupGraph;
	int mnclusters;//集群节点的数目
	int mnCores;//机器内部的核的数目（此处只考虑同构的）
	std::vector<FlatNode*>flatNodeSet;//用于记录flatNode的集合(cluster编号->flatNode)
	std::vector<ClusterGroup *> clusterGroupSet;//用于记录group的集合(cluster编号->group)
	//为了检测集群间有没有环路，引入两个map
	std::map<int, std::vector<int> > cluster2PrecedenceCluster;//当前cluster与前驱之间的map
	std::map<int, std::vector<int> > cluster2SuccessorCluster; //当前cluster与后继之间的map
public:
	ClusterPartition(){ };
	ClusterPartition(ClusterGroupGraph*);// 完成group的编号
	int GetClusters();//返回划分个数mnclusters
	void SetClusters(int);//设置集群节点数目（即进程数目）
	int GetCores();//返回划分个数mnCore
	void SetCores(int);
	void InitPartition(ClusterGroupGraph* groupGraph);//初始化分（当K不确定或者mnparts大于mnclusters时需要完成group与cluster节点之间的划分）
	void RefinePartition(ClusterGroupGraph *groupGraph);//细化调整（主要针对的是边界上节点）
	void MovingGroup(ClusterGroup *srcGroup,int destCluster,ClusterGroupGraph *graph );//将src，与dest合并，并修改graph的相关信息，返回的是移动后的收益
	int RefineMoveGroup(ClusterGroup *srcGroup, std::vector<ClusterGroup *>destClusterVec, ClusterGroupGraph *graph);//srcGroup将要移动的group，destClusterVec目的划分(候选)集合，graph是group的图，gain是移动的收益，返回的是src可能被移动到的cluster编号
	float ComputeCommDeltWeight(ClusterGroup *group);//计算group对内(与group在同一个cluster中)和对外(与group不在同一个cluster中)通信的增量（差值或比例）
	void MetisPartitionFlatNodeInCluster();//对cluster内部的flatnode节点划分，主要是映射到core上
	int GetClusterNum(ClusterGroup* group);//根据group查找cluster的编号
	std::pair<int, int> GetClusterCoreNum(FlatNode *flatNode);//根据flatNode查找cluster编号和core的编号
	std::vector<FlatNode*> GetFlatNodes(int clusterNum);//根据cluster编号找flatNode
	std::vector<ClusterGroup *> GetGroups(int clusterNum);//根据Cluster编号找该节点上的所有Group
	std::vector<FlatNode*> GetFlatNodes(int clusterNum,int coreNum);//根据cluster编号和核的编号找flatNode
	std::map<FlatNode *,int > GetFlatNode2Core(int clusterNum);//根据给定的集群节点的编号，获得在该机器上的划分（FlatNode与core之间的映射）
	std::multimap<int,FlatNode *> GetCore2FlatNode(int clusterNum);//根据给定的集群节点的编号，获得在该机器上的划分（core与FlatNode之间的映射）
	int GetSteadyCount(FlatNode *flatNode);//取一个FlatNode上的局部稳态次数
	ClusterPartition* RevisionClusterPartition(ClusterPartition* _tmpcp, SchedulerSSG* sssg,std::map<int, std::map<FlatNode *, int > >& _tmpcluster2FlatNode2Core); //根据cluster2FlatNode2Core和sssg修正划分的结果
	//取划分完成后的相应结果——121106 zww 添加
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
	void MetisPartitionGroupGraph(ClusterGroupGraph* groupGraph);//使用Metis划分group图的，并完成group与cluster节点间的映射
	int findID(FlatNode *flatnode,std::vector<FlatNode *> original);//根据flatnode找到其下标号 如source_0中的0
	//构造cluster之间的依赖关系
	void CreateClusterGraph();
	void CreateClusterSteadyCount();//构造稳定状态
	Bool HasClusterTopologicalSort(); 
	int ComputeWorkloadOfCluster(int cluster);//计算集群节点上的工作量
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

extern ClusterPartition* CCPartition;//集群，核 两级划分
GLOBAL ClusterPartition* SSGPartitionCluster(int nclusters, int nplaces);//进行集群间的二级划分


#endif