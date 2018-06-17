#include "GPUCodeGenerate.h"

#ifndef WIN32
#include <sys/stat.h> 
#define mkdir(tmp) mkdir(tmp,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

using namespace std;
#define  Default_Repeat_Count 10
static bool isInComma = false;//表示当前处于逗号表达式中
static int flag=0;
static int flag_Global = 0;//标识是否在输出GlobalVarCpp文件，初始化为0
static bool structflag = false;//表示是否在打印struct结构体，用于globalheader中
static bool isInFileReader = false;//表示当前处于FileReader中
static bool isInFileWriter = false;//表示当前处于FileWriter中
static bool existFileWriter = true;//用于标识是否存在FileWriter，默认为存在
//static bool kernelmake = false;//用于标识kernel生成时花括号的产生
map<string,string>mapStruct2Struct2OfGPU;//存放global.h文件中边的typedef映射关系
GPUCodeGenerate::GPUCodeGenerate(SchedulerSSG *Sssg, int ngpu, int buffer_size, const char *currentDir,StageAssignment *psa,HAFLPartition *haflptemp,TemplateClass *tc,string name,DNBPartition *dnbp)
{
	Dnbp = dnbp;
	BenchmarkName = name;
	Tc = tc;
	pSa = psa;
	Haflp=haflptemp;
	sssg_ = Sssg;
	flatNodes_ = Sssg->GetFlatNodes();
	pEdgeInfo = new ActorEdgeInfo(Sssg);
	nGpu_ = ngpu;
	nActors_ = flatNodes_.size();
	vTemplateNode_ = Tc->GetTemplateNode();
	nTemplateNode_ = vTemplateNode_.size();
	vTemplateName_ = Tc->GetTemplateName();
	mapFlatnode2Template_ = Tc->GetFlatnode2Template();
	dir_ = currentDir;
	buffer_size_ = buffer_size;
	int index = 0; 
	int i = 0; 
	while(1) 
	{ 
		string::size_type pos = dir_.find("\\", index); 
		string str1; 
		str1 = dir_.substr(0, pos); 

		if(pos == -1) break; 
#ifdef WIN32
		else if(i > 0) _mkdir(str1.c_str()); 
#else
		else if(i > 0) mkdir(str1.c_str()); 
#endif	
		i++; 
		index = pos + 1; 
	} 
	vector<FlatNode*>::iterator iter1,iter2,iter3;
	multimap<FlatNode*,string>::iterator iter4,iter5;
	for (iter1=flatNodes_.begin();iter1!=flatNodes_.end();++iter1)
	{
		for (iter2=(*iter1)->inFlatNodes.begin();iter2!=(*iter1)->inFlatNodes.end();++iter2)
		{
			string tempstring1=(*iter2)->name+"_"+(*iter1)->name;//得到形如A_B的边名称
			mapActor2InEdge.insert(make_pair((*iter1),tempstring1));
		}
		for (iter3=(*iter1)->outFlatNodes.begin();iter3!=(*iter1)->outFlatNodes.end();++iter3)
		{
			string tempstring2=(*iter1)->name+"_"+(*iter3)->name;
			mapActor2OutEdge.insert(make_pair((*iter1),tempstring2));
		}
	}
	vector<FlatNode*>tempactors;
	vector<FlatNode*>::iterator iter;
	set<int>tempstageset;
	for (int i=0;i<ngpu+1;i++)
	{
		tempstageset.clear();
		tempactors=Haflp->FindNodeInOneDevice(i);
		multimap<int,map<FlatNode*,int> >::iterator iter_datastage;
		map<FlatNode*,int>::iterator iter_submap;
		for (iter=tempactors.begin();iter!=tempactors.end();++iter)
		{
			tempstageset.insert(pSa->FindStage(*iter));
			for (iter_datastage = pSa->datastage.begin();iter_datastage != pSa->datastage.end();++iter_datastage)
			{
				iter_submap = iter_datastage->second.begin();
				if (iter_submap->first->name == (*iter)->name)
				{
					tempstageset.insert(iter_datastage->first);
				}
			}
		}
		mapNum2Stage.insert(make_pair(i,tempstageset));
	}
	/*string tmp = dir_ + "lib\\";
	_mkdir(tmp.c_str()); */

	extractDecl = false;
	isInParam = false;
}


void GPUCodeGenerate::CGrecv(FlatNode *actor, OperatorType ot, string streamName, stringstream &SRbuf
	, stringstream &initPeekBuf, stringstream &popingBuf, stringstream &initWorkBuf)
{
	SRbuf <<"\t// recvBuffer\n";
	for(int i = 0; i < actor->nIn; ++i)
	{
		stringstream recv0, recv1, consumer0, peek0;
		string peekNumber = actor->inPeekString[i];
		string popNumber = actor->inPopString[i];

		recv0 <<"recv_"<<i<<"_0";
		recv1 <<"recv_"<<i<<"_1";
		consumer0 <<"consumer_"<<i;
		peek0 << actor->peekString[i];

		// 接收边
		parameterBuf << "number_" << i << " :Int, " << peekNumber << " :Int, " << popNumber << " :Int, " << "edge_recv_" << i << " :Edge[" << streamName << "], "; 
		thisBuf << "\t\tthis." << peekNumber << " = " << peekNumber <<";\n";
		thisBuf << "\t\tthis." << popNumber << " = " << popNumber <<";\n";

		SRbuf <<"\tval "<<recv0.str()<<" = new Array["<<streamName<<"](0..BUFFER_SIZE,([i]:Point(1)) => null);\n";
		SRbuf <<"\tval "<<recv1.str()<<" = new Array["<<streamName<<"](0..BUFFER_SIZE,([i]:Point(1)) => null);\n";
		SRbuf <<"\tval "<<consumer0.str()<<" :Consumer["<<streamName<<"];\n";
		SRbuf <<"\tval "<<peekNumber<<" :Int;\n";
		SRbuf <<"\tval "<<popNumber<<" :Int;\n";
		SRbuf <<"\tval "<<peek0.str()<<" :Array["<<streamName<<"];\n\n";

		// 在构造函数里对接收边进行初始化（申请空间等）
		thisBuf<<"\t\t"<<consumer0.str()<<" = new Consumer["<<streamName<<"] ("<<"BUFFER_SIZE, "<<"number_"<<i<<", "<<recv0.str()<<", "<<recv1.str()<<", edge_recv_"<<i<<");\n";
		thisBuf<<"\t\t"<<peek0.str()<<" = new Array["<<streamName<<"](0..("<<peekNumber<<"-1),([i]:Point(1)) => null);\n\n";

		initWorkBuf <<"\t\t"<<consumer0.str()<<".init();\n";
		initPeekBuf <<"\t\tfor(var i:Int=0;i<"<<peekNumber<<";i++) "<<peek0.str()<<"(i) = "<<consumer0.str()<<".peek(i);\n";
		popingBuf <<"\t\tfor(var i:Int=0;i<"<<popNumber<<";i++) "<<consumer0.str()<<".pop();\n";
	}
}
void GPUCodeGenerate::CGdeclList(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	List *state = NULL;
	Node *var = NULL, *param = NULL;
	assert(actor);
	buf <<"\t// AST Variables\n";
	state = actor->contents->body->u.operBody.state;

	extractDecl = true;
	declList.str(""); // 清空declList内容
	buf << "\t/* *****composite param***** */\n";
	buf<<"//常量传播消除\n";
	//buf<<"\tint null_2\n";
	buf<< declList.str();
	declList.str(""); // 清空declList内容

	stringstream tmp;
	tmp << declInitList.str();
	declInitList.str(""); // 清空declInitList内容
	declList.str(""); // 清空declList内容
	kernelparam.clear();
	kernelparamtype.clear();
	//chenwenbin将composite中param加入模板类中
	buf<<"\t//将composite中param加入模板类中"<<endl;
	paramList *plist = actor->contents->params;
	for (int i = 0; i < actor->contents->paramSize; i++)
	{
		buf<<"\t"<<GetPrimDataType(plist->paramnode->u.id.value->u.Const.type)<<" ";
		buf<<plist->paramnode->u.id.text<<";\n";
		plist = plist->next;
	}
	SPL2GPU_List(state, 0);
	Actor2KernelPar.insert(make_pair(actor,kernelparam));
	Actor2KernelParType.insert(make_pair(actor,kernelparamtype));
	buf << "\t/* *****logic state***** */\n\t" << declList.str();
	//chenwenbin 增加pop,push数据变量
	buf<<"\t//增加pop,push变量"<<endl;
	int popSize = actor->inFlatNodes.size();
	int pushSize = actor->outFlatNodes.size();
	bool typeFlag = true;
	if (actor->GPUPart >= nGpu_)
	{
		for (int i = 0; i < pushSize; i++)
		{
			if (DataDependence(actor,actor->outFlatNodes[i]))
			{
				if (typeFlag)
				{
					buf<<"\tint ";
					typeFlag = false;
				}
				buf<<"pushValue"<<i;
				if(i == pushSize-1)
					buf<<";"<<endl;
				else
					buf<<",";
			}
		}
		typeFlag = true;
		for (int i = 0; i < popSize; i++)
		{
			if (DataDependence(actor->inFlatNodes[i],actor))
			{
				if (typeFlag)
				{
					buf<<"\tint ";
					typeFlag = false;
				}
				buf<<"popValue"<<i;
				if(i == popSize-1)
					buf<<";"<<endl;
				else
					buf<<",";
			}
		}
	}
	else
	{
		//GPU端每次执行work时读写的数据个数
		if (pushSize > 0)
		{
			buf<<"\tint ";
			for (int i = 0; i < pushSize; i++)
			{
				buf<<"pushValue"<<i;
				if(i == pushSize-1)
					buf<<";"<<endl;
				else
					buf<<",";
			}
		}
		if (popSize > 0)
		{
			buf<<"\tint ";
			for (int i = 0; i < popSize; i++)
			{	
				buf<<"popValue"<<i;
				if(i == popSize-1)
					buf<<";"<<endl;
				else
					buf<<",";
			}
		}
		//cwb GPU端边界节点的pop值,即通信时popToken值
		if (sssg_->IsUpBorder(actor))
		{
			for (int i = 0; i < popSize; i++)
			{
				if (DataDependence(actor->inFlatNodes[i],actor))
				{
					if (typeFlag)
					{
						buf<<"\tint ";
						typeFlag = false;
					}
					buf<<"popComm"<<i;
					if(i == popSize-1)
						buf<<";"<<endl;
					else
						buf<<",";
				}
			}
		}
		typeFlag = true;
		if (sssg_->IsDownBorder(actor))
		{
			for (int i = 0; i < pushSize; i++)
			{
				if (DataDependence(actor,actor->outFlatNodes[i]))
				{
					if (typeFlag)
					{
						buf<<"\tint ";
						typeFlag = false;
					}
					buf<<"pushComm"<<i;
					if(i == pushSize-1)
						buf<<";"<<endl;
					else
						buf<<",";
				}
			}
		}
	}
	extractDecl = false;
	stateInit << declInitList.str();
	declInitList.str(""); // 清空declInitList内容
	//declInitList << tmp.str();
}
void GPUCodeGenerate::CGinitVarAndState(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	buf << "\t// initVarAndState\n";
	buf << "\tvoid initVarAndState() {\n";

	declInitList << "\n";
	buf << declInitList.str();
	buf << "\n\t\t/**** State Init ****/\n";
	buf << stateInit.str();
	buf << "\n\t}\n"; // initVarAndState方法结束

	declInitList.str(""); // 清空
	stateInit.str(""); // 清空
}
void GPUCodeGenerate::CGlogicInit(FlatNode *actor, OperatorType ot, stringstream &buf, stringstream &logicInit)
{
	declInitList.str(""); // 清空
	buf << "\t// init\n";
	buf << "\tvoid init()";
	Node *init = actor->contents->body->u.operBody.init;

	if (init)
	{
		SPL2GPU_Node(init, 2);
		buf << declInitList.str(); // init结构必须带"{}"
	}
	else
	{
		buf << " {\n";
		buf << "\t}\n"; // '}'独占一行
	}


	declInitList.str(""); // 清空
}
void GPUCodeGenerate::CGthis(FlatNode *actor, OperatorType ot, stringstream &buf,string templatename)
{
	buf <<"\t// Constructor\n";
	buf << "\t"<<templatename<<"(" ;
	//chenwenbin 构造param变量
	paramList *plist = actor->contents->params;
	for (int i = 0; i < actor->contents->paramSize; i++)
	{
		//buf<<NodeConstantIntegralValue(plist->paramnode->u.id.value)<<" ";
		buf<<GetPrimDataType(plist->paramnode->u.id.value->u.Const.type)<<" ";
		buf<<plist->paramnode->u.id.text<<", ";
		plist = plist->next;
	}
	if (actor->GPUPart < nGpu_)
	{
		for (int i = 0; i < actor->inFlatNodes.size(); i++)
		{
			buf<<"int pop"<<i<<",";
		}
		for (int i = 0; i < actor->outFlatNodes.size(); i++)
		{
			buf<<"int push"<<i<<",";
		}
	}
	else
	{
		for (int i = 0; i < actor->inFlatNodes.size(); i++)
		{
			if(DataDependence(actor->inFlatNodes[i],actor))
				buf<<"int pop"<<i<<",";
		}
		for (int i = 0; i < actor->outFlatNodes.size(); i++)
		{
			if(DataDependence(actor,actor->outFlatNodes[i]))
				buf<<"int push"<<i<<",";
		}
	}
	//chenwenbin 初态稳态执行次数
	buf<<"int steadyCount,int initCount,";
	multimap<FlatNode*,string>::iterator iter1,iter2;
	List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	iter1 = mapActor2InEdge.find(actor);
	int index = 0;
	IterateList(&input_maker,inputList);
	while(NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		assert(iter1!=mapActor2InEdge.end()&&inputNode);
		string inputString;
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		if((actor->GPUPart >= nGpu_ || (actor->GPUPart < nGpu_ && sssg_->IsUpBorder(actor)))&& DataDependence(actor->inFlatNodes[index],actor))
			buf<<"Buffer<U>& "<<inputString<<",";
		edge2name.insert(make_pair(iter1->second,inputString));
		++iter1;
		++index;
	}
	iter2=mapActor2OutEdge.find(actor);
	index = 0;
	IterateList(&output_maker,outputList);
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		assert(iter2!=mapActor2OutEdge.end()&&outputNode);
		string outputString;
		if(outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		if((actor->GPUPart >= nGpu_ || (actor->GPUPart < nGpu_ && sssg_->IsDownBorder(actor)))&& DataDependence(actor,actor->outFlatNodes[index]))
			buf<<"Buffer<T>& "<<outputString<<",";
		edge2name.insert(make_pair(iter2->second,outputString));
		++iter2;
		++index;
	}
	//构造GPU存储空间
	if(actor->GPUPart < nGpu_)
	{
		vector<FlatNode*>::iterator iter,iter_1;
		for (int i = 0; i < nGpu_; i++)
		{
			for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
			{
				if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter)))
				{
					buf<<"cl::Buffer Inbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<",";
				}
			}
			for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
			{
				if(actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter))
				{
					buf<<"cl::Buffer Outbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<",";
				}
			}
			for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
			{	
				if((actor->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter_1)))
				{
					buf<<"cl::Buffer Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<",";
				}
			}
			for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
			{
				if(actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter_1))
				{
					buf<<"cl::Buffer Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<",";	
				}
			}
		}
	}
	buf<<"void *lazy = NULL):";
	//cwb
	if(actor->GPUPart < nGpu_)
	{
		//cwb
		vector<FlatNode*>::iterator iter,iter_1;
		for (int i = 0; i < nGpu_; i++)
		{
			for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
			{
				if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter)))
				{
					buf<<"Inbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<"(Inbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<"),";
				}
			}
			for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
			{
				if(actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter))
				{
					buf<<"Outbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<"(Outbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<"),";
				}
			}
			for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
			{	
				if((actor->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter_1)))
				{
					buf<<"Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<"(Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<"),";
				}
			}
			for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
			{
				if(actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter_1))
				{
					buf<<"Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<"(Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<"),";	
				}
			}
		}
	}
	index = 0;
	iter1=mapActor2InEdge.find(actor);
	iter2=mapActor2OutEdge.find(actor);
	inputNode = NULL;
	outputNode = NULL;
	IterateList(&input_maker,inputList);
	while(NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		assert(iter1!=mapActor2InEdge.end()&&inputNode);
		string inputString;
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		if((actor->GPUPart >= nGpu_ || (actor->GPUPart < nGpu_ && sssg_->IsUpBorder(actor)))&& DataDependence(actor->inFlatNodes[index],actor))
			buf<<inputString<<"("<<inputString<<"),";
		++iter1;
		++index;
	}
	index = 0;
	IterateList(&output_maker,outputList);
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		assert(iter2!=mapActor2OutEdge.end()&&outputNode);
		string outputString;
		if(outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		if((actor->GPUPart >= nGpu_ || (actor->GPUPart < nGpu_ && sssg_->IsDownBorder(actor)))&& DataDependence(actor,actor->outFlatNodes[index]))
			buf<<outputString<<"("<<outputString<<"),";
		++iter2;
		++index;
	}
	plist = actor->contents->params;
	for (int i = 0; i < actor->contents->paramSize; i++)
	{
		buf<<plist->paramnode->u.id.text<<"("<<plist->paramnode->u.id.text<<"),";
		plist = plist->next;
	}
	buf.seekp((int)buf.tellp()-1);
	buf<<"{\n";
	buf << thisBuf.str();
	//cwb
	if (actor->GPUPart < nGpu_)
	{
		for (int i = 0; i < actor->inFlatNodes.size(); i++)
		{
			buf<<"\t\tpopValue"<<i<<" = pop"<<i<<";"<<endl;
		}
		for (int i = 0; i < actor->outFlatNodes.size(); i++)
		{
			buf<<"\t\tpushValue"<<i<<" = push"<<i<<";"<<endl;
		}
	}
	else
	{
		for (int i = 0; i < actor->inFlatNodes.size(); i++)
		{
			if(DataDependence(actor->inFlatNodes[i],actor))
				buf<<"\t\tpopValue"<<i<<" = pop"<<i<<";"<<endl;
		}
		for (int i = 0; i < actor->outFlatNodes.size(); i++)
		{
			if(DataDependence(actor,actor->outFlatNodes[i]))
				buf<<"\t\tpushValue"<<i<<" = push"<<i<<";"<<endl;
		}
	}
	buf<< "\t\tsteadyScheduleCount = steadyCount;\n";
	buf<< "\t\tinitScheduleCount = initCount;\n";
	CGdatataginit(actor,buf);
	buf << "\t}\n"; // init方法结束
}

void GPUCodeGenerate::CGflush(stringstream &buf, string pFlush)
{
	buf << "\t// flush\n";
	buf << "\tdef flush() {\n";

	buf << pFlush;

	buf << "\t}\n"; // flush方法结束
}

void GPUCodeGenerate::CGinitWork(stringstream &buf, string initWorkBuf)
{
	buf << "\t// initWork\n";
	buf << "\tvoid initWork() {\n";
	buf << "\t\tinitVarAndState();\n";
	buf << "\t\tinit();\n";

	buf << initWorkBuf;

	buf << "\t}\n"; // initWork方法结束
}

void GPUCodeGenerate::CGinitPeek(stringstream &buf, string initPeekBuf)
{
	buf <<"\t// initPeek\n";
	buf << "\tdef initPeek() {\n";

	buf << initPeekBuf;

	buf << "\t}\n"; // initPeek方法结束
}

void GPUCodeGenerate::CGinitPush(stringstream &buf, string initPushBuf)
{
	buf <<"\t// initPush\n";
	buf << "\tdef initPush() {\n";

	buf << initPushBuf;

	buf << "\t}\n"; // initPush方法结束
}

