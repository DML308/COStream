#include "staticStreamGraph.h"
#define UPDATEEDGETAG 20
GLOBAL void GenerateWorkEst(StaticStreamGraph *ssg,bool WorkEstimateByDataFlow)
{
	int len = ssg->GetFlatNodes().size();//获取operator的长度

	for (int i=0;i<len;i++)
	{
		int w = 0,w_init = 0;
		FlatNode *tmpFn = (ssg->GetFlatNodes())[i];
		ChildNode *body =  tmpFn->contents->body;//取得operator的body，只需要检查这里
		if ( body != NULL)//计算operator的body内的工作量
		{
			w_init = workEstimate_init(body, w);
			if(WorkEstimateByDataFlow)
				w = workEstimateUseDataFlow(body,w);
			else
				w = workEstimate(body, w);
		}
		w += (tmpFn->outFlatNodes.size()+tmpFn->inFlatNodes.size())*UPDATEEDGETAG;//多核下调整缓冲区head和tail
		ssg->AddInitWork(tmpFn, w_init);
		ssg->AddSteadyWork(tmpFn, w);
	}
}
