#ifndef _FILTER_STATE_DETECTOR_H
#define _FILTER_STATE_DETECTOR_H

#include "flatNode.h"

class ActorStateDetector
{
public:
	ActorStateDetector(operatorNode *op){ opNode = op;stateful = FALSE;};
	/*�ж�actor�Ƿ���stateful*/
	void hasMutableState();	
	/*�ж�һ�������Ƿ���mutable����*/
	void IsMutableVar(List *list,Node *node);
	/*���ڱ���work�����ڵ���Ϣ*/
	void FSD_astwalk(Node *n,List *list);  
	void FSD_listwalk(List *l,List *list);
	Bool GetOperatorState(){return stateful;}
private:
	operatorNode *opNode;
	Bool stateful;
};

GLOBAL Bool DetectiveActorState(FlatNode *flatNode);
GLOBAL int UporDownStatelessNode(FlatNode *node);
#endif