void GPUCodeGenerate::CGpopToken(FlatNode *actor,stringstream &buf, string popingBuf)
{
	List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	IterateList(&input_maker,inputList);
	map<FlatNode*,FlatNode*>tempinedgename;
	vector<int>::iterator iter=actor->inPopWeights.begin();
	vector<FlatNode*>inflatnodes = actor->inFlatNodes;
	vector<FlatNode*>::iterator iter_inflatnodes = inflatnodes.begin();
	if(ExistDependence(actor))
	{
		buf << "\t// popToken\n";
		buf << "\tvoid popToken() {";
	}
	int index = 0;
	while(NextOnList(&input_maker,(GenericREF)&inputNode)&&iter_inflatnodes!=inflatnodes.end())
	{
		string inputString;
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		if(DataDependence(*iter_inflatnodes,actor))
			buf<<"\n\t"<<inputString<<".updatehead"<<"(popValue"<<index<<");\n";
		tempinedgename.clear();
		tempinedgename.insert(make_pair(*iter_inflatnodes,actor));
		BufferNameOfEdge.insert(make_pair(tempinedgename,inputString));
		iter++;
		iter_inflatnodes++;
		index++;
	}
	if (ExistDependence(actor))
	{
		buf << popingBuf;
		buf << "\t}\n"; // popToken方法结束
	}
}
void GPUCodeGenerate::CGpushToken(FlatNode *actor,stringstream &buf, string pushingBuf)
{
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker output_maker;
	Node *outputNode = NULL;
	IterateList(&output_maker,outputList);
	vector<int>::iterator iter=actor->outPushWeights.begin();
	vector<FlatNode*>outflatnodes = actor->outFlatNodes;
	vector<FlatNode*>::iterator iter_outflatnodes = outflatnodes.begin();
	map<FlatNode*,FlatNode*>tempoutedgename;
	bool flag = false;
	for (;iter_outflatnodes != outflatnodes.end(); ++iter_outflatnodes)
	{
		if(ExistDependence(*iter_outflatnodes))
		{
			flag = true;
			break;
		}
	}
	if (flag)
	{
		buf << "\t// pushToken\n";
		buf << "\tvoid pushToken() {";
	}
	iter_outflatnodes = outflatnodes.begin();
	int index = 0;
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		string outputString;
		if(outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		if(DataDependence(actor,*iter_outflatnodes) && flag)
			buf<<"\n\t"<<outputString<<".updatetail"<<"(pushValue"<<index<<");\n";
		tempoutedgename.clear();
		tempoutedgename.insert(make_pair(actor,*iter_outflatnodes));
		BufferNameOfEdge.insert(make_pair(tempoutedgename,outputString));
		iter++;
		iter_outflatnodes++;
		index++;
	}
	if (flag)
	{
		buf << pushingBuf;
		buf << "\t}\n"; // pushToken方法结束
	}
}
void GPUCodeGenerate::CGpopTokenForGPU(FlatNode *actor,stringstream &buf, string popingBuf)
{
	if (sssg_->IsUpBorder(actor) && ExistDependence(actor))
	{
		buf << "\t// popToken\n";
		buf << "\tvoid popToken() {\n\t";
	}
	stringstream steadybuf;
	List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	IterateList(&input_maker,inputList);
	vector<int>::iterator iter=actor->inPopWeights.begin();
	vector<FlatNode*>inflatnodes = actor->inFlatNodes;
	vector<FlatNode*>::iterator iter_inflatnodes = inflatnodes.begin();
	map<FlatNode*,FlatNode*>tempinedgename;
	int index = 0;
	while(NextOnList(&input_maker,(GenericREF)&inputNode)&&iter_inflatnodes!=inflatnodes.end())
	{
		string inputString;
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		if (sssg_->IsUpBorder(actor) && ExistDependence(actor))
		{
			buf<<"\t"<<inputString<<".updatehead"<<"(popComm"<<index<<");\n";
			//buf<<"\n\t\t"<<inputString<<".updatehead"<<"("<<sssg_->GetInitCount(*iter_inflatnodes)*ReturnPushWeight(*iter_inflatnodes,actor)<<");\n";
			//steadybuf<<"\n\t\t"<<inputString<<".updatehead"<<"("
			//steadybuf<<"\n\t\t"<<inputString<<".updatehead"<<"("<<sssg_->GetSteadyCount(*iter_inflatnodes)*Maflp->MultiNum2FlatNode[actor]*ReturnPushWeight(*iter_inflatnodes,actor)<<"/com_num);\n";
		}
		tempinedgename.clear();
		tempinedgename.insert(make_pair(*iter_inflatnodes,actor));
		BufferNameOfEdge.insert(make_pair(tempinedgename,inputString));
		iter++;
		iter_inflatnodes++;
		index++;
	}
	if (sssg_->IsUpBorder(actor) && ExistDependence(actor))
	{
		buf << popingBuf;
		//buf<<"\t}";
		/*buf<<"\n\telse\n\t{";
		buf<<steadybuf.str();
		buf<<"\n\t}\n";*/
		buf << "\t}\n"; // popToken方法结束
	}
}
void GPUCodeGenerate::CGpushTokenForGPU(FlatNode *actor,stringstream &buf, string pushingBuf)
{
	stringstream steadybuf;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker output_maker;
	Node *outputNode = NULL;
	IterateList(&output_maker,outputList);
	vector<int>::iterator iter=actor->outPushWeights.begin();
	vector<FlatNode*>outflatnodes = actor->outFlatNodes;
	vector<FlatNode*>::iterator iter_outflatnodes = outflatnodes.begin();
	map<FlatNode*,FlatNode*>tempoutedgename;
	bool flag = false;
	for (; iter_outflatnodes != outflatnodes.end(); ++iter_outflatnodes)
	{
		if(ExistDependence(*iter_outflatnodes))
		{
			flag = true;
			break;
		}
	}
	if (sssg_->IsDownBorder(actor) && flag)
	{
		buf << "\t// pushToken\n";
		buf << "\tvoid pushToken() {\n\t";
	}
	iter_outflatnodes = outflatnodes.begin();
	int index = 0;
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		string outputString;
		if(outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		if (sssg_->IsDownBorder(actor) && flag)
		{
			buf<<"\t"<<outputString<<".updatetail"<<"(pushComm"<<index<<");\n";
			//buf<<"\n\t"<<outputString<<".updatetail"<<"("<<(*iter)*sssg_->GetInitCount(actor)<<");\n";
			//steadybuf<<"\n\t"<<outputString<<".updatetail"<<"("<<(*iter)*sssg_->GetSteadyCount(actor)*Maflp->MultiNum2FlatNode[actor]<<"/com_num);\n";
		}
		tempoutedgename.clear();
		tempoutedgename.insert(make_pair(actor,*iter_outflatnodes));
		BufferNameOfEdge.insert(make_pair(tempoutedgename,outputString));
		iter++;
		iter_outflatnodes++;
		index++;
	}
	if (sssg_->IsDownBorder(actor) && flag)
	{
		buf << pushingBuf;
		buf << "\t}\n"; // pushToken方法结束
	}
}
void GPUCodeGenerate::CGpushToken(stringstream &buf, string pushingBuf)
{
	buf << "\t// pushToken\n";
	buf << "\tdef pushToken() {\n";

	buf << pushingBuf;

	buf << "\t}\n"; // pushToken方法结束
}
void GPUCodeGenerate::CGrun(stringstream &buf, string initFun)
{
	buf <<"\t// run\n";
	buf << "\tpublic void run() {\n";//run方法,类中最主要的方法

	buf << "\t\t" << initFun << ";\n"; // 调用init方法

	buf << strScheduler.str();

	buf << "\t}\n"; // run方法结束
}
void GPUCodeGenerate::CGrunInitScheduleWork(FlatNode *actor,stringstream &buf)
{
	buf <<"\t// runInitScheduleWork\n";
	buf << "\tvoid runInitScheduleWork(";
	//multimap<FlatNode*,string>::iterator iter1,iter2;
	List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	string inputString;
	string outputString;
	//iter1=mapActor2InEdge.find(actor);
	int index = 0;
	IterateList(&input_maker,inputList);
	while(NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		if(!DataDependence(actor->inFlatNodes[index],actor))
			buf<<pEdgeInfo->getEdgeInfo(actor->inFlatNodes[index],actor).typeName<<" *"<<inputString<<",";
		++index;
	//	++iter1;
	}
	index = 0;
	//iter2=mapActor2OutEdge.find(actor);
	IterateList(&output_maker,outputList);
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		if(outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		if(!DataDependence(actor,actor->outFlatNodes[index]))
			buf<<pEdgeInfo->getEdgeInfo(actor,actor->outFlatNodes[index]).typeName<<" *"<<outputString<<",";
		++index;
		//++iter2;
	}
	buf<<"void * p = NULL){\n";
	buf << "\t\tinitWork();\n"; // 调用initwork方法
	buf<<"\t\tfor(int i=0;i<initScheduleCount;i++)\n\t\t\twork(";
	//iter1=mapActor2InEdge.find(actor);
	index = 0;
	/*int totalinpopweight = 0;
	if (actor->inFlatNodes.size() != 0)
	{
		for (int i = 0; i < actor->inFlatNodes.size(); ++i)
		{
			totalinpopweight += actor->inPopWeights[i];
		}
	}*/
	IterateList(&input_maker,inputList);
	while(NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		if(!DataDependence(actor->inFlatNodes[index],actor))
			buf<<inputString<<"+i*"<<actor->inPopWeights[index]<<",";
		++index;
	}
	//iter2=mapActor2OutEdge.find(actor);
	/*int totaloutpopweight = 0;
	if (actor->outFlatNodes.size() != 0)
	{
		for (int i = 0; i < actor->outFlatNodes.size(); ++i)
		{
			totaloutpopweight += actor->outPushWeights[i];
		}
	}*/
	index = 0;
	IterateList(&output_maker,outputList);
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		if(!DataDependence(actor,actor->outFlatNodes[index]))
			buf<<outputString<<"+i*"<<actor->outPushWeights[index]<<",";
		++index;
	}
	buf<<"NULL);\n";
	buf << "\t}\n"; // CGrunInitScheduleWork方法结束
}
void GPUCodeGenerate::CGrunInitScheduleWorkforGPU(FlatNode *actor,stringstream &buf)
{
	buf <<"\t// runInitScheduleWork\n";
	for (int i = 0; i < nGpu_; i++)
	{
		buf << "\tvoid runInitScheduleWork_"<<i<<"() {\n";
		buf << "\t\tinitWork();\n"; // 调用initwork方法
		buf<<"\t\tif(initScheduleCount)\n";
		buf<<"\t\t\twork_"<<i<<"("<<sssg_->GetInitCount(actor)<<");\n";
		buf << "\t}\n"; // CGrunInitScheduleWork方法结束
	}
}
void GPUCodeGenerate::CGrunSteadyScheduleWork(FlatNode *actor,stringstream &buf)
{
	buf <<"\t// CGrunSteadyScheduleWork\n";
	buf << "\tvoid runSteadyScheduleWork(";
	//multimap<FlatNode*,string>::iterator iter1,iter2;
	List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	string inputString;
	string outputString;
	//iter1=mapActor2InEdge.find(actor);
	int index = 0;
	IterateList(&input_maker,inputList);
	while(NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		if(!DataDependence(actor->inFlatNodes[index],actor))
			buf<<pEdgeInfo->getEdgeInfo(actor->inFlatNodes[index],actor).typeName<<" *"<<inputString<<",";
		++index;
	//	++iter1;
	}
	index = 0;
	//iter2=mapActor2OutEdge.find(actor);
	IterateList(&output_maker,outputList);
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		if(outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		if(!DataDependence(actor,actor->outFlatNodes[index]))
			buf<<pEdgeInfo->getEdgeInfo(actor,actor->outFlatNodes[index]).typeName<<" *"<<outputString<<",";
		++index;
	//	++iter2;
	}
	buf<<"void * p = NULL){\n";
	buf<<"\t\tfor(int i=0;i<steadyScheduleCount;i++)\n\t\t\twork(";
	//iter1=mapActor2InEdge.find(actor);
	/*int totalinpopweight = 0;
	if (actor->inFlatNodes.size() != 0)
	{
		for (int i = 0; i < actor->inFlatNodes.size(); ++i)
		{
			totalinpopweight += actor->inPopWeights[i];
		}
	}*/
	index = 0;
	IterateList(&input_maker,inputList);
	while(NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		if(!DataDependence(actor->inFlatNodes[index],actor))
			buf<<inputString<<"+i*"<<actor->inPopWeights[index]<<",";
		++index;
	}
	//iter2=mapActor2OutEdge.find(actor);
	index = 0;
	/*int totaloutpopweight = 0;
	if (actor->outFlatNodes.size() != 0)
	{
		for (int i = 0; i < actor->outFlatNodes.size(); ++i)
		{
			totaloutpopweight += actor->outPushWeights[i];
		}
	}*/
	IterateList(&output_maker,outputList);
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		if(!DataDependence(actor,actor->outFlatNodes[index]))
			buf<<outputString<<"+i*"<<actor->outPushWeights[index]<<",";
		++index;
	}
	buf<<"NULL);\n";
	buf << "\t}\n"; // CGrunSteadyScheduleWork方法结束
}
void GPUCodeGenerate::CGrunSteadyScheduleWorkforGPU(FlatNode *actor,stringstream &buf)
{
	buf <<"\t// CGrunSteadyScheduleWork\n";
	for (int i = 0; i < nGpu_; i++)
	{
		buf << "\tvoid runSteadyScheduleWork_"<<i<<"() {\n";
		buf<<"\t\twork_"<<i<<"("<<sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor]<<");\n";
		buf << "\t}\n"; // CGrunSteadyScheduleWork方法结束
	}
}
void GPUCodeGenerate::CGEdgeParam(FlatNode *actor,stringstream &buf)
{
	buf<<"\t//edge param\n\t";
	multimap<FlatNode*,string>::iterator iter1,iter2;
	List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	int index = 0;
	iter1=mapActor2InEdge.find(actor);
	IterateList(&input_maker,inputList);
	while(NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		assert(iter1!=mapActor2InEdge.end()&&inputNode);
		string inputString;
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		if(DataDependence(actor->inFlatNodes[index],actor) && (actor->GPUPart >= nGpu_ || (actor->GPUPart < nGpu_ && actor->inFlatNodes[index]->GPUPart >= nGpu_)))
			buf<<"\tConsumer<U> "<<inputString<<";\n";
			//buf<<"Consumer<mystruct_x>"<<inputString<<";\n";
		++iter1;
		++index;
	}
	iter2=mapActor2OutEdge.find(actor);
	index = 0;
	IterateList(&output_maker,outputList);
	vector<string> vecOutEdgeName;
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		assert(iter2!=mapActor2OutEdge.end()&&outputNode);
		string outputString;
		if(outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		if(DataDependence(actor,actor->outFlatNodes[index]) && (actor->outFlatNodes[index]->GPUPart >= nGpu_ || (actor->outFlatNodes[index]->GPUPart < nGpu_ && actor->GPUPart >= nGpu_)))
			buf<<"\tProducer<T> "<<outputString<<";\n";
			//buf<<"Producer<mystruct_x>"<<outputString<<";\n";
		++iter2;
		++index;
		vecOutEdgeName.push_back(outputString);
	}
	//mapOperator2OutEdgeName.insert(make_pair(actor,vecOutEdgeName));
}
void GPUCodeGenerate::CGGlobalvar()
{
	stringstream buf;
	// 对全局变量进行代码生成
	flag_Global=1;
	SPL2GPU_List(gDeclList, 1);
	buf << declInitList.str() << "\n\n";
	declInitList.str("");
	flag_Global=0;
	//输出到文件
	stringstream ss;
	ss<<dir_<<"GlobalVar.cpp";
	OutputToFile(ss.str(),  buf.str());
}
void GPUCodeGenerate::CGGlobalvarextern()
{
	stringstream ss;
	globalvariablefinal = globalvariable;
	globalvartypefinal = globalvartype;
	array2Dnamefinal = array2Dname;
	array1Dnamefinal = array1Dname;
	ss<<dir_<<"GlobalVar.h";
	OutputToFile(ss.str(),globalvarbuf.str());
}
void GPUCodeGenerate::CGdatatag(FlatNode *actor,stringstream &buf)
{
	vector<FlatNode*>::iterator iter,iter_1;
	buf<<"\t//读写的标志位\n";
	for (int i = 0; i < nGpu_; i++)
	{
		for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
		{
			//if (!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3)&&(Maflp->UporDownStatelessNode(*iter)!=3)&&!DetectiveFilterState(*iter)&&Maflp->findPartitionNumForFlatNode(actor)==Maflp->findPartitionNumForFlatNode(*iter))
			if(actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter))
			{
				buf<<"\tint readtag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<";\n";
			}
			//if ((!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3)&&(DetectiveFilterState(*iter)||(Maflp->UporDownStatelessNode(*iter)==3)))||(!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3)&&(Maflp->UporDownStatelessNode(*iter)!=3)&&!DetectiveFilterState(*iter)&&Maflp->findPartitionNumForFlatNode(actor)!=Maflp->findPartitionNumForFlatNode(*iter)))
			if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter)))
			{
				buf<<"\tint readtag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<";\n";
				buf<<"\tint writedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<";\n";
			}
		}
		for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
		{
			//if (!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3)&&(Maflp->UporDownStatelessNode(*iter_1)!=3)&&!DetectiveFilterState(*iter_1)&&Maflp->findPartitionNumForFlatNode(actor)==Maflp->findPartitionNumForFlatNode(*iter_1))
			if(actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter_1))
			{
				buf<<"\tint writetag_"<<(actor)->name<<"_"<<(*iter_1)->name<<"_"<<i<<";\n";
			}
			if((actor->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter_1)))
			{
				buf<<"\tint writetag_"<<(actor)->name<<"_"<<(*iter_1)->name<<"_"<<i<<";\n";
				buf<<"\tint readdatatag_"<<(actor)->name<<"_"<<(*iter_1)->name<<"_"<<i<<";\n";
			}
		}
	}
	
}
void GPUCodeGenerate::CGdatataginit(FlatNode *actor,stringstream &buf)
{
	vector<FlatNode*>::iterator iter,iter_1;
	buf<<"\t//读写标志位的初始化\n";
	for (int i = 0;i < nGpu_; i++)
	{
		for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
		{
			//if (!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3)&&(Maflp->UporDownStatelessNode(*iter)!=3)&&!DetectiveFilterState(*iter)&&Maflp->findPartitionNumForFlatNode(actor)==Maflp->findPartitionNumForFlatNode(*iter))
			if(actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter))
			{
				buf<<"\t\treadtag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<" = 0;\n";
			}
			//if (!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3)&&(Maflp->UporDownStatelessNode(*iter)!=3)&&!DetectiveFilterState(*iter)&&Maflp->findPartitionNumForFlatNode(actor)!=Maflp->findPartitionNumForFlatNode(*iter))
			if(actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter))
			{
				buf<<"\t\treadtag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<" = 0;\n";
				buf<<"\t\twritedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<" = 0;\n";
			}
			//if (!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3)&&(DetectiveFilterState(*iter)||(Maflp->UporDownStatelessNode(*iter)==3)))
			if(actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_)
			{
				buf<<"\t\treadtag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<" = 0;\n";
				buf<<"\t\twritedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<" = 0;\n";
			}
		}
		for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
		{
			if(actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter_1))
			{
				buf<<"\t\twritetag_"<<(actor)->name<<"_"<<(*iter_1)->name<<"_"<<i<<" = 0;\n";
			}
			if(actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter_1))
			{
				buf<<"\t\twritetag_"<<(actor)->name<<"_"<<(*iter_1)->name<<"_"<<i<<" = 0;\n";
				buf<<"\t\treaddatatag_"<<(actor)->name<<"_"<<(*iter_1)->name<<"_"<<i<<" = 0;\n";
			}
			if(actor->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_)
			{
				buf<<"\t\twritetag_"<<(actor)->name<<"_"<<(*iter_1)->name<<"_"<<i<<" = 0;\n";
				buf<<"\t\treaddatatag_"<<(actor)->name<<"_"<<(*iter_1)->name<<"_"<<i<<" = 0;\n";
			}
		}
	}
	
}
void GPUCodeGenerate::CGexternbuffer(FlatNode *actor,stringstream &buf)
{
	vector<FlatNode*>::iterator iter,iter_1;
	buf<<"extern int com_num;\n";
	buf<<"extern vector<cl::Platform>platforms;\n";
	buf<<"extern vector<cl::Device>platformsDevices;\n";
	buf<<"extern cl::Context context;\n";
	for (int i = 0;i < nGpu_;i++)
	{
		buf<<"extern cl::CommandQueue queue_"<<i+1<<"_0;\n";
		buf<<"extern cl::CommandQueue queue_"<<i+1<<"_1;\n";
	}
	buf<<"extern cl::Program program;\n";
}
string GPUCodeGenerate::FindNumofNode(FlatNode *actor)
{
	int pos = actor->name.find('_');
	return actor->name.substr(pos+1);
}
int GPUCodeGenerate::ReturnBufferSize(FlatNode *actora,FlatNode *actorb)//a->b
{
	vector<FlatNode *>::iterator iter_nodes;
	vector<int>::iterator iter_push = actora->outPushWeights.begin();
	int stageminus,size;
	for (iter_nodes = actora->outFlatNodes.begin();iter_nodes != actora->outFlatNodes.end();++iter_nodes)
	{
		if ((*iter_nodes)->name == actorb->name)
		{
			stageminus = pSa->FindStage(actorb) - pSa->FindStage(actora);
			if(actora->GPUPart < nGpu_ && actorb->GPUPart >= nGpu_)
				size=(sssg_->GetInitCount(actora)+sssg_->GetSteadyCount(actora)*Haflp->MultiNum2FlatNode[actora]*stageminus)*(*iter_push);
			else if(actora->GPUPart >= nGpu_ && actorb->GPUPart < nGpu_)
				size=(sssg_->GetInitCount(actora)+sssg_->GetSteadyCount(actora)*Haflp->MultiNum2FlatNode[actora]/nGpu_*stageminus)*(*iter_push);
			else
				size=(sssg_->GetInitCount(actora)+sssg_->GetSteadyCount(actora)*Haflp->MultiNum2FlatNode[actora]*(stageminus+1))*(*iter_push);
			break;
		}
		iter_push++;
	}
	return size;
}

