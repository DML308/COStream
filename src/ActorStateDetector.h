#ifndef _FILTER_STATE_DETECTOR_H
#define _FILTER_STATE_DETECTOR_H

#include "flatNode.h"

class ActorStateDetector
{
public:
	ActorStateDetector(operatorNode *op){ opNode = op;stateful = FALSE;};
	/*判断actor是否是stateful*/
	void hasMutableState();	
	/*判断一个变量是否是mutable变量*/
	void IsMutableVar(List *list,Node *node);
	/*用于遍历work函数内的信息*/
	void FSD_astwalk(Node *n,List *list);  
	void FSD_listwalk(List *l,List *list);
	Bool GetOperatorState(){return stateful;}
private:
	operatorNode *opNode;
	Bool stateful;
};

GLOBAL Bool DetectiveActorState(FlatNode *flatNode);

#endif