#include "RRSPartition.h"

using namespace std;

RRSPartiton::RRSPartiton():Partition()
{
	this->totalActors = 0;
	this->perParts = 0;
}

void RRSPartiton::SssgPartition(SchedulerSSG *sssg ,int level){
	assert(level==1);
	this->totalActors = sssg->GetFlatNodes().size();
	this->perParts = (int)ceil((float)this->totalActors / this->mnparts);
	vector<int>part(this->totalActors);

	for (int i=0,total=this->totalActors;i<this->mnparts;i++)
		for (int j=0;j<this->perParts && total>0;j++,total--)
			part[this->perParts*i+j]=i;

	for (int i=0;i<this->totalActors;i++){
		FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i],part[i]));//�����ڵ㵽���ֱ�ŵ�ӳ��
	}

	for (int i=0;i<this->totalActors;i++)
		PartitonNum2FlatNode.insert(make_pair(part[i],sssg->GetFlatNodes()[i]));

#if 1 //��ӡͼ
	DumpStreamGraph(sssg,this,"RRSPartitionGraph.dot",NULL);//zww_20120605��ӵ��ĸ�����
#endif
}