void GPUCodeGenerate::CGAllKernel()
{
	stringstream buf;
	buf<<"#pragma OPENCL EXTENSION cl_khr_fp64 : enable;\n\n";
	map<string,string>::iterator iter_nametype;
	for (iter_nametype = allnameandtype.begin();iter_nametype != allnameandtype.end();++iter_nametype)
	{
		buf<<"typedef struct\n";
		buf<<"{\n";
		buf<<"\t"<<iter_nametype->first<<" "<<iter_nametype->second<<";\n";
		buf<<"}"<<iter_nametype->first<<"_x;\n";
	}
	int stageminus,size;
	vector<FlatNode*>::iterator iter,iter_1;
	vector<int>::iterator iter_2;
	for (iter_1 = vTemplateNode_.begin();iter_1 != vTemplateNode_.end();iter_1++)
	{
		if((*iter_1)->GPUPart < nGpu_)
		{
			buf<<"__kernel void kernel"<<mapFlatnode2Template_[*iter_1]<<"(";
			map<map<FlatNode*,FlatNode*>,string>::iterator iter_buffername;
			map<FlatNode*,FlatNode*>searchmap;
			for (iter=(*iter_1)->inFlatNodes.begin();iter!=(*iter_1)->inFlatNodes.end();iter++)
			{
				if((*iter)->GPUPart >= nGpu_)
				{
					searchmap.clear();
					searchmap.insert(make_pair(*iter,*iter_1));
					iter_buffername = BufferNameOfEdge.find(searchmap);
					buf<<"__global "<<pEdgeInfo->getEdgeInfo((*iter),(*iter_1)).typeName<<" *"<<iter_buffername->second<<", ";
					buf<<"int readtag_"<<(*iter)->name<<"_"<<(*iter_1)->name<<", ";
				}
				if((*iter)->GPUPart < nGpu_)
				{
					searchmap.clear();
					searchmap.insert(make_pair(*iter,*iter_1));
					iter_buffername = BufferNameOfEdge.find(searchmap);
					buf<<"__global "<<pEdgeInfo->getEdgeInfo((*iter),(*iter_1)).typeName<<" *"<<iter_buffername->second<<", ";
					buf<<"int readtag_"<<(*iter)->name<<"_"<<(*iter_1)->name<<", ";
				}
			}
			for (iter=(*iter_1)->outFlatNodes.begin();iter!=(*iter_1)->outFlatNodes.end();iter++)
			{	
				if((*iter)->GPUPart >= nGpu_)
				{
					searchmap.clear();
					searchmap.insert(make_pair(*iter_1,*iter));
					iter_buffername = BufferNameOfEdge.find(searchmap);
					buf<<"__global "<<pEdgeInfo->getEdgeInfo((*iter_1),(*iter)).typeName<<" *"<<iter_buffername->second<<", ";
					buf<<"int writetag_"<<(*iter_1)->name<<"_"<<(*iter)->name<<", ";
				}
				if((*iter)->GPUPart < nGpu_)
				{
					searchmap.clear();
					searchmap.insert(make_pair(*iter_1,*iter));
					iter_buffername = BufferNameOfEdge.find(searchmap);
					buf<<"__global "<<pEdgeInfo->getEdgeInfo((*iter_1),(*iter)).typeName<<" *";
					buf<<iter_buffername->second<<", ";
					buf<<"int writetag_"<<(*iter_1)->name<<"_"<<(*iter)->name<<", ";
				}
			}
			//chenwenbin 将param变量加入到kernel函数中
			paramList *plist = (*iter_1)->contents->params;
			for (int i = 0; i < (*iter_1)->contents->paramSize; i++)
			{
				buf<<GetPrimDataType(plist->paramnode->u.id.value->u.Const.type)<<" ";
				buf<<plist->paramnode->u.id.text<<", ";
				plist = plist->next;
			}
			map<FlatNode *,vector<string> >::iterator iter_param,iter_paramtype;
			vector<string>paramofactor,paramtypeofactor;
			vector<string>::iterator iter_paofac,iter_patyofac;
			iter_param = Actor2KernelPar.find((*iter_1));
			iter_paramtype = Actor2KernelParType.find(*iter_1);
			paramofactor = iter_param->second;
			paramtypeofactor = iter_paramtype->second;
			for (iter_paofac = paramofactor.begin(),iter_patyofac = paramtypeofactor.begin();(iter_paofac != paramofactor.end())&&(iter_patyofac != paramtypeofactor.end());iter_paofac++,iter_patyofac++)
			{
				buf<<(*iter_patyofac)<<" "<<(*iter_paofac)<<", ";
			}
			pair<multimap<FlatNode*,map<string,string> >::iterator,multimap<FlatNode*,map<string,string> >::iterator>pos_ptr = actor2ptr.equal_range(*iter_1);
			map<string,string>::iterator iter_ptr;
			while (pos_ptr.first != pos_ptr.second)
			{
				iter_ptr = pos_ptr.first->second.begin();
				buf<<"__global "<<(iter_ptr)->first<<" *"<<(iter_ptr)->second<<", ";
				pos_ptr.first++;
			}
			declInitList.str("");
			Node *work = (*iter_1)->contents->body->u.operBody.work;
			SPL2GPU_Node(work, 2);
			string kernelstring;
			if ((*iter_1)->outFlatNodes.size())
			{
				kernelstring = declInitList.str();
			}
			//cout<<kernelstring<<endl;
			//////////////////////////////////////////////////////////////////////////
			//全局变量以及静态变量参数
			int num = allstaticvariable.size();
			staticvariable.clear();
			staticvariablefinal.clear();
			map<string,map<string,string> >tempmapofstatic,tempmapofstaticfinal;
			pair<multimap<FlatNode *,map<string,map<string,string> > >::iterator,multimap<FlatNode *,map<string,map<string,string> > >::iterator>pos_static;
			map<string,map<string,string> >::iterator iter_substatic;
			pos_static = allstaticvariable.equal_range(*iter_1);
			while (pos_static.first != pos_static.second)
			{
				for(iter_substatic = pos_static.first->second.begin();iter_substatic != pos_static.first->second.end();++iter_substatic)
				{
					tempmapofstatic.clear();
					tempmapofstatic.insert(make_pair(iter_substatic->first,iter_substatic->second));
					staticvariable.push_back(tempmapofstatic);
					tempmapofstaticfinal.clear();
					tempmapofstaticfinal.insert(make_pair(iter_substatic->first,iter_substatic->second));
					staticvariablefinal.push_back(tempmapofstaticfinal);
				}
				pos_static.first++;
			}
			pair<multimap<FlatNode *,vector<string> >::iterator,multimap<FlatNode *,vector<string> >::iterator>pos_statictype;
			vector<string>::iterator iter_type;
			pos_statictype = allstatictype.equal_range(*iter_1);
			while (pos_statictype.first != pos_statictype.second)
			{
				for (iter_type = pos_statictype.first->second.begin();iter_type != pos_statictype.first->second.end();++iter_type)
				{
					staticvartype.push_back(*iter_type);
					staticvartypefinal.push_back(*iter_type);
				}
				pos_statictype.first++;
			}
			map<string,map<string,string> >::iterator iter_staticvar;
			map<string,string>::iterator iter_submapofstatic;
			vector<string>::iterator iter_typeofstatic = staticvartypefinal.begin();
			map<string,map<string,string> >::iterator iter_globalvar;
			map<string,string>::iterator iter_submapofglobal;
			vector<string>::iterator iter_typeofglobal = globalvartypefinal.begin();
			vector<map<string,map<string,string> > >::iterator iter_vecofstatic,iter_vecofglobal;
			for (iter_vecofstatic = staticvariablefinal.begin();iter_vecofstatic != staticvariablefinal.end();++iter_vecofstatic)
			{
				iter_staticvar = (*iter_vecofstatic).begin();
				{
					if (kernelstring.find(iter_staticvar->first) != -1)
					{
						if (array2Dname.count(iter_staticvar->first))//如果改变量是二维数组，则需要对其进行改造，变成一维数组
						{
							buf<<"__global "<<*iter_typeofstatic<<" *"<<iter_staticvar->first<<", ";
						}
						else if (array1Dname.count(iter_staticvar->first))//如果改变量是一维数组，则直接封装成buffer
						{
							buf<<"__global "<<*iter_typeofstatic<<" *"<<iter_staticvar->first<<", ";
						}
						else
						{
							buf<<"__global "<<*iter_typeofstatic<<" "<<iter_staticvar->first<<", ";
						}
					}
				}

			}
			for (iter_vecofglobal = globalvariablefinal.begin();iter_vecofglobal != globalvariablefinal.end();++iter_vecofglobal)
			{
				for (iter_globalvar = (*iter_vecofglobal).begin();iter_globalvar != (*iter_vecofglobal).end();iter_globalvar++,iter_typeofglobal++)
				{
					int pos;
					int startpos = 0;
					while((pos = kernelstring.find(iter_globalvar->first,startpos))!=-1)
					{
						if (!((((int)kernelstring[pos+(iter_globalvar->first).length()])>=65&&((int)kernelstring[pos+(iter_globalvar->first).length()])<=90)||(((int)kernelstring[pos+(iter_globalvar->first).length()])>=97&&((int)kernelstring[pos+(iter_globalvar->first).length()])<=122))&&!((((int)kernelstring[pos-1])>=65&&((int)kernelstring[pos-1])<=90)||(((int)kernelstring[pos-1])>=97&&((int)kernelstring[pos-1])<=122)))
						{
							if (array2Dname.count(iter_globalvar->first))//如果改变量是二维数组，则需要对其进行改造，变成一维数组
							{
								buf<<"__global "<<*iter_typeofglobal<<" *"<<iter_globalvar->first<<", ";
							}
							else if (array1Dname.count(iter_globalvar->first))//如果改变量是一维数组，则直接封装成buffer
							{
								buf<<"__global "<<*iter_typeofglobal<<" *"<<iter_globalvar->first<<", ";
							}
							/*else
							{
								buf<<"__global "<<*iter_typeofglobal<<" "<<iter_globalvar->first<<", ";
							}*/
						}
						startpos = pos+1;
					}
					
				}
			}
			//////////////////////////////////////////////////////////////////////////
			buf.seekp((int)buf.tellp()-2);
			buf<<")\n";
			buf<<"{\n";
			if ((*iter_1)->outFlatNodes.size())
			{
				buf<<"\tint id = get_global_id(0);\n";
			}
			//////////////////////////////////////////////////////////////////////////
			//字符串的处理
			vector<int>::iterator iter_pop,iter_push;//分别用来迭代actor的pop和push值
			iter_pop = (*iter_1)->inPopWeights.begin();
			iter_push = (*iter_1)->outPushWeights.begin();
			for (iter=(*iter_1)->inFlatNodes.begin();iter!=(*iter_1)->inFlatNodes.end();iter++)
			{
				searchmap.clear();
				searchmap.insert(make_pair(*iter,*iter_1));
				iter_buffername = BufferNameOfEdge.find(searchmap);
				int pos1,pos2,pos3;
				pos2 = 0;
				stringstream ss1,ss2;
				string buffersizestring;
				string popnumstring;
				string searchstring = iter_buffername->second+"[";
				while ((pos2!=-1)&&((pos1 = kernelstring.find(searchstring,pos2)) != -1))
				{
					if (pos1 != 0 &&!(((kernelstring[pos1-1]>=65)&&(kernelstring[pos1-1]<=90))||((kernelstring[pos1-1]>=97)&&(kernelstring[pos1-1]<=122))))
					{
						string substring;
						//处理括号匹配的问题
						stack<string>stk;
						pos2 = kernelstring.find("[",pos1);
						stk.push("[");
						int posstart = pos2+1;
						int poshere;
						string findstring = "[]";
						while ((poshere = kernelstring.find_first_of(findstring,posstart))!=-1)
						{
							if (kernelstring[poshere] == '[')
							{
								stk.push("[");
							}
							else
							{
								stk.pop();
							}
							if (stk.size() == 0)
							{
								break;
							}
							posstart = poshere+1;;
						}
						//	cout<<kernelstring[poshere]<<endl;
						pos3 = poshere;
						substring = kernelstring.substr(pos2+1,pos3-pos2-1);
						substring = "(" +substring+")"+"+readtag_"+(*iter)->name+"_"+(*iter_1)->name;
						int buffersize;
						if (ReturnBufferSize(*iter,*iter_1) == 0)
						{
							buffersize = (ReturnBufferSize(*iter,*iter_1)+1);
						}
						else
						{
							buffersize = ReturnBufferSize(*iter,*iter_1);
						}
						if (!IsMod)
						{
							buffersize = ReturnNumBiger(buffersize);
						}
						ss1<<buffersize;
						ss1>>buffersizestring;	
						ss2<<*iter_pop;
						popnumstring=ss2.str();
						if (IsMod)
						{
							substring = "(" +substring+"+id*"+popnumstring+")%"+buffersizestring;
						}
						else
						{
							substring = "(" +substring+"+id*"+popnumstring+")&("+buffersizestring+"-1)";
						}
						kernelstring.replace(pos2+1,pos3-pos2-1,substring);
						ss1.str("");
						ss2.str("");
						pos2 = pos3;
					}
					else if(pos1 == 0)
					{
						if (!(((kernelstring[pos1+iter_buffername->second.length()]>=65)&&(kernelstring[pos1+iter_buffername->second.length()]<=90))||((kernelstring[pos1+iter_buffername->second.length()]>=97)&&(kernelstring[pos1+iter_buffername->second.length()]<=122))))
						{
							string substring;
							//处理括号匹配的问题
							stack<string>stk;
							pos2 = kernelstring.find("[",pos1);
							stk.push("[");
							int posstart = pos2+1;
							int poshere;
							string findstring = "[]";
							while ((poshere = kernelstring.find_first_of(findstring,posstart))!=-1)
							{
								if (kernelstring[poshere] == '[')
								{
									stk.push("[");
								}
								else
								{
									stk.pop();
								}
								if (stk.size() == 0)
								{
									break;
								}
								posstart = poshere+1;;
							}
							//	cout<<kernelstring[poshere]<<endl;
							pos3 = poshere;
							substring = kernelstring.substr(pos2+1,pos3-pos2-1);
							substring = "(" +substring+")"+"+readtag_"+(*iter)->name+"_"+(*iter_1)->name;
							int buffersize;
							if (ReturnBufferSize(*iter,*iter_1) == 0)
							{
								buffersize = ReturnBufferSize(*iter,*iter_1)+1;
							}
							else
							{
								buffersize = ReturnBufferSize(*iter,*iter_1);
							}
							if (!IsMod)
							{
								buffersize = ReturnNumBiger(buffersize);
							}
							ss1<<buffersize;
							ss1>>buffersizestring;	
							ss2<<*iter_pop;
							popnumstring=ss2.str();
							if (IsMod)
							{
								substring = "(" +substring+"+id*"+popnumstring+")%"+buffersizestring;
							}
							else
							{
								substring = "(" +substring+"+id*"+popnumstring+")&("+buffersizestring+"-1)";
							}
							kernelstring.replace(pos2+1,pos3-pos2-1,substring);
							ss1.str("");
							ss2.str("");
							pos2 = pos3;
						}
					}
					else
					{
						pos2 = pos1+1;
					}
				}
				iter_pop++;
			}
			//cout<<"---------------------"<<endl;
			//cout<<kernelstring<<endl;
			for (iter=(*iter_1)->outFlatNodes.begin();iter!=(*iter_1)->outFlatNodes.end();iter++)
			{	
				searchmap.clear();
				searchmap.insert(make_pair(*iter_1,*iter));
				iter_buffername = BufferNameOfEdge.find(searchmap);
				int pos1,pos2,pos3;
				pos2 = 0;
				stringstream ss1,ss2;
				string buffersizestring;
				string pushnumstring;
				string searchstring = iter_buffername->second+"[";
				while ((pos1 = kernelstring.find(searchstring,pos2)) != -1)
				{
					if (pos1 != 0 &&!(((kernelstring[pos1-1]>=65)&&(kernelstring[pos1-1]<=90))||((kernelstring[pos1-1]>=97)&&(kernelstring[pos1-1]<=122))))
					{
						string substring;
						stack<string>stk;
						/*cout<<kernelstring<<endl;*/
						pos2 = kernelstring.find("[",pos1);
						stk.push("[");
						int posstart = pos2+1;
						int poshere;
						string findstring = "[]";
						while ((poshere = kernelstring.find_first_of(findstring,posstart))!=-1)
						{
							if (kernelstring[poshere] == '[')
							{
								stk.push("[");
							}
							else
							{
								stk.pop();
							}
							if (stk.size() == 0)
							{
								break;
							}
							posstart = poshere+1;;
						}
						pos3 = poshere;
						substring = kernelstring.substr(pos2+1,pos3-pos2-1);
						substring = "(" +substring+")"+"+writetag_"+(*iter_1)->name+"_"+(*iter)->name;
						int buffersize;
						if (ReturnBufferSize(*iter_1,*iter) == 0)
						{
							buffersize = ReturnBufferSize(*iter_1,*iter)+1;
						}
						else
						{
							buffersize = ReturnBufferSize(*iter_1,*iter);
						}
						if (!IsMod)
						{
							buffersize = ReturnNumBiger(buffersize);
						}
						//	ss1<<ReturnBufferSize(*iter_1,*iter);
						ss1<<buffersize;
						ss1>>buffersizestring;
						ss2<<(*iter_push);
						ss2>>pushnumstring;
						if (IsMod)
						{
							substring = "(" +substring+"+id*"+pushnumstring+")%"+buffersizestring;
						}
						else
						{
							substring = "(" +substring+"+id*"+pushnumstring+")&("+buffersizestring+"-1)";
						}
						kernelstring.replace(pos2+1,pos3-pos2-1,substring);
						ss1.str("");
						ss2.str("");
						pos2 = pos3;
					}
					else if(pos1 == 0)
					{
						if (!(((kernelstring[pos1+iter_buffername->second.length()]>=65)&&(kernelstring[pos1+iter_buffername->second.length()]<=90))||((kernelstring[pos1+iter_buffername->second.length()]>=97)&&(kernelstring[pos1+iter_buffername->second.length()]<=122))))
						{
							string substring;
							stack<string>stk;
							/*cout<<kernelstring<<endl;*/
							pos2 = kernelstring.find("[",pos1);
							stk.push("[");
							int posstart = pos2+1;
							int poshere;
							string findstring = "[]";
							while ((poshere = kernelstring.find_first_of(findstring,posstart))!=-1)
							{
								if (kernelstring[poshere] == '[')
								{
									stk.push("[");
								}
								else
								{
									stk.pop();
								}
								if (stk.size() == 0)
								{
									break;
								}
								posstart = poshere+1;;
							}
							pos3 = poshere;
							substring = kernelstring.substr(pos2+1,pos3-pos2-1);
							substring = "(" +substring+")"+"+writetag_"+(*iter_1)->name+"_"+(*iter)->name;
							int buffersize;
							if (ReturnBufferSize(*iter_1,*iter) == 0)
							{
								buffersize = ReturnBufferSize(*iter_1,*iter)+1;
							}
							else
							{
								buffersize = ReturnBufferSize(*iter_1,*iter);
							}
							if (!IsMod)
							{
								buffersize = ReturnNumBiger(buffersize);
							}
							//	ss1<<ReturnBufferSize(*iter_1,*iter);
							ss1<<buffersize;
							ss1>>buffersizestring;
							ss2<<(*iter_push);
							ss2>>pushnumstring;
							if (IsMod)
							{
								substring = "(" +substring+"+id*"+pushnumstring+")%"+buffersizestring;
							}
							else
							{
								substring = "(" +substring+"+id*"+pushnumstring+")&("+buffersizestring+"-1)";
							}
							kernelstring.replace(pos2+1,pos3-pos2-1,substring);
							ss1.str("");
							ss2.str("");
							pos2 = pos3;
						}
					}
					else
					{
						pos2 = pos1 + 1;
					}
				}
				iter_push++;
			}
			//////////////////////////////////////////////////////////////////////////
			//对kernel函数中的二维全局变量以及二维静态变量进行下标的修改，使之变为一维数组
			set<string>::iterator iter_2Darray;
			map<string,map<string,string> >::iterator iter_2Dimarray;
			map<string,string>::iterator iter_submap;
			string searchstring;
			for (iter_2Darray = array2Dname.begin();iter_2Darray != array2Dname.end();iter_2Darray++)
			{
				int pos1 = 0;
				int pos2,pos3,pos4,pos5,pos6;
				string num1string,num2string;
				searchstring = *iter_2Darray + "[";
				while(kernelstring.find(searchstring,pos1) != -1)
				{
					stringstream ss1,ss2;
					string substring;
					bool flag_global = false;
					bool flag_static = false;
					pos1 = kernelstring.find(searchstring,pos1);
					pos2 = kernelstring.find(*iter_2Darray,pos1);
					pos3 = kernelstring.find("[",pos2);
					pos4 = kernelstring.find("]",pos3);
					pos5 = kernelstring.find("[",pos4);
					pos6 = kernelstring.find("]",pos5);
					num1string = kernelstring.substr(pos3+1,pos4-pos3-1);
					num2string = kernelstring.substr(pos5+1,pos6-pos5-1);
					vector<map<string,map<string,string> > >::iterator iter_vecofglobal;
					map<string,map<string,string> >::iterator iter_subvecofglobal;
					for (iter_vecofglobal = globalvariablefinal.begin();iter_vecofglobal != globalvariablefinal.end();++iter_vecofglobal)
					{
						iter_subvecofglobal = (*iter_vecofglobal).begin();
						if (iter_subvecofglobal->first == (*iter_2Darray))
						{
							iter_2Dimarray = iter_subvecofglobal;
							flag_global = true;
							break;
						}
					}
					if (flag_global)
					{
						iter_submap = iter_2Dimarray->second.begin();
						substring = num1string+"*"+iter_submap->second+"+"+num2string;
						kernelstring.replace(pos3+1,pos6-pos3-1,substring);
						pos1 = kernelstring.rfind(substring) + 1;
					}
					else
					{
						vector<map<string,map<string,string> > >::iterator iter_vecofstatic;
						map<string,map<string,string> >::iterator iter_subvecofstatic;
						for (iter_vecofstatic = staticvariablefinal.begin();iter_vecofstatic != staticvariablefinal.end();iter_vecofstatic++)
						{
							iter_subvecofstatic = (*iter_vecofstatic).begin();
							if (iter_subvecofstatic->first == (*iter_2Darray))
							{
								iter_2Dimarray = iter_subvecofstatic;
								flag_static = true;
								break;
							}
						}
					}
					if (flag_static)
					{
						iter_submap = iter_2Dimarray->second.begin();
						substring = num1string+"*"+iter_submap->second+"+"+num2string;
						kernelstring.replace(pos3+1,pos6-pos3-1,substring);
						pos1 = kernelstring.rfind(substring) + 1;
					}
					/*cout<<kernelstring<<endl;*/
				}
			}
			/*cout<<"---------------"<<endl;
			cout<<kernelstring<<endl;*/
			//////////////////////////////////////////////////////////////////////////
			//对kernel中的局部二维数组的访问进行修改
			pair<multimap<FlatNode *,map<string,map<string,string> > >::iterator,multimap<FlatNode *,map<string,map<string,string> > >::iterator>pos_local;
			pos_local = alllocalvariable.equal_range(*iter_1);
			map<string,map<string,string> > localsubmap;
			map<string,map<string,string> >::iterator iter_local;
			while (pos_local.first != pos_local.second)
			{
				for (iter_local = pos_local.first->second.begin();iter_local != pos_local.first->second.end();iter_local++)
				{
					localsubmap.insert(make_pair(iter_local->first,iter_local->second));
				}
				pos_local.first++;
			}
			for (iter_2Darray = arraylocal2Dname.begin();iter_2Darray != arraylocal2Dname.end();iter_2Darray++)
			{
				int pos1 = 0;
				int pos2,pos3,pos4,pos5,pos6;
				string num1string,num2string;
				searchstring = *iter_2Darray + "[";
				while(kernelstring.find(searchstring,pos1) != -1)
				{
					stringstream ss1,ss2;
					string substring;
					pos1 = kernelstring.find(searchstring,pos1);
					pos2 = kernelstring.find(*iter_2Darray,pos1);
					pos3 = kernelstring.find("[",pos2);
					pos4 = kernelstring.find("]",pos3);
					pos5 = kernelstring.find("[",pos4);
					pos6 = kernelstring.find("]",pos5);
					num1string = kernelstring.substr(pos3+1,pos4-pos3-1);
					num2string = kernelstring.substr(pos5+1,pos6-pos5-1);
					iter_2Dimarray = localsubmap.find(*iter_2Darray);
					iter_submap = iter_2Dimarray->second.begin();
					substring = num1string+"*"+iter_submap->second+"+"+num2string;
					kernelstring.replace(pos3+1,pos6-pos3-1,substring);
					pos1 = kernelstring.rfind(substring) + 1;
					/*cout<<kernelstring<<endl;*/
				}
			}
			//////////////////////////////////////////////////////////////////////////
			buf << kernelstring;
			declInitList.str(""); // 清空
			buf<<"}\n";
		}
	}
	stringstream fileName;
	fileName<<dir_<<"allkernel.cl";
	OutputToFile(fileName.str(),buf.str());
}
void GPUCodeGenerate::CGCreateBuffer(FlatNode *actor,stringstream &buf)
{
	vector<FlatNode *>::iterator iter,iter_1;
	for (int i = 0; i < nGpu_; i++)
	{
		for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
		{
			if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter)))
			{
				buf<<"\tcl::Buffer Inbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<";\n";
			}
		}
		for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
		{
			if(actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter))
			{
				buf<<"\tcl::Buffer Outbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<";\n";
			}
		}
		for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
		{	
			if((actor->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter_1)))
			{
				buf<<"\tcl::Buffer Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<";\n";
			}
		}
		for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
		{
			if(actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter_1))
			{
				buf<<"\tcl::Buffer Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<";\n";	
			}
		}
	}
}
void GPUCodeGenerate::CGactor(FlatNode *actor,string templatename, OperatorType ot)
{
	string classNmae = templatename;
	stringstream buf, SRbuf, pFlush, pushingBuf, popingBuf,buf1,buf2;
	stringstream initPushBuf, initPeekBuf, initWorkBuf;
	stringstream logicInitBuf, workBuf ;
	vector<string>::iterator iter;
	map<string,string>tempprt;
	vector<string>::iterator iter_ptrtype,iter_ptrname;
	string streamName = "Token_Temp";
	curactor=actor;
	assert(actor);
	//cout<<actor->name<<"       "<<endl;
	actor->SetIOStreams();	
	buf <<"/**\n * Class "<<classNmae<<"\n */\n";
	//buf <<"import lib.*;\n"; // 相关库文件
	buf<<"#include \"Buffer.h\"\n";
	buf<<"#include \"Consumer.h\"\n";
	buf<<"#include \"Producer.h\"\n";
	buf<<"#include \"global.h\"\n";
	//buf<<"#include \"math.h\"\n";
	buf<<"#include \"iostream\"\n";
	buf<<"#include \"GlobalVar.h\"\n";
	buf<<"using namespace std;\n";
	//if (!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3))
	if(actor->GPUPart < nGpu_)//cwb
	{
		CGexternbuffer(actor,buf);
	}
	ptrname.clear();
	ptrtype.clear();
	ptrdim.clear();
	//buf<<"\tinclude \""<<classNmae<<".h\"\n";
	if((actor->inFlatNodes.size() == 0 && actor->outFlatNodes.size() != 0))
		buf<<"template<typename T>\n";
	else if((actor->inFlatNodes.size() != 0 && actor->outFlatNodes.size() == 0))
		buf<<"template<typename U>\n";
	else if(actor->inFlatNodes.size() != 0 && actor->outFlatNodes.size() != 0)
		buf<<"template<typename T,typename U>\n";
	buf <<"class "<<classNmae<<"{\n"; // 类块开始
	// 写入发送、接收信息
	parameterBuf.str(""); // 清空parameterBuf内容
	thisBuf.str(""); // 清空thisBuf内容
	// 接收边相关
	//CGrecv(actor, ot, streamName, SRbuf, initPeekBuf, popingBuf, initWorkBuf);
	// 发送边相关
	//CGsend(actor, ot, streamName, SRbuf, initPushBuf, pushingBuf, pFlush, initWorkBuf);
	buf << SRbuf.str();
	// 写入循环次数信息
	//buf << "\tint RepeatCount;\n";
	buf<<"private:\n";
	//cwb 模板中GPU存储空间
	CGCreateBuffer(actor,buf);
	//写入读写标志位
	CGdatatag(actor,buf);
	//写入边的私有变量
	CGEdgeParam(actor,buf);
	// 写入语法树成员变量信息（param, var, state）
	CGdeclList(actor, ot, buf);
	// 写入 var, state 的初始化信息
	CGinitVarAndState(actor, ot, buf);
	// 写入 init 的初始化信息(考虑到init可以定义局部变量，所以不与上部分合并)
	CGlogicInit(actor, ot, buf, logicInitBuf);
	// 写入popToken函数
	//if (DetectiveFilterState(actor)||(Maflp->UporDownStatelessNode(actor)==3)) //cwb
	if(actor->GPUPart >= nGpu_)
	{
		CGpopToken(actor,buf, popingBuf.str());
		CGpushToken(actor,buf,popingBuf.str());
	}
	else
	{
		CGpopTokenForGPU(actor,buf,popingBuf.str());
		CGpushTokenForGPU(actor,buf,popingBuf.str());
	}
	buf<<"public:\n\t";
	// 写入构造函数信息
	buf<<"\tint steadyScheduleCount;\t//稳态时一次迭代的执行次数\n";
	buf<<"\tint initScheduleCount;\n";
	CGthis(actor, ot, buf,classNmae);
	// 写入成员函数信息
	buf <<"\t// ------------Member Mehods------------\n";
	// 写入flush函数
	//CGflush(buf, pFlush.str());
	// 写入initWork函数
	CGinitWork(buf, initWorkBuf.str());
	// 写入initPeek函数
	//CGinitPeek(buf, initPeekBuf.str());
	// 写入initPush函数
	//CGinitPush(buf, initPushBuf.str());
	// 写入pushToken函数
	//CGpushToken(buf, pushingBuf.str());
	// 写入work函数
	CGwork(actor, ot, buf);
	//为分配在GPU上的actor写入数据传输函数(输入到GPU缓冲区中)
	//if (!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3))//cwb
	if(actor->GPUPart < nGpu_)
	{
		bool inactorofcpu = false;
		vector<FlatNode*>::iterator iter;
		for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
		{
			//if ((!DetectiveFilterState(actor)&&(DetectiveFilterState(*iter)||Maflp->UporDownStatelessNode(actor)==3))||((!DetectiveFilterState(actor)&&!DetectiveFilterState(*iter)&&(Maflp->UporDownStatelessNode(*iter)!=3)&&Maflp->findPartitionNumForFlatNode(actor)!=Maflp->findPartitionNumForFlatNode(*iter))))
			if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter)))
			{
				inactorofcpu = true;
			}
		}		
		if (inactorofcpu)
		{
			CGdataGetforGPU(actor,buf);
		}
	}
	//为分配在GPU上的actor写入数据传输函数(输出到CPU缓冲区中)
	//if (!DetectiveFilterState(actor)&&(Maflp->UporDownStatelessNode(actor)!=3))//cwb
	if(actor->GPUPart < nGpu_)
	{
		bool outactorofcpu = false;
		vector<FlatNode*>::iterator iter;
		for (iter=actor->outFlatNodes.begin();iter!=actor->outFlatNodes.end();iter++)
		{
			//if ((!DetectiveFilterState(actor)&&(DetectiveFilterState(*iter)||Maflp->UporDownStatelessNode(actor)==3))||(!DetectiveFilterState(actor)&&!DetectiveFilterState(*iter)&&(Maflp->UporDownStatelessNode(*iter)!=3)&&Maflp->findPartitionNumForFlatNode(actor)!=Maflp->findPartitionNumForFlatNode(*iter)))
			if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter)))
			{
				outactorofcpu = true;
			}
		}		
		if (outactorofcpu)
		{
			CGdataSendforGPU(actor,buf);
		}
	}
	//打印输出结果cwb

	if (actor->outFlatNodes.size() ==0 && PrintFlag && actor->GPUPart < nGpu_)
	{
		for (int i = 0; i < nGpu_; i++)
		{
			buf<<"\tvoid print_"<<i<<"(int count)\n\t{\n";
			int queuenum = i+1;
			vector<FlatNode*>::iterator iter = actor->inFlatNodes.begin();
			vector<int>::iterator iter_pop = actor->inPopWeights.begin();
			buf<<"\t\t"<<pEdgeInfo->getEdgeInfo((*iter),actor).typeName<<" *Outst = new "<<pEdgeInfo->getEdgeInfo((*iter),actor).typeName<<"[count*"<<*iter_pop<<"];\n";
			buf<<"\t\tqueue_"<<queuenum<<"_1.enqueueReadBuffer(Outbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<queuenum-1<<",CL_TRUE,0,count*"<<*iter_pop<<"*sizeof("<<pEdgeInfo->getEdgeInfo((*iter),actor).typeName<<"),Outst,NULL,NULL);\n";
			buf<<"\t\t\tfor(int i = 0;i < count*"<<*iter_pop<<";i++)\n";
			buf<<"\t\t\t\tcout<<Outst[i].x<<endl;\n";
			buf<<"\t\tdelete [] Outst;\n\t}\n";
		}
	}
	// 写入run函数
	//CGrun(buf, "initWork()");
	//写入runInitScheduleWork函数
	//写入runSteadyScheduleWork函数
	//if (DetectiveFilterState(actor)||(Maflp->UporDownStatelessNode(actor)==3))
	if(actor->GPUPart >= nGpu_)
	{
		CGrunInitScheduleWork(actor,buf);
		CGrunSteadyScheduleWork(actor,buf);
	}
	else
	{
		CGrunInitScheduleWorkforGPU(actor,buf);
		CGrunSteadyScheduleWorkforGPU(actor,buf);
	}
	buf <<"};";//类块结束
	buf1.str("");
	buf2.str("");
	buf2<<"#include \""<<actor->name<<".h\"\n";
	for (iter=staticname2value.begin();iter!=staticname2value.end();++iter)
	{
		buf1<<(*iter);
	}
	//输出到文件
	stringstream fileName;
	fileName<<dir_<<classNmae<<".h";
	//fileName1<<dir_<<classNmae<<".cpp";
	//fileName<<dir_<<"ActorClass.cpp";
	if (staticname2value.size()!=0)
	{
		buf<<"\n"<<buf1.str();
		//OutputToFile(fileName1.str(),buf2.str());
	}
	buf<<endl;
	OutputToFile(fileName.str(),buf.str());
	for (iter_ptrtype = ptrtype.begin(),iter_ptrname = ptrname.begin();(iter_ptrtype != ptrtype.end())&&(iter_ptrname != ptrname.end());++iter_ptrtype,++iter_ptrname)
	{
		tempprt.clear();
		tempprt.insert(make_pair((*iter_ptrtype),(*iter_ptrname)));
		actor2ptr.insert(make_pair(actor,tempprt));
	}
	staticname2value.clear();
	staticvariable.clear();
	staticvartype.clear();
}
void GPUCodeGenerate::CGwork(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	//if (DetectiveFilterState(actor)||(Maflp->UporDownStatelessNode(actor)==3))
	if (actor->GPUPart >= nGpu_)
	{
		buf << "\t// work\n";
		buf << "\tvoid work(";
		vector<string>::iterator iter;
		multimap<FlatNode*,string>::iterator iter1,iter2;
		Node *work = actor->contents->body->u.operBody.work;
		stringstream tmpBuf;
		string actorname = actor->name;
		List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
		List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
		ListMarker input_maker;
		ListMarker output_maker;
		Node *inputNode = NULL;
		Node *outputNode = NULL;
		string inputString;
		string outputString;
		//iter1=mapActor2InEdge.find(actor);
		int index = 0;
		IterateList(&input_maker,inputList);
		while(NextOnList(&input_maker,(GenericREF)&inputNode))
		{
			if(inputNode->typ == Id)inputString = inputNode->u.id.text;
			else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
			if(!DataDependence(actor->inFlatNodes[index],actor))
				buf<<pEdgeInfo->getEdgeInfo(actor->inFlatNodes[index],actor).typeName<<" *"<<inputString<<",";
			++index;
		//	++iter1;
		}
		index = 0;
	//	iter2=mapActor2OutEdge.find(actor);
		IterateList(&output_maker,outputList);
		while(NextOnList(&output_maker,(GenericREF)&outputNode))
		{
			if(outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
			if(!DataDependence(actor,actor->outFlatNodes[index]))
				buf<<pEdgeInfo->getEdgeInfo(actor,actor->outFlatNodes[index]).typeName<<" *"<<outputString<<",";
			++index;
		//	++iter2;
		}
		buf<<"void* p = NULL){\n\t";
		if(actorname.find("FileW",0)!=-1)
			buf<<"\t\t{\n\t\toutstream<<"<<inputString<<"[0];\n\t\t}\n";
		else if(actorname.find("FileR",0)!=-1)
			buf<<"\t\t{\n\t\tinstream>>"<<outputString<<"[0];\n\t\t}\n";
		else
			SPL2GPU_Node(work, 2);
		buf << declInitList.str();
		if(ExistDependence(actor))
			buf<<"\n\tpopToken();\n";
		bool flag = false;
		for (index = 0; index < actor->outFlatNodes.size(); ++index)
		{
			if(ExistDependence(actor->outFlatNodes[index]))
			{
				flag = true;
				break;
			}
		}
		if(flag)
			buf<<"\tpushToken();\n";
		buf<<"\t}\n";
		declInitList.str(""); // 清空
	}
	else
	{
		for (int i = 0; i < nGpu_; i++)
		{
			staticvariablefinal = staticvariable;
			staticvartypefinal = staticvartype;
			buf << "\t// work\n";
			buf << "\tvoid work_"<<i<<"(int count) \n\t{\n";
			string num;
			int pos = actor->name.find('_');
			num = actor->name.substr(pos+1);
			int argnum = 0;//kernel函数的参数个数
			buf<<"\t\tcl::Kernel kernel(program,\"kernel"<<mapFlatnode2Template_[actor]<<"\");\n";
			vector<FlatNode*>::iterator iter,iter_1;
			map<FlatNode*,vector<string> >::iterator iterofOut,iterofIn;
			vector<string>curkerpar,othkerpar,tempstring;
			vector<string>::iterator cur_iter,oth_iter;
			for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
			{
				//if ((!DetectiveFilterState(actor)&&DetectiveFilterState(*iter))||(!DetectiveFilterState(actor)&&!DetectiveFilterState(*iter)&&Maflp->findPartitionNumForFlatNode(actor)!=Maflp->findPartitionNumForFlatNode(*iter)))
				if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter)))
				{
					buf<<"\t\tkernel.setArg("<<argnum++<<",Inbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<");\n";
					buf<<"\t\tkernel.setArg("<<argnum++<<",readtag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<");\n";//buf<<"\t\tkernel.setArg("<<argnum++<<",writedatatag_"<<(*iter)->name<<"_"<<actor->name<<");\n";
				}
				if(actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter))
				{
					buf<<"\t\tkernel.setArg("<<argnum++<<","<<"Outbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<");\n";
					buf<<"\t\tkernel.setArg("<<argnum++<<",readtag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<");\n";
					//}
				}
			}
			for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
			{	
				if((actor->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter_1)))
				{
					buf<<"\t\tkernel.setArg("<<argnum++<<",Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<");\n";
					buf<<"\t\tkernel.setArg("<<argnum++<<",writetag_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<");\n";
				}
				tempstring.clear();
				//if (!DetectiveFilterState(actor)&&!DetectiveFilterState(*iter_1)&&Maflp->findPartitionNumForFlatNode(actor)==Maflp->findPartitionNumForFlatNode(*iter_1))
				if(actor->GPUPart < nGpu_ && (*iter_1)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)==Haflp->findPartitionNumForFlatNode(*iter_1))
				{
					buf<<"\t\tkernel.setArg("<<argnum++<<",Outbuffer_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<");\n";
					buf<<"\t\tkernel.setArg("<<argnum++<<",writetag_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<");\n";
					tempstring.push_back("Outbuffer_"+(*iter_1)->name);
				}
				OutbufferOfActor.insert(make_pair(actor,tempstring));
			}
			//chenwenbin 将param变量传给kernel函数
			paramList *pList = actor->contents->params;
			for (int i = 0; i < actor->contents->paramSize; i++)
			{
				buf<<"\t\tkernel.setArg("<<argnum++<<","<<pList->paramnode->u.id.text<<");\n";
				pList = pList->next;
			}
			map<FlatNode *,vector<string> >::iterator iter_param;
			vector<string>paramofactor;
			vector<string>::iterator iter_paofac;
			iter_param = Actor2KernelPar.find(actor);
			paramofactor = iter_param->second;
			vector<string>::iterator iter_prname;
			//////////////////////////////////////////////////////////////////////////
			//此处需要将数组封装成为内存对象
			for (iter_paofac = paramofactor.begin();iter_paofac != paramofactor.end();iter_paofac++)
			{
				buf<<"\t\tkernel.setArg("<<argnum++<<","<<(*iter_paofac)<<");\n";
			}
			vector<string>::iterator iter_ptrtype = ptrtype.begin();
			multimap<string,string>::iterator iter_ptrdim = ptrdim.begin();
			for (iter_prname = ptrname.begin();iter_prname != ptrname.end();++iter_prname)
			{
				if (iter_ptrdim->second == "0")
				{
					buf<<"\t\tcl::Buffer "<<(*iter_prname)<<"_buffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,"<<(iter_ptrdim->first)<<"*sizeof("<<(*iter_ptrtype)<<"),"<<(*iter_prname)<<",NULL);\n";
					buf<<"\t\tkernel.setArg("<<argnum++<<","<<(*iter_prname)<<"_buffer);\n";
				}
				else
				{
					//////////////////////////////////////////////////////////////////////////
					//如果是二维数组则需要转化为一维数组
					buf<<"\t\t"<<(*iter_ptrtype)<<" "<<(*iter_prname)<<"_new["<<(iter_ptrdim->first)<<"*"<<(iter_ptrdim->second)<<"];\n";
					buf<<"\t\tfor(int i = 0;i < "<<iter_ptrdim->first<<";i++)\n";
					buf<<"\t\t{\n";
					buf<<"\t\t\tfor(int j = 0;j < "<<iter_ptrdim->second<<";j++)\n";
					buf<<"\t\t\t{\n";
					buf<<"\t\t\t\t"<<(*iter_prname)<<"_new[i*"<<iter_ptrdim->second<<"+j] = "<<(*iter_prname)<<"[i][j];\n";
					buf<<"\t\t\t}\n";
					buf<<"\t\t}\n";
					buf<<"\t\tcl::Buffer "<<(*iter_prname)<<"_buffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,"<<(iter_ptrdim->first)<<"*"<<iter_ptrdim->second<<"*sizeof("<<(*iter_ptrtype)<<"),"<<(*iter_prname)<<"_new,NULL);\n";
					buf<<"\t\tkernel.setArg("<<argnum++<<","<<(*iter_prname)<<"_buffer);\n";
				}
				++iter_ptrdim;
				++iter_ptrtype;
			}
			Node *work = actor->contents->body->u.operBody.work;
			stringstream tmpBuf;
			string actorname = actor->name;
			List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
			List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
			ListMarker input_maker;
			ListMarker output_maker;
			Node *inputNode = NULL;
			Node *outputNode = NULL;
			string inputString;
			string outputString;
			IterateList(&input_maker,inputList);
			while(NextOnList(&input_maker,(GenericREF)&inputNode))
			{

				if(inputNode->typ == Id)inputString = inputNode->u.id.text;
				else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
			}
			IterateList(&output_maker,outputList);
			vector<string> vecOutEdgeName;
			while(NextOnList(&output_maker,(GenericREF)&outputNode))
			{

				if(outputNode->typ == Id)outputString = outputNode->u.id.text;
				else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
			}
			SPL2GPU_Node(work, 2);
			string workstring = declInitList.str();
			//cout<<declInitList.str()<<endl;
			//////////////////////////////////////////////////////////////////////////
			//需要在work部分寻找相关的全局以及静态变量
			map<string,map<string,string> >::iterator iter_staticvar;
			map<string,string>::iterator iter_submapofstatic;
			vector<string>::iterator iter_typeofstatic = staticvartypefinal.begin();
			map<string,map<string,string> >::iterator iter_globalvar;
			map<string,string>::iterator iter_submapofglobal;
			vector<string>::iterator iter_typeofglobal = globalvartypefinal.begin();
			vector<map<string,map<string,string> > >::iterator iter_vecofstatic,iter_vecofglobal;
			for (iter_vecofstatic = staticvariablefinal.begin();iter_vecofstatic != staticvariablefinal.end();++iter_vecofstatic)
			{
				for (iter_staticvar = (*iter_vecofstatic).begin();iter_staticvar != (*iter_vecofstatic).end();iter_staticvar++,iter_typeofstatic++)
				{
					if (workstring.find(iter_staticvar->first) != -1)
					{
						if (array2Dname.count(iter_staticvar->first))//如果改变量是二维数组，则需要对其进行改造，变成一维数组
						{
							iter_submapofstatic = iter_staticvar->second.begin();
							stringstream numss1,numss2;
							numss1<<iter_submapofstatic->first;
							int num1,num2;
							numss1>>num1;
							numss2<<iter_submapofstatic->second;
							numss2>>num2;
							buf<<"\t\t"<<*iter_typeofstatic<<" "<<iter_staticvar->first<<"_new["<<num1*num2<<"];\n";
							buf<<"\t\tfor(int i = 0;i < "<<iter_submapofstatic->first<<";i++)\n\t\t{";
							buf<<"\n\t\t\tfor(int j = 0;j <"<<iter_submapofstatic->second<<" ;j++)\n\t\t\t{\n";
							buf<<"\t\t\t\t"<<iter_staticvar->first<<"_new[i*"<<iter_submapofstatic->second<<"+j] = "<<iter_staticvar->first<<"[i][j];\n";
							buf<<"\t\t\t}\n";
							buf<<"\t\t}\n";
							//////////////////////////////////////////////////////////////////////////
							//将新生成的一维数组封装成buffer对象
							buf<<"\t\tcl::Buffer "<<iter_staticvar->first<<"_buffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,"<<(num1*num2)<<"*sizeof("<<(*iter_typeofstatic)<<"),"<<(iter_staticvar->first)<<"_new,NULL);\n";
							buf<<"\t\tkernel.setArg("<<argnum++<<","<<iter_staticvar->first<<"_buffer);\n";
						}
						else if (array1Dname.count(iter_staticvar->first))//如果改变量是一维数组，则直接封装成buffer
						{
							iter_submapofstatic = iter_staticvar->second.begin();
							stringstream numss1;
							numss1<<iter_submapofstatic->first;
							int num1;
							numss1>>num1;
							buf<<"\t\tcl::Buffer "<<iter_staticvar->first<<"_buffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,"<<num1<<"*sizeof("<<(*iter_typeofstatic)<<"),"<<(iter_staticvar->first)<<",NULL);\n";
							buf<<"\t\tkernel.setArg("<<argnum++<<","<<iter_staticvar->first<<"_buffer);\n";
						}
						else
						{
							buf<<"\t\tkernel.setArg("<<argnum++<<","<<iter_staticvar->first<<");\n";
						}
					}
				}

			}
			for (iter_vecofglobal = globalvariablefinal.begin();iter_vecofglobal != globalvariablefinal.end();++iter_vecofglobal)
			{
				for (iter_globalvar = (*iter_vecofglobal).begin();iter_globalvar != (*iter_vecofglobal).end();iter_globalvar++,iter_typeofglobal++)
				{
					int pos;
					int startpos = 0;
					while((pos = workstring.find(iter_globalvar->first,startpos))!=-1)
					{
						if (!((((int)workstring[pos+(iter_globalvar->first).length()])>=65&&((int)workstring[pos+(iter_globalvar->first).length()])<=90)||(((int)workstring[pos+(iter_globalvar->first).length()])>=97&&((int)workstring[pos+(iter_globalvar->first).length()])<=122))&&!((((int)workstring[pos-1])>=65&&((int)workstring[pos-1])<=90)||(((int)workstring[pos-1])>=97&&((int)workstring[pos-1])<=122)))
						{
							if (array2Dname.count(iter_globalvar->first))//如果改变量是二维数组，则需要对其进行改造，变成一维数组
							{
								iter_submapofglobal = iter_globalvar->second.begin();
								stringstream numss1,numss2;
								numss1<<iter_submapofglobal->first;
								int num1,num2;
								numss1>>num1;
								numss2<<iter_submapofglobal->second;
								numss2>>num2;
								buf<<"\t\t"<<*iter_typeofglobal<<" "<<iter_globalvar->first<<"_new["<<num1*num2<<"];\n";
								buf<<"\t\tfor(int i = 0;i < "<<iter_submapofglobal->first<<";i++)\n\t\t{";
								buf<<"\n\t\t\tfor(int j = 0;j <"<<iter_submapofglobal->second<<" ;j++)\n\t\t\t{\n";
								buf<<"\t\t\t\t"<<iter_globalvar->first<<"_new[i*"<<iter_submapofglobal->second<<"+j] = "<<iter_globalvar->first<<"[i][j];\n";
								buf<<"\t\t\t}\n";
								buf<<"\t\t}\n";
								//////////////////////////////////////////////////////////////////////////
								//将新生成的一维数组封装成buffer对象
								buf<<"\t\tcl::Buffer "<<iter_globalvar->first<<"_buffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,"<<(num1*num2)<<"*sizeof("<<(*iter_typeofglobal)<<"),"<<(iter_globalvar->first)<<"_new,NULL);\n";
								buf<<"\t\tkernel.setArg("<<argnum++<<","<<iter_globalvar->first<<"_buffer);\n";
							}
							else if (array1Dname.count(iter_globalvar->first))//如果改变量是一维数组，则直接封装成buffer
							{
								iter_submapofglobal = iter_globalvar->second.begin();
								stringstream numss1;
								numss1<<iter_submapofglobal->first;
								int num1;
								numss1>>num1;
								buf<<"\t\tcl::Buffer "<<iter_globalvar->first<<"_buffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,"<<num1<<"*sizeof("<<(*iter_typeofglobal)<<"),"<<(iter_globalvar->first)<<",NULL);\n";
								buf<<"\t\tkernel.setArg("<<argnum++<<","<<iter_globalvar->first<<"_buffer);\n";
							}
							/*else
							{
								buf<<"\t\tkernel.setArg("<<argnum++<<","<<iter_globalvar->first<<");\n";
							}*/
						}
						startpos = pos + 1;
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////
			vector<int>::iterator iter_outpushweight = actor->outPushWeights.begin();
			vector<int>::iterator iter_inpopweight = actor->inPopWeights.begin();
			buf<<"\t\tcl::NDRange offset(0);\n";
			buf<<"\t\tcl::NDRange global_size(count/com_num);\n";
			int queuenum = i + 1;
			//Node2QueueID.insert(make_pair(actor,queuenum));
			buf<<"\t\tif((count/com_num)%"<<LocalSizeofWork<<" == 0)\n";
			buf<<"\t\t{\n";
			buf<<"\t\t\tcl::NDRange local_size("<<LocalSizeofWork<<");\n";
			buf<<"\t\t\tqueue_"<<queuenum<<"_1.enqueueNDRangeKernel(kernel, offset, global_size, local_size);\n";
			buf<<"\t\t}\n";
			buf<<"\t\telse\n";
			buf<<"\t\t{\n";
			buf<<"\t\t\tcl::NDRange local_size("<<LocalSize<<");\n";
			buf<<"\t\t\tqueue_"<<queuenum<<"_1.enqueueNDRangeKernel(kernel, offset, global_size, local_size);\n";
			buf<<"\t\t}\n";
			if (!actor->outFlatNodes.size()&&PrintFlag)
			{
				
			}
			thread2queue.insert(make_pair(Haflp->findPartitionNumForFlatNode(actor),queuenum));
			int index = 0;
			for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
			{
				buf<<"\t\treadtag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<" += count/com_num*popValue"<<index<<";\n";
				index++;
			}
			index = 0;
			for (iter_1=actor->outFlatNodes.begin();iter_1!=actor->outFlatNodes.end();iter_1++)
			{
				buf<<"\t\twritetag_"<<actor->name<<"_"<<(*iter_1)->name<<"_"<<i<<" += count/com_num*pushValue"<<index<<";\n";
				index++;
			}
			buf << "\t}\n"; // work方法结束
			declInitList.str(""); // 清空
		}
	}
}

