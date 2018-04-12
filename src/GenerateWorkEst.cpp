#include "staticStreamGraph.h"
#define UPDATEEDGETAG 20
GLOBAL void GenerateWorkEst(StaticStreamGraph *ssg,bool WorkEstimateByDataFlow)
{
	int len = ssg->GetFlatNodes().size();//��ȡoperator�ĳ���

	for (int i=0;i<len;i++)
	{
		int w = 0,w_init = 0;
		FlatNode *tmpFn = (ssg->GetFlatNodes())[i];
		ChildNode *body =  tmpFn->contents->body;//ȡ��operator��body��ֻ��Ҫ�������
		if ( body != NULL)//����operator��body�ڵĹ�����
		{
			w_init = workEstimate_init(body, w);
			if(WorkEstimateByDataFlow)
				w = workEstimateUseDataFlow(body,w);
			else
				w = workEstimate(body, w);
		}
		w += (tmpFn->outFlatNodes.size()+tmpFn->inFlatNodes.size())*UPDATEEDGETAG;//����µ���������head��tail
		ssg->AddInitWork(tmpFn, w_init);
		ssg->AddSteadyWork(tmpFn, w);
	}
}
