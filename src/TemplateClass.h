#ifndef TEMPLATECLASS_H
#define TEMPLATECLASS_H
//#include "list.h"
#include "flatNode.h"
#include "schedulerSSG.h"
#include "ActorEdgeInfo.h"
using namespace std;
extern "C"
{
	extern GLOBAL Bool X86Backend;
	extern GLOBAL Bool GPUBackend;
	extern GLOBAL int GpuNum;
};
class TemplateClass{
public:
	SchedulerSSG *Sssg;
	vector<FlatNode *> vTemplateNode; //chenwenbin 20140724 存储所有模板结点
	vector<std::string> vTemplateName; //chenwenbin 记录每个模板类的名字
	map<FlatNode *,std::string> mapFlatnode2Template; //chenwenbin 存放flatnode对应的模板类
	ActorEdgeInfo* pEdgeInfo;//存放各个边的类型信息

	TemplateClass(SchedulerSSG *sssg);
	void ResetTemplateName(); //chenwenbin 重置模板名字
	void SetTemplateNode(); //chenwenbin 20140724 获取所有模板结点
	bool InTemplate(FlatNode *); //chenwenbin 判断flatnode是否已经在模板类中
	bool IsSplitJoinNode(FlatNode *); //chenwenbin判断flatnode是否为SplitJoin结点
	bool DimValueSame(FlatNode *,FlatNode *); //chenwenbin 判断两个flatnode的表示数组维度的变量是否相等
	bool DataTypeSame(FlatNode*,FlatNode*);//cwb判断连接两个actor的边的数据类型是否相同
	bool StorageValueSame(FlatNode*,FlatNode*);//cwb判断连接两个actor的存储边的空间大小是否相等
	int GetTemplateIndex(FlatNode *);
	bool SameofSplitjoinNode(FlatNode*,FlatNode*); //相同的splitjoin结点
	inline std::vector<FlatNode *> GetTemplateNode(void)
	{
		return vTemplateNode;
	}
	inline std::vector<std::string> GetTemplateName(void)
	{return vTemplateName;}
	inline std::map<FlatNode *,std::string> GetFlatnode2Template(void)
	{return mapFlatnode2Template;}
};
#endif