int GPUCodeGenerate::ReturnPushWeight(FlatNode *actorA,FlatNode *actorB)
{
	vector<FlatNode*>::iterator iter_actor;
	vector<int>PushWeight = actorA->outPushWeights;
	vector<int>::iterator iter_push = PushWeight.begin();
	for (iter_actor = actorA->outFlatNodes.begin();iter_actor != actorA->outFlatNodes.end();iter_actor++)
	{
		if ((*iter_actor)->name == (actorB)->name)
		{
			return *iter_push;
		}
		iter_push++;
	}
}

int GPUCodeGenerate::ReturnPeekWeight(FlatNode *actorA,FlatNode *actorB)
{
	vector<FlatNode*>::iterator iter_actor;
	vector<int>PeekWeight = actorB->inPeekWeights;
	vector<int>::iterator iter_peek = PeekWeight.begin();
	for (iter_actor = actorB->inFlatNodes.begin();iter_actor != actorB->inFlatNodes.end();iter_actor++)
	{
		if ((*iter_actor)->name == (actorA)->name)
		{
			return *iter_peek;
		}
		iter_peek++;
	}
}

string GPUCodeGenerate::ReturnNameOfTheEdge(string searchstring)//此处的参数形如A->name_B->name
{
	map<string,string>::iterator iter = edge2name.find(searchstring);
	return iter->second;
}
void GPUCodeGenerate::CGdataGetforGPU(FlatNode *actor,stringstream &buf)
{
	for (int i = 0; i < nGpu_; i++)
	{
		buf<<"\tvoid dataget_"<<i<<"(int count,";
		//multimap<FlatNode*,string>::iterator iter1;
		vector<FlatNode*>::iterator iter;
		vector<string>InPutString;
		vector<string>::iterator iter_instring;
		List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
		ListMarker input_maker;
		Node *inputNode = NULL;
		Node *outputNode = NULL;
		//iter1 = mapActor2InEdge.find(actor);
		int inflatnodeindex = 0;
		IterateList(&input_maker,inputList);
		while(NextOnList(&input_maker,(GenericREF)&inputNode))
		{
			string inputString;
			if(inputNode->typ == Id)inputString = inputNode->u.id.text;
			else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
			if(sssg_->IsDownBorder(actor->inFlatNodes[inflatnodeindex]) && !DataDependence(actor->inFlatNodes[inflatnodeindex],actor))
				buf<<pEdgeInfo->getEdgeInfo(actor->inFlatNodes[inflatnodeindex],actor).typeName<<" *"<<inputString<<",";
			++inflatnodeindex;
			InPutString.push_back(inputString);
		}
		iter_instring = InPutString.begin();
		vector<int>::iterator iter_inpopweight = actor->inPopWeights.begin();
		multimap<string,string>::iterator iterxy=mapedge2xy2.begin();
		pair<multimap<string,string>::iterator,multimap<string,string>::iterator>pos;

		buf<<"void* p = NULL)\n\t{\n";
		buf<<"\t\tif(count == initScheduleCount)\n";
		buf<<"\t\t{\n";
		buf<<"\t\t\tif(count){\n";
		int index = 0;
		for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
		{
			int argnum = 0;
			if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_))
			{
				int queuenum = i + 1;
				pos = mapedge2xy2.equal_range(iterxy->first);
				string searchstring = (*iter)->name+"_"+actor->name;
				buf<<"\t\t\t\tqueue_"<<queuenum<<"_0.enqueueWriteBuffer(Inbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<", CL_FALSE, 0, "<<sssg_->GetInitCount(*iter)<<"*"<<ReturnPushWeight(*iter,actor)<<"*sizeof("<<pEdgeInfo->getEdgeInfo((*iter),actor).typeName<<"),&"<<ReturnNameOfTheEdge(searchstring)<<"[0], NULL,NULL);\n";

				buf<<"\t\t\t\twritedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<" += "<<sssg_->GetInitCount(*iter)<<"*"<<ReturnPushWeight(*iter,actor)<<";\n";
				if(ExistDependence(actor))
				{
					buf<<"\t\t\t\tpopComm"<<index<<" = "<<sssg_->GetInitCount(*iter)*ReturnPushWeight(*iter,actor)<<";\n";
				}
					//buf<<"\t\t\t\tpopToken("<<sssg_->GetInitCount(*iter)*ReturnPushWeight(*iter,actor)<<");\n";
				//buf<<"\t\t\t}\n";
			}
			iter_inpopweight++;
			iter_instring++;
			index++;
		}
		buf<<"\t\t\t}\n";
		buf<<"\t\t}\n";
		iter_inpopweight = actor->inPopWeights.begin();
		iterxy=mapedge2xy2.begin();
		iter_instring = InPutString.begin();
		index = 0;
		buf<<"\t\telse if(count == steadyScheduleCount)\n";
		buf<<"\t\t{\n";
		buf<<"\t\t\tif(count){\n";
		for (iter=actor->inFlatNodes.begin();iter!=actor->inFlatNodes.end();iter++)
		{
			int argnum = 0;
			if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_) || (actor->GPUPart < nGpu_ && (*iter)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(actor)!=Haflp->findPartitionNumForFlatNode(*iter)))
			{
				int queuenum = i + 1;
				int stagemins = pSa->FindStage(actor) - pSa->FindStage(*iter);
				pos = mapedge2xy2.equal_range(iterxy->first);
				string searchstring = (*iter)->name+"_"+actor->name;
				buf<<"\t\t\t\tif(writedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<"+"<<sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor]*ReturnPushWeight(*iter,actor)<<" > ";
				buf<<(sssg_->GetInitCount(*iter)+sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor]*stagemins)*ReturnPushWeight(*iter,actor)<<")\n";
				buf<<"\t\t\t\t{\n";
				buf<<"\t\t\t\t\tqueue_"<<queuenum<<"_0.enqueueWriteBuffer(Inbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<", CL_FALSE,writedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<"*sizeof("<<pEdgeInfo->getEdgeInfo((*iter),actor).typeName<<"), ("<<(sssg_->GetInitCount(*iter)+sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor]*stagemins)*ReturnPushWeight(*iter,actor)<<"-writedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<")*sizeof("<<pEdgeInfo->getEdgeInfo((*iter),actor).typeName<<"),&"<<ReturnNameOfTheEdge(searchstring)<<"[0], NULL,NULL);\n";
				buf<<"\t\t\t\t\tqueue_"<<queuenum<<"_0.enqueueWriteBuffer(Inbuffer_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<", CL_FALSE,0, ("<<sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor]*ReturnPushWeight(*iter,actor)<<"-("<<(sssg_->GetInitCount(*iter)+sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor]*stagemins)*ReturnPushWeight(*iter,actor)<<"-writedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<"))*sizeof("<<pEdgeInfo->getEdgeInfo((*iter),actor).typeName<<"),&"<<ReturnNameOfTheEdge(searchstring)<<"[0+"<<(sssg_->GetInitCount(*iter)+sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor]*stagemins)*ReturnPushWeight(*iter,actor)<<"-writedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<"], NULL,NULL);\n";
				buf << "\t\t\t\t\twritedatatag_" << (*iter)->name << "_" << actor->name << "_" << i << " = " << sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor] * ReturnPushWeight(*iter, actor) << "-(" << (sssg_->GetInitCount(*iter) + sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor] * stagemins)*ReturnPushWeight(*iter, actor) << "-writedatatag_" << (*iter)->name << "_" << actor->name << "_" << i << ");";
				buf<<"\n\t\t\t\t}\n";
				buf<<"\t\t\t\telse{\n";
				buf << "\t\t\t\t\tqueue_" << queuenum << "_0.enqueueWriteBuffer(Inbuffer_" << (*iter)->name << "_" << actor->name << "_" << i << ", CL_FALSE,writedatatag_" << (*iter)->name << "_" << actor->name << "_" << i << "*sizeof(" << pEdgeInfo->getEdgeInfo((*iter), actor).typeName << "), " << sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor] * ReturnPushWeight(*iter, actor) << "*sizeof(" << pEdgeInfo->getEdgeInfo((*iter), actor).typeName << "),&" << ReturnNameOfTheEdge(searchstring) << "[0], NULL,NULL);\n";
				buf << "\t\t\t\t\twritedatatag_" << (*iter)->name << "_" << actor->name << "_" << i << " += " << sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor] << "*" << ReturnPushWeight(*iter, actor) << ";\n";
				//buf<<"\t\t\t\t\tif(writedatatag_"<<(*iter)->name<<"_"<<actor->name<<"_"<<i<<" == "<<(sssg_->GetInitCount(*iter)+sssg_->GetSteadyCount(*iter)*Maflp->MultiNum2FlatNode[actor]*stagemins)*ReturnPushWeight(*iter,actor)<<")\n";
				buf << "\t\t\t\t\twritedatatag_" << (*iter)->name << "_" << actor->name << "_" << i << " %= " << (sssg_->GetInitCount(*iter) + sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor] * stagemins)*ReturnPushWeight(*iter, actor) << ";\n";
				buf<<"\t\t\t\t}\n";
				if(ExistDependence(actor))
				{
					buf << "\t\t\t\tpopComm" << index << " = " << sssg_->GetSteadyCount(*iter)*Haflp->MultiNum2FlatNode[actor] * ReturnPushWeight(*iter, actor) << ";\n";
				}
			}
			iter_instring++;
			iter_inpopweight++;
			index++;
		}
		buf<<"\t\t\t}\n";
		buf<<"\t\t}\n";
		if(ExistDependence(actor))
			buf<<"\t\tpopToken();\n";
		buf<<"\t}\n";
	}
	
}

void GPUCodeGenerate::CGdataSendforGPU(FlatNode *actor,stringstream &buf)
{
	bool pushflag = false;
	for (int outindex = 0; outindex < actor->outFlatNodes.size(); ++outindex)
	{
		if (DataDependence(actor,actor->outFlatNodes[outindex]))
		{
			pushflag = true;
			break;
		}
	}
	for (int i = 0; i < nGpu_; i++)
	{
		buf<<"\tvoid datasend_"<<i<<"(int count,";
		//multimap<FlatNode*,string>::iterator iter1;
		vector<FlatNode*>::iterator iter;
		vector<string>OutPutString;
		vector<string>::iterator iter_outstring;
		List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
		ListMarker output_maker;
		Node *outputNode = NULL;
		//iter1 = mapActor2OutEdge.find(actor);
		int outflatnodeindex = 0;
		IterateList(&output_maker,outputList);
		while(NextOnList(&output_maker,(GenericREF)&outputNode))
		{
			string outputString;
			if(outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
			if(sssg_->IsUpBorder(actor->outFlatNodes[outflatnodeindex]) && !DataDependence(actor,actor->outFlatNodes[outflatnodeindex]))
				buf<<pEdgeInfo->getEdgeInfo(actor,actor->outFlatNodes[outflatnodeindex]).typeName<<" *"<<outputString<<",";
			++outflatnodeindex;
			OutPutString.push_back(outputString);
		}
		iter_outstring = OutPutString.begin();
		vector<int>::iterator iter_outpushweight = actor->outPushWeights.begin();
		multimap<string,string>::iterator iterxy=mapedge2xy2.begin();
		pair<multimap<string,string>::iterator,multimap<string,string>::iterator>pos;

		buf<<"void* p = NULL)\n\t{\n";
		buf<<"\t\tif(count == initScheduleCount)\n";
		buf<<"\t\t{\n";
		buf<<"\t\t\tif(count){\n";
		iter_outpushweight = actor->outPushWeights.begin();
		int index = 0;
		for (iter=actor->outFlatNodes.begin();iter!=actor->outFlatNodes.end();iter++)
		{
			int argnum = 0;
			if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_))
			{
				int queuenum = i + 1;
				string searchstring = actor->name+"_"+(*iter)->name;
				buf<<"\t\t\t\tqueue_"<<queuenum<<"_0.enqueueReadBuffer(Outbuffer_"<<actor->name<<"_"<<(*iter)->name<<"_"<<i<<",CL_FALSE,0,"<<sssg_->GetInitCount(*iter)*(*iter_outpushweight)<<"*sizeof("<<pEdgeInfo->getEdgeInfo(actor,(*iter)).typeName<<"),&"<<ReturnNameOfTheEdge(searchstring)<<"[0],NULL,NULL);\n";
				buf<<"\t\t\t\treaddatatag_"<<actor->name<<"_"<<(*iter)->name<<"_"<<i<<" += "<<sssg_->GetInitCount(*iter)*(*iter_outpushweight)<<";\n";
			    if(pushflag)
				{
					buf<<"\t\t\tpushComm"<<index<<" = "<<sssg_->GetInitCount(*iter)*(*iter_outpushweight)<<";\n";
				}
			}
			iter_outpushweight++;
			index++;
		}
		buf<<"\t\t\t}\n";
		buf<<"\t\t}\n";
		iter_outstring = OutPutString.begin();
		iter_outpushweight = actor->outPushWeights.begin();
		iterxy=mapedge2xy2.begin();
		buf<<"\t\telse\n";
		buf<<"\t\t{\n";
		index = 0;
		for (iter=actor->outFlatNodes.begin();iter!=actor->outFlatNodes.end();iter++)
		{
			int argnum = 0;
			if((actor->GPUPart < nGpu_ && (*iter)->GPUPart >= nGpu_))
			{
				int queuenum = i + 1;
				string searchstring = actor->name+"_"+(*iter)->name;
				int stageminus = pSa->FindStage(*iter)-pSa->FindStage(actor);
				buf << "\t\t\tif(readdatatag_" << actor->name << "_" << (*iter)->name << "_" << i << "+" << sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * ReturnPushWeight(actor, *iter) << " > " << (sssg_->GetInitCount(actor) + sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * stageminus)*ReturnPushWeight(actor, *iter) << ")\n";
				buf<<"\t\t\t{\n";
				buf << "\t\t\t\tqueue_" << queuenum << "_0.enqueueReadBuffer(Outbuffer_" << actor->name << "_" << (*iter)->name << "_" << i << ",CL_FALSE,readdatatag_" << actor->name << "_" << (*iter)->name << "_" << i << "*sizeof(" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << "),(" << (sssg_->GetInitCount(actor) + sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * stageminus)*ReturnPushWeight(actor, *iter) << "-readdatatag_" << actor->name << "_" << (*iter)->name << "_" << i << ")*sizeof(" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << "),&" << ReturnNameOfTheEdge(searchstring) << "[0],NULL,NULL);\n";
				buf << "\t\t\t\tqueue_" << queuenum << "_0.enqueueReadBuffer(Outbuffer_" << actor->name << "_" << (*iter)->name << "_" << i << ",CL_FALSE,0,(" << sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * ReturnPushWeight(actor, *iter) << "-(" << (sssg_->GetInitCount(actor) + sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * stageminus)*ReturnPushWeight(actor, *iter) << "-readdatatag_" << actor->name << "_" << (*iter)->name << "_" << i << "))*sizeof(" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << "),&" << ReturnNameOfTheEdge(searchstring) << "[0+" << (sssg_->GetInitCount(actor) + sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * stageminus)*ReturnPushWeight(actor, *iter) << "-readdatatag_" << actor->name << "_" << (*iter)->name << "_" << i << "],NULL,NULL);\n";
				buf << "\t\t\t\treaddatatag_" << actor->name << "_" << (*iter)->name << "_" << i << " = " << sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * ReturnPushWeight(actor, *iter) << "-(" << (sssg_->GetInitCount(actor) + sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * stageminus)*ReturnPushWeight(actor, *iter) << "-readdatatag_" << actor->name << "_" << (*iter)->name << "_" << i << ");\n";
				buf<<"\t\t\t}\n";
				buf<<"\t\t\telse{\n";
				buf << "\t\t\t\tqueue_" << queuenum << "_0.enqueueReadBuffer(Outbuffer_" << actor->name << "_" << (*iter)->name << "_" << i << ",CL_FALSE,readdatatag_" << actor->name << "_" << (*iter)->name << "_" << i << "*sizeof(" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << ")," << sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * ReturnPushWeight(actor, *iter) << "*sizeof(" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << "),&" << ReturnNameOfTheEdge(searchstring) << "[0],NULL,NULL);\n";
				buf << "\t\t\t\treaddatatag_" << actor->name << "_" << (*iter)->name << "_" << i << " += " << sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * ReturnPushWeight(actor, *iter) << ";\n";
				buf << "\t\t\t\treaddatatag_" << actor->name << "_" << (*iter)->name << "_" << i << " %= " << (sssg_->GetInitCount(actor) + sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * stageminus)*ReturnPushWeight(actor, *iter) << ";\n";
				buf<<"\t\t\t}\n";
				if (pushflag)
				{
					buf << "\t\t\tpushComm" << index << " = " << sssg_->GetSteadyCount(actor)*Haflp->MultiNum2FlatNode[actor] * ReturnPushWeight(actor, *iter) << ";\n";
				}
			}
			iter_outpushweight++;
			index++;
		}
		buf<<"\t\t}\n";
		if(pushflag)
			buf<<"\t\t\tpushToken();\n";
		buf<<"\t}\n";
	}
}

