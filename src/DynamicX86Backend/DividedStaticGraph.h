#ifndef _DIVIDED_STATIC_GRAPH_
#define _DIVIDED_STATIC_GRAPH_
#include "../flatNode.h"
#include <set>
#include "../staticStreamGraph.h"
#include "../schedulerSSG.h"
#include <vector>
using namespace std;

extern "C"
{
	extern int NThreads;
}

enum CombineState
{
	FULLCORE, MERGEONE,MERGETWO, MERGEUPONE, MERGEUPTWO,MERGEDOWNONE,MERGEDOWNTWO, NOTMERGE, NOTJUDGE
	//FULLCORE��ʾ��8�ˣ������кϲ���MERGE��ʾҪ�ϲ����ϲ��Ķ�����
	//��һ��MERGE.NOTMERGE��ʾ��������ϲ��������󣬲����кϲ���
	//���������ʣ��һ����ͼ�����кϲ�,NOTJUDGE��ʾû���д���
	//MERGEUP��ʾ������ͼѹ����MERGEDOWN��ʾ������ͼѹ��
};
enum ActorPosition		//���ڱ�ʾ��������ͼ�е�λ��
{
	FIRST, LAST, MIDDLE,BOTH//BOTH��ʾһ���������϶���������ߺͱ�������߶��Ƕ�̬�����ʾ����ͼֻ��һ������
};
class DividedStaticGraph
{
public:
	std::vector<StaticStreamGraph*> staticChildGraph;	//��̬��ͼ�б�
	std::vector<SchedulerSSG*> scheStaticChildGraph;	//���Ⱥ���ͼ�б�
	std::vector<int> maxUseCoreNumList;		//ÿ����ͼʹ�õĺ�����δѹ��֮ǰ
	std::vector<CombineState> combineList;//�ó�Ա�洢��Ӧ��ͼ�Ƿ�combine�����±���scheStaticChildGraph��staticChildGraph��ͬ
	std::vector<std::pair<SchedulerSSG*, int> >ssg2coreNum;//Ĭ����8��ѹ�����Ϊ���·���ĺ���������һ����ͼ��ʹ�ú�����ӳ��
	std::map<FlatNode*, int> flatNode2graphindex;		//��������ͼ��ŵ�ӳ��
	std::map<FlatNode*, ActorPosition> flatNode2position;		//һ��������λ�õ�ӳ��
	//Ŀǰcombine�Ļ����Ǵӵ�һ����ͼ��ʼ
	//�����Ϊ8����������Ϊ0
	int graphCount;			//��ͼ����
	int lastGraph;			//���һ����ͼ
public:
	DividedStaticGraph(StaticStreamGraph*ssg);			//���캯����������ͼ
	void setCount(int);		//X
	int getCount();				//X�����ͼ����
	int getlastGraph();		//X������һ����ͼ�±�
	bool isLastGraph(int);		//X�ж��Ƿ������һ����ͼ
	void updateSsg2Core(SchedulerSSG*graph, int coreNum);
	std::vector<FlatNode*> getAllFlatNodes();			//X����DSGͼ��ȫ���ڵ�
	void allocatePosition();						//��������λ��,����������FIRST,LAST,MIDDLE����BOTH
	ActorPosition getActorPosition(FlatNode*);		//�ж�����λ�ú���
	bool isSink(FlatNode*);			//�ж�λ��ΪLAST�Ľ���Ƿ�������sink���
	bool isSource(FlatNode*);		//�ж�λ��ΪFIRST�Ľ���Ƿ���source���

	int getSteadyCount(FlatNode*);		//�ú�������һ��actor���Ҷ�ӦSSSGͼ���������̬���ȴ���

	int getNextGraphFirstNodePop(FlatNode*);	//��ȡ��һ����ͼ����ʼactor������������������������ǰ�����̬�����˻���
	int getThisNodePop(FlatNode*);		//��ȡ�����ӵ�����������������������ǰ�����̬�����˻���
		
	int getGraphIndexByFlatNode(FlatNode*);		//����flatnode��ȡ����ͼ���
};


GLOBAL void SchedulingSSG(DividedStaticGraph*dsg);
GLOBAL extern DividedStaticGraph *DSG;

GLOBAL void GenerateWorkEst(DividedStaticGraph*dsg, bool WorkEstimateByDataFlow);
GLOBAL void ReviseDSGFunction(DividedStaticGraph*dsg);

#endif