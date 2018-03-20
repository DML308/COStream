/* Begin ���� wangliang */
#ifndef _GREEDY_PARTITION_H_
#define _GREEDY_PARTITION_H_

#include "Partition.h"
#include "ActorStateDetector.h"
#include<algorithm>
#include<map>

enum FlatNodeState{ INSIDE = 0, BORDER, ALONE };
extern "C"
{
	extern GLOBAL Bool X86Backend;
	extern GLOBAL Bool GPUBackend;
	extern GLOBAL Bool DynamicX86Backend;
	extern GLOBAL double BalanceFactor;
};

class GreedyPartition :public Partition
{
public:
	GreedyPartition(SchedulerSSG *sssg){
		nvtxs = 0;
		ee = BalanceFactor;
	}
	~GreedyPartition(){

	}
	void SssgPartition(SchedulerSSG *sssg, int level);//һ������,����doPartition
	void doPartition(SchedulerSSG *sssg, int k);
	int chooseMaxGain(SchedulerSSG *sssg, vector<FlatNode *>& S, vector<FlatNode *>& Xi, vector<FlatNode *>& X0);
	void updateCandidate(SchedulerSSG *sssg, vector<FlatNode *>& X0, vector<FlatNode *>& S, int index);
	int getPartEdge(int index){
		return edge[index];
	}
	int getTotalEdge(SchedulerSSG *sssg, int k);
	int getPart(FlatNode *p){
		for (int i = 0; i < X.size(); i++){
			if (find(X[i].begin(), X[i].end(), p) != X[i].end())return i;
		}
	}
	int getPartWeight(int index){
		return w[index];
	}

	FlatNodeState getFlatNodeState(FlatNode *p);
	void upDateFlatNodeState(FlatNode *p);

	void doTabuSearch(SchedulerSSG *sssg, int k);
	void errorDecrease(SchedulerSSG *sssg, int k);
	void errorDecrease2(SchedulerSSG *sssg, int k);
	int orderPartitionResult();
	void setActorWorkload(SchedulerSSG *sssg);

private:
	vector<vector<FlatNode *>> X;//���ֵĽ��
	vector<int> w;//���ֵ�K����ͼÿ����ͼ���ܹ�����
	vector<int> edge;//���ֵ�K����ͼÿ����ͼ��ͨ�űߵ�Ȩ��

	vector<int> vwgt; //ÿ�������Ȩ��(�ܹ�����)
	vector<FlatNode *> S;//��ѡ�ڵ�ļ���
	vector<int> part;//ÿ���ڵ��Ӧ�ĺ˺�
	int nvtxs;//�ڵ�ĸ���
	double ee;//ƽ������
	map<FlatNode *, FlatNodeState> FlatNodeToState;//ÿ���ڵ��Ӧ��״̬
};


//struct statePartition{
//	vector<FlatNode *> insideVec;
//	vector<FlatNode *> borderVec;
//	vector<FlatNode *> aloneVec;
//};

#endif

/* End ���� wangliang */