void GPUCodeGenerate::OutputToFile(string fileName, string oldContents)
{
	ofstream fw;
	try{
		fw.open(fileName.c_str());
		fw<<oldContents;
		fw.close();
	}catch(...){
		cout<<"error:output to file"<<endl;
	}
}
void GPUCodeGenerate::CGactors()
{
	stringstream ss,buf;
	std::map<operatorNode *, string>::iterator pos;

	for (int i = 0; i < nTemplateNode_; ++i)//生成 各个 类模板
	{
		int len = ListLength(vTemplateNode_[i]->oldContents->decl->u.decl.type->u.operdcl.outputs);
		int nOut = vTemplateNode_[i]->nOut;
		OperatorType ot = vTemplateNode_[i]->oldContents->ot;

		pos = mapOperator2ClassName.find(vTemplateNode_[i]->oldContents);
		//if (pos == mapOperator2ClassName.end()) // 新的类模板进入了
		{
			string name = vTemplateNode_[i]->name;
			int index = name.find_first_of('_');
			string tmp = name.substr(0, index);
			mapOperator2ClassName.insert(make_pair(vTemplateNode_[i]->oldContents, vTemplateNode_[i]->name));

			if (len == nOut)			//nOut为每个operator的输出边个数
			{
				buf<<"#include \""<<vTemplateName_[i]<<".h\"\n";
				if (strcmp(tmp.c_str(), "FileReader") == 0)
					isInFileReader = true;
				if (strcmp(tmp.c_str(), "FileWriter") == 0)
					isInFileWriter = true; 
				CGactor(vTemplateNode_[i],vTemplateName_[i], ot);

				isInFileReader = isInFileWriter = false;
			}
			else // 一条边被多个actor共用, 暂不处理
			{
				cout << "test" << endl;
				UNREACHABLE;
			}

		}

	}
	ss<<dir_<<"AllActorHeader.h";
	OutputToFile(ss.str(),buf.str());
}
string GPUCodeGenerate::MakeTabs(int tabs)
{
	stringstream tmp;
	while (tabs--) tmp<<"\t";
	return tmp.str();
}
void GPUCodeGenerate::OutputCRSpaceAndTabs(int tabs)
{
	declInitList<<"\n"; 
	while (tabs--) declInitList<<"\t";
}
void GPUCodeGenerate::OutputArgList(List *list, int offset)
{ 
	ListMarker marker;
	Node *item;
	int i=0;
	IterateList(&marker, list);
	while (NextOnList(&marker, (GenericREF) &item))
	{
		if(i!=0) declInitList<<", ";
		SPL2GPU_Node(item, offset);
		i++;
	}
}
void GPUCodeGenerate::OutputTabs(int tabs)
{
	while (tabs--) declInitList<<"\t";
}
void GPUCodeGenerate::AddSemicolon()
{

}
string GPUCodeGenerate::ReturnTypeOfEdge(FlatNode *actorA,FlatNode *actorB)
{
	map<FlatNode *,FlatNode *>searchmap;
	searchmap.insert(make_pair(actorA,actorB));
	map<map<FlatNode *,FlatNode *>,map<string,string> >::iterator iter = edge2nameandtype.find(searchmap);
	map<string,string>::iterator iter_str;
	iter_str = iter->second.begin();
	return iter_str->first;
}
string GPUCodeGenerate::ReturnNameOfEdge(FlatNode *actorA,FlatNode *actorB)
{
	map<FlatNode *,FlatNode *>searchmap;
	searchmap.insert(make_pair(actorA,actorB));
	map<map<FlatNode *,FlatNode *>,map<string,string> >::iterator iter = edge2nameandtype.find(searchmap);
	map<string,string>::iterator iter_str;
	iter_str = iter->second.begin();
	return iter_str->second;
}
string GPUCodeGenerate::ReturnTypeOfEdgestr(string typestr)
{
	map<string,map<string,string> >::iterator iter = edgestrnameandtype.find(typestr);
	map<string,string>::iterator iter_str;
	iter_str = iter->second.begin();
	return iter_str->first;
}
string GPUCodeGenerate::ReturnNameOfEdgestr(string namestr)
{
	map<string,map<string,string> >::iterator iter = edgestrnameandtype.find(namestr);
	map<string,string>::iterator iter_str;
	iter_str = iter->second.begin();
	return iter_str->second;
}
void GPUCodeGenerate::CGglobalHeader()
{
	stringstream buf;
	string edgename;
	vector<FlatNode *>::iterator iter_1,iter_2;
	buf<<"#ifndef _GLOBAL_H\n";
	buf<<"#define _GLOBAL_H\n";
	buf<<"/*全局变量，用于存储边的信息*/\n";
	buf<<"/*边的命名规则：A_B,其中A->B*/\n\n";
	buf<<"#include \"Buffer.h\"\n";
	buf<<"#include \"CL/cl.hpp\"\n";
	if (FileRW)
	{
		buf<<"#include <sstream>\n";
		buf<<"#include <fstream>\n";
	}
	buf<<"#include <string>\n";
	buf<<"#include <iostream>\n";
	buf<<"#include \"MathExtension.h\"\n";
	buf<<"using namespace std;\n";
	if (Win)
	{
		buf<<"#define MAX_ITER 10\n";
	}
	pEdgeInfo->DeclEdgeType(buf);
	//cout<<flatNodes_.size();
	string notCommonEdgeStrName;
	vector<bool> vecEdgeDefine(flatNodes_.size(),0);
	vector<bool>::iterator iedgeIndex= vecEdgeDefine.begin();
	map<string,string>::iterator iterName;
	map<string,string> tmpVec;
	int stageminus; //stageminus表示两个actor所分配的阶段差
	bool printStruct = false;
	for (iter_1=flatNodes_.begin();iter_1!=flatNodes_.end();++iter_1)//遍历所有结点
	{
		int flag = 0;
		int outflatnodeindex = 0;
		for (iter_2=(*iter_1)->outFlatNodes.begin();iter_2!=(*iter_1)->outFlatNodes.end();iter_2++)
		{
			if((*iter_1)->name == "LowPassFilter_24")
				int i = 1;
			if((*iter_2)->name == "Duplicate_2_1")
				int i = 1;
			string declaredEdgeStr;
			flag = 0;
			edgename=(*iter_1)->name+"_"+(*iter_2)->name;
			/*if (!printStruct)
			{
				buf<<"struct mystruct_x\n{\n";
			}*/
			if(!flag&&(*iter_1)->contents->ot != Common_)
			{
				if(( iterName= mapOperator2EdgeName.find((*iter_1)->name))!=mapOperator2EdgeName.end())
				{
					flag = 1;
					declaredEdgeStr = iterName->second;
					if((*iter_2)->contents->ot != Common_)
					{
						mapOperator2EdgeName.insert(make_pair((*iter_2)->name,declaredEdgeStr));
						UpdateFlatNodeEdgeName(iter_2,declaredEdgeStr);
					}
				}
			}
			if(!flag&&(*iter_2)->contents->ot != Common_)
			{
				if(( iterName= mapOperator2EdgeName.find((*iter_2)->name))!=mapOperator2EdgeName.end())
				{
					flag = 1;
					declaredEdgeStr = iterName->second;
				}
			}
			if(flag)
			{
				vector<FlatNode *>::iterator iter_3 = iter_1;
				map<string,string>submap;
				submap.clear();
				submap.insert(make_pair(ReturnTypeOfEdgestr(declaredEdgeStr),ReturnNameOfEdgestr(declaredEdgeStr)));
				map<FlatNode *,FlatNode *>mapofflatnodes;
				mapofflatnodes.clear();
				mapofflatnodes.insert(make_pair(*iter_1,*iter_2));
				edge2nameandtype.insert(make_pair(mapofflatnodes,submap));
				while((*iter_3)->contents->ot != Common_)
				{
					map<string,string>::iterator strname= mapOperator2EdgeName.find((*iter_3)->name);
					if( strname !=mapOperator2EdgeName.end())
					{
						tmpVec.insert(make_pair(edgename,strname->second));
						//mapStruct2Struct.swap(tmpVec);
						//mapStruct2Struct.clear();
						break;
					}
					iter_3 = (*iter_3)->inFlatNodes.begin();
				}
			}
			else
			{
				Node *outputNode = NULL;
				List *outputList =(*iter_1)->contents->decl->u.decl.type->u.operdcl.outputs;
				ListMarker output_maker; 
				structflag = true;
				map<FlatNode *,FlatNode *>submapofstruct;
				submapofstruct.clear();
				submapofstruct.insert(make_pair(*iter_1,*iter_2));
				
				//buf<<"class "<<edgename<<"_str\n{\npublic:\n";
				StreamType *streamType = NULL;
				IterateList(&output_maker,outputList);
				while(NextOnList(&output_maker,(GenericREF)&outputNode))
				{
					if(outputNode->typ == Id) streamType = outputNode->u.id.decl->u.decl.type->u.strdcl.type;
					else if (outputNode->typ == Decl)  streamType = outputNode->u.decl.type->u.strdcl.type;
				}
				List *streamField = streamType->fields;
				ListMarker streamField_maker;
				IterateList(&streamField_maker,streamField);
				Node *fieldNode = NULL;
				while(NextOnList(&streamField_maker,(GenericREF)&fieldNode))
				{
					declList.str("");
					edgexy.str("");
					ExtractDeclVariables(fieldNode);
					string s = edgexy.str();
					mapedge2xy.insert(make_pair(edgename,s));
					mapedge2xy2.insert(make_pair(edgename,s));
					/*if(!printStruct)
						buf<<declList.str();*/
				}	
				edge2nameandtype.insert(make_pair(submapofstruct,nameandtypeofedge));
				edgestrnameandtype.insert(make_pair((*iter_1)->name+"_"+(*iter_2)->name,nameandtypeofedge));
				structflag = false;
				if((*iter_1)->contents->ot != Common_){
					mapOperator2EdgeName.insert(make_pair((*iter_1)->name,edgename));
					UpdateFlatNodeEdgeName(iter_1,edgename);
				}
				if((*iter_2)->contents->ot != Common_){
					mapOperator2EdgeName.insert(make_pair((*iter_2)->name,edgename));
					UpdateFlatNodeEdgeName(iter_2,edgename);
				}
			}	
			/*if(!printStruct)
			{
				buf<<"};\n";
				printStruct = true;
			}*/
			stageminus = pSa->FindStage(*iter_2) - pSa->FindStage(*iter_1);
			if((*iter_1)->GPUPart >= nGpu_ || (*iter_2)->GPUPart >= nGpu_)
			{
				if (!DataDependence(*iter_1,*iter_2))
				{
					if((*iter_2)->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_)
						buf << "extern " << pEdgeInfo->getEdgeInfo(*iter_1, *iter_2).typeName << " " << edgename << "[" << stageminus << "][" << (sssg_->GetInitCount(*iter_1) + sssg_->GetSteadyCount(*iter_1)*Haflp->MultiNum2FlatNode[*iter_1])*(*iter_1)->outPushWeights[outflatnodeindex] << "];\n";
					else if((*iter_1)->GPUPart < nGpu_ && (*iter_2)->GPUPart >= nGpu_)
						buf << "extern " << pEdgeInfo->getEdgeInfo(*iter_1, *iter_2).typeName << " " << edgename << "[" << stageminus << "][" << (sssg_->GetInitCount(*iter_1) + sssg_->GetSteadyCount(*iter_1)*Haflp->MultiNum2FlatNode[*iter_1] * nGpu_)*(*iter_1)->outPushWeights[outflatnodeindex] << "];\n";
					else	
						buf << "extern " << pEdgeInfo->getEdgeInfo(*iter_1, *iter_2).typeName << " " << edgename << "[" << stageminus + 1 << "][" << (sssg_->GetInitCount(*iter_1) + sssg_->GetSteadyCount(*iter_1)*Haflp->MultiNum2FlatNode[*iter_1])*(*iter_1)->outPushWeights[outflatnodeindex] << "];\n";
				}
				else
				{
					if((*iter_2)->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_)
						buf<<"extern Buffer<"<<pEdgeInfo->getEdgeInfo(*iter_1,*iter_2).typeName<<"> "<<edgename<<";\n";
						//<<"("<<(sssg_->GetInitCount(*iter_1)+sssg_->GetSteadyCount(*iter_1)*Maflp->MultiNum2FlatNode[*iter_1]*stageminus)*(*iter_1)->outPushWeights[outflatnodeindex]<<");\n";
				    else if((*iter_1)->GPUPart < nGpu_ && (*iter_2)->GPUPart >= nGpu_)
						buf<<"extern Buffer<"<<pEdgeInfo->getEdgeInfo(*iter_1,*iter_2).typeName<<"> "<<edgename<<";\n";
						//<<"("<<(sssg_->GetInitCount(*iter_1)+sssg_->GetSteadyCount(*iter_1)*Maflp->MultiNum2FlatNode[*iter_1]*nGpu_*stageminus)*(*iter_1)->outPushWeights[outflatnodeindex]<<");\n";
				    else
						buf<<"extern Buffer<"<<pEdgeInfo->getEdgeInfo(*iter_1,*iter_2).typeName<<"> "<<edgename<<";\n";
						//<<"("<<(sssg_->GetInitCount(*iter_1)+sssg_->GetSteadyCount(*iter_1)*Maflp->MultiNum2FlatNode[*iter_1]*(stageminus+1))*(*iter_1)->outPushWeights[outflatnodeindex]<<");\n";
				}
			}
			outflatnodeindex++;
		//	buf<<"extern Buffer<"<<edgename<<"_str>"<<edgename<<";\n";
		}
	}
	if (FileRW)
	{
		mapStruct2Struct2OfGPU=tmpVec;
		map<string,string>::iterator iter_s2s;
		pair<multimap<string,string>::iterator,multimap<string,string>::iterator>pos_s2t;
		for (iter_s2s=mapStruct2Struct2OfGPU.begin();iter_s2s!=mapStruct2Struct2OfGPU.end();++iter_s2s)
		{
			pos_s2t = mapedge2xy.equal_range(iter_s2s->second);
			while(pos_s2t.first!=pos_s2t.second)
			{
				mapedge2xy2.insert(make_pair(iter_s2s->first,pos_s2t.first->second));
				++pos_s2t.first;
			}
		}
		//mapedge2xy.swap(mapedge2xy2);
		buf<<"extern stringstream outstream;\n";
		buf<<"extern stringstream instream;\n";
		multimap<string,string>::iterator iterxy;
		multimap<string,string>::iterator iterxy1=mapedge2xy2.begin();
		multimap<string,string>::iterator iterxy2=mapedge2xy2.end();
		multimap<string,string>::iterator iterxy3=mapedge2xy2.begin();
		int Rtag=0,Wtag=0;
		for (iterxy=mapedge2xy2.begin();iterxy!=mapedge2xy2.end();++iterxy)
		{
			string temps = iterxy->first;
			if (temps.find("FileW",0)!=-1&&!Rtag)
			{
				iterxy1=iterxy;
				Rtag++;
			}
			if (temps.find("FileR",0)!=-1&&!Wtag)
			{
				iterxy2=iterxy;
				Wtag++;
			}
		}
		//string tests = iterxy2->first;
		if (iterxy1==iterxy3)//Benchmark中只有FileReader
		{
			existFileWriter = false;
			buf<<"istream& operator>>(stringstream& is, "<<iterxy2->first<<"_str &object);\n";
			buf<<"void FileReader(string path,int mode);\n";
		}
		else
		{
			buf<<"ostream& operator<<(stringstream& os, const "<<iterxy1->first<<"_str &object);\n";
			buf<<"istream& operator>>(stringstream& is, "<<iterxy2->first<<"_str &object);\n";
			buf<<"void FileWriter(string path,int mode);\n";
			buf<<"void FileReader(string path,int mode);\n";
		}

	}	
	/*multimap<string,string>::iterator iterxy=mapedge2xy2.begin();
	pair<multimap<string,string>::iterator,multimap<string,string>::iterator>pos = mapedge2xy2.equal_range(iterxy->first);*/
	/*map<string,string>::iterator iter_nametype;
	for (iter_nametype = allnameandtype.begin();iter_nametype != allnameandtype.end();++iter_nametype)
	{
		buf<<"typedef struct\n";
		buf<<"{\n";
		buf<<"\t"<<iter_nametype->first<<" "<<iter_nametype->second<<";\n";
		buf<<"}mystruct_"<<iter_nametype->first<<";\n";
	}*/
	/*while (pos.first != pos.second)
	{
	buf<<"\t"<<structdatatype<<" "<<pos.first->second<<";\n";
	pos.first++;
	}*/
	buf<<"#endif\n";
	//输出到文件
	stringstream ss;
	ss<<dir_<<"global.h";
	mapOperator2EdgeName.clear();
	OutputToFile(ss.str(),  buf.str());
}
void GPUCodeGenerate::UpdateFlatNodeEdgeName(vector<FlatNode *>::iterator iter,string edgeName)
{
	vector<FlatNode *>::iterator iter_3;

	for(iter_3=(*iter)->inFlatNodes.begin();iter_3!=(*iter)->inFlatNodes.end();iter_3++)
	{
		if((*iter_3)->contents->ot != Common_){
			mapOperator2EdgeName.insert(make_pair((*iter_3)->name,edgeName));
			UpdateFlatNodeEdgeName(iter_3,edgeName);
		}
	}
}
void GPUCodeGenerate::CGglobalCpp()
{
	stringstream buf;
	string edgename;
	int size;//缓冲区的大小
	vector<FlatNode *>::iterator iter_1,iter_2;
	vector<int>::iterator iter;
	int stageminus; //stageminus表示两个actor所分配的阶段差
	buf<<"/*cpp文件,全局变量，用于存储边的信息*/\n";
	buf<<"#include \"Buffer.h\"\n";
	buf<<"#include \"global.h\"\n";
	for (iter_1=flatNodes_.begin();iter_1!=flatNodes_.end();++iter_1)//遍历所有结点
	{
		iter=(*iter_1)->outPushWeights.begin();
		for (iter_2=(*iter_1)->outFlatNodes.begin();iter_2!=(*iter_1)->outFlatNodes.end();iter_2++)
		{
			edgename=(*iter_1)->name+"_"+(*iter_2)->name;
			stageminus=pSa->FindStage(*iter_2)-pSa->FindStage(*iter_1);
			if ((*iter_1)->GPUPart >= nGpu_ || (*iter_2)->GPUPart >= nGpu_)
			{
				if (!DataDependence(*iter_1,*iter_2))
				{
					if((*iter_2)->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_)
						buf << pEdgeInfo->getEdgeInfo(*iter_1, *iter_2).typeName << " " << edgename << "[" << stageminus << "][" << (sssg_->GetInitCount(*iter_1) + sssg_->GetSteadyCount(*iter_1)*Haflp->MultiNum2FlatNode[*iter_1])*(*iter) << "];\n";
					else if((*iter_1)->GPUPart < nGpu_ && (*iter_2)->GPUPart >= nGpu_)
						buf << pEdgeInfo->getEdgeInfo(*iter_1, *iter_2).typeName << " " << edgename << "[" << stageminus << "][" << (sssg_->GetInitCount(*iter_1) + sssg_->GetSteadyCount(*iter_1)*Haflp->MultiNum2FlatNode[*iter_1] * nGpu_)*(*iter) << "];\n";
					else	
						buf << pEdgeInfo->getEdgeInfo(*iter_1, *iter_2).typeName << " " << edgename << "[" << stageminus + 1 << "][" << (sssg_->GetInitCount(*iter_1) + sssg_->GetSteadyCount(*iter_1)*Haflp->MultiNum2FlatNode[*iter_1])*(*iter) << "];\n";
				}
				else
				{
					if((*iter_2)->GPUPart < nGpu_ && (*iter_1)->GPUPart >= nGpu_)
						buf << "Buffer<" << pEdgeInfo->getEdgeInfo(*iter_1, *iter_2).typeName << "> " << edgename << "(" << (sssg_->GetInitCount(*iter_1) + sssg_->GetSteadyCount(*iter_1)*Haflp->MultiNum2FlatNode[*iter_1] * stageminus)*(*iter) << ");\n";
					else if((*iter_1)->GPUPart < nGpu_ && (*iter_2)->GPUPart >= nGpu_)
						buf << "Buffer<" << pEdgeInfo->getEdgeInfo(*iter_1, *iter_2).typeName << "> " << edgename << "(" << (sssg_->GetInitCount(*iter_1) + sssg_->GetSteadyCount(*iter_1)*Haflp->MultiNum2FlatNode[*iter_1] * nGpu_*stageminus)*(*iter) << ");\n";
					else
						buf << "Buffer<" << pEdgeInfo->getEdgeInfo(*iter_1, *iter_2).typeName << "> " << edgename << "(" << (sssg_->GetInitCount(*iter_1) + sssg_->GetSteadyCount(*iter_1)*Haflp->MultiNum2FlatNode[*iter_1] * (stageminus + 1))*(*iter) << ");\n";
				}
				iter++;
			}
		}
	}
	if (FileRW)
	{
		buf<<"stringstream outstream;\n";
		buf<<"stringstream instream;\n";
		pair<multimap<string,string>::iterator,multimap<string,string>::iterator>pos_reader;
		pair<multimap<string,string>::iterator,multimap<string,string>::iterator>pos_writer;
		multimap<string,string>::iterator iterxy;
		multimap<string,string>::iterator iterxy1=mapedge2xy2.begin();
		multimap<string,string>::iterator iterxy2=mapedge2xy2.end();
		multimap<string,string>::iterator iterxy3=mapedge2xy2.begin();
		int Rtag=0,Wtag=0;
		for (iterxy=mapedge2xy2.begin();iterxy!=mapedge2xy2.end();++iterxy)
		{
			string temps = iterxy->first;
			if (temps.find("FileW",0)!=-1&&!Rtag)
			{
				iterxy1=iterxy;
				Rtag++;
			}
			if (temps.find("FileR",0)!=-1&&!Wtag)
			{
				iterxy2=iterxy;
				Wtag++;
			}
		}
		if (iterxy1==iterxy3)//Benchmark中没有FileWriter，只有FileReader
		{
			pos_reader = mapedge2xy2.equal_range(iterxy2->first);
			buf<<"istream& operator>>(stringstream& is, "<<iterxy2->first<<"_str &object)\n{\n\t";
			buf<<"if(is";
			while(pos_reader.first!=pos_reader.second)
			{
				buf<<">>";
				buf<<"(object."<<pos_reader.first->second<<")";
				++pos_reader.first;
			}
			buf<<")\n\t\treturn is;\n";
			buf<<"\telse\n\t{\n";
			buf<<"\t\tcout<<\"原始数据不够！\"<<endl;\n";
			buf<<"\t\tsystem(\"pause\");\n\t";
			buf<<"}\n";
			buf<<"}\n";
		}
		else//Benchmark中同时存在FileReader和FileWriter
		{
			if ((iterxy1->first).find("FileW",0)!=-1)
			{
				string s=iterxy1->first;
				pos_writer = mapedge2xy2.equal_range(iterxy1->first);
				string s_s=iterxy2->first;
				pos_reader = mapedge2xy2.equal_range(iterxy2->first);
				buf<<"ostream& operator<<(stringstream& os, const "<<iterxy1->first<<"_str &object)\n{\n\t";
				buf<<"os";
				while(pos_writer.first!=pos_writer.second)
				{
					buf<<"<<";
					buf<<"object."<<pos_writer.first->second<<"<<\" \"";
					++pos_writer.first;
				}
				buf<<";\n";
				buf<<"\treturn os;\n}";
				buf<<"\n";
				buf<<"istream& operator>>(stringstream& is, "<<iterxy2->first<<"_str &object)\n{\n\t";
				buf<<"if(is";
				while(pos_reader.first!=pos_reader.second)
				{
					buf<<">>";
					buf<<"(object."<<pos_reader.first->second<<")";
					++pos_reader.first;
				}
				buf<<")\n\t\treturn is;\n";
				buf<<"\telse\n\t{\n";
				buf<<"\t\tcout<<\"原始数据不够！\"<<endl;\n";
				buf<<"\t\tsystem(\"pause\");\n\t";
				buf<<"}\n";
				buf<<"}\n";
			}
			else
			{
				pos_writer = mapedge2xy2.equal_range(iterxy2->first);
				pos_reader = mapedge2xy2.equal_range(iterxy1->first);
				buf<<"ostream& operator<<(stringstream& os, const "<<iterxy2->first<<"_str &object)\n{\n\t";
				buf<<"os";
				while(pos_writer.first!=pos_writer.second)
				{
					buf<<"<<";
					buf<<"object."<<pos_writer.second->second<<"<<\" \"";
					++pos_writer.first;
				}
				buf<<";\n";
				buf<<"\treturn os;\n\t}";
				buf<<"\n";
				buf<<"istream& operator>>(stringstream& is, "<<iterxy1->first<<"_str &object)\n{\n\t";
				buf<<"if(is";
				while(pos_reader.first!=pos_reader.second)
				{
					buf<<">>";
					buf<<"(object."<<pos_reader.second->second<<")";
					++pos_reader.first;
				}
				buf<<")\n\t\treturn is;\n";
				buf<<"\telse\n\t{\n";
				buf<<"\t\tcout<<\"原始数据不够！\"<<endl;\n";
				buf<<"\t\tsystem(\"pause\");\n\t";
				buf<<"}\n";
				buf<<"}\n";
			}
		}	
		buf<<"void FileWriter(string path,int mode)\n";
		buf<<"{\n\t";
		buf<<"ofstream ofile;\n\t";
		buf<<"if(mode==0)\n\t";
		buf<<"{\n\t\t";
		buf<<"ofile.open(path.c_str());\n\t";
		buf<<"}\n\t";
		buf<<"ofile<<outstream.str();\n\t";
		buf<<"ofile.close();\n";
		buf<<"}\n\n";
		buf<<"void FileReader(string path,int mode)\n";
		buf<<"{\n\t";
		buf<<"ifstream ifile;\n\t";
		buf<<"ifile.open(path.c_str());\n\t";
		buf<<"instream<<ifile.rdbuf();\n\t";
		buf<<"ifile.close();\n";
		buf<<"}";
	}
	//输出到文件
	stringstream ss;
	ss<<dir_<<"global.cpp";
	OutputToFile(ss.str(),  buf.str());

}
void GPUCodeGenerate::CGMakefile()
{
	stringstream buf;
	buf<<"PROGRAM := "<<output_file<<"\n";
	buf<<"SOURCES := $(wildcard ./*.cpp)\n";
	buf<<"SOURCES += $(wildcard ./src/*.cpp)\n";
	buf<<"OBJS    := $(patsubst %.cpp,%.o,$(SOURCES))\n";
	buf<<"CC      := g++\n";
	buf<<"CFLAGS  := -ggdb -Wall\n";
	buf<<"INCLUDE := -I . -Ihome/lihao/GPU/runtime/GPULib2 . -I/home/lihao/GPU/src/3rdpart/include\n";
	buf<<"LIBPATH := -L/home/lihao/GPU/src/3rdpart/lib\n";
	buf<<"LIB     := -lpthread -ldl -llock\n\n\n";
	buf<<".PHONY: clean install\n\n";
	buf<<"$(PROGRAM): $(OBJS)\n";
	buf<<"\t$(CC) -o $@ $^ $(LIB) $(LIBPATH)\n";
	buf<<"%.o: %.cpp\n";
	buf<<"\t$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)\n\n\n";
	buf<<"clean:\n";
	buf<<"\trm $(OBJS) $(PROGRAM) -f\n\n\n";
	buf<<"install: $(PROGRAM)\n";
	buf<<"\tcp $(PROGRAM) ./bin/\n";
	stringstream ss;
	ss<<dir_<<"Makefile";
	OutputToFile(ss.str(),  buf.str());
}
void GPUCodeGenerate::CGThreads()
{
	string threadname;
	stringstream buf;
	char a[10];  
	string numname;//存储thread结尾数字转化为字符串
	if (Haflp->CPUNodes.size() > 0)
	{
		for (int i=0;i<nGpu_*2+NCpuThreads;i++)
		{
			buf<<"/*该文件定义各thread的入口函数，在函数内部完成软件流水迭代*/\n";
			stringstream ss;
			//itoa(i, a, 10);  
			sprintf(a, "%d", i);
			numname=a;
			threadname="thread_"+numname;
			ss<<dir_<<threadname<<".cpp";
			CGThread(i,buf);
			OutputToFile(ss.str(),buf.str());
			ss.str("");
			buf.str("");
		}
	}
	else
	{
		for (int i=0;i<nGpu_;i++)
		{
			buf<<"/*该文件定义各thread的入口函数，在函数内部完成软件流水迭代*/\n";
			stringstream ss;
			//itoa(i, a, 10);  
			sprintf(a, "%d", i);
			numname=a;
			threadname="thread_"+numname;
			ss<<dir_<<threadname<<".cpp";
			CGThread(i,buf);
			OutputToFile(ss.str(),buf.str());
			ss.str("");
			buf.str("");
		}
	}
}
 void GPUCodeGenerate::CGThread(int index,stringstream&buf)
{
	char a[10]; 
	//itoa(index, a, 10);  
	sprintf(a, "%d", index);
	string numname=a;
	int totalstagenum=pSa->MaxStageNumForGPU();
	vector<FlatNode *>tempactorset;
	map<FlatNode*,string> actor2name;
	vector<FlatNode *>::iterator iter;
	set<int>*ptempstagenum;
	pair<multimap<FlatNode *,string>::iterator,multimap<FlatNode *,string>::iterator>pos1,pos2;
	map<int,set<int> >::iterator iter1;
	buf<<"#include \"Buffer.h\"\n";
	buf<<"#include \"Producer.h\"\n";
	buf<<"#include \"Consumer.h\"\n";
	buf<<"#include \"global.h\"\n";
	buf<<"#include \"barrier_sync.h\"\t//包含barrier函数\n";
	buf<<"#include \"AllActorHeader.h\"\t//包含所有actor的头文件\n";
	buf<<"#include \"rdtsc.h\"\n";
	if(CHECKEACHACTORTIME)
	{	
		buf<<"#include <sstream>\n";
		buf<<"#include <fstream>\n";
	}
	buf<<"//extern pthread_barrier_t barrier;\n";
	if (PrintTime)
	{
		if(index > nGpu_ && index < 2*nGpu_+1)
			buf<<"extern double cpu_gpu_com"<<index-nGpu_-1<<";\n";
		else if(index == nGpu_)
			buf<<"extern double cpu0_compute;\n";
		else if(index >= 2*nGpu_+1)
			buf<<"extern double cpu"<<index-2*nGpu_<<"_compute;\n";
		else
			buf<<"extern double gpu"<<index<<"_compute;\n";
	}
	for (int i = 0;i < nGpu_;i++)
	{
		buf<<"extern cl::CommandQueue queue_"<<i+1<<"_0;\n";
		buf<<"extern cl::CommandQueue queue_"<<i+1<<"_1;\n";
	}
	/*buf<<"extern cl::CommandQueue queue_1;\n";
	buf<<"extern cl::CommandQueue queue_2;\n";
	buf<<"extern cl::CommandQueue queue_3;\n";*/
	if (Linux)
	{	
		buf<<"extern int MAX_ITER;\n";
	}
	bool IsDataTransfer = false;
	if (index > nGpu_ && index < 2*nGpu_+1)
	{
		IsDataTransfer = true;
		//tempactorset=Maflp->findNodeSetInPartition(index-nGpu_-1);
	}
	//cwb
	typedef multimap<int,pair<FlatNode*,string> >::iterator index_iter;
	if (DNBPFlag)
	{
		if (index == nGpu_)
		{
			pair<index_iter,index_iter> pos = Dnbp->Cputhread2Actor.equal_range(0);
			while(pos.first != pos.second)
			{
				tempactorset.push_back(pos.first->second.first);
				actor2name.insert(pos.first->second);
				pos.first++;
			}
		}
		else if (index >= 2*nGpu_+1)
		{
			pair<index_iter,index_iter> pos = Dnbp->Cputhread2Actor.equal_range(index-2*nGpu_);
			while(pos.first != pos.second)
			{
				tempactorset.push_back(pos.first->second.first);
				actor2name.insert(pos.first->second);
				pos.first++;
			}
		}
		else
			tempactorset = Haflp->FindNodeInOneDevice(index);
	}
	else
	{
		tempactorset = Haflp->FindNodeInOneDevice(index);
		if (index == nGpu_ || index >= 2*nGpu_+1)
		{
			for (int i = 0; i < tempactorset.size(); i++)
			{
				actor2name.insert(make_pair(tempactorset[i],tempactorset[i]->name));
			}
		}
	}
	if(!CHECKBARRIERTIME)
	{
		if ((index == nGpu_ || index >= 2*nGpu_+1))
		{
			map<FlatNode*,string>::iterator mapiter;
			for (mapiter = actor2name.begin(); mapiter != actor2name.end(); ++mapiter)
			{
				if((mapiter->first)->inFlatNodes.size() == 0 && (mapiter->first)->outFlatNodes.size() != 0)
					buf<<"extern "<<mapFlatnode2Template_[(mapiter->first)]<<"<"<<pEdgeInfo->getEdgeInfo((mapiter->first),(mapiter->first)->outFlatNodes[0]).typeName<<"> "<<mapiter->second<<"_obj;\n";
				else if((mapiter->first)->inFlatNodes.size() != 0 && (mapiter->first)->outFlatNodes.size() == 0)
					buf<<"extern "<<mapFlatnode2Template_[(mapiter->first)]<<"<"<<pEdgeInfo->getEdgeInfo((mapiter->first)->inFlatNodes[0],(mapiter->first)).typeName<<"> "<<mapiter->second<<"_obj;\n";
				else if((mapiter->first)->inFlatNodes.size() != 0 && (mapiter->first)->outFlatNodes.size() != 0)
					buf<<"extern "<<mapFlatnode2Template_[(mapiter->first)]<<"<"<<pEdgeInfo->getEdgeInfo((mapiter->first),(mapiter->first)->outFlatNodes[0]).typeName<<","<<pEdgeInfo->getEdgeInfo((mapiter->first)->inFlatNodes[0],(mapiter->first)).typeName<<"> "<<mapiter->second<<"_obj;\n";
			}
		}
		else
		{
			for (iter=tempactorset.begin();iter!=tempactorset.end();++iter)
			{
				if((*iter)->inFlatNodes.size() == 0 && (*iter)->outFlatNodes.size() != 0)
					buf<<"extern "<<mapFlatnode2Template_[(*iter)]<<"<"<<pEdgeInfo->getEdgeInfo(*iter,(*iter)->outFlatNodes[0]).typeName<<"> "<<(*iter)->name<<"_obj;\n";
				else if((*iter)->inFlatNodes.size() != 0 && (*iter)->outFlatNodes.size() == 0)
					buf<<"extern "<<mapFlatnode2Template_[(*iter)]<<"<"<<pEdgeInfo->getEdgeInfo((*iter)->inFlatNodes[0],*iter).typeName<<"> "<<(*iter)->name<<"_obj;\n";
				else if((*iter)->inFlatNodes.size() != 0 && (*iter)->outFlatNodes.size() != 0)
					buf<<"extern "<<mapFlatnode2Template_[(*iter)]<<"<"<<pEdgeInfo->getEdgeInfo(*iter,(*iter)->outFlatNodes[0]).typeName<<","<<pEdgeInfo->getEdgeInfo((*iter)->inFlatNodes[0],*iter).typeName<<"> "<<(*iter)->name<<"_obj;\n";
			}
		}
	}
	if (TRACE)
	{
		buf<<"extern double (*deltatimes)["<<nGpu_<<"][2];\n";
		buf<<"#define	MEASURE_RUNS 100000\n";
	}
	if (index)
	{
		buf<<"void thread_"<<numname<<"_fun()\n{\n";
		//buf<<"\tworkerSync("<<index<<");\n";
	}
	else
	{
		buf<<"void thread_"<<numname<<"_fun()\n{\n";
	}
	if(CHECKEACHACTORTIME)						//输出每个actor在每个stage上的steadywork时间
	{
		buf<<"\tofstream txtfw;\n\tstringstream ss;\n";
	}
	if(TRACE){
		buf<<"\ttsc_counter c0, c1;\n";	
		buf<<"\tRDTSC(c0);\n";
	}
	if(!CHECKBARRIERTIME)
	{
		buf<<"\tchar stage["<<totalstagenum<<"]={0};\n";
		buf<<"\tstage[0]=1;\n";
	}
	if(PrintTime)
		buf<<"\ttsc_counter t0,t1;\n";
	if (IsDataTransfer)
	{
		iter1=mapNum2Stage.find(index-nGpu_-1);
	}
	else
	{
		if(index == nGpu_ || index >= 2*nGpu_+1)
			iter1=mapNum2Stage.find(nGpu_);
		else
			iter1=mapNum2Stage.find(index);
	}
	ptempstagenum=&iter1->second;
	set<int>::iterator enditer = ptempstagenum->end();
	map<FlatNode*,string>::iterator actorEnditer = actor2name.end();
	buf<<"\tfor(int _stageNum=0;_stageNum<"<<totalstagenum<<";_stageNum++)\n";
	buf<<"\t{\n";
		
	if(!CHECKBARRIERTIME){


		for (int i=totalstagenum-1;i>=0;i--)	//迭代stage
		{
			set<int>::iterator stageiter = ptempstagenum->find(i);
			if(stageiter!=enditer){					//该stage在该thread上
				vector<FlatNode*> flatVec = pSa->FindActor(i);
				vector<FlatNode*>::iterator iter1;
				if (flatVec.size())
				{	
					if (!IsDataTransfer)
					{
						if (index == nGpu_ || index >= 2*nGpu_+1)
						{
							bool stageFlag = true;
							for(iter1=flatVec.begin();iter1!=flatVec.end();++iter1)
							{
								map<FlatNode*,string>::iterator actorIter = actor2name.find(*iter1);
								if (actorIter != actorEnditer)
								{
									if (stageFlag)
									{
										buf<<"\t\tif("<<i<<"==_stageNum)\n\t\t{\n";
										stageFlag = false;
									}
									if (Haflp->findPartitionNumForFlatNode(*iter1) == Haflp->Index2Device(index))
									{
										buf<<"\t\t\t"<<actor2name[*iter1]<<"_obj.runInitScheduleWork(";
										multimap<FlatNode*,string>::iterator iter3,iter2;
										List *inputList =(*iter1)->contents->decl->u.decl.type->u.operdcl.inputs;
										List *outputList =(*iter1)->contents->decl->u.decl.type->u.operdcl.outputs;
										ListMarker input_maker;
										ListMarker output_maker;
										Node *inputNode = NULL;
										Node *outputNode = NULL;
										iter3=mapActor2InEdge.find((*iter1));
										int inflatnodeindex = 0;
										IterateList(&input_maker,inputList);
										while(NextOnList(&input_maker,(GenericREF)&inputNode))
										{
											if (!DataDependence((*iter1)->inFlatNodes[inflatnodeindex],*iter1))
											{
												buf<<"&"<<iter3->second<<"[0][0],";
											}
											++iter3;
											++inflatnodeindex;
										}
										iter2=mapActor2OutEdge.find((*iter1));
										int outflatnodeindex = 0;
										IterateList(&output_maker,outputList);
										while(NextOnList(&output_maker,(GenericREF)&outputNode))
										{
											if (!DataDependence(*iter1,(*iter1)->outFlatNodes[outflatnodeindex]))
											{
												buf<<"&"<<iter2->second<<"[0][0],";
											}
											++iter2;
											++outflatnodeindex;
										}
										buf<<"NULL);\n";
									}
								}
							}
							if(!stageFlag)
								buf<<"\t\t}\n";
						}
						else if(index < nGpu_)
						{
							buf<<"\t\tif("<<i<<"==_stageNum)\n\t\t{\n";
							for(iter1=flatVec.begin();iter1!=flatVec.end();++iter1)
							{
								if (Haflp->findPartitionNumForFlatNode(*iter1) == Haflp->Index2Device(index))
								{
									buf<<"\t\t\t"<<(*iter1)->name<<"_obj.runInitScheduleWork_"<<index<<"();\n";
								}
							}
							buf<<"\t\t}\n";
						}
					}
				}
				if(IsDataTransfer)
				{
					flatVec = pSa->FindDataOfActor(i);
					if (flatVec.size())
					{
						buf<<"\t\tif("<<i<<"==_stageNum)\n\t\t{\n";
						map<FlatNode*,int>::iterator iter_subdatastage;
						for(iter1=flatVec.begin();iter1!=flatVec.end();++iter1)
						{
							pair<multimap<int,map<FlatNode*,int> >::iterator,multimap<int,map<FlatNode*,int> >::iterator>pos = pSa->datastage.equal_range(i);
							if (Haflp->Index2Device(index - nGpu_ - 1) == Haflp->findPartitionNumForFlatNode(*iter1))
							{

								while(pos.first!=pos.second)
								{
									for (iter_subdatastage = pos.first->second.begin();iter_subdatastage != pos.first->second.end();iter_subdatastage++)
									{
										if (iter_subdatastage->first->name == (*iter1)->name)
										{
											if (iter_subdatastage->second == 1)
											{
												buf<<"\t\t\t\t"<<(*iter1)->name<<"_obj.dataget_"<<index-nGpu_-1<<"("<<(*iter1)->name<<"_obj.initScheduleCount,";
												multimap<FlatNode*,string>::iterator iter3;
												List *inputList =(*iter1)->contents->decl->u.decl.type->u.operdcl.inputs;
												ListMarker input_maker;
												Node *inputNode = NULL;
												iter3=mapActor2InEdge.find((*iter1));
												int inflatnodeindex = 0;
												IterateList(&input_maker,inputList);
												while(NextOnList(&input_maker,(GenericREF)&inputNode))
												{
													if (!DataDependence((*iter1)->inFlatNodes[inflatnodeindex],*iter1))
													{
														if(sssg_->IsDownBorder((*iter1)->inFlatNodes[inflatnodeindex]) && sssg_->IsUpBorder(*iter1))
														{
															buf<<"&"<<iter3->second<<"[0][0],";
														}
													}
													++iter3;
													++inflatnodeindex;
												}
												buf<<"NULL);\n";
											}
											else
											{
												buf<<"\t\t\t\t"<<(*iter1)->name<<"_obj.datasend_"<<index-nGpu_-1<<"("<<(*iter1)->name<<"_obj.initScheduleCount,";
												multimap<FlatNode*,string>::iterator iter2;
												List *outputList =(*iter1)->contents->decl->u.decl.type->u.operdcl.outputs;
												ListMarker output_maker;
												Node *outputNode = NULL;
												iter2=mapActor2OutEdge.find((*iter1));
												int outflatnodeindex = 0;
												IterateList(&output_maker,outputList);
												while(NextOnList(&output_maker,(GenericREF)&outputNode))
												{
													if (!DataDependence(*iter1,(*iter1)->outFlatNodes[outflatnodeindex]))
													{
														if(sssg_->IsUpBorder((*iter1)->outFlatNodes[outflatnodeindex]) && sssg_->IsDownBorder(*iter1))
															buf<<"&"<<iter2->second<<"[0][0],";
													}
													++iter2;
													++outflatnodeindex;
												}
												buf<<"NULL);\n";
											}
										}
									}
									++pos.first;
								}
								//buf<<"\t\t\t\t"<<(*iter1)->name<<"_obj.datatransform();\n";

							}
						}
						buf<<"\t\t}\n";
					}
				}
			}
		}
	}
	if (TRACE)
	{
		buf<<"\t\tRDTSC(c1);\n";
		buf<<"\t\tdeltatimes[_stageNum]["<<index<<"][0] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		buf<<"\t\tc0 = c1;\n";
	}
	if (index > nGpu_ && index < 2*nGpu_+1)
	{
		buf<<"\t\tqueue_"<<index-nGpu_<<"_0.finish();\n";
		buf<<"\t\tqueue_"<<index-nGpu_<<"_1.finish();\n";
	}
	else if (index < nGpu_)
	{
		buf<<"\t\tqueue_"<<index+1<<"_0.finish();\n";
		buf<<"\t\tqueue_"<<index+1<<"_1.finish();\n";
	}
	/*for (int i = 0;i < nGpu_;i++)
	{
		buf<<"\t\tqueue_"<<i+1<<"_0.finish();\n";
		buf<<"\t\tqueue_"<<i+1<<"_1.finish();\n";
	}*/
	buf<<"\t\tawait();\n";
	if(TRACE){
		buf<<"\t\tRDTSC(c1);\n";
		buf<<"\t\tdeltatimes[_stageNum]["<<index<<"][1] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		buf<<"\t\tc0 = c1;\n";		
	}
	buf<<"\t\n\t//pthread_barrier_wait (&barrier);\n\t}\n";
	buf<<"\tfor(int _stageNum=0;_stageNum<"<<totalstagenum<<"+MAX_ITER-1;_stageNum++)\n\t{\n";
	if(!CHECKBARRIERTIME)	
	{	
		if(PrintTime)
			buf<<"\t\tRDTSC(t0);\n";
		if(CHECKEACHACTORTIME)
		{
			buf<<"\t\tss<<\"stage:\\t\"<<_stageNum<<endl;\n";
			buf<<"\t\tRDTSC(c1);\n";
			buf<<"\t\tss<<\"thread "<<index<<"\\t\"<<c1.int64<<endl;\n";
		}
		for (int i=totalstagenum-1;i>=0;i--)	//迭代stage
		{			

			set<int>::iterator stageiter = ptempstagenum->find(i);
			if(stageiter!=enditer){					//该stage在该thread上
				vector<FlatNode*> flatVec = pSa->FindActor(i);
				vector<FlatNode*>::iterator iter1;
				if (flatVec.size())
				{
					if (!IsDataTransfer)
					{
						if (index == nGpu_ || index >= 2*nGpu_+1)
						{
							bool stageFlag = true;
							for(iter1=flatVec.begin();iter1!=flatVec.end();++iter1)
							{
								map<FlatNode*,string>::iterator actorIter = actor2name.find(*iter1);
								if (actorIter != actorEnditer)
								{
									if (stageFlag)
									{
										buf<<"\t\tif("<<i<<"==_stageNum)\n\t\t{\n";
										stageFlag = false;
									}
									if (Haflp->findPartitionNumForFlatNode(*iter1) == Haflp->Index2Device(index))
									{
										buf<<"\t\t\t"<<actor2name[*iter1]<<"_obj.runSteadyScheduleWork(";
										multimap<FlatNode*,string>::iterator iter3,iter2;
										List *inputList =(*iter1)->contents->decl->u.decl.type->u.operdcl.inputs;
										List *outputList =(*iter1)->contents->decl->u.decl.type->u.operdcl.outputs;
										ListMarker input_maker;
										ListMarker output_maker;
										Node *inputNode = NULL;
										Node *outputNode = NULL;
										iter3=mapActor2InEdge.find((*iter1));
										int inflatnodeindex = 0;
										IterateList(&input_maker,inputList);
										while(NextOnList(&input_maker,(GenericREF)&inputNode))
										{
											if (!DataDependence((*iter1)->inFlatNodes[inflatnodeindex],*iter1))
											{
												if(DNBPFlag)
													buf<<"&"<<iter3->second<<"[(_stageNum-"<<i<<")%(sizeof("<<(*iter1)->inFlatNodes[inflatnodeindex]->name<<"_"<<(*iter1)->name<<")/sizeof("<<(*iter1)->inFlatNodes[inflatnodeindex]->name<<"_"<<(*iter1)->name<<"[0]))]["<<(sssg_->GetInitCount(*iter1)+(Dnbp->actorName2count[actor2name[*iter1]]).first)*(*iter1)->inPopWeights[inflatnodeindex]<<"],";
											    else
													buf<<"&"<<iter3->second<<"[(_stageNum-"<<i<<")%(sizeof("<<(*iter1)->inFlatNodes[inflatnodeindex]->name<<"_"<<(*iter1)->name<<")/sizeof("<<(*iter1)->inFlatNodes[inflatnodeindex]->name<<"_"<<(*iter1)->name<<"[0]))]["<<sssg_->GetInitCount(*iter1)*(*iter1)->inPopWeights[inflatnodeindex]<<"],";
											}
											++iter3;
											++inflatnodeindex;
										}
										iter2=mapActor2OutEdge.find((*iter1));
										int outflatnodeindex = 0;
										IterateList(&output_maker,outputList);
										while(NextOnList(&output_maker,(GenericREF)&outputNode))
										{
											if (!DataDependence(*iter1,(*iter1)->outFlatNodes[outflatnodeindex]))
											{
												if(DNBPFlag)
													buf<<"&"<<iter2->second<<"[(_stageNum-"<<i<<")%(sizeof("<<(*iter1)->name<<"_"<<(*iter1)->outFlatNodes[outflatnodeindex]->name<<")/sizeof("<<(*iter1)->name<<"_"<<(*iter1)->outFlatNodes[outflatnodeindex]->name<<"[0]))]["<<(sssg_->GetInitCount(*iter1)+(Dnbp->actorName2count[actor2name[*iter1]]).first)*(*iter1)->outPushWeights[outflatnodeindex]<<"],";
											    else
													buf<<"&"<<iter2->second<<"[(_stageNum-"<<i<<")%(sizeof("<<(*iter1)->name<<"_"<<(*iter1)->outFlatNodes[outflatnodeindex]->name<<")/sizeof("<<(*iter1)->name<<"_"<<(*iter1)->outFlatNodes[outflatnodeindex]->name<<"[0]))]["<<sssg_->GetInitCount(*iter1)*(*iter1)->outPushWeights[outflatnodeindex]<<"],";
											}
											++iter2;
											++outflatnodeindex;
										}
										buf<<"NULL);\n";
									}
								}
							}
							if(!stageFlag)
								buf<<"\t\t}\n";
						}
						else
						{
							buf<<"\t\tif(stage["<<i<<"])\n\t\t{\n";
							for(iter1=flatVec.begin();iter1!=flatVec.end();++iter1)
							{
								if (Haflp->findPartitionNumForFlatNode(*iter1) == Haflp->Index2Device(index))
								{
									buf<<"\t\t\t"<<(*iter1)->name<<"_obj.runSteadyScheduleWork_"<<index<<"();\n";
									if (PrintFlag)
									{
										if ((*iter1)->outFlatNodes.size() == 0 && (*iter1)->GPUPart < nGpu_)
										{
											buf << "\t\t\t" << (*iter1)->name << "_obj.print_" << index << "(" << sssg_->GetSteadyCount(*iter1)*Haflp->MultiNum2FlatNode[*iter1] << ");\n";
										}
									}
								}
							}
							buf<<"\t\t}\n";
						}
					}
				}
				if(IsDataTransfer)
				{
					flatVec = pSa->FindDataOfActor(i);
					if (flatVec.size())
					{
						buf<<"\t\tif(stage["<<i<<"])\n\t\t{\n";
						map<FlatNode*,int>::iterator iter_subdatastage;
						for(iter1=flatVec.begin();iter1!=flatVec.end();++iter1)
						{
							pair<multimap<int,map<FlatNode*,int> >::iterator,multimap<int,map<FlatNode*,int> >::iterator>pos = pSa->datastage.equal_range(i);
							if (Haflp->Index2Device(index - nGpu_ - 1) == Haflp->findPartitionNumForFlatNode(*iter1))
							{

								while(pos.first!=pos.second)
								{
									for (iter_subdatastage = pos.first->second.begin();iter_subdatastage != pos.first->second.end();iter_subdatastage++)
									{
										if (iter_subdatastage->first->name == (*iter1)->name)
										{
											if (iter_subdatastage->second == 1)
											{
												//buf<<"\t\t\t\t"<<(*iter1)->name<<"_obj.dataget_"<<index-nGpu_-1<<"("<<(*iter1)->name<<"_obj.steadyScheduleCount,(_stageNum-"<<i+index-nGpu_-1<<")%"<<(nGpu_+1)<<");\n";
												buf<<"\t\t\t\t"<<(*iter1)->name<<"_obj.dataget_"<<index-nGpu_-1<<"("<<(*iter1)->name<<"_obj.steadyScheduleCount,";
												multimap<FlatNode*,string>::iterator iter3;
												List *inputList =(*iter1)->contents->decl->u.decl.type->u.operdcl.inputs;
												ListMarker input_maker;
												Node *inputNode = NULL;
												iter3=mapActor2InEdge.find((*iter1));
												int inflatnodeindex = 0;
												IterateList(&input_maker,inputList);
												while(NextOnList(&input_maker,(GenericREF)&inputNode))
												{
													if (!DataDependence((*iter1)->inFlatNodes[inflatnodeindex],*iter1))
													{
														if(sssg_->IsDownBorder((*iter1)->inFlatNodes[inflatnodeindex]) && sssg_->IsUpBorder(*iter1))
														{
															int j;
															for (j = 0; j < (*iter1)->inFlatNodes[inflatnodeindex]->outFlatNodes.size(); ++j)
															{
																if((*iter1)->inFlatNodes[inflatnodeindex]->outFlatNodes[j] == *iter1)
																	break;
															}
															buf << "&" << iter3->second << "[(_stageNum-" << i << ")%(sizeof(" << (*iter1)->inFlatNodes[inflatnodeindex]->name << "_" << (*iter1)->name << ")/sizeof(" << (*iter1)->inFlatNodes[inflatnodeindex]->name << "_" << (*iter1)->name << "[0]))][" << sssg_->GetInitCount((*iter1)->inFlatNodes[inflatnodeindex])*(*iter1)->inFlatNodes[inflatnodeindex]->outPushWeights[j] + sssg_->GetSteadyCount(*iter1)*(*iter1)->inPopWeights[inflatnodeindex] * Haflp->MultiNum2FlatNode[*iter1] * (index - nGpu_ - 1) << "],";
														}
													}
													++iter3;
													++inflatnodeindex;
												}
												buf<<"NULL);\n";
											}
											else
											{
												//buf<<"\t\t\t\t"<<(*iter1)->name<<"_obj.datasend_"<<index-nGpu_-1<<"("<<(*iter1)->name<<"_obj.steadyScheduleCount,(_stageNum-"<<i+index-nGpu_-1<<")%"<<(nGpu_+1)<<");\n";
												buf<<"\t\t\t\t"<<(*iter1)->name<<"_obj.datasend_"<<index-nGpu_-1<<"("<<(*iter1)->name<<"_obj.steadyScheduleCount,";
												multimap<FlatNode*,string>::iterator iter2;
												List *outputList =(*iter1)->contents->decl->u.decl.type->u.operdcl.outputs;
												ListMarker output_maker;
												Node *outputNode = NULL;
												iter2=mapActor2OutEdge.find((*iter1));
												int outflatnodeindex = 0;
												IterateList(&output_maker,outputList);
												while(NextOnList(&output_maker,(GenericREF)&outputNode))
												{
													if (!DataDependence(*iter1,(*iter1)->outFlatNodes[outflatnodeindex]))
													{
														if(sssg_->IsUpBorder((*iter1)->outFlatNodes[outflatnodeindex]) && sssg_->IsDownBorder(*iter1))
															buf << "&" << iter2->second << "[(_stageNum-" << i << ")%(sizeof(" << (*iter1)->name << "_" << (*iter1)->outFlatNodes[outflatnodeindex]->name << ")/sizeof(" << (*iter1)->name << "_" << (*iter1)->outFlatNodes[outflatnodeindex]->name << "[0]))][" << sssg_->GetInitCount(*iter1)*(*iter1)->outPushWeights[outflatnodeindex] + sssg_->GetSteadyCount(*iter1)*(*iter1)->outPushWeights[outflatnodeindex] * Haflp->MultiNum2FlatNode[*iter1] * (index - nGpu_ - 1) << "],";
													}
													++iter2;
													++outflatnodeindex;
												}
												buf<<"NULL);\n";
											}
										}
									}
									++pos.first;
								}
							}
						}
						buf<<"\t\t}\n";
					}
				}
			}
		}
		if (PrintTime)
		{
			buf<<"\t\tRDTSC(t1);\n";
			if(index < nGpu_)
			    buf<<"\t\tgpu"<<index<<"_compute += COUNTER_DIFF(t1,t0,1);\n";
			else if(index == nGpu_)
				buf<<"\t\tcpu0_compute += COUNTER_DIFF(t1,t0,1);\n";
			else if(index >= 2*nGpu_+1)
				buf<<"\t\tcpu"<<index-2*nGpu_<<"_compute += COUNTER_DIFF(t1,t0,1);\n";
			else
				buf<<"\t\tcpu_gpu_com"<<index-nGpu_-1<<" += COUNTER_DIFF(t1,t0,1);\n";
		}
		buf<<"\t\tfor(int index="<<totalstagenum-1<<"; index>= 1; --index)\n";
		buf<<"\t\t\tstage[index] = stage[index-1];\n\t\tif(_stageNum == (MAX_ITER - 1))\n\t\t{\n\t\t\tstage[0]=0;\n\t\t}\n";
	}
	if (TRACE)
	{
		buf<<"\t\tRDTSC(c1);\n";
		buf<<"\t\tdeltatimes["<<pSa->MaxStageNum()<<"+_stageNum]["<<index<<"][0] =  COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		buf<<"\t\tc0 = c1;\n";
	}
	if (index > nGpu_ && index < 2*nGpu_+1)
	{
		buf<<"\t\tqueue_"<<index-nGpu_<<"_0.finish();\n";
		buf<<"\t\tqueue_"<<index-nGpu_<<"_1.finish();\n";
	}
	else if (index < nGpu_)
	{
		buf<<"\t\tqueue_"<<index+1<<"_0.finish();\n";
		buf<<"\t\tqueue_"<<index+1<<"_1.finish();\n";
	}
	/*for (int i = 0;i < nGpu_;i++)
	{
	buf<<"\t\tqueue_"<<i+1<<"_0.finish();\n";
	buf<<"\t\tqueue_"<<i+1<<"_1.finish();\n";
	}*/
	/*buf<<"\t\tqueue_1.finish();\n";
	buf<<"\t\tqueue_2.finish();\n";
	buf<<"\t\tqueue_3.finish();\n";*/
	/*if(index)
		buf<<"\t\n\t\tworkerSync("<<index<<");\n";
	else
		buf<<"\t\n\t\tmasterSync("<<nGpu_+1<<");\n";*/
	buf<<"\t\tawait();\n";
	if (TRACE)
	{
		buf<<"\t\tRDTSC(c1);\n";
		buf<<"\t\tdeltatimes["<<pSa->MaxStageNum()<<"+_stageNum]["<<index<<"][1] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		buf<<"\t\tc0 = c1;\n";
	}
	buf<<"\t\n\t//pthread_barrier_wait (&barrier);\n\t}\n";
	if(CHECKEACHACTORTIME)
	{
		buf<<"\ttry\n\t{\n\t\ttxtfw.open(\"thread "<<index<<"'s stage.txt\");\n\t\ttxtfw<<ss.str();\n\t\ttxtfw.close();\n\t}\n\tcatch(...)\n\t\t{cout<<\"error:output to file\"<<endl;\n\t}\n";
	}
	
	buf<<"}\n";
}


