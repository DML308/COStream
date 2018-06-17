#ifndef _CLUSTER_GROUP_GRAPH_H
#define _CLUSTER_GROUP_GRAPH_H


#include <iostream>
#include <map>
#include <vector>
#include "ClusterGroup.h"
#include "schedulerSSG.h"

GLOBAL extern SchedulerSSG *SSSG;

class ClusterGroupGraph
{//该内的目的就是将图划分成K份（执行类似于多层K路划分）
private:
	std::map<ClusterGroup *,float>clusterGroup2Radio;//记录当前所有的group以及通的计算通信比
	std::multimap<ClusterGroup*, ClusterGroup*> groupSrc2SnkChannel;//group间的边(每一对clustergroup 只会出现一次)
	std::map<FlatNode *,ClusterGroup *>flatNode2ClusterGroup;//flatNode与group间的映射(只记录最高一层group的映射关系)
	std::vector<ClusterGroup *>clusterGroupVec;//记录初始化分后所有的group(按组group的先后顺序存放)
	int mngroups;//group的数目
	std::vector< std::pair<ClusterGroup*,ClusterGroup*> > coarseGroupSort;//记录在粗化是group的融合顺序
	std::vector<ClusterGroup*> coarseGroupResult;//记录每一对group合并后形成的新的group节点，与 coarseGroupSort对应

public:
	ClusterGroupGraph(SchedulerSSG *SSSG);//根据SSG构造最初的group图
	void CreateCoarseGraph(int k);//粗化（相邻的group之间两两聚合，最后聚合成K份——要保证划分的连通性）
	void CreateCoarseGraph();//粗化(根据实际的情况，最终聚合得到K的数目并不确定)
	std::vector< std::pair<ClusterGroup*,ClusterGroup*> > GetCoarseGroupSort();//取融合的顺序
	std::vector<ClusterGroup *> GetCoarseGroupResult();
	//为了完成粗化要做的事情
	ClusterGroup* FussGroup(ClusterGroup *,ClusterGroup *);//合并一对相邻的group,形成新的group（计算通信比提高最大的一个）
	float CompToCommRadio(ClusterGroup *);//计算一group的计算通信比
	Bool HasGroupTopologicalSort(std::vector<ClusterGroup *>original);//group间的拓扑排序,有则返回ture，(主要用于检测环路的)
	//取相应的数据成员的信息
	ClusterGroup *GetClusterGroup(FlatNode *);//取一个flatNode所在的group
	std::vector<ClusterGroup *> GetClusterGroupSet(); //取所有的group
	std::multimap<ClusterGroup*, ClusterGroup*> GetGroupSrc2SnkChannel();//取group所有的边
	int GetGroupNum();//取划分数目
	void SetGroupNum(int );//设置划分数目
	void ModifyConnectionOfGroup(ClusterGroup *group1,ClusterGroup *group2,ClusterGroup *group);//修改group图中的连接关系
	std::map<FlatNode *,ClusterGroup *> GetFlatNode2ClusterGroup();//返回flatNode与group之间的map
};

#endif