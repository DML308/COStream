#include "TemplateClass.h"
TemplateClass::TemplateClass(SchedulerSSG *sssg)
{
	Sssg = sssg;
	pEdgeInfo = new ActorEdgeInfo(Sssg);
}
int TemplateClass::GetTemplateIndex(FlatNode *node)
{
	int index = 0;
	if(IsSplitJoinNode(node))
	{
		for (;index < vTemplateNode.size(); index++)
		{
			if(node == vTemplateNode[index])
				break;
			else if (node->PreName == vTemplateNode[index]->PreName && SameofSplitjoinNode(node,vTemplateNode[index]))
				break;
		}
	}
	else
	{
		if (X86Backend)
		{
			for(;index < vTemplateNode.size(); index++)
				if(node->PreName == vTemplateNode[index]->PreName && DimValueSame(node,vTemplateNode[index]))
					break;
		}
		else if (GPUBackend)
		{
			for(;index < vTemplateNode.size(); index++)
				if(node->PreName == vTemplateNode[index]->PreName && DimValueSame(node,vTemplateNode[index]))
				{
					if((node == vTemplateNode[index])|| (node->GPUPart >= GpuNum && vTemplateNode[index]->GPUPart >= GpuNum) || (node->GPUPart < GpuNum && vTemplateNode[index]->GPUPart < GpuNum && DataTypeSame(node,vTemplateNode[index]) && StorageValueSame(node,vTemplateNode[index]) &&((Sssg->IsUpBorder(node) && Sssg->IsUpBorder(vTemplateNode[index])) || (Sssg->IsDownBorder(node) && Sssg->IsDownBorder(vTemplateNode[index])) || (!Sssg->IsUpBorder(node) && !Sssg->IsDownBorder(node) && !Sssg->IsUpBorder(vTemplateNode[index]) && !Sssg->IsDownBorder(vTemplateNode[index])))))
						break;
				}
		}
	}
	return index;
}
void TemplateClass::ResetTemplateName()
{
	for (int i = 0; i < vTemplateName.size(); i++)
	{
		stringstream ss;
		string temp;
		ss<<i;
		ss>>temp;
		vTemplateName[i] = vTemplateName[i] + "_" + temp;
	}
	for (int i = 0; i < Sssg->flatNodes.size(); i++)
	{
		mapFlatnode2Template.insert(make_pair(Sssg->flatNodes[i],vTemplateName[GetTemplateIndex(Sssg->flatNodes[i])]));
	}
}
bool TemplateClass::IsSplitJoinNode(FlatNode *node)
{
	if(node->PreName == "Duplicate" || node->PreName == "Roundrobin" || node->PreName == "Join")
		return true;
	else
		return false;
}
bool TemplateClass::DimValueSame(FlatNode *node1,FlatNode *node2)
{
	bool flag = true;
	//先判断常量值是否相等
	bool otherType = false;
	paramList *p = node1->contents->dimParams,*q = node2->contents->dimParams;
	while(p != NULL)
	{
		if(p->paramnode->u.Const.type->typ == Prim)
		{
			switch(p->paramnode->u.Const.type->u.prim.basic)
			{
			case Sint:  if(p->paramnode->u.Const.value.i != q->paramnode->u.Const.value.i)flag = false;break;
			case Uint:  if(p->paramnode->u.Const.value.u != q->paramnode->u.Const.value.u)flag = false;break;
			case Slong: if(p->paramnode->u.Const.value.l != q->paramnode->u.Const.value.l)flag = false;break;
			case Ulong: if(p->paramnode->u.Const.value.ul != q->paramnode->u.Const.value.ul)flag = false;break;
			case Float: if(p->paramnode->u.Const.value.f != q->paramnode->u.Const.value.f)flag = false;break;
			case Double:if(p->paramnode->u.Const.value.d != q->paramnode->u.Const.value.d)flag = false;break;
			default: otherType = true;break;
			}
		}
		if(otherType)assert(("Unexpected constant type", FALSE));
		if(!flag)break;
		p = p->next;
		q = q->next;
	}
	//再判断常数组是否相等
	if (flag)
	{
		constArrayList *al1 = node1->contents->ArrayInit,*al2 = node2->contents->ArrayInit;
		while(al1 != NULL)
		{
			List *l1 = al1->arraynode->u.initializer.exprs,*l2 = al2->arraynode->u.initializer.exprs;
			while(l1 != NULL)
			{
				if(FirstItem(l1) != FirstItem(l2))
				{
					flag = false;break;
				}
				l1 = GetNextList(l1);
				l2 = GetNextList(l2);
			}
			if(!flag)break;
			al1 = al1->next;
			al2 = al2->next;
		}
	}
	return flag;
}
bool TemplateClass::DataTypeSame(FlatNode *actorA,FlatNode *actorB)
{
	bool Flag = false;
	if (actorA->inFlatNodes.size() != 0 && actorB->inFlatNodes.size() != 0 && actorA->outFlatNodes.size() != 0 && actorB->outFlatNodes.size() != 0)
	{
		if(pEdgeInfo->getEdgeInfo(actorA->inFlatNodes[0],actorA).typeName == pEdgeInfo->getEdgeInfo(actorB->inFlatNodes[0],actorB).typeName && pEdgeInfo->getEdgeInfo(actorA,actorA->outFlatNodes[0]).typeName == pEdgeInfo->getEdgeInfo(actorB,actorB->outFlatNodes[0]).typeName)

			Flag = true;
	}
	return Flag;
}
bool TemplateClass::StorageValueSame(FlatNode *actorA,FlatNode *actorB)
{
	bool Flag = false;
	if (actorA->inFlatNodes.size() != 0 && actorB->inFlatNodes.size() != 0 && actorA->outFlatNodes.size() != 0 && actorB->outFlatNodes.size() != 0)
	{
		int InputValueA = 0,OutputValueA = 0,InputValueB = 0,OutputValueB = 0;
		for (int i = 0; i < actorA->inPopWeights.size(); i++)
			InputValueA += (Sssg->GetInitCount(actorA) + Sssg->GetSteadyCount(actorA)*2)*actorA->inPopWeights[i];
		for(int i = 0; i < actorB->inPopWeights.size(); i++)
			InputValueB += (Sssg->GetInitCount(actorB) + Sssg->GetSteadyCount(actorB)*2)*actorB->inPopWeights[i];
		if(InputValueA != InputValueB)
			return Flag;
		for(int i = 0; i < actorA->outPushWeights.size(); i++)
			OutputValueA += (Sssg->GetInitCount(actorA) + Sssg->GetSteadyCount(actorA)*2)*actorA->outPushWeights[i];
		for(int i = 0; i < actorB->outPushWeights.size(); i++)
			OutputValueB += (Sssg->GetInitCount(actorB) + Sssg->GetSteadyCount(actorB)*2)*actorB->outPushWeights[i];
		if(OutputValueA == OutputValueB)
			Flag = true;
	}
	return Flag;
}
bool TemplateClass::InTemplate(FlatNode * node)
{
	if(vTemplateNode.size() == 0)
		return false;
	bool flag = false;
	if (!IsSplitJoinNode(node))
	{
		if (X86Backend)
		{
			for (int i = 0; i < vTemplateNode.size(); i++)
			{
				if(node->PreName == vTemplateNode[i]->PreName && node->contents->paramSize == vTemplateNode[i]->contents->paramSize && DimValueSame(node,vTemplateNode[i]))
				{
					flag = true;
					break;
				}
			}
		}
		else if (GPUBackend)
		{
			for (int i = 0; i < vTemplateNode.size(); i++)
			{
				if(node->PreName == vTemplateNode[i]->PreName && node->contents->paramSize == vTemplateNode[i]->contents->paramSize && DimValueSame(node,vTemplateNode[i]))
				{
					if((node->GPUPart >= GpuNum && vTemplateNode[i]->GPUPart >= GpuNum) || (node->GPUPart < GpuNum && vTemplateNode[i]->GPUPart < GpuNum && DataTypeSame(node,vTemplateNode[i]) && StorageValueSame(node,vTemplateNode[i]) &&((Sssg->IsUpBorder(node) && Sssg->IsUpBorder(vTemplateNode[i])) || (Sssg->IsDownBorder(node) && Sssg->IsDownBorder(vTemplateNode[i])) || (!Sssg->IsUpBorder(node) && !Sssg->IsDownBorder(node) && !Sssg->IsUpBorder(vTemplateNode[i]) && !Sssg->IsDownBorder(vTemplateNode[i])))))
					{
						flag = true;
						break;
					}
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < vTemplateNode.size(); i++)
		{
			if(node->PreName == vTemplateNode[i]->PreName && SameofSplitjoinNode(node,vTemplateNode[i]))
			{
				flag = true;
				break;
			}
		}
	}
	return flag;
}

bool TemplateClass::SameofSplitjoinNode(FlatNode *actorA,FlatNode *actorB)
{
	bool flag = true;
	if (actorA->inFlatNodes.size() != actorB->inFlatNodes.size())
		return false;
	else
	{
		for (int i = 0; i < actorA->inFlatNodes.size(); i++)
		{
			if(actorA->inFlatNodes[i]->PreName != actorB->inFlatNodes[i]->PreName)
			{
				flag = false;
				break;
			}
		}
	}
	if(!flag)
		return false;
	if (actorA->outFlatNodes.size() != actorB->outFlatNodes.size())
		return false;
	else
	{
		for (int i = 0; i < actorB->outFlatNodes.size(); i++)
		{
			if (actorA->outFlatNodes[i]->PreName != actorB->outFlatNodes[i]->PreName)
			{
				flag = false;
				break;
			}
		}
	}
	if(!flag)
		return false;
	return flag;
}

void TemplateClass::SetTemplateNode()
{
	for (int i = 0; i < Sssg->flatNodes.size(); i++)
	{
		if(!InTemplate(Sssg->flatNodes[i]))
		{
			vTemplateNode.push_back(Sssg->flatNodes[i]);
			vTemplateName.push_back(Sssg->flatNodes[i]->PreName);
		}
	}
}