void GPUCodeGenerate::CGAllActorHeader()
{
	vector<FlatNode*>::iterator iter;
	stringstream buf,ss;
	buf<<"/*包含所有actor的头文件，主要是为了方便主文件包含*/\n\n";
	for (iter=flatNodes_.begin();iter!=flatNodes_.end();++iter)
	{	
		buf<<"#include \""<<(*iter)->name<<".h\"\n";	
	}
	ss<<dir_<<"AllActorHeader.h";
	OutputToFile(ss.str(),buf.str());
}

void GPUCodeGenerate::CGAllActorCpp()
{

}

int GPUCodeGenerate::ReturnNumBiger(int size)
{
	int num = 1;
	while (num < size)
	{
		num *= 2;
	}
	return num;
}

int GPUCodeGenerate::RetrunBufferSizeOfGpu(int num)
{
	vector<FlatNode *>nodes = Haflp->findNodeSetInPartition(num);
	vector<FlatNode *>Innodes,Outnodes;
	set<string>ExitedEdge;
	int sumbuffersize = 0;
	vector<FlatNode *>::iterator iter1,iter2,iter3;
	map<string,int>::iterator iter;
	for (iter1 = nodes.begin();iter1 != nodes.end();iter1++)
	{
		Innodes = (*iter1)->inFlatNodes;
		for (iter2 = Innodes.begin();iter2 != Innodes.end();iter2++)
		{
			string edgename = (*iter2)->name + "_" + (*iter1)->name;
			if (!ExitedEdge.count(edgename))
			{
				iter = Edge2Buffersize.find(edgename);
				if ((*iter1)->GPUPart >= nGpu_ || ((*iter1)->GPUPart < nGpu_ && (*iter2)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode(*iter1) != Haflp->findPartitionNumForFlatNode(*iter2)))
				{
					sumbuffersize += 2*(iter->second);
				}
				else
				{
					sumbuffersize += iter->second;
				}
				ExitedEdge.insert(edgename);
			}
		}
		Outnodes = (*iter1)->outFlatNodes;
		for (iter3 = Outnodes.begin();iter3 != Outnodes.end();iter3++)
		{
			string edgename = (*iter1)->name + "_" + (*iter3)->name;
			if (!ExitedEdge.count(edgename))
			{
				iter = Edge2Buffersize.find(edgename);
				sumbuffersize += iter->second;
				ExitedEdge.insert(edgename);
			}
		}
	}
	return sumbuffersize;
}

