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
	vector<FlatNode *> vTemplateNode; //chenwenbin 20140724 �洢����ģ����
	vector<std::string> vTemplateName; //chenwenbin ��¼ÿ��ģ���������
	map<FlatNode *,std::string> mapFlatnode2Template; //chenwenbin ���flatnode��Ӧ��ģ����
	ActorEdgeInfo* pEdgeInfo;//��Ÿ����ߵ�������Ϣ

	TemplateClass(SchedulerSSG *sssg);
	void ResetTemplateName(); //chenwenbin ����ģ������
	void SetTemplateNode(); //chenwenbin 20140724 ��ȡ����ģ����
	bool InTemplate(FlatNode *); //chenwenbin �ж�flatnode�Ƿ��Ѿ���ģ������
	bool IsSplitJoinNode(FlatNode *); //chenwenbin�ж�flatnode�Ƿ�ΪSplitJoin���
	bool DimValueSame(FlatNode *,FlatNode *); //chenwenbin �ж�����flatnode�ı�ʾ����ά�ȵı����Ƿ����
	bool DataTypeSame(FlatNode*,FlatNode*);//cwb�ж���������actor�ıߵ����������Ƿ���ͬ
	bool StorageValueSame(FlatNode*,FlatNode*);//cwb�ж���������actor�Ĵ洢�ߵĿռ��С�Ƿ����
	int GetTemplateIndex(FlatNode *);
	bool SameofSplitjoinNode(FlatNode*,FlatNode*); //��ͬ��splitjoin���
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