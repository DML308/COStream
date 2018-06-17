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
	std::vector<FlatNode *> srcFlatNode;//group 的所有起始节点（其他group指向该节点，一个节点可能既是起始点又是终止点）
	std::vector<FlatNode *> snkFlatNode;//group 的所有终止结点（该节点有指向其他group的边）
	std::vector<FlatNode *> flatNodes;//group 中所包含的所有的flatNode节点

	std::vector<ClusterGroup *> subClusterGroups;//当前group内部所有的子group（表明这些group是有那些更小的子group组成）
	//ClusterGroup *parentClusterGroup;//指向两两融合后形成的新节点

	std::map<FlatNode *,int> flatNode2SteadyCount;//group 中各个flatNode的局部稳态次数
	std::vector<FlatNode *> precedenceFlatNode;//group 的前驱flatNode节点
	std::vector<FlatNode *> successorFlatNode;//group 的后继flatNode节点
	std::vector<ClusterGroup *> precedenceClusterGroup;//group的前驱group(记录与当前group同一级别的)
	std::vector<ClusterGroup *> successorClusterGroup;//group的后继group(记录与当前group同一级别的)

	std::vector<ClusterGroup *> boundaryGroupSet;//用于记录一个group边界group

	int groupSize;//记录节点中flatNode的数目
	float commCost;//group对外的的总通信开销
	int workload;//group的工作量

public:
	Bool lock;//再细化调整是使用（如果一个group被移动一次就不能在被移动）

	ClusterGroup(){};//默认构造函数
	ClusterGroup(ClusterGroup* group1,ClusterGroup* group2);//根据两个group构造一个新的group
	ClusterGroup(FlatNode *);//给定一个flatNode构造一个group
	
	/*对group的源节点的操作*/
	std::vector<FlatNode *> GetSrcFlatNode();//取src

	/*对group的源节点的操作*/
	std::vector<FlatNode *> GetSnkFlatNode();//取src

	/*取group中所有的节点*/
	std::vector<FlatNode *> GetFlatNodes();

	/*取图的前驱和后继flatNode节点*/
	std::vector<FlatNode *> GetPrecedenceFlatNode();//前驱
	void AddPrecedenceFlatNode(FlatNode *);
	void DeletePrecedenceFlatNode(FlatNode *);
	std::vector<FlatNode *> GetSuccessorFlatNode();//后继
	void AddSuccessorFlatNode(FlatNode *);
	void DeleteSuccessorFlatNode(FlatNode *);


	/*取group内的节点稳态调度的结果*/
	std::map<FlatNode *,int> GetSteadyCountMap();

	/*取图的前驱和后继ClusterGroup节点*/
	std::vector<ClusterGroup *> GetClusterGroups();//取当前group中的所有子group
	
	std::vector<ClusterGroup *> GetPrecedenceClusterGroup();//前驱
	void AddPrecedenceClusterGroup(ClusterGroup *);
	Bool DeletePrecedenceClusterGroup(ClusterGroup *);
	void SetPrecedenceClusterGroup(std::vector<ClusterGroup *>);
	
	std::vector<ClusterGroup *> GetSuccessorClusterGroup();//后继
	void AddSuccessorClusterGroup(ClusterGroup *);
	Bool DeleteSuccessorClusterGroup(ClusterGroup *);
	void SetSuccessorClusterGroup(std::vector<ClusterGroup *>);

	std::vector<ClusterGroup *> GetBoundaryClusterGroup();//取边界group
	float GetCommunicationCost();//取group的通信开销
	void  SetCommunicationCost(float cost);

	int GetWorkload();//取group的工作量
	void SetWorkload(int weight);

	float GetCompCommRadio();//获取计算通信比
	~ClusterGroup(){};

};
GLOBAL std::map<FlatNode *, int> SteadySchedulingGroup(std::vector<FlatNode *> );//对vector中的点集进行局部稳态调度

int lcm(int a, int b); // 求a,b的最小公倍数
int gcd(int a, int b); // 求a,b的最大公约数	



#endif