void GPUCodeGenerate::CGMain()
{
	stringstream buf,ss;
	char a[10];
	buf<<"///当前的扩大因子是"<<MultiNum<<"\n";
	buf<<"#include \"iostream\"\n";
	buf<<"#include \"stdlib.h\"\n";
	buf<<"#include <fstream>\n";
	if(Win)
	{
		buf<<"#include \"windows.h\"\n";
		buf<<"#include \"process.h\"\n";
		buf<<"#include \"stdlib.h\"\n";
	}
	else
	{
		buf<<"#include <pthread.h>\n";
		buf<<"#include \"setCpu.h\"\n";
	}
	if(TRACE){
		buf<<"#include \"rdtsc.h\"\n";
		buf<<"#include <sstream>\n";
		buf<<"#include <fstream>\n";
	}	
	buf<<"#include \"global.h\"\n";
	buf<<"#include \"barrier_sync.h\"\t//包含barrier函数\n";
	buf<<"#include \"AllActorHeader.h\"\t//包含所有actor的头文件\n";
	buf<<"using namespace std;\n";//home/chenwenbin/Desktop/GPU201310/FFT5/barrier_sync.cpp
	buf<<"int com_num = "<<Comm_num<<";\n"; //cwb
	if (Linux)
	{
		if (Haflp->CPUNodes.size() > 0)
			buf<<"int ThreadNum="<<2*nGpu_+NCpuThreads<<";\n";
		else
			buf<<"int ThreadNum="<<nGpu_<<";\n";
		buf<<"int MAX_ITER=1;//默认的执行次数是1\n";
	}
	if(Win)
	{
		buf<<"///int MAX_ITER;\n";
	}
	if (PrintTime)  //cwb 统计时间
	{
		buf<<"double cpu0_compute = 0;\n";
		if(NCpuThreads > 1)
		{
			for (int i = 0; i < NCpuThreads-1;i++)
				buf<<"double cpu"<<i+1<<"_compute = 0;\n";
		}
		for (int i = 0; i < nGpu_; i++)
		{
			buf<<"double cpu_gpu_com"<<i<<" = 0;\n";
			buf<<"double gpu"<<i<<"_compute = 0;\n";
		}
	}
	/************************************************************************/
	/*                     OpenCL环境初始化代码                             */
	/************************************************************************/
	buf<<"vector<cl::CommandQueue>allqueue;\n";
	buf<<"vector<cl::Platform>platforms;\n";
	buf<<"vector<cl::Device>platformsDevices;\n";
	buf<<"vector<cl::Event>dataevents;\n";
	buf<<"cl_int err_0 = cl::Platform::get(&platforms);\n";
	buf<<"cl_int err_1 = platforms[0].getDevices(CL_DEVICE_TYPE_GPU,&platformsDevices);\n";
	buf<<"cl::Context context(platformsDevices);\n";
	buf<<"std::ifstream programFile(\"allkernel.cl\");\n";
	buf<<"string programString(istreambuf_iterator<char>(programFile),(istreambuf_iterator<char>()));\n";
	buf<<"cl::Program::Sources source_opencl(1,make_pair(programString.c_str(),programString.length()+1));\n";
	buf<<"cl::Program program(context,source_opencl);\n";
	buf<<"cl_int err_2 = program.build(platformsDevices);\n";
	for (int i = 0;i < nGpu_;i++)
	{
		buf<<"cl::CommandQueue queue_"<<i+1<<"_0(context,platformsDevices["<<i<<"]);\n";
		buf<<"cl::CommandQueue queue_"<<i+1<<"_1(context,platformsDevices["<<i<<"]);\n";
	}
	/*buf<<"cl::CommandQueue queue_1(context,platformsDevices[0]);\n";
	buf<<"cl::CommandQueue queue_2(context,platformsDevices[1]);\n";
	buf<<"cl::CommandQueue queue_3(context,platformsDevices[2]);\n";*/
	vector<FlatNode*>::iterator iter1,iter2;
	vector<int>::iterator iter;
	int stageminus,size;
	//map<FlatNode *,int>::iterator iterofqueueid,iterofqueueid1;
	for (int i = 0; i < nGpu_; i++)
	{
		for (iter1 = flatNodes_.begin();iter1!=flatNodes_.end();iter1++)
		{
			iter=(*iter1)->outPushWeights.begin();
			for (iter2=(*iter1)->outFlatNodes.begin();iter2!=(*iter1)->outFlatNodes.end();iter2++)
			{
				stageminus=pSa->FindStage(*iter2)-pSa->FindStage(*iter1);
				//size=(sssg_->GetInitCount(*iter1)+sssg_->GetSteadyCount(*iter1)*Maflp->MultiNum2FlatNode[*iter1]*(stageminus+1))*(*iter);
				if (!IsMod)
				{
					size = ReturnNumBiger(size);
				}
				if ((*iter1)->GPUPart >= nGpu_ && (*iter2)->GPUPart < nGpu_)
				{
					size = (sssg_->GetInitCount(*iter1) + sssg_->GetSteadyCount(*iter1)*Haflp->MultiNum2FlatNode[*iter1] / nGpu_*stageminus)*(*iter);
					int pos = (*iter2)->name.find('_');
					string substring = (*iter2)->name.substr(pos);
					buf<<"cl::Buffer Inbuffer_"<<(*iter1)->name<<"_"<<(*iter2)->name<<"_"<<i<<"(context,CL_MEM_READ_ONLY ,"<<size<<"*sizeof("<<pEdgeInfo->getEdgeInfo((*iter1),(*iter2)).typeName<<"),NULL,NULL);\n";
				}
				
				if((*iter1)->GPUPart < nGpu_ && (*iter2)->GPUPart < nGpu_)
				{
					size = (sssg_->GetInitCount(*iter1) + sssg_->GetSteadyCount(*iter1)*Haflp->MultiNum2FlatNode[*iter1] * (stageminus + 1))*(*iter);
					int pos = (*iter1)->name.find('_');
					string substring = (*iter1)->name.substr(pos);
					buf<<"cl::Buffer Outbuffer_"<<(*iter1)->name<<"_"<<(*iter2)->name<<"_"<<i<<"(context,CL_MEM_WRITE_ONLY ,"<<size<<"*sizeof("<<pEdgeInfo->getEdgeInfo((*iter1),(*iter2)).typeName<<"),NULL,NULL);\n";
				}
				if((*iter1)->GPUPart < nGpu_ && (*iter2)->GPUPart >= nGpu_)
				{
					size = (sssg_->GetInitCount(*iter1) + sssg_->GetSteadyCount(*iter1)*Haflp->MultiNum2FlatNode[*iter1] * stageminus)*(*iter);
					int pos = (*iter1)->name.find('_');
					string substring = (*iter1)->name.substr(pos);
					//iterofqueueid = Node2QueueID.find(*iter1);
					buf<<"cl::Buffer Outbuffer_"<<(*iter1)->name<<"_"<<(*iter2)->name<<"_"<<i<<"(context,CL_MEM_WRITE_ONLY ,"<<size<<"*sizeof("<<pEdgeInfo->getEdgeInfo((*iter1),(*iter2)).typeName<<"),NULL,NULL);\n";
				}
				iter++;
			}
		}
	}
	for (iter1=flatNodes_.begin();iter1!=flatNodes_.end();++iter1)
	{
		//cwb 确定actorfission的个数
		int actorFissionNum = 1;
		typedef multimap<FlatNode*,pair<int,int> >::iterator fission_iter;
		pair<fission_iter,fission_iter> pos;
		if (DNBPFlag)
		{
			
			if((*iter1)->GPUPart >= nGpu_)
			{
				pos = Dnbp->FissionNodes2count.equal_range(*iter1);
				if (pos.first != pos.second)
				{
					while(pos.first != pos.second){actorFissionNum++;pos.first++;}
					actorFissionNum--;
				}
			}
			pos = Dnbp->FissionNodes2count.equal_range(*iter1);
		}
		for (int index = 0; index < actorFissionNum; index++)
		{
			//chenwenbin各actor实例化
			buf<<mapFlatnode2Template_[*iter1]<<"<";
			if((*iter1)->inFlatNodes.size() == 0 && (*iter1)->outFlatNodes.size() != 0)
				buf<<pEdgeInfo->getEdgeInfo(*iter1,(*iter1)->outFlatNodes[0]).typeName;
			else if((*iter1)->inFlatNodes.size() != 0 && (*iter1)->outFlatNodes.size() == 0)
				buf<<pEdgeInfo->getEdgeInfo((*iter1)->inFlatNodes[0],*iter1).typeName;
			else if((*iter1)->inFlatNodes.size() != 0 && (*iter1)->outFlatNodes.size() != 0)
				buf<<pEdgeInfo->getEdgeInfo(*iter1,(*iter1)->outFlatNodes[0]).typeName<<","<<pEdgeInfo->getEdgeInfo((*iter1)->inFlatNodes[0],*iter1).typeName;
			if (actorFissionNum > 1)
			{
				buf<<"> "<<(*iter1)->name<<"_"<<index<<"_obj(";//定义actor对象，actor->name_obj,调用构造函数，参数为输入输出边的全局变量
			}
			else
				buf<<"> "<<(*iter1)->name<<"_obj(";//定义actor对象，actor->name_obj,调用构造函数，参数为输入输出边的全局变量
			//chenwenbin 将各actor对应的param作为参数传入构造函数
			paramList *pList = (*iter1)->contents->params;
			for (int i = 0; i < (*iter1)->contents->paramSize; i++)
			{
				if (pList->paramnode->u.id.value->u.Const.type->typ == Prim) {
					switch (pList->paramnode->u.id.value->u.Const.type->u.prim.basic) {
					case Sint:   buf<<pList->paramnode->u.id.value->u.Const.value.i<<",";break;
					case Uint:   buf<<pList->paramnode->u.id.value->u.Const.value.u<<",";break;
					case Slong:  buf<<pList->paramnode->u.id.value->u.Const.value.l<<",";break;
					case Ulong:  buf<<pList->paramnode->u.id.value->u.Const.value.ul<<",";break;
					case Float:  buf<<pList->paramnode->u.id.value->u.Const.value.f<<",";break;
					case Double: buf<<pList->paramnode->u.id.value->u.Const.value.d<<",";break;
					default:     break;
					}
				}
				pList = pList->next;
			}
			//cwb 将actor每次的读写数据个数传入构造函数
			if ((*iter1)->GPUPart < nGpu_)
			{
				for (int i = 0; i < (*iter1)->inPopWeights.size(); i++)
				{
					buf<<(*iter1)->inPopWeights[i]<<",";
				}
				for (int i = 0; i < (*iter1)->outPushWeights.size(); i++)
				{
					buf<<(*iter1)->outPushWeights[i]<<",";
				}
			}
			else
			{
				for (int i = 0; i < (*iter1)->inPopWeights.size(); i++)
				{
					if(DataDependence((*iter1)->inFlatNodes[i],(*iter1)))
						buf<<(*iter1)->inPopWeights[i]<<",";
				}
				for (int i = 0; i < (*iter1)->outPushWeights.size(); i++)
				{
					if(DataDependence((*iter1),(*iter1)->outFlatNodes[i]))
						buf<<(*iter1)->outPushWeights[i]<<",";
				}
			}
			//chenwenbin 初态稳态执行次数构造
			if (DNBPFlag && actorFissionNum > 1)
			{
				buf<<pos.first->second.second<<","<<sssg_->GetInitCount(*iter1)<<",";
				pos.first++;
			}
			else
				buf << sssg_->GetSteadyCount(*iter1)*Haflp->MultiNum2FlatNode[*iter1] << "," << sssg_->GetInitCount(*iter1) << ",";
			//cwb 
			if ((*iter1)->GPUPart >= nGpu_)
			{
				int index = 0;
				for (; index < (*iter1)->inFlatNodes.size(); ++index)
				{
					if(DataDependence((*iter1)->inFlatNodes[index],(*iter1)))
						buf<<(*iter1)->inFlatNodes[index]->name<<"_"<<(*iter1)->name<<",";
				}
				for (index = 0; index < (*iter1)->outFlatNodes.size(); ++index)
				{
					if(DataDependence((*iter1),(*iter1)->outFlatNodes[index]))
						buf<<(*iter1)->name<<"_"<<(*iter1)->outFlatNodes[index]->name<<",";
				}
			}
			else
			{
				int index = 0;
				for (; index < (*iter1)->inFlatNodes.size(); ++index)
				{
					if(DataDependence((*iter1)->inFlatNodes[index],(*iter1)) && sssg_->IsUpBorder((*iter1)))
						buf<<(*iter1)->inFlatNodes[index]->name<<"_"<<(*iter1)->name<<",";
				}
				for (index = 0; index < (*iter1)->outFlatNodes.size(); ++index)
				{
					if(DataDependence((*iter1),(*iter1)->outFlatNodes[index]) && sssg_->IsDownBorder((*iter1)))
						buf<<(*iter1)->name<<"_"<<(*iter1)->outFlatNodes[index]->name<<",";
				}
			}
			if ((*iter1)->GPUPart < nGpu_)
			{
				vector<FlatNode*>::iterator iter3,iter4;
				for (int i = 0; i < nGpu_; i++)
				{
					for (iter3=(*iter1)->inFlatNodes.begin();iter3!=(*iter1)->inFlatNodes.end();iter3++)
					{
						if (((*iter1)->GPUPart < nGpu_ && (*iter3)->GPUPart >= nGpu_) || ((*iter1)->GPUPart < nGpu_ && (*iter3)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode((*iter1)) != Haflp->findPartitionNumForFlatNode(*iter3)))
						{
							buf<<"Inbuffer_"<<(*iter3)->name<<"_"<<(*iter1)->name<<"_"<<i<<",";
						}
					}
					for (iter3=(*iter1)->inFlatNodes.begin();iter3!=(*iter1)->inFlatNodes.end();iter3++)
					{
						if ((*iter1)->GPUPart < nGpu_ && (*iter3)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode((*iter1)) == Haflp->findPartitionNumForFlatNode(*iter3))
						{
							buf<<"Outbuffer_"<<(*iter3)->name<<"_"<<(*iter1)->name<<"_"<<i<<",";
						}
					}
					for (iter4=(*iter1)->outFlatNodes.begin();iter4!=(*iter1)->outFlatNodes.end();iter4++)
					{	
						if (((*iter1)->GPUPart < nGpu_ && (*iter4)->GPUPart >= nGpu_) || ((*iter1)->GPUPart < nGpu_ && (*iter4)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode((*iter1)) != Haflp->findPartitionNumForFlatNode(*iter4)))
						{
							buf<<"Outbuffer_"<<(*iter1)->name<<"_"<<(*iter4)->name<<"_"<<i<<",";
						}
					}
					for (iter4=(*iter1)->outFlatNodes.begin();iter4!=(*iter1)->outFlatNodes.end();iter4++)
					{
						if ((*iter1)->GPUPart < nGpu_ && (*iter4)->GPUPart < nGpu_ && Haflp->findPartitionNumForFlatNode((*iter1)) == Haflp->findPartitionNumForFlatNode(*iter4))
						{
							buf<<"Outbuffer_"<<(*iter1)->name<<"_"<<(*iter4)->name<<"_"<<i<<",";	
						}
					}
				}
			}
			buf<<"NULL);\n";
		}
	}
	if (TRACE)
	{
		buf<<"double (*deltatimes)["<<nGpu_<<"][2];\n";
		buf<<"typedef double time_array["<<nGpu_<<"][2];\n";
	}
	if (Haflp->CPUNodes.size() > 0)
	{
		for (int i=0;i<nGpu_*2+NCpuThreads;i++)
		{
			//itoa(i, a, 10);  
			sprintf(a, "%d", i);			
			string numname=a;
			buf<<"extern void thread_"<<numname<<"_fun();\n";
		}
	}
	else
	{
		for (int i=0;i<nGpu_;i++)
		{
			//itoa(i, a, 10);  
			sprintf(a, "%d", i);
			string numname=a;
			buf<<"extern void thread_"<<numname<<"_fun();\n";
		}
	}
	if(Win)
	if (Haflp->CPUNodes.size() > 0)
		{
			for(int i=1;i<nGpu_*2+NCpuThreads;i++)
			{
				buf<<"unsigned int WINAPI thread_"<<i<<"_fun_start(void *)\n{\n\tthread_"<<i<<"_fun();\n\treturn 0;\n}\n";
			}
		}
		else
		{
			for(int i=1;i<nGpu_;i++)
			{
				buf<<"unsigned int WINAPI thread_"<<i<<"_fun_start(void *)\n{\n\tthread_"<<i<<"_fun();\n\treturn 0;\n}\n";
			}
		}
	else
	{
		if (Haflp->CPUNodes.size() > 0)
		{
			for(int i=1;i<nGpu_*2+NCpuThreads;i++)
			{
				buf<<"void* thread_"<<i<<"_fun_start(void *)\n{\n\tset_cpu("<<i<<");\n\tthread_"<<i<<"_fun();\n\treturn 0;\n}\n";
			}
		}
		else
		{
			for(int i=1;i<nGpu_;i++)
			{
				buf<<"void* thread_"<<i<<"_fun_start(void *)\n{\n\tset_cpu("<<i<<");\n\tthread_"<<i<<"_fun();\n\treturn 0;\n}\n";
			}
		}
	}
		buf<<"int main(int argc,char **argv)\n{\n";
		buf<<"\tofstream resultfw;\n";
		if(Linux)
		{
			buf<<"\tint oc;\n";
			//buf<<"\tchar *b_opt_arg;\n";
			buf<<"\twhile((oc=getopt(argc,argv,\"i:\"))!=-1)\n";
			buf<<"\t{\n";
			buf<<"\t\tswitch(oc)\n";
			buf<<"\t\t{\n";
			buf<<"\t\t\tcase 'i':\n";
			buf<<"\t\t\t\tMAX_ITER=atoi(optarg);\n";
			buf<<"\t\t\t\tbreak;\n";
			buf<<"\t\t}\n";
			buf<<"\t}\n";
			//buf<<"\tcpu_set_t mask;
			buf<<"\tset_cpu(0);\n";
		}
		if(TRACE){
			buf<<"\tdeltatimes = new time_array[MAX_ITER+"<<2*pSa->MaxStageNum()-1<<"];\n";
		}
		if (Win&&FileRW)
		{
			if (existFileWriter)
			{
				buf<<"\tstring outpath = "<<OutputPath<<";\n";
				buf<<"\tstring inpath = "<<InputPath<<";\n";
				buf<<"\tFileReader(inpath,0);\n";
			}
			else
			{
				buf<<"\tstring inpath = "<<InputPath<<";\n";
				buf<<"\tFileReader(inpath,0);\n";
			}

		}
	//	buf<<"\tallocBarrier("<<nGpu_+1<<");\n";
		if(Win)
		{
			//for windows thread
			if (Haflp->CPUNodes.size() > 0)
			{
				for (int i=1;i<nGpu_*2+NCpuThreads;i++)
				{
					buf<<"\t_beginthreadex(NULL,0,thread_"<<i<<"_fun_start,NULL,0,NULL);\n";
				}
			}
			else
			{
				for (int i=1;i<nGpu_;i++)
				{
					buf<<"\t_beginthreadex(NULL,0,thread_"<<i<<"_fun_start,NULL,0,NULL);\n";
				}
			}
			buf<<"\tthread_0_fun();\n";
			if (FileRW&&existFileWriter)
			{
				buf<<"\tFileWriter(outpath,0);\n";
			}
		}
		else
		{
			//for linux thread
			//buf<<"\tpthread_barrier_init (&barrier, NULL, "<<nGpu_<<");\n";
			if (Haflp->CPUNodes.size() > 0)
			{
				buf<<"\tpthread_t tid["<<nGpu_*2+NCpuThreads-1<<"];\n";
				for (int i=1;i<nGpu_*2+NCpuThreads;i++)
				{
					buf<<"\tpthread_create (&tid["<<i-1<<"], NULL, thread_"<<i<<"_fun_start, (void*)NULL);\n";
				}
			}
			else
			{
				buf<<"\tpthread_t tid["<<nGpu_-1<<"];\n";
				for (int i=1;i<nGpu_;i++)
				{
					buf<<"\tpthread_create (&tid["<<i-1<<"], NULL, thread_"<<i<<"_fun_start, (void*)NULL);\n";
				}
			}
			buf<<"\tthread_0_fun();\n";
		}
		if (TRACE)		//trace for txt
		{
			buf<<"\tstring txtDataFileName = \"traceDat.txt\";\n\tstringstream txtDataContent;\n";
			buf<<"\tdouble perThreadTotalTime["<<nGpu_<<"]={0},perThreadCalucatorTime["<<nGpu_<<"]={0};\n";
			buf<<"\tfor(int threadindex=0;threadindex<"<<nGpu_<<";threadindex++)\n\t{\n";
			buf<<"\t\ttxtDataContent<<\"线程\"<<threadindex<<\"的执行时间如下:\"<<endl;\n";
			buf<<"\t\ttxtDataContent<<\"initworkstage:\"<<endl;\n";
			buf<<"\t\tfor(int stageindex=0;stageindex<"<<pSa->MaxStageNum()<<";stageindex++)\n\t\t{\n";
			buf<<"\t\t\ttxtDataContent<<stageindex<<\":\t\"<<deltatimes[stageindex][threadindex][0]<<\"\t\"<<deltatimes[stageindex][threadindex][1]<<endl;\n\n";
			buf<<"\t\t\tperThreadCalucatorTime[threadindex] += deltatimes[stageindex][threadindex][0];\n";
			buf<<"\t\t\tperThreadTotalTime[threadindex] += (deltatimes[stageindex][threadindex][0]+deltatimes[stageindex][threadindex][1]);\n\t\t}\n";
			buf<<"\t\ttxtDataContent<<\"steadyworkstage:\"<<endl;\n";
			buf<<"\t\tfor(int stageindex=0;stageindex<MAX_ITER+"<<pSa->MaxStageNum()-1<<";stageindex++)\n\t\t{\n";
			buf<<"\t\t\ttxtDataContent<<stageindex<<\":\t\"<<deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][0]<<\"\t\"<<deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][1]<<endl;\n";
			buf<<"\t\t\tperThreadCalucatorTime[threadindex] += deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][0];\n";
			buf<<"\t\t\tperThreadTotalTime[threadindex] += (deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][0]+deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][1]);\n\t\t}\n\t}\n";
			buf<<"\ttxtDataContent<<\"每个线程所用的总时间为：\"<<endl;\n";
			buf<<"\tfor(int threadindex=0;threadindex<"<<nGpu_<<";threadindex++)\n";
			buf<<"\t\ttxtDataContent<<perThreadTotalTime[threadindex]<<endl;\n";
			buf<<"\ttxtDataContent<<\"每个线程总的计算时间为：\"<<endl;\n";
			buf<<"\tfor(int threadindex=0;threadindex<"<<nGpu_<<";threadindex++)\n";
			buf<<"\t\ttxtDataContent<<perThreadCalucatorTime[threadindex]<<endl;\n";
			buf<<"\tofstream txtfw;\n\ttry\n\t{\n\t\ttxtfw.open(txtDataFileName.c_str());\n\t\ttxtfw<<txtDataContent.str();\n\t\ttxtfw.close();\n\t}\n\tcatch(...)\n\t{\n\t\tcout<<\"error:output to file\"<<endl;\n\t}\n";
		}
		if (TRACE)			//trace for image
		{
			buf<<"\tstring imageDataFileName = \"traceDat.dat\";\n\tstringstream imageDataContent;\n\tdouble base;\n";
			buf<<"\tint rectangleIndex = 1;\n\tdouble rectangleHeight = 0.02;\n\timageDataContent<<\"reset;\"\n;\n";
			buf<<"\tfor(int threadindex=0;threadindex<"<<nGpu_<<";threadindex++)\n\t{\n";
			buf<<"\t\tbase = 0;\n";
			buf<<"\t\tfor(int stageindex=0;stageindex<MAX_ITER+"<<2*pSa->MaxStageNum()-1<<";stageindex++)\n\t\t{\n";
			buf<<"\timageDataContent<<\"set object \"<<rectangleIndex++<<\" rectangle from \"<<base<<\",\"<<threadindex*rectangleHeight<<\" to \";\n";
			buf<<"\t\t\tbase += deltatimes[stageindex][threadindex][0];\n";
			buf<<"\timageDataContent<<base<<\",\"<<(threadindex+1)*rectangleHeight<<\" fc rgb \\\"red\\\";\\n\";\n";
			buf<<"\timageDataContent<<\"set object \"<<rectangleIndex++<<\" rectangle from \"<<base<<\",\"<<threadindex*rectangleHeight<<\" to \";\n";
			buf<<"\t\t\tbase += deltatimes[stageindex][threadindex][1];\n";
			buf<<"\timageDataContent<<base<<\",\"<<(threadindex+1)*rectangleHeight<<\" fc rgb \\\"green\\\";\\n\";\n\t\t}\n\t}\n";
			buf<<"\timageDataContent<<\"plot [0:\"<<int(base+2)<<\"][0:\"<<"<<nGpu_<<"*rectangleHeight<<\"] 10000000 notitle;\\n\"; \n";
			buf<<"\tofstream imagefw;\n\ttry\n\t{\n\t\timagefw.open(imageDataFileName.c_str());\n\t\timagefw<<imageDataContent.str();\n\t\timagefw.close();\n\t}\n\tcatch(...)\n\t{\n\t\tcout<<\"error:output to file\"<<endl;\n\t}\n";
		}
		//buf<<"\ttry\n\t{\n\t\ttxtfw.open(\"result.txt\");\n\t\ttxtfw<<coutstream.str();\n\t\tresultfw.close();\n\t}\n\tcatch(...)\n\t\t{cout<<\"error:output to file\"<<endl;\n\t}\n";
		if (!existFileWriter)
		{
			buf<<"\tresultfw.open(\"result.txt\");\n";
			buf<<"\tresultfw<<outstream.str();\n";
			buf<<"\tresultfw.close();\n";
		}
		if (PrintTime)
		{
			if (TimeofEverycore)
			{
				if(NCpuThreads > 0)
					for(int i = 0; i < NCpuThreads; i++)
						buf<<"\tcout<<\"cpu"<<i<<"_compute:  \"<<cpu"<<i<<"_compute<<endl;\n";
				for (int i = 0; i < nGpu_; i++)
				{
					buf<<"\tcout<<\"cpu_gpu_com"<<i<<":  \"<<cpu_gpu_com"<<i<<"<<endl;\n";
				}
				for(int i = 0; i < nGpu_; i++)
					buf<<"\tcout<<\"gpu"<<i<<"_compute: \"<<gpu"<<i<<"_compute<<endl;\n";
			}
			else
			{
				buf<<"\tcpu0_compute /= MAX_ITER;"<<endl;
				for(int i = 0; i < nGpu_; i++)
				{
					buf<<"\tgpu"<<i<<"_compute /= MAX_ITER;"<<endl;
					buf<<"\tcpu_gpu_com"<<i<<" /= MAX_ITER;"<<endl;
				}
				buf<<"\tdouble gpu_compute = gpu0_compute,cpu_gpu_com = cpu_gpu_com0;"<<endl;
				for (int i = 1; i < nGpu_; i++)
				{
					buf<<"\tgpu_compute = (gpu_compute > gpu"<<i<<"_compute)?gpu_compute:gpu"<<i<<"_compute;"<<endl;
				}
				for (int i = 1; i < nGpu_; i++)
				{
					buf<<"\tcpu_gpu_com = (cpu_gpu_com > cpu_gpu_com"<<i<<")?cpu_gpu_com:cpu_gpu_com"<<i<<";"<<endl;
				}
				buf<<"\tcout<<\""<<BenchmarkName<<":\"<<cpu0_compute<<\" \"<<gpu_compute<<\" \"<<cpu_gpu_com<<endl;"<<endl;
			}
		}
		buf<<"\treturn 0;\n";
		buf<<"\n}"<<endl;
		ss<<dir_<<"main.cpp";
		OutputToFile(ss.str(),buf.str());
}
int GPUCodeGenerate::OutputChar(char val)
{
	switch(val) 
	{
	case '\n': declInitList<<"\\n";break;
	case '\t': declInitList<<"\\t";break;
	case '\v': declInitList<<"\\v";break;
	case '\b': declInitList<<"\\b";break;
	case '\r': declInitList<<"\\r";break;
	case '\f': declInitList<<"\\f";break;
	case '\a': declInitList<<"\\a";break;
	case '\\': declInitList<<"\\\\";break;
	case '\?': declInitList<<"\\\?";break;
	case '\"': declInitList<<"\\\"";break;
	case '\'': declInitList<<"\\\'";break;
	default:
		if (isprint(val)) //判断val是否为可打印字符
		{
			declInitList<<(val);
		} else 
		{
			declInitList<<"\\"<<val;
		}
	}

	return 1;
}
int GPUCodeGenerate::OutputString(const char *s)
{ 
	int len = 0;

	declInitList<<"\"";
	while (*s != 0) 
	{
		len += OutputChar(*s++);
	}
	declInitList<<"\"";

	return len + 2;
}
void GPUCodeGenerate::CharToText(char *str, unsigned char val){ 
	if (val < ' ') 
	{
		static const char *names[32] = {
			"nul","soh","stx","etx","eot","enq","ack","bel",
			"\\b", "\\t", "\\n", "\\v", "ff", "cr", "so", "si",
			"dle","dc1","dc2","dc3","dc4","nak","syn","etb",
			"can","em", "sub","esc","fs", "gs", "rs", "us" };
			sprintf(str, "0x%02x (%s)", val, names[val]);
	} else if (val < 0x7f) 
	{
		sprintf(str, "'%c'", val);
	} else if (val == 0x7f) 
	{
		strcpy(str, "0x7f (del)");
	} else 
	{ /* val >= 0x80 */
		sprintf(str, "0x%x", val);
	}
}
void GPUCodeGenerate::OutputStmt(Node *node, int offset)
{
	if (node->typ != Block)
		OutputTabs(offset);
	if (node == NULL)
	{
		/* empty statement */
		declInitList << ";";
		return;
	}
	SPL2GPU_Node(node, offset);
	switch (node->typ)
	{
	case Id:
	case Const:
	case Binop:
	case Unary:
	case Cast:
	case Ternary:
	case Comma:
	case Call:
	case Array:
	case ImplicitCast:
		//OutputExpr(out, node);
		declInitList << ";\n";
		break;
	case Label:

		break;
	case Switch:

		break;
	case Case:

		break;
	case Default:

		break;
	case If:

		break;
	case IfElse:

		break;
	case While:

		break;
	case Do:

		break;
	case For:

		break;	
	case Goto:

		break;
	case Continue:

		break;
	case Break:

		break;
	case Return:

		break;
	case Block:

		break;
	case Text:

		break;
	default:
		fprintf(stderr, "Internal error: unexpected node");
		PrintNode(stderr, node, 2);
		UNREACHABLE;
	}

}
void GPUCodeGenerate::OutputStmtList(List *list, int offset)
{
	ListMarker marker; 
	Node *item = NULL; 

	IterateList(&marker, list); 
	while (NextOnList(&marker, (GenericREF)&item))
	{ 
		OutputStmt(item, offset);
	}
}
//多维数组初始化过程（递归）
void GPUCodeGenerate::RecursiveAdclInit(List *init)
{
	//多维数组则init是一个多维的链表
	ListMarker marker;
	Node *item;
	int i=1;
	int len = ListLength(init);
	IterateList(&marker, init);
	while ( NextOnList(&marker, (GenericREF) &item) ) 
	{
		if(i==1) declInitList<<"{";
		if (item->typ == Unary)
		{
			if(i!=1) declInitList<<",";
			declInitList<<GetOpType(item->u.unary.op);
			SPL2GPU_Node(item->u.unary.expr,0);
		}
		else if(item->typ == Const) // 如果数组成员是基本类型
		{
			if(i!=1) declInitList<<",";
			SPL2GPU_Node(item,0);
			//if(i == 1) declInitList<<" as "<<ArrayDT;//为GPU-2.2作出的调整
		}
		else if (item->typ == Initializer)//如果数组成员是一个数组则递归
		{
			RecursiveAdclInit(item->u.initializer.exprs);
			if(i != len)
			{
				declInitList<<",";
				OutputCRSpaceAndTabs(4);
			}
		}
		else if (item->typ == ImplicitCast)//基本类型的隐式转换
		{
			if(i!=1) declInitList<<",";
			SPL2GPU_Node(item->u.implicitcast.value,0);
		}
		i++;
	}
	declInitList<<"}";
}
void GPUCodeGenerate::CGTestFile()
{
	stringstream buf;
	stringstream fileName;
	buf<<"/*测试代码，无实际含义*/\n\n";
	fileName<<dir_<<"test.txt";
	OutputToFile(fileName.str(),buf.str());
}
void GPUCodeGenerate::AdclInit(Node *from, int offset)
{
	Node *arrayNode = from->u.decl.type;
	Node *tmpNode = from->u.decl.type;
	Node *initNode = from->u.decl.init;
	string name = from->u.decl.name;
	string arrayType = GetArrayDataType(tmpNode->u.adcl.type);
	string dim = GetArrayDim(tmpNode->u.adcl.dim);
	//declList << "\tvar "<< name << " :Array[" << arrayType << "](1);\n";
	//declList<<arrayType<<" "<<name<<"[];\n";
	if (initNode == NULL) //如果没有初始化，则按数组类型进行初始化
	{
		//declInitList << "\t\t"<<name<<" = new Array["<<arrayType<<"](0..("<<dim<<"-1), ";
		declList<<arrayType<<"*"<<name<<";\n";
		//declInitList<<"\t\t"<<name<<"=new "<<arrayType<<"["<<dim<<"];\n";
		declInitList<<declInitList_temp.str()<<"={0};\n";
		declInitList_temp.str("");
		//GetArrayDataInitVal(tmpNode, declInitList); //数组初始化
		//declInitList << ");\n";//一个数组声明加初始化
	}
	else//如果存在初始化则初始化为指定值
	{
		declInitList<<declInitList_temp.str();
		declInitList_temp.str("");
		if(from->u.decl.type->typ == Adcl && ((from->u.decl.type)->u.adcl.type)->typ== Adcl)
		{
			//declList<<arrayType<<" "<<name<<"["<< <<"]"		//GPU-2.2多维数组声明的改动
			string dim1= GetArrayDim(from->u.decl.type->u.adcl.type->u.adcl.dim);
			declList<<arrayType<<" "<<name<<"["<<dim<<"]["<<dim1<<"];\n";
		}   
		else if (from->u.decl.type->typ == Adcl)
		{
			string dim = GetArrayDim(tmpNode->u.adcl.dim);
			declList<<arrayType<<" "<<name<<"["<<dim<<"];\n";
		}
		List *tmp = initNode->u.initializer.exprs;
		int n = ListLength(tmp);
		if(n == 1) // 如果初始化个数1个单一值时，则将数组所有成员初始化为该值
		{
			Node *item = (Node *)FirstItem(tmp);
			if(item->typ == Const)
			{
				stringstream ss;
				ss << item->u.Const.text;
				declInitList <<name<<" :Array["<<arrayType<<"](1) = new Array["<<arrayType<<"](0..("<<dim<<"-1), "<<ss.str()<<");\n";
				declInitList<<arrayType<<" *"<<name<<";\n";
			}
		}
		else //初始的个数和数组维数一致，则可以采取这样的赋值形式：val pp:Array[Int](1) = [1,2,3,4,5];
		{	
			declInitList<<" = ";
			RecursiveAdclInit(tmp);
			declInitList<<";\n";

		}
	}
}
void GPUCodeGenerate::OutputConstant(Node *c, Bool with_name)
{
	int len = 0;
	const char *tmpString = c->u.Const.text;
	//if (tmpString[0]=='0' && tmpString[1]=='x')//如果是0x表示十六进制
	//{
	//	declInitList<< c->u.Const.text; // 如果是0x表示十六进制则以0x表示
	//	return;
	//}

	switch (c->u.Const.type->typ)
	{
	case Prim:
		switch (c->u.Const.type->u.prim.basic) 
		{
		case Sint:
			declInitList<< c->u.Const.value.i;
			break;
		case Uint:
			declInitList<< c->u.Const.value.u;
			break;
		case Slong:
			declInitList<< c->u.Const.value.l<<"L";
			break;
		case Ulong:
			declInitList<< c->u.Const.value.ul<<"UL";
			break;
		case Float:
			if(c->u.Const.value.d==0.0) declInitList<<"0.0";
			else
				declInitList<<c->u.Const.value.f;
			break;
		case Double:
			if(c->u.Const.value.d==0.0) declInitList<<"0.0";
			else if(c->u.Const.value.d - (int)c->u.Const.value.d == 0)
				//declInitList<<c->u.Const.value.d<<".0";
				declInitList<<c->u.Const.value.d;
			else 
				declInitList<<c->u.Const.value.d;
			break;
		case Char:
		case Schar:
		case Uchar:
			OutputChar(c->u.Const.value.i);
			break;

		default:
			Fail(__FILE__, __LINE__, "");
			return ;
		}
		break;
	case Ptr:
		UNREACHABLE;
		// declInitList<< c->u.Const.value.u;//spl没指针，可以忽略
		break;
		/* Used for strings */
	case Adcl:
		OutputString(c->u.Const.value.s);
		break;
	default:
		assert(("Unrecognized constant type", TRUE));
		return ;
	}
}

void GPUCodeGenerate::SPL2GPU_Node(Node *node, int offset)
{
	if (node == NULL) return;

	if(node->parenthesized == TRUE) declInitList << "("; //加括号保证逻辑性

#define CODE(name, node, union) SPL2GPU_##name(node, union, offset)
	ASTSWITCH(node, CODE)
#undef CODE

		if(node->parenthesized == TRUE) declInitList << ")"; //加括号保证逻辑性
}

void GPUCodeGenerate::SPL2GPU_List(List *list, int offset)
{
	ListMarker marker; 
	Node *item = NULL; 

	IterateList(&marker, list); 
	while (NextOnList(&marker, (GenericREF)&item))
	{ 
		SPL2GPU_Node(item, offset);
	}

}

/*******************************************************************

********************************************************************/
void GPUCodeGenerate::SPL2GPU_STRdcl(Node *node, strdclNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Const(Node *node, ConstNode *u, int offset)
{
	OutputConstant(node, TRUE);
}

void GPUCodeGenerate::SPL2GPU_Id(Node *node, idNode *u, int offset)
{
	declInitList << u->text;
	//cout<<"\nID= "<<u->text<<endl;
}
string GPUCodeGenerate::GetOpType(OpType op)
{
	switch(op)
	{
	case '.':return ".";
	case '&':return "&";
	case '*':return " * ";
	case UPLUS:return "+";
	case '+':return " + ";
	case UMINUS:return "-";
	case '-':return " - ";
	case '~':return "~";
	case '!':return "!";
	case '/':return " / ";
	case '%':return " % ";
	case '<':return " < ";
	case '>':return " > ";
	case '^':return "^";
	case '|':return " | ";
	case '?':return " ? ";
	case ':':return " : ";
	case ';':return "; ";
	case '=':return " = ";
	case ARROW:return "->";
	case PREINC:
	case POSTINC:return "++";
	case POSTDEC:
	case PREDEC:return "--";
	case LS:return " << ";
	case RS:return " >> ";
	case LE:return " <= ";
	case GE:return " >= ";
	case EQ:return " == ";
	case NE:return " != ";
	case ANDAND:return " && ";
	case OROR:return " || ";
	case MULTassign:return " *= ";
	case DIVassign:return " /= ";
	case MODassign:return " %= ";
	case PLUSassign:return " += ";
	case MINUSassign:return " -= ";
	case LSassign:return " <<= ";
	case RSassign:return " >>= ";
	case ANDassign:return " &= ";
	case ERassign:return " ^= ";
	case ORassign:return " |= ";
	default:
		FAIL("Unrecognized node type in CodeGeneration!"); break; 
	}
}
void GPUCodeGenerate::SPL2GPU_Binop(Node *node, binopNode *u, int offset)
{
	//OutputTabs(offset);
	if (node->u.binop.left) 
		SPL2GPU_Node(node->u.binop.left,offset); 
	declInitList << GetOpType(node->u.binop.op);
	if (node->u.binop.right) 
		SPL2GPU_Node(node->u.binop.right, offset);

	//declInitList << ";\n";
}

void GPUCodeGenerate::SPL2GPU_Unary(Node *node, unaryNode *u, int offset)
{
	if (node->u.unary.op != POSTINC && node->u.unary.op != POSTDEC)
		declInitList << GetOpType(node->u.binop.op);
	if (node->u.unary.expr) 
		SPL2GPU_Node(node->u.unary.expr, offset); 
	if(node->u.unary.op == POSTINC || node->u.unary.op == POSTDEC)
		declInitList << GetOpType(node->u.binop.op);
}
string GPUCodeGenerate::GetPrimDataType(Node *from)//类型定义
{
	string type;

	switch(from->u.prim.basic){
	case Sshort:
	case Sint:
		type = "int";;
		break;
		/*Manish 2/3 hack to print pointer constants */
	case Uint:
	case Ushort:
		type = "UInt";;
		break;
	case Slong:
		type = "Long";;
		break;
	case Ulong:
		type = "ULong";;
		break;
	case Float:
		type = "float";
		break;
	case Double:
		type = "double";
		break;
	case Char:
	case Schar:
	case Uchar:
		type = "char";
		break;
	default: type = "Any";
		break;
	}
	/*if (from->u.prim.basic >= Uchar && from->u.prim.basic <= Char)
	{
	type = "Char";
	}else if (from->u.prim.basic >= Ushort && from->u.prim.basic <= Int_ParseOnly)
	{
	type = "Int";
	}else if (from->u.prim.basic >= Ulong && from->u.prim.basic <= Slonglong)
	{
	type = "Long";
	}else if (from->u.prim.basic == Float)
	{
	type = "Float";
	}else if(from->u.prim.basic >= Double && from->u.prim.basic <= Longdouble){
	type = "Double";
	}else if (from->u.prim.basic == string8)
	{
	type = "String";
	}else
	type = "Any";*/
	return type;
}
//取数组成员的初始值
void GPUCodeGenerate::GetArrayDataInitVal(Node *node, stringstream &strInit)
{
	assert(node->typ == Adcl);
	if (node->u.adcl.type->typ == Adcl) //如果数组的成员仍然是一个数组的话则递归
	{
		node = node->u.adcl.type; // 取出成员数组
		string arrayType = GetArrayDataType(node->u.adcl.type);
		string dim = GetArrayDim(node->u.adcl.dim);
		strInit <<"([i]:Point(1)) => new Array["<<arrayType<<"](0..("<<dim<<"-1),";
		GetArrayDataInitVal(node,strInit);
		strInit<<")";
	} 
	else if(node->u.adcl.type->typ == Prim) // 数组成员是基本类型
	{
		strInit << GetDataInitVal(GetArrayDataType(node->u.adcl.type));
	}
	else  // 暂不处理其他复合类型
		UNREACHABLE;
}

//取数据成员的初始值
string GPUCodeGenerate::GetDataInitVal(string type)
{
	string s ;
	if (type == "Char")
	{
		s = "''";
	}else if (type == "UInt")
	{
		s = "0u";
	}
	else if (type == "Int")
	{
		s = "0";
	}else if (type == "Long")
	{
		s = "0l";
	}
	else if (type == "ULong")
	{
		s = "0ul";
	}
	else if (type == "Float" || type == "Double")
	{
		s = "0.0";
	}
	else if (type == "String")
	{
		s = "\"\"";
	}
	else if (type == "Any")
	{
		s = "null";
	}else//如果数组成员非基本类型则初始化为null，作为一个未知类的初值，例如：数组成员也是个数组
		s = "null";
	return s;
}
//取数组的维数
string GPUCodeGenerate::GetArrayDim(Node *from)
{
	string dim;
	if (from->typ == Const)//如果维数节点类型为常量，例如a[10]
	{
		if(from->u.Const.text)dim = from->u.Const.text;
		else 
		{
			char *tmpdim = (char *)malloc(20);
			sprintf(tmpdim,"%d",from->u.Const.value.l);//20120322 zww添加
			dim = tmpdim;
		}
	}
	else if (from->typ == Id)
	{
		dim = from->u.id.text;
	}
	else if(from->typ == Binop)
	{
		string tmp = declInitList.str(); // 保存
		stringstream tmp2;

		declInitList.str("");
		SPL2GPU_Node(from, 0);
		tmp2 << "(" << declInitList.str() << ")";
		dim = tmp2.str();
		declInitList.str("");
		declInitList << tmp; // 恢复 
	}
	else
		UNREACHABLE;

	return dim;
}

string GPUCodeGenerate::GetArrayDataType(Node *node)
{
	string type;
	if (node->typ == Prim) //基本类型
	{
		type = GetPrimDataType(node);
	}
	else if (node->typ == Adcl) // 也是个数组则递归查找类型
	{
		stringstream ss;
		//ss<<"Array["<<GetArrayDataType(node->u.adcl.type)<<"]";
		ss<<GetArrayDataType(node->u.adcl.type);
		type = ss.str();
	}
	else // 如果数组的成员是复杂类型，则有待扩展
	{
		Warning(1,"this arrayDataType can not be handle!");
		type = "Any";// 暂时返回一种通用类型
		UNREACHABLE;
	}
	return type;
}
void GPUCodeGenerate::ExtractDeclVariables(Node *from)
{
	stringstream tempdeclList,tempdeclInitList;
	if (from->u.decl.type->typ == Prim) // 基本类型
	{
		Node *typeNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string type = GetPrimDataType(typeNode);
		string name = from->u.decl.name;
		char tempvalude[20];
		//	declList << "\tvar "<< name << " :"<<type<<";\n"; // 建立变量声明
		if (flag)//表明此时正在输出param参数
		{
			//sprintf(tempvalude,"%g",initNode->u.Const.value);
			declList<<"\t"<<type<<" "<<name<<";\n";
			nameandtypeofedge.clear();
			nameandtypeofedge.insert(make_pair(type,name));
			allnameandtype.insert(make_pair(type,name));
			edgexy<<name;
			structdatatype = type;
			//structdataname = name;
		}
		else
		{
			declList<<"\t"<<type<<" "<<name<<";\n";
			nameandtypeofedge.clear();
			nameandtypeofedge.insert(make_pair(type,name));
			allnameandtype.insert(make_pair(type,name));
			edgexy<<name;
			structdatatype = type;
			//structdataname = name;
			kernelparamtype.push_back(type);
			kernelparam.push_back(name);
		}	
		if (initNode) // 存在初始化则进行初始化
		{
			declInitList << "\t\t"<<name<<" = ";
			SPL2GPU_Node(initNode, 0);
			declInitList <<";\n";
		}

		if (isInParam)
		{
			//parameterBuf << name << " :"<<type<<", ";//生成this构造函数参数
			parameterBuf<<type<<" "<<name<<", ";
			thisBuf << "\t\tthis." << name << " = " << name <<";\n";
		}
	}
	else if (from->u.decl.type->typ == Adcl) // 数组, 最多处理二维数组, 高维待扩展
	{
		Node *arrayNode = from->u.decl.type;
		Node *tmpNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string name = from->u.decl.name;
		string arrayType = GetArrayDataType(tmpNode->u.adcl.type);
		string dim = GetArrayDim(tmpNode->u.adcl.dim);
		string dim1;
		bool isInString = false;

		if (strcmp(arrayType.c_str(), "char") == 0) // 对字符串数组作处理， 2012.03.07
		{
			char *tmp = new char[20];
			sprintf(tmp, "%d", from->u.decl.type->u.adcl.size);
			dim = tmp;
			isInString = true;
		}
		else
			dim = GetArrayDim(tmpNode->u.adcl.dim);

		if (isInString)
			//declList << "\tvar "<< name << " :String;\n";
		{
			declList<<"string "<<name<<";\n";
		}
		if (strcmp(name.c_str(),"path")==0&&isInFileReader)
		{
			InputPath = from->u.decl.init->u.Const.text;
		}
		else if (strcmp(name.c_str(),"path")==0&&isInFileWriter)
		{
			OutputPath = from->u.decl.init->u.Const.text;
		}
		if (strcmp(name.c_str(),"path")==0&&isInFileReader)
		{
			InputPath = from->u.decl.init->u.Const.text;
		}
		else if (strcmp(name.c_str(),"path")==0&&isInFileWriter)
		{
			OutputPath = from->u.decl.init->u.Const.text;
		}
		if (initNode == NULL) //如果没有初始化，则按数组类型进行初始化
		{
			map<string,string>submap1;
			map<string,map<string,string> >submap2;
			if(from->u.decl.type->typ == Adcl && ((from->u.decl.type)->u.adcl.type)->typ== Adcl)
			{
				dim1= GetArrayDim(from->u.decl.type->u.adcl.type->u.adcl.dim);
				declList<<arrayType<<" (*"<<name<<")["<<dim1<<"];\n";
				declInitList << "\t\t"<<name<<" = new "<<arrayType<<"["<<dim<<"]["<<dim1<<"]();\n";
				ptrname.push_back(name);
				ptrtype.push_back(arrayType);
				ptrdim.insert(make_pair(dim,dim1));
				submap1.clear();
				submap1.insert(make_pair(dim,dim1));
				submap2.insert(make_pair(name,submap1));
				alllocalvariable.insert(make_pair(curactor,submap2));
				arraylocal2Dname.insert(name);
			}   
			else if (from->u.decl.type->typ == Adcl)
			{
				declList<<arrayType<<" *"<<name<<";\n";
				declInitList << "\t\t"<<name<<" = new "<<arrayType<<"["<<dim<<"]();\n";
				ptrname.push_back(name);
				ptrtype.push_back(arrayType);
				ptrdim.insert(make_pair(dim,"0"));
				submap1.insert(make_pair(dim,"0"));
				submap2.insert(make_pair(name,submap1));
				alllocalvariable.insert(make_pair(curactor,submap2));
			}
		}
		else//如果存在初始化则初始化为指定值
		{
			if (1)
			{
				int nodeIndex;
				for (nodeIndex = 0; nodeIndex < vTemplateNode_.size(); nodeIndex++)
				{
					if(curactor == vTemplateNode_[nodeIndex])
						break;
				}
				//cwb
				if(curactor->inFlatNodes.size() == 0 && curactor->outFlatNodes.size() != 0)
				{
					tempdeclList<<"template<typename T>\n";
					tempdeclList<<arrayType<<" "<<vTemplateName_[nodeIndex]<<"<T>";
				}
				else if(curactor->inFlatNodes.size() != 0 && curactor->outFlatNodes.size() == 0)
				{
					tempdeclList<<"template<typename U>\n";
					tempdeclList<<arrayType<<" "<<vTemplateName_[nodeIndex]<<"<U>";
				}
				else if(curactor->inFlatNodes.size() != 0 && curactor->outFlatNodes.size() != 0)
				{
					tempdeclList<<"template<typename T,typename U>\n";
					tempdeclList<<arrayType<<" "<<vTemplateName_[nodeIndex]<<"<T,U>";
				}
				if (isInFileReader||isInFileWriter)
				{
					declInitList << "\t\t"<<name<<" = "<<from->u.decl.init->u.Const.text;
					declInitList << ";\n";
				}
				else
				{
					map<string,string>submapofstaticvar;
					map<string,map<string,string> >submapofstatic;
					if(from->u.decl.type->typ == Adcl && ((from->u.decl.type)->u.adcl.type)->typ== Adcl)
					{
						//declList<<arrayType<<" "<<name<<"["<< <<"]"		//GPU-2.2多维数组声明的改动
						dim1= GetArrayDim(from->u.decl.type->u.adcl.type->u.adcl.dim);
						declList<<"static "<<arrayType<<" "<<name<<"["<<dim<<"]["<<dim1<<"];\n";
						submapofstaticvar.clear();
						submapofstaticvar.insert(make_pair(dim,dim1));
						submapofstatic.clear();
						submapofstatic.insert(make_pair(name,submapofstaticvar));
						staticvariable.push_back(submapofstatic);
						staticvartype.push_back(arrayType);
						allstaticvariable.insert(make_pair(curactor,submapofstatic));
						allstatictype.insert(make_pair(curactor,staticvartype));
						tempdeclList<<"::"<<name<<"["<<dim<<"]["<<dim1<<"]=";
						array2Dname.insert(name);
					}   
					else if (from->u.decl.type->typ == Adcl)
					{
						declList<<"static "<<arrayType<<" "<<name<<"["<<dim<<"];\n";
						submapofstaticvar.clear();
						submapofstaticvar.insert(make_pair(dim,"0"));
						submapofstatic.clear();
						submapofstatic.insert(make_pair(name,submapofstaticvar));
						staticvariable.push_back(submapofstatic);
						staticvartype.push_back(arrayType);
						allstaticvariable.insert(make_pair(curactor,submapofstatic));
						allstatictype.insert(make_pair(curactor,staticvartype));
						tempdeclList<<"::"<<name<<"["<<dim<<"]=";
						array1Dname.insert(name);
					}
					else
						declInitList << "\t\t"<<name<<" = "<<from->u.decl.init->u.Const.text;
					//declInitList << ";\n";
				}

			}
			//else
			//{
			if (!isInFileReader&&!isInFileWriter)
			{
				List *tmp = initNode->u.initializer.exprs;
				int n = ListLength(tmp);
				if(n == 1) // 如果初始化个数1个单一值时，则将数组所有成员初始化为该值
				{
					Node *item = (Node *)FirstItem(tmp);
					if(item->typ == Const)
					{
						stringstream ss;
						ss << item->u.Const.text;
						if (ss.str()=="NULL")
						{
							declInitList<<arrayType<<" "<<name<<"["<<dim<<"];\n";
						}
						else
						{
							declInitList<<arrayType<<" "<<name<<"["<<dim<<"]={"<<ss.str()<<"};\n";
						}
					}
				}
				else //初始的个数和数组维数一致，则可以采取这样的赋值形式：val pp:Array[Int](1) = [1,2,3,4,5];
				{	
					RecursiveAdclInit(tmp);
					tempdeclInitList<<tempdeclList.str()<<declInitList.str()<<";\n";
					declInitList.str("");
				}
				//}
			}
			staticname2value.push_back(tempdeclInitList.str());
		}

		if (isInParam)
		{
			//parameterBuf << name << " :Array[" << arrayType << "](1);\n";
			parameterBuf<<arrayType<<" "<<name<<"[];\n";
			thisBuf << "\t\tthis." << name << " = " << name <<";\n";
		}
	}
	else if (from->u.decl.type->typ == Ptr) // 指针，只能出现在param中
	{
	}
	else
		UNREACHABLE;
}

void GPUCodeGenerate::SPL2GPU_Cast(Node *node, castNode *u, int offset)
{
	declInitList<<"(";
	declInitList<<GetPrimDataType(node->u.cast.type)<<")";
	SPL2GPU_Node(node->u.cast.expr,offset);

}

void GPUCodeGenerate::SPL2GPU_Comma(Node *node, commaNode *u, int offset)
{
	isInComma = true;//正处于逗号表达式中
	SPL2GPU_List(u->exprs,offset);
	isInComma = false;
}

void GPUCodeGenerate::SPL2GPU_Ternary(Node *node, ternaryNode *u, int offset)
{
	SPL2GPU_Node(u->cond,offset);
	declInitList<<" ? ";
	SPL2GPU_Node(u->true_,offset);
	declInitList<<":";
	SPL2GPU_Node(u->false_,offset);
}

void GPUCodeGenerate::SPL2GPU_Array(Node *node, arrayNode *u, int offset)
{
	SPL2GPU_Node(u->name,offset);
	List *tmp = u->dims;
	while(tmp != NULL)//可能是多维数组，需要遍历dim这个list
	{
		declInitList<<"[";
		Node *item = (Node *)FirstItem(tmp);
		SPL2GPU_Node(item,offset);
		declInitList<<"]";
		tmp = Rest(tmp);
	}
}

void GPUCodeGenerate::SPL2GPU_Call(Node *node, callNode *u, int offset)
{
	assert(u->name->typ == Id);
	{
		int flag=1;//标识是否加括号
		const char *ident = u->name->u.id.text;
		if (strcmp(ident,"acos") == 0) declInitList<<"acos";
		else if (strcmp(ident,"acosh")==0) declInitList<<"acosh";
		else if (strcmp(ident,"acosh")==0) declInitList<<"acosh";
		else if (strcmp(ident,"asin")==0) declInitList<<"asin";
		else if (strcmp(ident,"asinh")==0) declInitList<<"asinh";
		else if (strcmp(ident,"atan")==0) declInitList<<"atan";
		else if (strcmp(ident,"atan2")==0) declInitList<<"atan2";
		else if (strcmp(ident,"atanh")==0) declInitList<<"atanh";
		else if (strcmp(ident,"ceil")==0) declInitList<<"ceil";
		else if (strcmp(ident,"cos")==0) declInitList<<"cos";
		else if (strcmp(ident,"cosh")==0) declInitList<<"cosh";
		else if (strcmp(ident,"exp")==0) declInitList<<"exp";
		else if (strcmp(ident,"expm1")==0) declInitList<<"expm1";
		else if (strcmp(ident,"floor")==0) declInitList<<"floor";
		else if (strcmp(ident,"fmod")==0) declInitList<<"fmod";
		else if (strcmp(ident,"frexp")==0) declInitList<<"frexp";
		else if (strcmp(ident,"log")==0) declInitList<<"log";
		else if (strcmp(ident,"log10")==0) declInitList<<"log10";
		else if (strcmp(ident,"log1p")==0) declInitList<<"log1p";
		else if (strcmp(ident,"modf")==0) declInitList<<"modf";
		else if (strcmp(ident,"pow")==0) declInitList<<"pow";
		else if (strcmp(ident,"sin")==0) declInitList<<"sin";
		else if (strcmp(ident,"sinh")==0) declInitList<<"sinh";
		else if (strcmp(ident,"sqrt")==0) declInitList<<"sqrt";
		else if (strcmp(ident,"tan")==0) declInitList<<"tan";
		else if (strcmp(ident,"tanh")==0) declInitList<<"tanh";
		// not from profiling: round(x) is currently macro for floor((x)+0.5)
		else if (strcmp(ident,"round")==0) declInitList<<"round";
		// not from profiling: just stuck in here to keep compilation of gmti 
		// from spewing warnings.
		else if (strcmp(ident,"abs")==0) declInitList<<"abs";
		else if (strcmp(ident,"max")==0) declInitList<<"max";
		else if (strcmp(ident,"min")==0) declInitList<<"min";
		else if (strcmp(ident,"println")==0) {flag=2;}
		else if (strcmp(ident,"printf")==0) {declInitList<<"cout<<";flag=3;}
		else if (strcmp(ident,"print")==0) 
		{
			//declInitList<<"cout<<";
			flag=4;
		}
		else //unkonwn methods
		{
			;
		}
		if (flag==1)
		{
			declInitList<<"(";
			OutputArgList(u->args,offset);//参数
			declInitList<<")";
		}
		else if(flag==2)
		{
			if (FileRW)
			{
				declInitList<<"outstream<<";
				OutputArgList(u->args,offset);
				declInitList<<"<<endl";
			}
			else
			{
				declInitList<<"cout<<";
				OutputArgList(u->args,offset);//参数
				declInitList<<"<<endl";
			}
		}
		else if (flag==4)
		{
			if (FileRW)
			{
				declInitList<<"outstream<<";
				OutputArgList(u->args,offset);
				//declInitList<<"<<endl";
			}
			else
			{
				declInitList<<"cout<<";
				OutputArgList(u->args,offset);//参数
			}
		}
		else
		{
			OutputArgList(u->args,offset);//参数
		}
	}
}

void GPUCodeGenerate::SPL2GPU_Initializer(Node *node, initializerNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_ImplicitCast(Node *node, implicitcastNode *u, int offset)
{
	SPL2GPU_Node(u->expr, offset);
}

void GPUCodeGenerate::SPL2GPU_Label(Node *node, labelNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Switch(Node *node, SwitchNode *u, int offset)
{
	declInitList<<"switch (";
	SPL2GPU_Node(u->expr,offset);
	declInitList<<")";
	SPL2GPU_Node(u->stmt,offset);
}

void GPUCodeGenerate::SPL2GPU_Case(Node *node, CaseNode *u, int offset)
{
	declInitList<<"case ";
	SPL2GPU_Node(u->expr,offset);
	declInitList<<":";
	OutputCRSpaceAndTabs(offset+1);
	SPL2GPU_Node(u->stmt,offset);
}

void GPUCodeGenerate::SPL2GPU_Default(Node *node, DefaultNode *u, int offset)
{
	declInitList<<"default: ";
	OutputCRSpaceAndTabs(offset);
	SPL2GPU_Node(u->stmt,offset);
}

void GPUCodeGenerate::SPL2GPU_If(Node *node, IfNode *u, int offset)
{
	declInitList<<"if (";
	SPL2GPU_Node(u->expr,offset);
	declInitList<<")";
	if (u->stmt->typ != Block) // 如果是非block结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2GPU_Node(u->stmt,offset+1);
	if(u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList<<";";
}

void GPUCodeGenerate::SPL2GPU_IfElse(Node *node, IfElseNode *u, int offset)
{
	declInitList<<"if (";
	SPL2GPU_Node(u->expr,offset);
	declInitList<<")";
	if (u->true_->typ != Block)//如果是非block结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2GPU_Node(u->true_,offset);
	if(u->true_->typ == Binop || u->true_->typ == Unary || u->true_->typ == Ternary || u->true_->typ == Call || u->true_->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList<<";";
	OutputCRSpaceAndTabs(offset);
	declInitList<<"else ";
	if (u->false_->typ != Block && u->false_->typ != IfElse)//如果是非block结点或者ifelse结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2GPU_Node(u->false_,offset);
	if(u->false_->typ == Binop || u->false_->typ == Unary || u->false_->typ == Ternary || u->false_->typ == Call || u->false_->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList<<";";
}

void GPUCodeGenerate::SPL2GPU_While(Node *node, WhileNode *u, int offset)
{
	declInitList<<"while (";
	SPL2GPU_Node(u->expr,offset);
	declInitList<<")";
	if (u->stmt->typ != Block)//如果是非block结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2GPU_Node(u->stmt,offset);
	if(u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList<<";";
}

void GPUCodeGenerate::SPL2GPU_Do(Node *node, DoNode *u, int offset)
{
	declInitList<<"do";
	SPL2GPU_Node(u->stmt,offset);
	declInitList<<"while (";
	SPL2GPU_Node(u->expr,offset);
	declInitList<<")";
}

void GPUCodeGenerate::SPL2GPU_For(Node *node, ForNode *u, int offset)
{
	declInitList<<"for (";
	SPL2GPU_Node(u->init,offset);
	declInitList<<";";
	SPL2GPU_Node(u->cond,offset);
	declInitList<<";";
	SPL2GPU_Node(u->next,offset);
	declInitList<<")";
	if (u->stmt->typ != Block)//如果是非block结点，则需要换行对齐
	{
		declInitList<<"\n";
	}
	OutputStmt(u->stmt,offset+1);
}

void GPUCodeGenerate::SPL2GPU_Goto(Node *node, GotoNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Continue(Node *node, ContinueNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Break(Node *node, BreakNode *u, int offset)
{
	declInitList<<"break";
}

void GPUCodeGenerate::SPL2GPU_Return(Node *node, ReturnNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Block(Node *node, BlockNode *u, int offset)
{
	declInitList <<"{\n";
	//OutputCRSpaceAndTabs(offset + 1);
	SPL2GPU_List(u->decl,  offset);

	declInitList<<"\n"; // 另起一行
	OutputStmtList(u->stmts, offset);	
	OutputCRSpaceAndTabs(offset);
	declInitList<<"}\n"; // '}'独占一行
}

void GPUCodeGenerate::SPL2GPU_Prim(Node *node, primNode *u, int offset)
{
	declInitList << GetPrimDataType(node);
	globalvartype.push_back(GetPrimDataType(node));
	globalvarbuf<<"extern "<<GetPrimDataType(node)<<" ";
}

void GPUCodeGenerate::SPL2GPU_Tdef(Node *node, tdefNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Ptr(Node *node, ptrNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Adcl(Node *node, adclNode *u, int offset)
{
#if 0
	/*GPU-2.2后多维的数组需要去掉具体类型。例如
	val kkey=[[14,4,13,1],[2,5,67,80]];
	让GPU编译器自己判断类型。
	*/
	if((node->u.adcl.type)->typ != Adcl){ 
		string arrayType = getArrayDataType(node->u.adcl.type);
		declInitList<<"Array["<<arrayType<<"](1)";
	}
#endif

#if 1 //GPU-2.1数组处理方式
	string arrayType = GetArrayDataType(node->u.adcl.type);
	//declInitList<<"Array["<<arrayType<<"](1)";
	declInitList<<arrayType<<" ";
	globalvartype.push_back(arrayType);
	globalvarbuf<<"extern "<<arrayType<<" ";
#endif
}

void GPUCodeGenerate::SPL2GPU_Fdcl(Node *node, fdclNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Sdcl(Node *node, sdclNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Udcl(Node *node, udclNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Edcl(Node *node, edclNode *u, int offset)
{
}


void GPUCodeGenerate::SPL2GPU_Decl(Node *node, declNode *u, int offset)
{

	if (extractDecl)
	{
		ExtractDeclVariables(node);
		return;
	}
	else
	{
		map<string,string>submapofglobalvar;
		map<string,map<string,string> >submapofvecglobal;
		OutputCRSpaceAndTabs(offset);
		//declInitList << "\n";
		SPL2GPU_Node(u->type, offset);
		if(u->type->typ == Adcl && ((u->type)->u.adcl.type)->typ== Adcl)
		{
			string dim = GetArrayDim(u->type->u.adcl.dim);
			string dim1= GetArrayDim(u->type->u.adcl.type->u.adcl.dim);
			if (flag_Global)
			{
				declInitList<<node->u.decl.name<<"["<<dim<<"]["<<dim1<<"]"; //GPU-2.2多维数组声明的改动
			}
			else
			{
				declInitList_temp<<node->u.decl.name<<"["<<dim<<"]["<<dim1<<"]";
				//declInitList<<node->u.decl.name<<"["<<dim<<"]["<<dim1<<"]={0}";
				//declInitList<<node->u.decl.name<<"["<<dim<<"]["<<dim1<<"]";
			}
			temp_declInitList<<declInitList.str()<<";\n";
			globalvarbuf<<node->u.decl.name<<"["<<dim<<"]["<<dim1<<"];\n";
			submapofglobalvar.clear();
			submapofglobalvar.insert(make_pair(dim,dim1));
			submapofvecglobal.clear();
			submapofvecglobal.insert(make_pair(node->u.decl.name,submapofglobalvar));
			globalvariable.push_back(submapofvecglobal);
			array2Dname.insert(node->u.decl.name);
		}
		else if (u->type->typ == Adcl)
		{
			string dim = GetArrayDim(u->type->u.adcl.dim);
			if (flag_Global)
			{
				declInitList<<node->u.decl.name<<"["<<dim<<"]";
			}
			else
			{
				declInitList_temp<<node->u.decl.name<<"["<<dim<<"]";
				//declInitList<<node->u.decl.name<<"["<<dim<<"]={0}";
				//declInitList<<node->u.decl.name<<"["<<dim<<"]";
			}
			temp_declInitList<<declInitList.str()<<";\n";
			globalvarbuf<<node->u.decl.name<<"["<<dim<<"];\n";
			submapofglobalvar.clear();
			submapofglobalvar.insert(make_pair(dim,"0"));
			submapofvecglobal.clear();
			submapofvecglobal.insert(make_pair(node->u.decl.name,submapofglobalvar));
			globalvariable.push_back(submapofvecglobal);
			array1Dname.insert(node->u.decl.name);
		}
		else
		{
			declInitList<<" "<<node->u.decl.name;
			temp_declInitList<<declInitList.str()<<";\n";
			globalvarbuf<<node->u.decl.name<<";\n";
			submapofglobalvar.clear();
			submapofglobalvar.insert(make_pair("0","0"));
			submapofvecglobal.clear();
			submapofvecglobal.insert(make_pair(node->u.decl.name,submapofglobalvar));
			globalvariable.push_back(submapofvecglobal);
		}
	}

	//SPL2GPU_Node(u->type, offset);
	if(u->type->typ == Adcl)
	{
		AdclInit(node,offset);
	}
	else
	{
		if (node->u.decl.init) 
		{
			if (u->type->typ == Prim && u->type->u.prim.basic == Char)//如果是个字符声明
			{
				declInitList<<" = "<<u->init->u.implicitcast.expr->u.Const.text;
			}
			else
			{
				declInitList<<" = ";
				SPL2GPU_Node(u->init,offset );
			}
		}
	}
	if (u->type->typ != Adcl)
		declInitList<<";";
}

void GPUCodeGenerate::SPL2GPU_Attrib(Node *node, attribNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Proc(Node *node, procNode *u, int offset)
{
}

void GPUCodeGenerate::SPL2GPU_Text(Node *node, textNode *u, int offset)
{
}

//bool GPUCodeGenerate::IsUpBorder(FlatNode *actor)
//{
//	bool flag = false;   //cwb 如果是cpu与gpu的边界结点
//	vector<FlatNode *>::iterator iter1;
//	if (actor != sssg_->GetFlatNodes()[0])
//	{
//		for (iter1 = actor->inFlatNodes.begin(); iter1 != actor->inFlatNodes.end(); ++iter1)
//		{
//			if(actor->GPUPart != (*iter1)->GPUPart)
//			{
//				flag = true;
//				break;
//			}
//		}
//	}
//	return flag;
//}
//
//bool GPUCodeGenerate::IsDownBorder(FlatNode *actor)
//{
//	bool flag = false;   //cwb 如果是cpu与gpu的边界结点
//	vector<FlatNode *>::iterator iter1;
//	if (actor != sssg_->GetFlatNodes()[sssg_->flatNodes.size()-1])
//	{
//		for (iter1 = actor->outFlatNodes.begin(); iter1 != actor->outFlatNodes.end(); ++iter1)
//		{
//			if(actor->GPUPart != (*iter1)->GPUPart)
//			{
//				flag = true;
//				break;
//			}
//		}
//	}
//	return flag;
//}
bool GPUCodeGenerate::DataDependence(FlatNode *actora,FlatNode *actorb)//a->b
{
	bool flag = false;
	vector<FlatNode *>::iterator iter;
	int index = 0;
	for (iter = actorb->inFlatNodes.begin();iter != actorb->inFlatNodes.end(); ++iter)
	{
		if ((*iter) == actora)
		{
			if(actorb->inPeekWeights[index] > actorb->inPopWeights[index])
			{
				flag = true;
				break;
			}
		}
		++index;
	}
	return flag;
}
bool GPUCodeGenerate::ExistDependence(FlatNode *actor)
{
	bool flag = false;
	int index = 0;
	for (;index < actor->inFlatNodes.size(); ++index)
	{
		if(actor->inPeekWeights[index] > actor->inPopWeights[index])
		{
			flag = true;
			break;
		}
	}
	return flag;
}
