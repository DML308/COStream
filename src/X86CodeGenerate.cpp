# include "X86CodeGenerate.h"
#include <algorithm>
#ifndef WIN32
#include <sys/stat.h> 
#endif
using namespace std;
#define  Default_Repeat_Count 10
#ifdef WIN32
	#define mkdir(tmp) _mkdir(tmp.c_str());
#else
	#define mkdir(tmp) mkdir(tmp.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif	
static bool isInComma = false;//��ʾ��ǰ���ڶ��ű��ʽ��
static int flag=0;
static int flag_Global = 0;//��ʶ�Ƿ������GlobalVarCpp�ļ�����ʼ��Ϊ0
//static bool FileRW = 0;//��ʶ�Ƿ����ļ���дactor

static bool isInFileReader = false;//��ʾ��ǰ����FileReader��
static bool isInFileWriter = false;//��ʾ��ǰ����FileWriter��
static bool FileRW = 0;//��ʶ�Ƿ����ļ���дactor
static bool FileR  = 0; // ��ʶ�Ƿ����ļ��� actor
static bool FileW  = 0; // ��ʶ�Ƿ����ļ�д actor
static bool existFileWriter = false;//���ڱ�ʶ�Ƿ����FileWriter��Ĭ��Ϊ��

static bool isExternType = false; //��ʾĿǰ����������Ƕ��ʽ�Ľṹ�ļ�ExternType��
static bool needExternType = false; //��ʾʱ����Ҫ���ɽӿ��ļ�

//static bool existFileWriter = false, existFileReader = false;//���ڱ�ʶ�Ƿ����FileWriter��Ĭ��Ϊ��
static bool isCGGlobalVar = false;//��ʾĿǰ����������ȫ�ֱ���
static FlatNode* curActor = NULL;//���ڱ�ʾ��ǰ������actor�ļ�
extern char* objName;
X86CodeGenerate::X86CodeGenerate(SchedulerSSG *Sssg, int nCpucore, const char *currentDir,StageAssignment *psa,MetisPartiton *Mptemp,TemplateClass *tc)
{
	Tc = tc;
	pSa = psa;
	Mp=Mptemp;
	sssg_ = Sssg;
	flatNodes_ = Sssg->GetFlatNodes();
	pEdgeInfo = new ActorEdgeInfo(Sssg);
	nCpucore_ = nCpucore;
	nActors_ = flatNodes_.size();
	vTemplateNode_ = Tc->GetTemplateNode();
	nTemplateNode_ = vTemplateNode_.size();
	vTemplateName_ = Tc->GetTemplateName();
	mapFlatnode2Template_ = Tc->GetFlatnode2Template();
	readerActor = writerActor = NULL;
	dir_ = currentDir;
	//������������Ŀ���ļ���Ŀ¼

	int index = 0; 
	int i = 0; 
	while(1) 
	{ 
		string::size_type pos = dir_.find("\\", index); 
		string tmp; 
		tmp = dir_.substr(0, pos); 

		if(pos == -1) break; 
		else if(i > 0) mkdir(tmp); 
		i++; 
		index = pos + 1; 
	} 
	//����mapActor2InEdge��mapActor2OutEdge�Լ�readerActor��writeActor
	vector<FlatNode*>::iterator iter1,iter2,iter3;
	for (iter1=flatNodes_.begin();iter1!=flatNodes_.end();++iter1)
	{
		for (iter2=(*iter1)->inFlatNodes.begin();iter2!=(*iter1)->inFlatNodes.end();++iter2)
		{
			string tempstring1=(*iter2)->name+"_"+(*iter1)->name;//�õ�����A_B�ı�����
			mapActor2InEdge.insert(make_pair((*iter1),tempstring1));
		}
		for (iter3=(*iter1)->outFlatNodes.begin();iter3!=(*iter1)->outFlatNodes.end();++iter3)
		{
			string tempstring2=(*iter1)->name+"_"+(*iter3)->name;
			mapActor2OutEdge.insert(make_pair((*iter1),tempstring2));
		}
		if((*iter1)->name.find("FileReader") != -1)
		{
			readerActor = (*iter1);
			//existFileReader = true;
			FileR = 1;
		}			
		if((*iter1)->name.find("FileWriter") != -1)
		{	
			writerActor = (*iter1);
			existFileWriter = true;
			FileW = 1;
		}
	}
	//����ÿ���߳��ϵ�stage����mapNum2Stage
	vector<FlatNode*>tempactors;
	vector<FlatNode*>::iterator iter;
	for (int i=0;i<nCpucore;i++)
	{
		set<int>tempstageset;
		tempstageset.clear();
		tempactors=Mp->findNodeSetInPartition(i);		//�����̺߳��ҵ�actor����
		//����actor
		for (iter=tempactors.begin();iter!=tempactors.end();++iter)
		{
			tempstageset.insert(pSa->FindStage(*iter));
		}
		mapNum2Stage.insert(make_pair(i,tempstageset));
	}

	extractDecl = false;
	isInParam = false;
}
void X86CodeGenerate::CGdeclList(FlatNode *actor, OperatorType ot, stringstream &buf)		//�����������
{
	List *state = NULL;
	Node  *param = NULL;
	assert(actor);
	buf <<"\t// AST Variables\n";
	//state��var��param��������������
	state = actor->contents->body->u.operBody.state;


	extractDecl = true;//��־����decl
	declList.str(""); // ���declList����
	buf<<"\tint steadyScheduleCount;\t//��̬ʱһ�ε�����ִ�д���\n";
	buf<<"\tint initScheduleCount;\n";
	//chenwenbin��composite��param����ģ������
	buf<<"\t//��composite��param����ģ������"<<endl;
	paramList *plist = actor->contents->params;
	for (int i = 0; i < actor->contents->paramSize; i++)
	{
		buf<<"\t"<<GetPrimDataType(plist->paramnode->u.id.value->u.Const.type)<<" ";
		buf<<plist->paramnode->u.id.text<<";\n";
		plist = plist->next;
	}
	//state
	SPL2X86_List(state, 0);//���state
	buf << "\t/* *****logic state***** */\n\t" << declList.str();
	
	//chenwenbin ����pop,push���ݱ���
	buf<<"\t//����pop,push����"<<endl;
	int popSize = actor->inFlatNodes.size();
	int pushSize = actor->outFlatNodes.size();
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
	
	extractDecl = false;
	//state init
	stateInit << declInitList.str();
	declInitList.str(""); // ���declInitList����
}
void X86CodeGenerate::CGinitVarAndState(FlatNode *actor, OperatorType ot, stringstream &buf)	//���initVar��State
{
	buf << "\t// initVarAndState\n";
	buf << "\tvoid initVarAndState() {\n";

	declInitList << "\n";
	buf << declInitList.str();
	buf << "\n\t\t/**** State Init ****/\n";
	buf << stateInit.str();
	buf << "\n\t}\n"; // initVarAndState��������

	declInitList.str(""); // ���
	stateInit.str(""); // ���
}
void X86CodeGenerate::CGlogicInit(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	declInitList.str(""); // ���
	buf << "\t// init\n";
	buf << "\tvoid init()";
	Node *init = actor->contents->body->u.operBody.init;
	if (init)
	{
		SPL2X86_Node(init, 2);
		buf << declInitList.str(); // init�ṹ�����"{}"
	}
	else
	{
		buf << " {\n";
		buf << "\t}\n"; // '}'��ռһ��
	}
	declInitList.str(""); // ���
}
void X86CodeGenerate::CGthis(FlatNode *actor, OperatorType ot, stringstream &buf,string templatename)	//���actor�Ĺ��캯��
{
	buf <<"\t// Constructor\n";
	buf << "\t"<<templatename<<"(" ;
	//chenwenbin ����param����
	paramList *plist = actor->contents->params;
	for (int i = 0; i < actor->contents->paramSize; i++)
	{
		//buf<<NodeConstantIntegralValue(plist->paramnode->u.id.value)<<" ";
		buf<<GetPrimDataType(plist->paramnode->u.id.value->u.Const.type)<<" ";
		buf<<plist->paramnode->u.id.text<<", ";
		plist = plist->next;
	}
	//chenwenbin ����pop,push����
	if (actor->inPopWeights.size() > 0)
	{
		for (int i = 0; i < actor->inPopWeights.size(); i++)
		{
			buf<<"int popValue"<<i<<",";
		}
	}
	if (actor->outPushWeights.size() > 0)
	{
		for (int i = 0; i < actor->outPushWeights.size(); i++)
		{
			buf<<"int pushValue"<<i<<",";
		}
	}
	//chenwenbin ��̬��ִ̬�д���
	buf<<"int steadyCount,int initCount,";
	vector<FlatNode*>::iterator iter,end;	
	//���������
	iter = actor->outFlatNodes.begin();
	end = actor->outFlatNodes.end();
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker output_maker;
	Node *outputNode = NULL;
	IterateList(&output_maker,outputList);/*int i=0;*/
	while(iter!=end)
	{ 
		NextOnList(&output_maker,(GenericREF)&outputNode);
		string outputString;
		if(outputNode->typ == Id) outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		//buf<<"Buffer<"<<pEdgeInfo->getEdgeInfo(actor,(*iter)).typeName<<">& "<<outputString<<",";
		buf<<"Buffer<T>& "<<outputString<<",";
		iter++;
	}
	//���������
	iter = actor->inFlatNodes.begin();
	end = actor->inFlatNodes.end();
	//��ȡactor������ߵ�����
	List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
	ListMarker input_maker;
	Node *inputNode = NULL;
	IterateList(&input_maker,inputList);
	while(iter!=end)
	{
		NextOnList(&input_maker,(GenericREF)&inputNode);
		string inputString;
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		//buf<<"Buffer<"<<pEdgeInfo->getEdgeInfo((*iter),actor).typeName<<">& "<<inputString<<",";
		buf<<"Buffer<U>& "<<inputString<<",";
		iter++;
	}
	buf<<parameterBuf.str();
	if (parameterBuf.str()=="")
	{
		buf.seekp((int)buf.tellp()-1);
	}
	else
	{
		buf.seekp((int)buf.tellp()-2);
	}
	buf<<"):";
	plist = actor->contents->params;
	for (int i = 0; i < actor->contents->paramSize; i++)
	{
		buf<<plist->paramnode->u.id.text<<"("<<plist->paramnode->u.id.text<<"),";
		plist = plist->next;
	}
	for (int i = 0; i < actor->inPopWeights.size(); i++)
	{
		buf<<"popValue"<<i<<"(popValue"<<i<<"),";
	}
	for (int i = 0; i < actor->outPushWeights.size(); i++)
	{
		buf<<"pushValue"<<i<<"(pushValue"<<i<<"),";
	}
	IterateList(&input_maker,inputList);
	while(NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		string inputString;
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		buf<<inputString<<"("<<inputString<<"),";
	}
		IterateList(&output_maker,outputList);
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		string outputString;
		if(outputNode->typ == Id) outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		buf<<outputString<<"("<<outputString<<"),";
	}
	buf.seekp((int)buf.tellp()-1);
	buf<<"{\n";
	buf << thisBuf.str();
	//buf << "\t\tRepeatCount = rc;\n ";
	buf<< "\t\tsteadyScheduleCount = steadyCount;\n";
	buf<< "\t\tinitScheduleCount = initCount;\n";
	buf << "\t}\n"; // init��������
}


void X86CodeGenerate::CGinitWork(stringstream &buf)
{
	buf << "\t// initWork\n";
	buf << "\tvoid initWork() {\n";
	buf << "\t\tinitVarAndState();\n";
	buf << "\t\tinit();\n";

	buf << "\t}\n"; // initWork��������
}

void X86CodeGenerate::CGinitPeek(stringstream &buf, string initPeekBuf)
{
	buf <<"\t// initPeek\n";
	buf << "\tdef initPeek() {\n";

	buf << initPeekBuf;

	buf << "\t}\n"; // initPeek��������
}


void X86CodeGenerate::CGpopToken(FlatNode *actor,stringstream &buf)
{
	buf << "\t// popToken\n";
	buf << "\tvoid popToken() {";
	List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	IterateList(&input_maker,inputList);
	//vector<int>::iterator iter=actor->inPopWeights.begin();
	int index = 0;
	while(NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		string inputString;
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		buf<<"\n\t"<<inputString<<".updatehead"<<"(popValue"<<index<<");\n";
		index++;
	}
	buf << "\t}\n"; // popToken��������
}
void X86CodeGenerate::CGpushToken(FlatNode *actor,stringstream &buf)
{
	buf << "\t// pushToken\n";
	buf << "\tvoid pushToken() {";
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker output_maker;
	Node *outputNode = NULL;
	IterateList(&output_maker,outputList);
	//vector<int>::iterator iter=actor->outPushWeights.begin();
	int index = 0;
	while(NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		string outputString;
		if(outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		buf<<"\n\t"<<outputString<<".updatetail"<<"(pushValue"<<index<<");\n";
		index++;
	}
	//buf << popingBuf;

	buf << "\t}\n"; // popToken��������
}

void X86CodeGenerate::CGpushToken(stringstream &buf, string pushingBuf)
{
	buf << "\t// pushToken\n";
	buf << "\tdef pushToken() {\n";

	buf << pushingBuf;

	buf << "\t}\n"; // pushToken��������
}
void X86CodeGenerate::CGrun(stringstream &buf, string initFun)
{
	buf <<"\t// run\n";
	buf << "\tpublic void run() {\n";//run����,��������Ҫ�ķ���

	buf << "\t\t" << initFun << ";\n"; // ����init����

	buf << strScheduler.str();

	buf << "\t}\n"; // run��������
}
void X86CodeGenerate::CGrunInitScheduleWork(FlatNode *actor,stringstream &buf)
{
	buf <<"\t// runInitScheduleWork\n";
	buf << "\tvoid runInitScheduleWork() {\n";
	buf << "\t\tinitWork();\n"; // ����initwork����
	buf<<"\t\tfor(int i=0;i<initScheduleCount;i++)\n\t\t\twork();\n";
	if(NoCheckBuffer){
		//����
		List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
		ListMarker output_maker;
		Node *outputNode = NULL;
		IterateList(&output_maker,outputList);
		while(NextOnList(&output_maker,(GenericREF)&outputNode))
		{
			string outputString;
			if(outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
			buf<<"\t\t"<<outputString<<".resetTail();\n";
		}
		//���
		List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
		ListMarker input_maker;
		Node *inputNode = NULL;
		IterateList(&input_maker,inputList);
		while(NextOnList(&input_maker,(GenericREF)&inputNode))
		{
			string inputString;
			if(inputNode->typ == Id)inputString = inputNode->u.id.text;
			else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
			buf<<"\t\t"<<inputString<<".resetHead();\n";
		}
		//buf<<"\t\t//cout<<\""<<actor->name<<" init over\"<<endl;\n";
	}
	buf << "\t}\n"; // CGrunInitScheduleWork��������
}
void X86CodeGenerate::CGrunSteadyScheduleWork(FlatNode *actor,stringstream &buf)
{
	buf <<"\t// CGrunSteadyScheduleWork\n";
	buf << "\tvoid runSteadyScheduleWork() {\n";
	if(AmplifySchedule)
	{
		buf<<"\t\tfor(int j=0;i<AMPLIFYFACTOR;j++)\n\t\t{\n";
	}	
	buf<<"\t\tfor(int i=0;i<steadyScheduleCount;i++)\n\t\t\twork();\n";
	if(NoCheckBuffer)
	{
		//����
		List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
		ListMarker output_maker;
		Node *outputNode = NULL;
		IterateList(&output_maker,outputList);
		while(NextOnList(&output_maker,(GenericREF)&outputNode))
		{
			string outputString;
			if(outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
			buf<<"\t\t"<<outputString<<".resetTail();\n";
		}
		//���
		List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
		ListMarker input_maker;
		Node *inputNode = NULL;
		IterateList(&input_maker,inputList);
		while(NextOnList(&input_maker,(GenericREF)&inputNode))
		{
			string inputString;
			if(inputNode->typ == Id)inputString = inputNode->u.id.text;
			else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
			buf<<"\t\t"<<inputString<<".resetHead();\n";
		}
		//buf<<"\t\t//cout<<\""<<actor->name<<" steady over\"<<endl;\n";
	}
	if(AmplifySchedule)
		buf<<"\t\t}\n";
	buf << "\t}\n"; // // CGrunSteadyScheduleWork��������
}
void X86CodeGenerate::CGEdgeParam(FlatNode *actor,stringstream &buf)
{
	buf<<"\t//edge param\n\t";
	
	vector<FlatNode*>::iterator iter,end;	//������������ߣ�����producer��consumer	
	//�����
	iter = actor->outFlatNodes.begin();
	end = actor->outFlatNodes.end();
	//��ȡactor����ߵ�����
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker output_maker;
	Node *outputNode = NULL;
	IterateList(&output_maker,outputList);
	while(iter!=end)
	{
		NextOnList(&output_maker,(GenericREF)&outputNode);
		string outputString;
		if(outputNode->typ == Id) outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		//buf<<"\tProducer<"<<pEdgeInfo->getEdgeInfo(actor,(*iter)).typeName<<"> "<<outputString<<";\n";
		buf<<"\tProducer<T> "<<outputString<<";\n";
		iter ++;
	}
	//���
	iter = actor->inFlatNodes.begin();
	end = actor->inFlatNodes.end();
	//��ȡactor������ߵ�����
	List *inputList =actor->contents->decl->u.decl.type->u.operdcl.inputs;
	ListMarker input_maker;
	Node *inputNode = NULL;
	IterateList(&input_maker,inputList);
	while(iter!=end)
	{
		NextOnList(&input_maker,(GenericREF)&inputNode);
		string inputString;
		if(inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		//buf<<"\tConsumer<"<<pEdgeInfo->getEdgeInfo((*iter),actor).typeName<<"> "<<inputString<<";\n";
		buf<<"\tConsumer<U> "<<inputString<<";\n";
		iter++;
	}

}
void X86CodeGenerate::CGGlobalvar()
{
	isCGGlobalVar = true;
	stringstream buf;
	// ��ȫ�ֱ������д�������
	flag_Global = 1;
	SPL2X86_List(gDeclList, 1);
	buf <<" #include \""<< string(objName) <<"GlobalVar.h\"\n";
	int len = string(objName).length();
		
	buf <<" namespace COStream" << string(objName).substr(0,len-4)<<"{ \n";
	buf << declInitList.str() << "\n\n";
	buf <<"} \n";
	declInitList.str("");
	flag_Global=0;
	//������ļ�
	stringstream ss;
	isCGGlobalVar = false;
	ss<<dir_<<string(objName)<<"GlobalVar.cpp";
	OutputToFile(ss.str(),  buf.str());
}
void X86CodeGenerate::CGGlobalvarextern()
{
	stringstream ss;
	ss<<dir_<<string(objName)<<"GlobalVar.h";
	string strHeader = "#ifndef _GLOBALVAR_H\n#define _GLOBALVAR_H\n";
	string strEnder  = "#endif";
	stringstream buf;
	buf << strHeader;
	buf<<"#include \"ExternType.h\"\n";
	buf<<globalvarbuf.str();
	buf << strEnder;
	OutputToFile(ss.str(), buf.str());
}

void X86CodeGenerate::CGExternType()
{
	stringstream ss;
	stringstream buf;
	if(needExternType == TRUE){
		//	stringstream ss;
		//	stringstream buf;
		buf<<"#ifndef EXTERN_TYPE_H\n";
		buf<<"#define EXTERN_TYPE_H\n";
		buf<<ExternTypeBuf.str();
		buf<<"#endif\n";
		//	ss<<dir_<<"ExternType.h";
		//	OutputToFile(ss.str(),buf.str());
	}
	ss<<dir_<<"ExternType.h";
	OutputToFile(ss.str(),buf.str());
}

void X86CodeGenerate::CGactor(FlatNode *actor,string templatename, OperatorType ot)
{
	stringstream buf; //��������actorͷ�ļ����ַ�����
	stringstream srcBuf;//��������actorԴ�ļ����ַ����������ж�actor�ڵ�var��state���ݽ��г�ʼ��
	vector<string>::iterator iter;
	string classNmae = templatename;
	curactor=actor;
	assert(actor);
	//cout<<actor->name<<"       "<<endl;
	actor->SetIOStreams();
	//ע���ļ�����
	buf <<"/**\n * Class "<<classNmae<<"\n */\n";
	//���ͷ�ļ�
	string headstr = "";
	for(int i = 0; i < classNmae.size(); i++)
		if(classNmae[i]-'a' >= 0 && classNmae[i]-'a' <= 26)
			headstr += (char)('A'+classNmae[i]-'a');
	    else
			headstr += classNmae[i];
	buf<<"#ifndef "<<headstr<<"_H\n";
	buf<<"#define  "<<headstr<<"_H\n";
	buf<<"#include \"Buffer.h\"\n";
	buf<<"#include \"Consumer.h\"\n";
	buf<<"#include \"Producer.h\"\n";
//	buf<<"#include \"global.h\"\n";
	buf<<"#include \"" <<string(objName) <<"global.h\"\n";
	buf<<"#include <string>\n";
	buf<<"#include \"iostream\"\n";
//	buf<<"#include \"GlobalVar.h\"\n";
	buf<<"#include \"" << string(objName) <<"GlobalVar.h\"\n";
	if(actor->memorizedNode)
		buf<<"#include \"p-stable_lsh.h\"\n";
	buf<<"using namespace std;\n";
	//buf<<"\tinclude \""<<classNmae<<".h\"\n";
	if(CallModelEmbed)	
		buf <<" namespace COStream" << string(objName)<<"{ \n";//������ֿռ�
	if((actor->inFlatNodes.size() == 0 && actor->outFlatNodes.size() != 0))
		buf<<"template<typename T>\n";
	else if((actor->inFlatNodes.size() != 0 && actor->outFlatNodes.size() == 0))
		buf<<"template<typename U>\n";
	else if(actor->inFlatNodes.size() != 0 && actor->outFlatNodes.size() != 0)
	    buf<<"template<typename T,typename U>\n";
	buf <<"class "<<classNmae<<"{\n"; // ģ����鿪ʼ	
	//����actor�������consumer�������producer
	CGEdgeParam(actor,buf);
	/*FileReader��FileWriter��������*/		
	if(actor == readerActor)
	{
		SPL2X86_List(actor->contents->body->u.operBody.state, 0);
		isInFileReader = FALSE;
		buf<<"\tint dataCount;\n";
		buf<<"\tint steadyScheduleCount;\n";
		buf<<"\tint initScheduleCount;\n";
		CGpushToken(actor,buf);	
		CGFileReaderActor(buf);
	}
	else if(actor == writerActor)
	{
		SPL2X86_List(actor->contents->body->u.operBody.state, 0);
		isInFileWriter = FALSE;
		buf<<"\tint dataCount;\n";
		buf<<"\tint steadyScheduleCount;\n";
		buf<<"\tint initScheduleCount;\n";
		CGpopToken(actor,buf);
		CGFileWriterActor(buf);
	}
	else 
	{
		// д�뷢�͡�������Ϣ
		parameterBuf.str(""); // ���parameterBuf����
		thisBuf.str(""); // ���thisBuf����
		buf<<"private:\n\t";
		// д���﷨����Ա������Ϣ��param, var, state��
		CGdeclList(actor, ot, buf);
		// д�� var, state �ĳ�ʼ����Ϣ
		CGinitVarAndState(actor, ot, buf);
		// д�� init �ĳ�ʼ����Ϣ(���ǵ�init���Զ���ֲ����������Բ����ϲ��ֺϲ�)
		CGlogicInit(actor, ot, buf);			//����init����
		// д��popToken��pushToken����������������
		CGpopToken(actor,buf);
		CGpushToken(actor,buf);	
		// д��initWork����
		CGinitWork(buf);
		// д��work����
		CGwork(actor, ot, buf);
	}
	//��������ÿ��actor�ĳ�Ա�������������캯������̬���Ⱥ�������̬���Ⱥ���
	buf<<"public:\n\t";
	// д�빹�캯����Ϣ
	CGthis(actor, ot, buf,classNmae);
	//д��runInitScheduleWork����
	CGrunInitScheduleWork(actor,buf);
	//д��runSteadyScheduleWork����
	CGrunSteadyScheduleWork(actor,buf);
		buf <<"};\n";//������
	if(CallModelEmbed)
		buf<<"}\n";//���ֿռ�����
	//����Դ�ļ�����
	//srcBuf<<"#include \""<<actor->name<<".h\"\n";
	for (iter=staticNameInit.begin();iter!=staticNameInit.end();++iter)
		srcBuf<<(*iter);
	//������ļ�
	stringstream headerFileName;	//����ͷ�ļ���
	headerFileName<<dir_<<string(objName)<<classNmae<<".h";
	//srcFileName<<dir_<<classNmae<<".cpp";
	if (staticNameInit.size()!=0)		//chenwenbin��������̬��Ա������ģ�����ж�����г�ʼ��
		buf<<srcBuf.str();
	buf<<"#endif\n";
	OutputToFile(headerFileName.str(),buf.str());	//ͷ�ļ�
	staticNameInit.clear();
}
void X86CodeGenerate::CGwork(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	vector<string>::iterator iter;
	Node *work = actor->contents->body->u.operBody.work;
	string actorname = actor->name;
	stringstream tmpBuf;
	buf << "\t// work\n";
	buf << "\tvoid work() {\n";

	Node *curInStream, *curOutStream;
	if(actor->memorizedNode && actor->inPeekWeights.size()==1 && actor->outPushWeights.size()==1){
		curInStream = (Node*)FirstItem(actor->contents->decl->u.decl.type->u.operdcl.inputs);
		curOutStream = (Node*)FirstItem(actor->contents->decl->u.decl.type->u.operdcl.outputs);
		buf << "\t// ��ʱ����inout\n";
		buf << "\tdouble inputMemorizedWindow["<<actor->inPeekWeights[0]<<"];\n";
		buf << "\tdouble outputMemorizedWindow["<<actor->outPushWeights[0]<<"] = {0.0};\n";
		buf << "\tfor(int i=0;i<"<<actor->inPeekWeights[0]<<";i++)\n";
		buf << "\t\tinputMemorizedWindow[i] = "<<curInStream->u.id.text<<"[i].x;\n";
		buf << "\t// Ԥ�����ϣ��\n";
		buf << "\tpstablePreProccess();\n";
		buf << "\t// ���ݲ���\n";
		buf << "\tif(pstableQuery(inputMemorizedWindow,outputMemorizedWindow)){\n";
		buf << "\t\tfor(int i=0;i<"<<actor->outPushWeights[0]<<";i++)\n";
		buf << "\t\t\t"<<curOutStream->u.id.text<<"[i].x = outputMemorizedWindow[i];\n";
		buf << "\t}\n";
		buf << "\telse{\n";
	}

	SPL2X86_Node(work, 2);		//����work��work�е����������declInitList
	buf << declInitList.str();

	if(actor->memorizedNode && actor->inPeekWeights.size()==1 && actor->outPushWeights.size()==1){
		buf << "\t// �����ϣ��\n";
		buf << "\t\tfor(int i=0;i<"<<actor->outPushWeights[0]<<";i++)\n";
		buf << "\t\t\toutputMemorizedWindow[i] = "<<curOutStream->u.id.text<<"[i].x;\n";
		buf << "\t\tpstableInsert(inputMemorizedWindow,outputMemorizedWindow,"<<actor->inPeekWeights[0]<<","<<actor->outPushWeights[0]<<");\n";
		buf << "\t}\n";
	}

	buf << "\n\t\tpushToken();\n";
	buf << "\n\t\tpopToken();\n";
	buf << "\t}\n"; // work��������
	declInitList.str(""); // ���
	//ptrname.clear();//���
}
void X86CodeGenerate::OutputToFile(string fileName, string oldContents)
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
void X86CodeGenerate::CGactors()
{
	stringstream ss,buf;
	std::map<operatorNode *, string>::iterator pos;
	for (int i = 0; i < nTemplateNode_; ++i)//cwb���� ���� ��ģ��
	{
		int len = ListLength(vTemplateNode_[i]->oldContents->decl->u.decl.type->u.operdcl.outputs);
		int nOut = vTemplateNode_[i]->nOut;
		OperatorType ot = vTemplateNode_[i]->oldContents->ot;
	   
		pos = mapOperator2ClassName.find(vTemplateNode_[i]->oldContents);
		//if (pos == mapOperator2ClassName.end()) // �µ���ģ�������
		{

			string name = vTemplateNode_[i]->name;
			int index = name.find_first_of('_');
			string tmp = name.substr(0, index);
			mapOperator2ClassName.insert(make_pair(vTemplateNode_[i]->oldContents, vTemplateNode_[i]->name));

			if (len == nOut)			//nOutΪÿ��operator������߸���
			{
				buf<<"#include \""<<string(objName)<<vTemplateName_[i]<<".h\"\n";
				if (strcmp(tmp.c_str(), "FileReader") == 0)
					isInFileReader = true;
				if (strcmp(tmp.c_str(), "FileWriter") == 0)
					isInFileWriter = true; 
				curActor = vTemplateNode_[i];
				CGactor(vTemplateNode_[i],vTemplateName_[i], ot);
			}
			else // һ���߱����actor����, �ݲ�����
			{
				cout << "test" << endl;
				UNREACHABLE;
			}
			
		}
	
	}
	ss<<dir_<<string(objName)<<"AllActorHeader.h";
	OutputToFile(ss.str(),buf.str());

}
string X86CodeGenerate::MakeTabs(int tabs)
{
	stringstream tmp;
	while (tabs--) tmp<<"\t";
	return tmp.str();
}
void X86CodeGenerate::OutputCRSpaceAndTabs(int tabs)
{
	declInitList<<"\n"; 
	while (tabs--) declInitList<<"\t";
}
void X86CodeGenerate::OutputArgList(List *list, int offset)
{ 
	ListMarker marker;
	Node *item;
	int i=0;
	IterateList(&marker, list);
	while (NextOnList(&marker, (GenericREF) &item))
	{
		if(i!=0) declInitList<<", ";
		SPL2X86_Node(item, offset);
		i++;
	}
}
void X86CodeGenerate::OutputTabs(int tabs)
{
	while (tabs--) declInitList<<"\t";
}
void X86CodeGenerate::AddSemicolon()
{

}
void X86CodeGenerate::CGglobalHeader()
{
	stringstream buf;
	string edgename;
	ListMarker marker;
	Node *node;
	vector<FlatNode *>::iterator iter_1,iter_2;
	buf<<"#ifndef _GLOBAL_H\n";
	buf<<"#define _GLOBAL_H\n";
	buf<<"/*ȫ�ֱ��������ڴ洢�ߵ���Ϣ*/\n";
	buf<<"/*�ߵ���������A_B,����A->B*/\n\n";
	if (FileR || FileW)		//�����ļ���д����
	{
		buf<<"#include <sstream>\n";
		buf<<"#include <fstream>\n";
		buf<<"#include \"tinystr.h\"\n";
		buf<<"#include \"tinyxml.h\"\n";
	}
	IterateList(&marker, Program);
	while (NextOnList(&marker, (GenericREF) &node)) {
		if (node == NULL)
			continue;

		if (node->coord.includedp)
			continue;

		if (node->typ == Text) {

			buf<<node->u.text.text<<endl;
			continue;
		}
	}
	buf<<"#include \"Buffer.h\"\n";
	buf<<"#include \"MathExtension.h\"\n";
	buf<<"#include <string>\n";
	buf<<"using namespace std;\n";
	if (Win)
	{
		buf<<"#define MAX_ITER 10\n";
	}
	if(CallModelEmbed)
		buf <<" namespace COStream" << string(objName)<<"{ \n";//���ֿռ�
	//���stream�ṹ�����Ͷ��� 
	pEdgeInfo->DeclEdgeType(buf);
	//���ȫ�ֱ���Buffer������
	vector<FlatNode *>::iterator iter;
	for (iter=flatNodes_.begin();iter!=flatNodes_.end();++iter)//�������н��
	{
		for (iter_2=(*iter)->outFlatNodes.begin();iter_2!=(*iter)->outFlatNodes.end();iter_2++)
		{
			string edgename=(*iter)->name+"_"+(*iter_2)->name;
			buf<<"extern Buffer<";
			buf<<pEdgeInfo->getEdgeInfo((*iter),(*iter_2)).typeName<<"> "<<edgename<<";"<<endl;
		}	
	}
	if(readerActor)		//���ļ�Actor
	{		
		string typeName = pEdgeInfo->getEdgeInfo(readerActor,readerActor->outFlatNodes[0]).typeName;
		buf<<"extern "<<typeName <<"* source;\n";
		buf<<"int FileReader(string path,int mode);\n";
		buf<<"istream& operator>>(istream& is, "<<typeName<<" &object);\n";
	}
	if(writerActor)
	{
		string typeName = pEdgeInfo->getEdgeInfo(writerActor->inFlatNodes[0],writerActor).typeName;
		buf<<"extern "<<typeName <<"* sink;\n";
		buf<<"void FileWriter(string path,int mode);\n";
		buf<<"ostream& operator<<(ostream& os, const "<<typeName<<" &object);\n";
		buf<<"extern int outPutCount;\n";
	}
	if(CallModelEmbed)
		buf<<"}\n";//���ֿռ����
	buf<<"#endif\n";
	//������ļ�
	stringstream ss;
	ss<<dir_<<string(objName)<<"global.h";
	OutputToFile(ss.str(),  buf.str());
}
void X86CodeGenerate::CGglobalCpp()
{
	stringstream buf;			//��������globalCpp���ַ�����
	buf<<"/*cpp�ļ�,ȫ�ֱ��������ڴ洢�ߵ���Ϣ*/\n";
	buf<<"#include \"Buffer.h\"\n";
	buf<<"#include \"" << string(objName) <<"global.h\"\n";
	buf<<"#include <vector>\n";
	buf<<"using namespace std;\n";
	if(CallModelEmbed)
		buf <<" namespace COStream" << string(objName)<<"{ \n";//���ֿռ�
	int init1,init2;//����actor�ͽ���actor��̬���Ȳ����ͽ��ܵ�������
	for (vector<FlatNode *>::iterator iter_1=flatNodes_.begin();iter_1!=flatNodes_.end();++iter_1)//�������н��
	{
		for (vector<FlatNode *>::iterator iter_2=(*iter_1)->outFlatNodes.begin();iter_2!=(*iter_1)->outFlatNodes.end();iter_2++)
		{
			int stageminus; //stageminus��ʾ����actor������Ľ׶β�
			int size;	//�������Ĵ�С
			string edgename=(*iter_1)->name+"_"+(*iter_2)->name;	//�ߵ�����
			stageminus=pSa->FindStage(*iter_2)-pSa->FindStage(*iter_1);//���ͷ��ͽ��ܷ��������ˮ�׶β�
			int edgePos = iter_2 - (*iter_1)->outFlatNodes.begin();	//iter_2��iter_1������ߵ����
			int perSteadyPushCount = sssg_->GetSteadyCount(*iter_1)*(*iter_1)->outPushWeights.at(edgePos);//����actorÿ�ε���steadywork��Ҫpush�ĸ���,nc
			//����NoCheckBufferѡ���Ƿ�ʹ�ñ߽���Ļ���������
			if(NoCheckBuffer)			//ʹ���ޱ߽���Ļ����������������ݿ���ʵ��
			{	 
				int copySize=0,copyStartPos=0;	//���������ݴ�С��copy��ŵĿ�ʼλ��
				for(int inEdgeIndex =0;inEdgeIndex<(*iter_2)->inFlatNodes.size();inEdgeIndex++)
					if((*iter_2)->inFlatNodes.at(inEdgeIndex) == (*iter_1))
					{
						int perWorkPeekCount = (*iter_2)->inPeekWeights[inEdgeIndex];//���ձ�actorÿ��peek�ĸ���,b
						int perWorkPopCount = (*iter_2)->inPopWeights[inEdgeIndex];//���ձ�actorÿ�ε���work��Ҫpop�ĸ���,c
						init1 = sssg_->GetInitCount(*iter_1)*(*iter_1)->outPushWeights.at(edgePos);//����actor����initwork������������
						init2 =sssg_->GetInitCount(*iter_2)*perWorkPopCount;
						size = init1 + perSteadyPushCount*(stageminus+2);
						if(perWorkPeekCount == perWorkPopCount)	//peek == pop
						{
							if(perSteadyPushCount){
								copySize = (init1-init2)%perSteadyPushCount;
								copyStartPos = init2%perSteadyPushCount;
							}
						}
						else
						{
							int leftnum =  ((init1-init2)%perSteadyPushCount+perSteadyPushCount-(perWorkPeekCount-perWorkPopCount)%perSteadyPushCount)%perSteadyPushCount;
							copySize = leftnum+perWorkPeekCount-perWorkPopCount;
							int addtime = copySize%perSteadyPushCount?copySize/perSteadyPushCount+1:copySize/perSteadyPushCount;
							copyStartPos = init2%perSteadyPushCount;
							size += addtime*perSteadyPushCount;
						}
						buf<<"Buffer<"<<pEdgeInfo->getEdgeInfo((*iter_1),(*iter_2)).typeName<<"> "<<edgename<<"("<<size<<","<<copySize<<","<<copyStartPos<<");\n";
						break;
					}
			}
			else		//ʹ�ñ߽��飬��������С=����ִ̬�д���+����̬�׶β�+1��*��ִ̬�д�����* ÿ��push��������
			{
				size=(sssg_->GetInitCount(*iter_1)+sssg_->GetSteadyCount(*iter_1)*(stageminus+1))*(*iter_1)->outPushWeights.at(edgePos);
				int tempSize = 1;
				while(size>tempSize)
					tempSize <<= 1;
				size = tempSize;
				buf<<"Buffer<"<<pEdgeInfo->getEdgeInfo((*iter_1),(*iter_2)).typeName<<"> "<<edgename<<"("<<size<<");\n";
			}
		}
	}
	if(readerActor)		//���ļ�Actor
	{
		StreamEdgeInfo edgeInfo = pEdgeInfo->getEdgeInfo(readerActor,readerActor->outFlatNodes[0]);
		buf<<edgeInfo.typeName <<"* source;\n";
		buf<<"int FileReader(string path,int mode)\n{\n";
		buf<<"\tvector<"<<edgeInfo.typeName<<"> sourceDataVec;\n";
		//		buf<<"\tifstream ifile;\n\tifile.open(path.c_str());\n";
		buf<<"\t"<<edgeInfo.typeName<<" temp;\n";
		buf<<"\tstringstream instream;\n";

		buf<<"\tif(path.find(\".txt\")!=-1){\n";
		buf<<"\tifstream infile;\n\tinfile.open(path.c_str());\n";
		buf<<"\tif(!infile)\n\t{\n\t\tprintf(\"file open error\");\n\t\treturn -1;\n\t}\n\telse\n\t{\n";
		buf<<"\t\tdouble value;\n\t\twhile(infile>>value)\n\t\tinstream<<\"x \"<<value;\n\t\tinfile.close();\n\t}";
		buf<<"\t}\n\telse\n\t{\n\t\t";

		buf<<"TiXmlDocument *pDoc =  new TiXmlDocument(path.c_str());\n\t";
		buf<<"if(!pDoc->LoadFile())\n\t";
		buf<<"{\n\t\treturn 0;\n\t}\n\t";
		buf<<"TiXmlElement *RootElement = pDoc->RootElement();\n\t";
		buf<<"TiXmlElement *currentNode = RootElement;\n\t";
		buf<<"while(currentNode)\n\t{\n\t\t";
		buf<<"TiXmlElement *idElement = currentNode->FirstChildElement();\n\t\t";
		buf<<"while(idElement)\n\t\t{\n\t\t\t";
		//	buf<<"stringstream sstr;\n\t\t\tsstr << idElement->FirstChild()->Value();\n\t\t\tfloat a;\n\t\t\tsstr >> a;\n\t\t\t";	
		//	buf<<"instream <<\"x \"<< a;\n\t\t\tidElement = idElement->NextSiblingElement();\n\t\t}\n\t\t";
		buf<<"stringstream sstr;\n\t\t\tstring type;\n\t\t\tsstr << idElement->FirstChild()->Value();\n\t\t\ttype = const_cast<char *>(idElement->Value());\n\t\t\t";
		buf<<"if(type == \"int\")\n\t\t\t{\n\t\t\t\tint a;\n\t\t\t\tsstr >> a;\n\t\t\t\tinstream <<\"x \"<< a;\n\t\t\t}\n\t\t\t";
		buf<<"else if(type == \"float\")\n\t\t\t{\n\t\t\t\tfloat a;\n\t\t\t\tsstr >> a;\n\t\t\t\tinstream <<\"x \"<< a;\n\t\t\t}\n\t\t\t";
		buf<<"else if(type == \"double\")\n\t\t\t{\n\t\t\t\tdouble a;\n\t\t\t\tsstr >> a;\n\t\t\t\tinstream <<\"x \"<< a;\n\t\t\t}\n\t\t\t";
		buf<<"idElement = idElement->NextSiblingElement();\n\t\t}\n\t\t";
		buf<<"currentNode = currentNode->NextSiblingElement();\n\t}\n";

		buf<<"\t}\n";

		buf<<"\twhile(instream >> temp)\n\t\tsourceDataVec.push_back(temp);\n\tint size = sourceDataVec.size();\n\tsource = new "<< edgeInfo.typeName<<"[size];\n\tfor(int i=0;i<size;i++)\n\t\tsource[i] = sourceDataVec[i];\n";
		buf<<"\treturn size;\n}\n";
		buf<<"istream& operator>>(istream& is, "<<edgeInfo.typeName<<"& object)\n{\n";
		buf<<"\tstring s;\n\tis";
		int begin = edgeInfo.typedefine.find_first_of(' ');
		int end;
		while(begin!=-1)
		{
			end = edgeInfo.typedefine.find_first_of('\n',begin);
			buf<<">>s>>object."<< edgeInfo.typedefine.substr(begin+1,end-begin-2);
			begin = end;
			begin = edgeInfo.typedefine.find_first_of(' ',begin);
		}
		buf<<";\n\treturn is;\n";
		buf<<"}\n";
	}
	if(writerActor)
	{
		StreamEdgeInfo edgeInfo = pEdgeInfo->getEdgeInfo(writerActor->inFlatNodes[0],writerActor);
		buf<<edgeInfo.typeName <<"* sink;\n";
		buf<<"int outPutCount;\n";
		buf<<"void FileWriter(string path,int mode)\n{\n";
		buf<<"\tstringstream outstream;\n";
		buf<<"\tint i=0;\n\twhile(i<outPutCount)\n\t\toutstream <<sink[i++];\n\tdelete[] sink;\n\t";

		buf<<"TiXmlDocument doc;\n\t";
		buf<<"TiXmlElement *root = new TiXmlElement(\"stream\");\n\tdoc.LinkEndChild(root);\n\tstring s1, s2;\n\t";
		buf<<"while(outstream >> s1 >> s2)\n\t{\n\t\t";
		buf<<"const char *cc1 = s1.c_str();\n\t\tTiXmlElement *sub = new TiXmlElement(cc1);\n\t\tchar *c1 = const_cast<char *>(s2.c_str());\n\t\t";
		buf<<"TiXmlText *con = new TiXmlText(c1);\n\t\tsub->LinkEndChild(con);\n\t\troot->LinkEndChild(sub);\n\t}\n\t";
		buf<<"const char* pp = path.c_str();\n\t";
		buf<<"doc.SaveFile(pp);\n";

		buf << "\n}\n";
		buf<<"ostream& operator<<(ostream& os, const "<<edgeInfo.typeName<<"& object)\n{\n";
		buf<<"\tos";
		int begin = edgeInfo.typedefine.find_first_of(' ');
		int end;
		int bb = 0;
		while(begin!=-1)
		{
			end = edgeInfo.typedefine.find_first_of('\n',begin);
			buf<<"<< \" "<< edgeInfo.typedefine.substr(bb,begin-bb) << " \" <<object."<< edgeInfo.typedefine.substr(begin+1,end-begin-2);
			begin = end;
			bb = end + 1;
			begin = edgeInfo.typedefine.find_first_of(' ',begin);
		}
		buf<<"; \n\treturn os;\n";
		buf<<"}\n";
	}
	if(CallModelEmbed)
		buf<<"}\n";
	//������ļ�
	stringstream ss;
	ss<<dir_<< string(objName)<<"global.cpp";
	OutputToFile(ss.str(),  buf.str());
	

}
void X86CodeGenerate::CGMakefile()
{
#ifndef WIN32
	stringstream buf;
	string path = getenv("COSTREAM_LIB");

	//string path = COStreamPath;
	string newObjName = ((string)objName).substr(0,((string)objName).length()-4);
	buf<<"PROGRAM := "<<newObjName<<"\n";

	buf<<"SOURCES := $(wildcard ./*.cpp)\n";
	buf<<"SOURCES += $(wildcard ./src/*.cpp)\n";
	buf<<"SOURCES += $(wildcard $COSTREAM_LIB/*.cpp)\n";
	buf<<"OBJS    := $(patsubst %.cpp,%.o,$(SOURCES))\n";
	buf<<"CC      := g++\n";
	buf<<"CFLAGS  := -ggdb -Wall\n";
	// buf<<"INCLUDE := -I . -I"<<path<<"/runtime/X86Lib2 . -I"<<path<<"/src/3rdpart/include\n";
	// buf<<"LIBPATH := -L"<<path<<"/src/3rdpart/lib\n";
	//buf<<"INCLUDE := -I . -I"<<path<<"runtime/X86Lib2\n";
	//buf<<"LIBPATH := -L"<<path<<"runtime/X86Lib2\n";
	buf<<"INCLUDE := -I . -I"<<path<<"\n";
	buf<<"LIBPATH := -L"<<path<<"\n";
	buf<<"LIB     := -lpthread -ldl\n\n\n";
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
	cout<<"***dir"<<dir_<<endl;
	OutputToFile(ss.str(),buf.str());
#endif
}
void X86CodeGenerate::CGThreads()   //�����̴߳���
{
	string threadname;
	stringstream buf; 
	for (int i=0;i<nCpucore_;i++)
	{
		buf<<"/*���ļ������thread����ں������ں����ڲ���������ˮ����*/\n";
		stringstream ss;
		ss<<dir_<< string(objName)<<"thread_"<<i<<".cpp";
		CGThread(i,buf);			//�����߳�i
		OutputToFile(ss.str(),buf.str());
		ss.str("");
		buf.str("");
	}

}
void X86CodeGenerate::CGThread(int index,stringstream&buf)
{
	int totalstagenum=pSa->MaxStageNum();//���׶κ�
	pair<multimap<FlatNode *,string>::iterator,multimap<FlatNode *,string>::iterator>pos1,pos2;
	//���ͷ�ļ�
	buf<<"#include \"Buffer.h\"\n";
	buf<<"#include \"Producer.h\"\n";
	buf<<"#include \"Consumer.h\"\n";
	buf<<"#include \""<< string(objName) <<"global.h\"\n";
	buf<<"#include \"lock_free_barrier.h\"\t//����barrier����\n";
	buf<<"#include \"" << string(objName)<< "AllActorHeader.h\"\t//��������actor��ͷ�ļ�\n";
	buf<<"#include \"rdtsc.h\"\n";
	buf<<"#include <fstream>\n";
	vector<FlatNode *>tempactorset = Mp->findNodeSetInPartition(index);//tempactorset����߳�index������actor�ļ���
	//����tempactorset����actor������������ʵ��
	for (vector<FlatNode *>::iterator iter=tempactorset.begin();iter!=tempactorset.end();++iter)	
	{//chenwenbin��actorʵ����
		buf<<"extern "<<mapFlatnode2Template_[*iter]<<"<";
		if((*iter)->inFlatNodes.size() == 0 && (*iter)->outFlatNodes.size() != 0)
			buf<<pEdgeInfo->getEdgeInfo(*iter,(*iter)->outFlatNodes[0]).typeName;
		else if((*iter)->inFlatNodes.size() != 0 && (*iter)->outFlatNodes.size() == 0)
			buf<<pEdgeInfo->getEdgeInfo((*iter)->inFlatNodes[0],*iter).typeName;
		else if((*iter)->inFlatNodes.size() != 0 && (*iter)->outFlatNodes.size() != 0)
			buf<<pEdgeInfo->getEdgeInfo(*iter,(*iter)->outFlatNodes[0]).typeName<<","<<pEdgeInfo->getEdgeInfo((*iter)->inFlatNodes[0],*iter).typeName;
		buf<<"> "<<(*iter)->name<<"_obj;\n";//����actor����actor->name_obj,���ù��캯��������Ϊ��������ߵ�ȫ�ֱ���
	}
	if(MakeProfile)
	{
		buf<<"#include  <sys/stat.h>\n";
		buf<<"#include <sys/types.h>\n";
	}
	if(CHECKEACHACTORTIME || MakeProfile)		//��¼ÿ��actor��ִ��ʱ��
	{	
		buf<<"#include <sstream>\n";
		
	}
	if(CallModelEmbed)		//Ƕ�������ֿռ�COStream��
		buf <<" namespace COStream" << string(objName)<<"{ \n";
	//�����ⲿ�����ȫ�ֱ���
	if (Linux)
	{	
		buf<<"extern int MAX_ITER;\n";//ִ�д�����������������
	}
	if (TRACE)
	{
		buf<<"extern double (*deltatimes)["<<nCpucore_<<"][2];\n";	//���Ϊtrace�Ļ������¼ÿ���߳�ÿ�ε�����ͬ���ͼ���ʱ�䳤��
		buf<<"#define	MEASURE_RUNS 100000\n";					//MEASURE_RUNSΪʱ�ӳ��ȵ�λ��Ϊ10Mʱ������
	}
	else if(CALRATIO)
		buf<<"#define	MEASURE_RUNS 100000\n";
	if (index)			//���߳�0�⣬�����̵߳���workSync�ȴ��߳�0���߳�0�����ۼ������߳��Ƿ�����������ñ�־λ
	{
		buf<<"void thread_"<<index<<"_fun()\n{\n";
		buf<<"\tworkerSync("<<index<<");\n";
	}
	else
	{
		buf<<"void thread_"<<index<<"_fun()\n{\n";
		buf<<"\tmasterSync("<<nCpucore_<<");\n";
	}	
	if(TRACE||CALRATIO){
		buf<<"\ttsc_counter c0, c1;\n";	
	}
	if(CALRATIO)
		buf<<"\tdouble cal=0,total=0;\n";
	if(CHECKEACHACTORTIME)						//���ÿ��actor��ÿ��stage�ϵ�steadyworkʱ��
	{
		buf<<"\tofstream txtfw;\n\tstringstream ss;\n";
		buf<<"\tss<<\"���ļ���¼���߳���ÿ��actor��ִ̬�е�ʱ�䣬��һ��Ϊactor�����ƣ��ڶ���Ϊ�ô���ִ̬�е�ʱ������������λΪ10^6��\"<<endl;\n";
		buf<<"\ttsc_counter c2, c3;\n";	
	}
	else if(CALRATIO)
		buf<<"\tofstream txtfw;\n";
	if(MakeProfile)            //cwb����¼ÿ��actor����̬��ִ��ʱ��
	{
		buf<<"\tofstream txtpf;\n\tstringstream sst;\n";
		buf<<"\ttsc_counter cc2, cc3;\n";	
	}

	
	//stage��ʾ�׶κ����飬��ʼ��0�ⶼΪ0
	buf<<"\tchar stage["<<totalstagenum<<"]={0};\n";
	buf<<"\tstage[0]=1;\n";
	//�������߳��ϵ����еĽ׶κţ��ڶ�Ӧ�Ľ׶κ��ڵ���ÿ��actor��initwork
	buf<<"\tfor(int _stageNum=0;_stageNum<"<<totalstagenum<<";_stageNum++)\n";
	buf<<"\t{\n";
	set<int>* ptempstagenum=&mapNum2Stage.find(index)->second;		//���Ҹ�thread��Ӧ�Ľ׶κż���
	set<int>::iterator endIter = ptempstagenum->end();//��ǽ׶κż��ϵ�End
	if(CALRATIO||TRACE)			
			buf<<"\t\tRDTSC(c0);\n";
		for (int i=totalstagenum-1;i>=0;i--)	//����stage Num
		{
			set<int>::iterator stageiter = ptempstagenum->find(i);		//���Ҹ��̶߳�Ӧ�ڽ׶�i�Ƿ���actor
			if(stageiter!=endIter){					//��stage�ڸ�thread��
				buf<<"\t\tif("<<i<<"==_stageNum)\n\t\t{\n";
				vector<FlatNode*> flatVec = pSa->FindActor(i);	//ȡ���ڸý׶ε�����actor����
				for(vector<FlatNode*>::iterator iter1=flatVec.begin();iter1!=flatVec.end();++iter1)//����actor�����ó�̬initScheduleWork
				{
					if(index == Mp->findPartitionNumForFlatNode(*iter1))
					{
						buf<<"\t\t\t"<<(*iter1)->name<<"_obj.runInitScheduleWork();\n";
					}
				}
				buf<<"\t\t}\n";
			}
		}	
	if (TRACE)		//��¼ʱ��
	{
		buf<<"\t\tRDTSC(c1);\n";
		if(CALRATIO)
			buf<<"\t\tcal += deltatimes[_stageNum]["<<index<<"][0] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		else
			buf<<"\t\tdeltatimes[_stageNum]["<<index<<"][0] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		buf<<"\t\tc0 = c1;\n";
	}
	else if(CALRATIO)
	{
		buf<<"\t\tRDTSC(c1);\n";
		buf<<"\t\tcal += COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
	}
	//�����߳�0�������߳�
	if(index)
		buf<<"\t\n\t\tworkerSync("<<index<<");\n";
	else
		buf<<"\t\n\t\tmasterSync("<<nCpucore_<<");\n";
	if(TRACE){
		buf<<"\t\tRDTSC(c1);\n";
		if(CALRATIO)
			buf<<"\t\ttotal += deltatimes[_stageNum]["<<index<<"][1] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		else
			buf<<"\t\tdeltatimes[_stageNum]["<<index<<"][1] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		buf<<"\t\tc0 = c1;\n";		
	}
	else if(CALRATIO)
	{
		buf<<"\t\tRDTSC(c1);\n";
		buf<<"\t\ttotal += COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
	}
	//��̬�������
	buf<<"\t}\n";
	//���¿�ʼ������߳�ÿ���׶�ÿ��actor����̬����
	buf<<"\tfor(int _stageNum="<<totalstagenum<<";_stageNum<2*"<<totalstagenum<<"+MAX_ITER-1;_stageNum++)\n\t{\n";
	if(CALRATIO||TRACE)
		buf<<"\t\tRDTSC(c0);\n";
		
	if(CHECKEACHACTORTIME)
	{
		buf<<"\t\tss<<\"stage:\\t\"<<_stageNum<<\"\t\"<<\"actorname\\t\"<<\"time(cycles)\"<<\"steadycount\\t\"<<\"averagetime\"<<endl;\n";
		
	}
	if(MakeProfile)       //cwb
	{
		buf<<"\t\tint steadycount;\n";
	}
	for (int i=totalstagenum-1;i>=0;i--)	//����stage
	{			

		set<int>::iterator stageiter = ptempstagenum->find(i);
		if(stageiter!=endIter){					//��stage�ڸ�thread��
			buf<<"\t\tif(stage["<<i<<"])\n\t\t{\n";

			vector<FlatNode*> flatVec = pSa->FindActor(i);
			vector<FlatNode*>::iterator iter1;
			for(iter1=flatVec.begin();iter1!=flatVec.end();++iter1){
				if(index == Mp->findPartitionNumForFlatNode(*iter1))
				{
					if(CHECKEACHACTORTIME)
					{
						buf<<"\t\t\tRDTSC(c2);\n";
					}
					if(MakeProfile)                         //cwb
					{
						buf<<"\t\t\tRDTSC(cc2);\n";
					}
					buf<<"\t\t\t"<<(*iter1)->name<<"_obj.runSteadyScheduleWork();\n";
					if(CHECKEACHACTORTIME)
					{
						buf<<"\t\t\tRDTSC(c3);\n";
						buf<<"\t\t\tss<<\"\\t"<<(*iter1)->name<<":\"<<COUNTER_DIFF(c3,c2,CYCLES)<<endl;\n";
					}
					if(MakeProfile)                    //cwb,�����actor�ô�work�������ʱ������
					{
						buf<<"\t\t\tRDTSC(cc3);\n";
						buf<<"\t\t\tsteadycount = "<<sssg_->GetSteadyCount((*iter1))<<";\n";
						buf<<"\t\t\t"<<(*iter1)->name<<"_time += COUNTER_DIFF(cc3,cc2,CYCLES)/steadycount;\n";
					}
				}
			}
			buf<<"\t\t}\n";
		}
	}
	if(CHECKEACHACTORTIME)
		buf<<"\t\tss<<endl;\n";
	buf<<"\t\tfor(int index="<<totalstagenum-1<<"; index>= 1; --index)\n";
	buf<<"\t\t\tstage[index] = stage[index-1];\n\t\tif(_stageNum == (MAX_ITER - 1 + "<<totalstagenum<<"))\n\t\t{\n\t\t\tstage[0]=0;\n\t\t}\n";
	if (TRACE)
	{
		buf<<"\t\tRDTSC(c1);\n";
		if(CALRATIO)
			buf<<"\t\tcal += deltatimes[_stageNum]["<<index<<"][0] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		else
			buf<<"\t\tdeltatimes[_stageNum]["<<index<<"][0] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		buf<<"\t\tc0 = c1;\n";
	}
	else if(CALRATIO)
	{
		buf<<"\t\tRDTSC(c1);\n";
		buf<<"\t\tcal += COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
	}
	if(index)
		buf<<"\t\n\t\tworkerSync("<<index<<");\n";
	else
		buf<<"\t\n\t\tmasterSync("<<nCpucore_<<");\n";
	if (TRACE)
	{
		buf<<"\t\tRDTSC(c1);\n";
		if(CALRATIO)
			buf<<"\t\ttotal += deltatimes[_stageNum]["<<index<<"][1] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		else
			buf<<"\t\tdeltatimes[_stageNum]["<<index<<"][1] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
//		buf<<"\t\tdeltatimes["<<pSa->MaxStageNum()<<"+_stageNum]["<<index<<"][1] = COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
		buf<<"\t\tc0 = c1;\n";
	}
	else if(CALRATIO)
	{
		buf<<"\t\tRDTSC(c1);\n";
		buf<<"\t\ttotal += COUNTER_DIFF(c1, c0, CYCLES)/MEASURE_RUNS;\n";
	}
	buf<<"\t}\n";//��̬���Ƚ�ֹ
	if(CHECKEACHACTORTIME)
	{
		buf<<"\ttry\n\t{\n\t\ttxtfw.open(\"thread "<<index<<"'s stage.txt\");\n\t\ttxtfw<<ss.str();\n\t\ttxtfw.close();\n\t}\n\tcatch(...)\n\t\t{cout<<\"error:output to file\"<<endl;\n\t}\n";
	}
	if(MakeProfile)    //cwb
	{
		//���ÿ��actor��ƽ������ʱ��
		for (vector<FlatNode *>::iterator iter=tempactorset.begin();iter!=tempactorset.end();++iter)
			//buf<<"\tsst<<\""<<(*iter)->name<<" \"<<(int)"<<(*iter)->name<<"_time/MAX_ITER<<endl;\n";
			buf<<"\tsst<<\""<<(*iter)->name<<" \"<<(int)"<<(*iter)->name<<"_time/MAX_ITER<<endl;\n";
		mkdir(string("profileresult"));
		buf<<"\ttry\n\t{\n\t\tmkdir(\"profile\",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);\n\t\ttxtpf.open(\""<<string(objName)<<".profile\");\n\t\ttxtpf<<sst.str();\n\t\ttxtpf.close();\n\t\tsystem(\"mv "<<string(objName)<<".profile ./profile\");\n\t}\n\tcatch(...)\n\t\t{cout<<\"error:output to file\"<<endl;\n\t}\n";
	}
	if(CALRATIO)
		buf<<"\ttry\n\t{\n\t\ttxtfw.open(\"thread "<<index<<"'s CALRATIO.txt\");\n\t\ttxtfw<<\"����:\t\"<<cal<<endl<<\"��ʱ��:\t\"<<total<<endl;\n\t\ttxtfw.close();\n\t}\n\tcatch(...)\n\t\t{cout<<\"error:output to file\"<<endl;\n\t}\n";
	buf<<"}\n";
	if(CallModelEmbed)
		buf<<"}";
}
void X86CodeGenerate::CGAllActorHeader()
{
	vector<FlatNode*>::iterator iter;
	stringstream buf,ss;
	buf<<"/*��������actor��ͷ�ļ�����Ҫ��Ϊ�˷������ļ�����*/\n\n";
	for (iter=flatNodes_.begin();iter!=flatNodes_.end();++iter)
	{	
		buf<<"#include \""<< string(objName) <<(*iter)->name<<".h\"\n";	
	}
	ss<<dir_<< string(objName)<<"AllActorHeader.h";
	OutputToFile(ss.str(),buf.str());
}
void X86CodeGenerate::CGAllActorCpp()
{

}
//����main.cpp������main�����Ķ���
void X86CodeGenerate::CGMain()
{
	stringstream buf,ss;
	char a[10];
	if(TRACE)
	{
		buf<<"#include \"rdtsc.h\"\n";
		buf<<"#include <sstream>\n";
		buf<<"#include <fstream>\n";
	}
	string Tab;
	if(CallModelEmbed)
	{
		buf<<"#include \"" << string(objName) <<"RunCOStream.h\"\n";
		buf<<"#include \"setCpu.h\"\n";
		buf <<" namespace COStream" << string(objName)<<"{ \n";
		Tab = "\t";
	}
	else
	{
		buf<<"#include \"iostream\"\n";
		buf<<"#include <unistd.h>\n";
		buf<<"#include \"stdlib.h\"\n";
		buf<<"#include <pthread.h>\n";
		buf<<"#include \"setCpu.h\"\n";
		buf<<"#include \"lock_free_barrier.h\"	//����barrier����\n";
		buf<<"#include \"" << string(objName)<<"AllActorHeader.h\"\t//��������actor��ͷ�ļ�\n";
		buf<<"#include \"" << string(objName) <<"global.h\"\n";
		buf<<"using namespace std;\n";
	}
	buf<<Tab<<"int MAX_ITER=1;//Ĭ�ϵ�ִ�д�����1\n";
	if(needExternType)
		buf<<Tab<<"SPLExternType *splExtern; //�ӿڽṹ��\n";
	pair<multimap<FlatNode *,string>::iterator,multimap<FlatNode *,string>::iterator>pos1,pos2;
	for (vector<FlatNode*>::iterator iter = sssg_->flatNodes.begin();iter != sssg_->flatNodes.end(); iter++)
	{
		buf<<mapFlatnode2Template_[*iter]<<"<";
		if((*iter)->inFlatNodes.size() == 0 && (*iter)->outFlatNodes.size() != 0)
			buf<<pEdgeInfo->getEdgeInfo(*iter,(*iter)->outFlatNodes[0]).typeName;
		else if((*iter)->inFlatNodes.size() != 0 && (*iter)->outFlatNodes.size() == 0)
			buf<<pEdgeInfo->getEdgeInfo((*iter)->inFlatNodes[0],*iter).typeName;
		else if((*iter)->inFlatNodes.size() != 0 && (*iter)->outFlatNodes.size() != 0)
			buf<<pEdgeInfo->getEdgeInfo(*iter,(*iter)->outFlatNodes[0]).typeName<<","<<pEdgeInfo->getEdgeInfo((*iter)->inFlatNodes[0],*iter).typeName;
		buf<<"> "<<(*iter)->name<<"_obj(";//����actor����actor->name_obj,���ù��캯��������Ϊ��������ߵ�ȫ�ֱ���
		//chenwenbin ����actor��Ӧ��param��Ϊ�������빹�캯��
		paramList *pList = (*iter)->contents->params;
		for (int i = 0; i < (*iter)->contents->paramSize; i++)
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
		//chenwenbin ��pop,pushvalue���츳ֵ
		for (int i = 0; i < (*iter)->inPopWeights.size(); i++)
		{
			buf<<(*iter)->inPopWeights[i]<<",";
		}
		for (int i = 0; i < (*iter)->outPushWeights.size(); i++)
		{
			buf<<(*iter)->outPushWeights[i]<<",";
		}
		//chenwenbin ��̬��ִ̬�д�������
		buf<<sssg_->GetSteadyCount(*iter)<<","<<sssg_->GetInitCount(*iter)<<",";
		//mapActor2InEdge��mapActor2OutEdge��Ÿ�actor����Ӧ�����������buffer������
		pos1=mapActor2InEdge.equal_range(*iter);		
		pos2=mapActor2OutEdge.equal_range(*iter);
		while (pos2.first!=pos2.second)
		{
			buf<<pos2.first->second<<",";
			++pos2.first;
		}
		while(pos1.first!=pos1.second)
		{
			buf<<pos1.first->second<<",";
			++pos1.first;
		}

		buf.seekp((int)buf.tellp()-1);
		buf<<");\n";
	}
	if (TRACE)
	{
		buf<<Tab<<"double (*deltatimes)["<<nCpucore_<<"][2];\n";
		buf<<Tab<<"typedef double time_array["<<nCpucore_<<"][2];\n";
	}
	for (int i=0;i<nCpucore_;i++)
	{
		buf<<Tab<<"extern void thread_"<<i<<"_fun();\n";
	}	
	for(int i=1;i<nCpucore_;i++)
	{
		buf<<Tab<<"void* thread_"<<i<<"_fun_start(void *)\n"<<Tab<<"{\n\t"<<Tab<<"set_cpu("<<i<<");\n\t"<<Tab<<"thread_"<<i<<"_fun();\n\t"<<Tab<<"return 0;\n"<<Tab<<"}\n";
	}
	if(CallModelEmbed){
		if(readerActor)	//����filereader��������������in�����������������Դ
		{
			int fileReaderPush,fileReaderSteadyCount,fileReaderInitCount;
			StreamEdgeInfo  readerEdgeInfo = pEdgeInfo->getEdgeInfo(readerActor,readerActor->outFlatNodes[0]);
			fileReaderInitCount = sssg_->GetInitCount(readerActor);
			fileReaderSteadyCount = sssg_->GetSteadyCount(readerActor);
			fileReaderPush = readerActor->outPushWeights[0];
			buf<<"\tRunCOStream::RunCOStream(stringstream& in)\n\t{\n";
			buf<<"\t\tvector<"<<readerEdgeInfo.typeName<<"> sourceDataVec;\n";
			buf<<"\t\t"<<readerEdgeInfo.typeName<<" temp;\n";
			buf<<"\t\twhile(in >> temp)\n\t\t\tsourceDataVec.push_back(temp);\n\t\tint size = sourceDataVec.size();\n\t\tsource = new "<<readerEdgeInfo.typeName<<"[size];\n\t\tfor(int i=0;i<size;i++)\n\t\t\tsource[i] = sourceDataVec[i];\n";
			buf<<"\t\tMAX_ITER = (size-"<<fileReaderInitCount<<"*"<<fileReaderPush<<")/("<<fileReaderSteadyCount<<"*"<<fileReaderPush<<");\n";//���õ���ִ�д��� (size - ��ִ̬�д���*push)/(��ִ̬�д���*push)

			StreamEdgeInfo  writerEdgeInfo;
			int fileWriterPush,fileWriterSteadyCount,fileWriterInitCount;
			if(writerActor)	//����filewriter�������������ָ��
			{
				writerEdgeInfo = pEdgeInfo->getEdgeInfo(writerActor->inFlatNodes[0],writerActor);
				fileWriterInitCount = sssg_->GetInitCount(writerActor);
				fileWriterSteadyCount = sssg_->GetSteadyCount(writerActor);
				fileWriterPush = writerActor->inPopWeights[0];
				buf<<"\t\tsink = new "<<writerEdgeInfo.typeName<<"[MAX_ITER*"<<(fileWriterInitCount+fileWriterSteadyCount)*fileWriterPush<<"];\n";	//��������С
				buf<<"\t\toutputNum = MAX_ITER*"<<(fileWriterInitCount+fileWriterSteadyCount)*fileWriterPush<<";\n";
			}
			buf<<"\t}\n";

			/*Դ��������ָ�빹��*/
			buf<<"\tRunCOStream::RunCOStream(void* pSrc,int srcDataCount)\n\t{\n";
			buf<<"\t\tsource = ("<<readerEdgeInfo.typeName<<"*)pSrc;\n";
			buf<<"\t\tMAX_ITER = (srcDataCount-"<<fileReaderInitCount<<"*"<<fileReaderPush<<")/("<<fileReaderSteadyCount<<"*"<<fileReaderPush<<");\n";//���õ���ִ�д��� (size - ��ִ̬�д���*push)/(��ִ̬�д���*push)
			if(writerActor)
			{
				buf<<"\t\tsink = new "<<writerEdgeInfo.typeName<<"[MAX_ITER*"<<(fileWriterInitCount+fileWriterSteadyCount)*fileWriterPush<<"];\n";
				buf<<"\t\toutputNum = MAX_ITER*"<<(fileWriterInitCount+fileWriterSteadyCount)*fileWriterPush<<";\n";
			}
			buf<<"\t}\n";

			/*ͨ���ļ�·������*/
			buf<<"\tRunCOStream::RunCOStream(string path)\n\t{\n";
			buf<<"\t\tifstream ifile;\n";
			buf<<"\t\tifile.open(path.c_str());\n";
			buf<<"\t\tstringstream instream;\n";
			buf<<"\t\tinstream<<ifile.rdbuf();\n";
			buf<<"\t\tifile.close();\n";
			buf<<"\t\tvector<"<<readerEdgeInfo.typeName<<"> sourceDataVec;\n";
			buf<<"\t\t"<<readerEdgeInfo.typeName<<" temp;\n";
			buf<<"\t\twhile(instream >> temp)\n\t\t\tsourceDataVec.push_back(temp);\n\t\tint size = sourceDataVec.size();\n\t\tsource = new "<<readerEdgeInfo.typeName<<"[size];\n\t\tfor(int i=0;i<size;i++)\n\t\t\tsource[i] = sourceDataVec[i];\n";
			buf<<"\t\tMAX_ITER = (size-"<<fileReaderInitCount<<"*"<<fileReaderPush<<")/("<<fileReaderSteadyCount<<"*"<<fileReaderPush<<");\n";//���õ���ִ�д��� (size - ��ִ̬�д���*push)/(��ִ̬�д���*push)
			if(writerActor)
			{
				buf<<"\t\tsink = new "<<writerEdgeInfo.typeName<<"[MAX_ITER*"<<(fileWriterInitCount+fileWriterSteadyCount)*fileWriterPush<<"];\n";
				buf<<"\t\toutputNum = MAX_ITER*"<<(fileWriterInitCount+fileWriterSteadyCount)*fileWriterPush<<";\n";
			}
			buf<<"\t}\n";
		}
		else{			//û��filereader�ڵ㣬���������������Լ���������Դ��ֻ��Ҫ����Ĭ�Ϲ��캯��
			buf<<"\tRunCOStream::RunCOStream()\n\t{\n\t}\n";	
		}
		/*Run,����stream����*/	
		//		if(needExternType)
		//			buf<<"\tvoid RunCOStream::Run(int iter_num,SPLExternType *splPoint)\n\t{\n";
		//		else
		buf<<"\tvoid RunCOStream::Run(int iter_num)\n\t{\n";
		if(CALRATIO){
			buf<<"\t\t tsc_counter c0, c1;//�����������run��ʱ�� lihe\n";
			buf<<"\t\t long totalCal,totalSyn;\n";
		}
		//		buf<<"\t\tMAX_ITER = iter_num;\n";
		//		if(needExternType)
		//			buf<<"\t\tsplExtern = splPoint;\n";
		//for linux thread
		buf<<"\t\tpthread_t tid["<<nCpucore_-1<<"];\n";
		//buf<<"\t\tset_cpu(0);\n";
		if(CALRATIO)
			buf<<"\t\tRDTSC(c0);//����ʼʱ�䣬 lihe\n";
		buf<<"\t\tallocBarrier("<<nCpucore_<<");\n";
		for (int i=1;i<nCpucore_;i++)
		{
			buf<<"\t\tpthread_create (&tid["<<i-1<<"], NULL, thread_"<<i<<"_fun_start, (void*)NULL);\n";
		}
		buf<<"\t\tthread_0_fun();\n";
		if(CALRATIO){
			buf<<"\t\tRDTSC(c1);//�������ʱ�䣬 lihe\n";
			buf<<"\t\tlong timeCount = COUNTER_DIFF(c1, c0, CYCLES);//�õ�����������ʱ�� lihe\n";
			buf<<"\t\tcout<<\"totalTime:	\"<<timeCount<<endl;//��� lihe\n";
			buf<<"\t\tfor(int i=0;i<"<<nCpucore_<<";i++){\n";
			buf<<"\t\t\tcout<<calArr[i]<<\"	\"<<synArr[i]<<endl;//�������ʱ���ͬ��ʱ�䣬 lihe\n";
			buf<<"\t\t\ttotalCal += calArr[i];//�ۼ���������ļ���ʱ�䣬 lihe\n" ;
			buf<<"\t\t\ttotalSyn += synArr[i];//�ۼ����������ͬ��ʱ�䣬 lihe\n" ;
			buf<<"\t\t}\n";
			buf<<"\t\tcout<<totalCal<<\"	\"<<totalSyn<<endl;//�����������ļ���ʱ���ͬ��ʱ�䣬 lihe\n";
		}
		if(writerActor)
			buf<<"\t\toutArray = sink;\n";
		buf<<"\t}\n";

		buf<<"}\n";
		ss<<dir_<< string(objName) <<"RunCOStream.cpp";
	}
	else				//����ģʽ������main
	{
		buf<<"int main(int argc,char **argv)\n{\n";
		buf<<"\tvoid setRunIterCount(int,char**);\n";
		buf<<"\tsetRunIterCount(argc,argv);\n";
		buf<<"\tset_cpu(0);\n";	
		buf<<"\tallocBarrier("<<nCpucore_<<");\n";	
		buf<<"\tpthread_t tid["<<nCpucore_-1<<"];\n";
		for (int i=1;i<nCpucore_;i++)
		{
			buf<<"\tpthread_create (&tid["<<i-1<<"], NULL, thread_"<<i<<"_fun_start, (void*)NULL);\n";
		}
		buf<<"\tthread_0_fun();\n";	
		if (FileW)
		{
			OutputPath = outfileName;
			buf<<"\tstring outpath = "<<OutputPath<<";\n";
			buf<<"\tFileWriter(outpath,0);\n";
		}
		if (TRACE)			
		{
			buf<<"\tstring txtDataFileName = \"traceDat.txt\";\n\tstringstream txtDataContent;\n";
			buf<<"\tdouble perThreadTotalTime["<<nCpucore_<<"]={0},perThreadCalucatorTime["<<nCpucore_<<"]={0};\n";
			buf<<"\ttxtDataContent<<\"���ļ���¼ÿ���߳�ÿ�ε��������ͬ������Ҫ��ʱ������������λΪ10^6\"<<endl;\n";
			buf<<"\tfor(int threadindex=0;threadindex<"<<nCpucore_<<";threadindex++)\n\t{\n";
			buf<<"\t\ttxtDataContent<<\"�߳�\"<<threadindex<<\"��ִ��ʱ������:\"<<endl;\n";
			buf<<"\t\ttxtDataContent<<\"initworkstage:\\t\\t����ʱ��\\t\\tͬ��ʱ��\"<<endl;\n";
			buf<<"\t\tfor(int stageindex=0;stageindex<"<<pSa->MaxStageNum()<<";stageindex++)\n\t\t{\n";
			buf<<"\t\t\ttxtDataContent<<stageindex<<\":\t\"<<deltatimes[stageindex][threadindex][0]<<\"\t\t\t\"<<deltatimes[stageindex][threadindex][1]<<endl;\n\n";
			buf<<"\t\t\tperThreadCalucatorTime[threadindex] += deltatimes[stageindex][threadindex][0];\n";
			buf<<"\t\t\tperThreadTotalTime[threadindex] += (deltatimes[stageindex][threadindex][0]+deltatimes[stageindex][threadindex][1]);\n\t\t}\n";
			buf<<"\t\ttxtDataContent<<\"steadyworkstage:\\t\\t����ʱ��\\t\\tͬ��ʱ��\"<<endl;\n";
			buf<<"\t\tfor(int stageindex=0;stageindex<MAX_ITER+"<<pSa->MaxStageNum()-1<<";stageindex++)\n\t\t{\n";
			buf<<"\t\t\ttxtDataContent<<stageindex<<\":\t\"<<deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][0]<<\"\t\t\t\"<<deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][1]<<endl;\n";
			buf<<"\t\t\tperThreadCalucatorTime[threadindex] += deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][0];\n";
			buf<<"\t\t\tperThreadTotalTime[threadindex] += (deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][0]+deltatimes[stageindex+"<<pSa->MaxStageNum()<<"][threadindex][1]);\n\t\t}\n\t}\n";
			buf<<"\ttxtDataContent<<\"\\n\\n�߳�����ʱ��ֲ����£�\"<<endl;\n";
			buf<<"\ttxtDataContent<<\"�߳�		����ʱ��		ͬ��ʱ��		����ٷֱ�\"<<endl;\n";
			buf<<"\tfor(int threadindex=0;threadindex<"<<nCpucore_<<";threadindex++)\n";
			buf<<"\t\ttxtDataContent<<\"�߳�\"<<threadindex<<\":		\"<<perThreadTotalTime[threadindex]<<\"		\"<<(perThreadTotalTime[threadindex]- perThreadCalucatorTime[threadindex])<<\"		\"<<100*perThreadCalucatorTime[threadindex]/perThreadTotalTime[threadindex]<<\"%\"<<endl;\n";
			buf<<"\tofstream txtfw;\n\ttry\n\t{\n\t\ttxtfw.open(txtDataFileName.c_str());\n\t\ttxtfw<<txtDataContent.str();\n\t\ttxtfw.close();\n\t}\n\tcatch(...)\n\t{\n\t\tcout<<\"error:output to file\"<<endl;\n\t}\n";
			//trace for image
			buf<<"\tstring imageDataFileName = \"traceDat.dat\";\n\tstringstream imageDataContent;\n\tdouble base;\n";
			buf<<"\tint rectangleIndex = 1;\n\tdouble rectangleHeight = 0.02;\n\timageDataContent<<\"reset;\";\n";
			buf<<"\tfor(int threadindex=0;threadindex<"<<nCpucore_<<";threadindex++)\n\t{\n";
			buf<<"\t\tbase = 0;\n";
			buf<<"\t\tfor(int stageindex=0;stageindex<MAX_ITER+"<<2*pSa->MaxStageNum()-1<<";stageindex++)\n\t\t{\n";
			buf<<"\timageDataContent<<\"set object \"<<rectangleIndex++<<\" rectangle from \"<<base<<\",\"<<threadindex*rectangleHeight<<\" to \";\n";
			buf<<"\t\t\tbase += deltatimes[stageindex][threadindex][0];\n";
			buf<<"\timageDataContent<<base<<\",\"<<(threadindex+1)*rectangleHeight<<\" fc rgb \\\"red\\\";\\n\";\n";
			buf<<"\timageDataContent<<\"set object \"<<rectangleIndex++<<\" rectangle from \"<<base<<\",\"<<threadindex*rectangleHeight<<\" to \";\n";
			buf<<"\t\t\tbase += deltatimes[stageindex][threadindex][1];\n";
			buf<<"\timageDataContent<<base<<\",\"<<(threadindex+1)*rectangleHeight<<\" fc rgb \\\"green\\\";\\n\";\n\t\t}\n\t}\n";
			buf<<"\timageDataContent<<\"plot [0:\"<<int(base+2)<<\"][0:\"<<"<<nCpucore_<<"*rectangleHeight<<\"] 10000000 notitle;\\n\"; \n";
			buf<<"\tofstream imagefw;\n\ttry\n\t{\n\t\timagefw.open(imageDataFileName.c_str());\n\t\timagefw<<imageDataContent.str();\n\t\timagefw.close();\n\t}\n\tcatch(...)\n\t{\n\t\tcout<<\"error:output to file\"<<endl;\n\t}\n";
		}
		buf<<"\treturn 0;\n";
		buf<<"\n}\n";
		buf<<"void setRunIterCount(int argc,char **argv)\n{\n";
		buf<<"\tint oc;\n";
		buf<<"\tchar *b_opt_arg;\n";
		buf<<"\twhile((oc=getopt(argc,argv,\"i:\"))!=-1)\n";
		buf<<"\t{\n";
		buf<<"\t\tswitch(oc)\n";
		buf<<"\t\t{\n";
		buf<<"\t\t\tcase 'i':\n";
		buf<<"\t\t\t\tMAX_ITER=atoi(optarg);\n";
		buf<<"\t\t\t\tbreak;\n";
		buf<<"\t\t}\n";
		buf<<"\t}\n";
		if (FileR)
		{
			InputPath = infileName;
			if (existFileWriter)
			{
				buf<<"\tstring inpath = "<<InputPath<<";\n";
				buf<<"\tint sourceCount = FileReader(inpath,0);\n";
			}
			else
			{
				buf<<"\tstring inpath = "<<InputPath<<";\n";
				buf<<"\tint sourceCount = FileReader(inpath,0);\n";
			}
			int initworkcount = sssg_->GetInitCount(flatNodes_[0]);
			int steadyworkcount = sssg_->GetSteadyCount(flatNodes_[0]);;
			int perWorkItem = flatNodes_[0]->outPushWeights[0];
			buf<<"\tint maxtimes = (sourceCount-"<<perWorkItem<<"*"<<initworkcount<<")/("<<steadyworkcount<<"*"<<perWorkItem<<");\n";//���õ���ִ�д���
			buf<<"\tif(MAX_ITER>maxtimes)\n\t{\t\tcout<<\"�ļ������ݲ���ִ��\"<<MAX_ITER<<\"��,ʵ��ִֻ��\"<<maxtimes<<\"��\";\n\t\tMAX_ITER = maxtimes;\n\t}\n";

		}
		if(writerActor && FileW)
		{
			int initworkcount = sssg_->GetInitCount(flatNodes_[0]);
			int steadyworkcount = sssg_->GetSteadyCount(flatNodes_[0]);;
			int perWorkItem = flatNodes_[0]->outPushWeights[0];
			StreamEdgeInfo  edgeInfo =  pEdgeInfo->getEdgeInfo(writerActor->inFlatNodes[0],writerActor);
			initworkcount = sssg_->GetInitCount(writerActor);
			steadyworkcount = sssg_->GetSteadyCount(writerActor);
			perWorkItem = writerActor->inPopWeights[0];
			buf<<"\tsink = new "<<edgeInfo.typeName<<"[MAX_ITER*"<<(initworkcount+steadyworkcount)*perWorkItem<<"];\n";
			buf<<"\toutPutCount = MAX_ITER*"<<(initworkcount+steadyworkcount)*perWorkItem<<";\n";
		}
		if(TRACE)
			buf<<"\t\tdeltatimes = new time_array[MAX_ITER-1+2*"<<pSa->MaxStageNum()<<"];\n";
	
		buf<<"}\n";
		ss<<dir_<<"main.cpp";
	}
	OutputToFile(ss.str(),buf.str());

	if(CallModelEmbed)	//����RunCOStream.h��COStream.h
	{
		//����RunCOStream.h
		buf.str("");
		ss.str("");
		if(needExternType)
			buf<<"#include \"ExternType.h\" \n";


		buf<<"#include \"" << string(objName)<<"COStream.h\"\n";
		buf<<"#include \"iostream\"\n";
		buf<<"#include \"stdlib.h\"\n";
		buf<<"#include <pthread.h>\n";
		//buf<<"#include \"setCpu.h\"\n";
		buf<<"#include \"lock_free_barrier.h\"	//����barrier����\n";
		buf<<"#include \"" << string(objName) <<"global.h\"\n";
		buf<<"#include <vector>\n";
		buf<<"using namespace std;\n";

		buf <<" namespace COStream" << string(objName)<<"{ \n";
		buf<<"\tclass RunCOStream\n\t{\n\tpublic:\n";
		buf<<"\t\tRunCOStream(stringstream& in);\n";
		buf<<"\t\tRunCOStream();\n";
		buf<<"\t\tRunCOStream(string path);\n";
		buf<<"\t\tvoid Run(void* pSrc,int srcDataCount);\n";
		/*GetOutputNum��ȡ��������ݸ���*/
		buf<<"\t\tint GetOutputNum()\n\t\t{\n\t\t\treturn outputNum;\n\t\t}\n";
		/*GetOutputArray,ȡ�������������ָ��*/
		buf<<"\t\tvoid* GetOutputArray()\n\t\t{\n\t\t\treturn outArray;\n\t\t}\n";
		/*��������*/
		buf<<"\t\t~RunCOStream()\n\t\t{\n\t\t\tdelete[] outArray;\n\t\t}\n";
		/*˽�г�Ա����*/
		buf<<"\tprivate:\n\t\tint outputNum;\n\t\tvoid * outArray;\n";
		buf<<"\t};\n";
		buf<<"}\n";
		ss<<dir_<< string(objName) <<"RunCOStream.h";
		OutputToFile(ss.str(),buf.str());
		//����COStream.h
		buf.str("");
		ss.str("");
		buf <<" namespace COStream" << string(objName)<<"{ \n";
		buf<<"\tclass RunCOStream;\n}\n";
		ss<<dir_<< string(objName) <<"COStream.h";
		OutputToFile(ss.str(),buf.str());
	}
}
//����FileReaderActor��initwork��work����
void X86CodeGenerate::CGFileReaderActor(stringstream &buf)		
{
	/*����initWork()*/
	buf<<"\tvoid initWork()\n\t{\n\t\tdataCount = 0;\n\t}\n";
	/*FileReader��work����*/
	buf<<"\tvoid work()\n\t{\n";
	buf<<"\t\t"<<readerActor->pushString[0]<<"[0] = source[dataCount++];\n\t\tpushToken();\n\t}\n";
}
//����FileWriterActor��initwork��work����
void X86CodeGenerate::CGFileWriterActor(stringstream &buf)
{
	/*����initWork()*/
	buf<<"\tvoid initWork()\n\t{\n\t\tdataCount = 0;\n\t}\n";
	/*Filewriter��work����*/
	buf<<"\tvoid work()\n\t{\n";
	//	buf<<"\t\t"<<writerActor->peekString[0]<<"[0] = sink[dataCount++];\n\t\tpopToken();\n\t}\n";
	buf<<"\t\t"<<"sink[dataCount++] = " << writerActor->peekString[0]<<"[0];\n\t\tpopToken();\n\t}\n";
}
int X86CodeGenerate::OutputChar(char val)
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
		if (isprint(val)) //�ж�val�Ƿ�Ϊ�ɴ�ӡ�ַ�
		{
			declInitList<<(val);
		} else 
		{
			declInitList<<"\\"<<val;
		}
	}

	return 1;
}
int X86CodeGenerate::OutputString(const char *s)
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
void X86CodeGenerate::CharToText(char *str, unsigned char val){ 
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
//����actor�ڵ����
void X86CodeGenerate::OutputStmt(Node *node, int offset)
{
	if (node->typ != Block)
		OutputTabs(offset);
	if (node == NULL)
	{
		declInitList << ";";
		return;
	}
	//�������node
	SPL2X86_Node(node, offset);
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
void X86CodeGenerate::OutputStmtList(List *list, int offset)
{
	ListMarker marker; 
	Node *item = NULL; 

	IterateList(&marker, list); 
	while (NextOnList(&marker, (GenericREF)&item))
	{ 
		OutputStmt(item, offset);
	}
}
//��ά�����ʼ�����̣��ݹ飩
void X86CodeGenerate::RecursiveAdclInit(List *init)
{
	//��ά������init��һ����ά������
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
			SPL2X86_Node(item->u.unary.expr,0);
		}
		else if(item->typ == Const) // ��������Ա�ǻ�������
		{
			if(i!=1) declInitList<<",";
			SPL2X86_Node(item,0);
		}
		else if (item->typ == Initializer)//��������Ա��һ��������ݹ�
		{
			RecursiveAdclInit(item->u.initializer.exprs);
			if(i != len)
			{
				declInitList<<",";
				OutputCRSpaceAndTabs(4);
			}
		}
		else if (item->typ == ImplicitCast)//�������͵���ʽת��
		{
			if(i!=1) declInitList<<",";
			SPL2X86_Node(item->u.implicitcast.value,0);
		}
		i++;
	}
	declInitList<<"}";
}
void X86CodeGenerate::AdclInit(Node *from, int offset)
{
	Node *arrayNode = from->u.decl.type;
	Node *tmpNode = from->u.decl.type;
	Node *initNode = from->u.decl.init;//������ʼ��node
	string name = from->u.decl.name;
	string arrayType = GetArrayDataType(tmpNode->u.adcl.type);
	string dim = GetArrayDim(tmpNode->u.adcl.dim);
	if (initNode == NULL) //���û�г�ʼ�������������ͽ��г�ʼ��
	{
		Node* tempNode = from->u.decl.type;
		declList<<arrayType;
		while(tempNode->u.decl.type){
			dim = GetArrayDim(tempNode->u.adcl.dim);
			declList<<"["<<dim<<"]";
			tempNode = tempNode->u.adcl.type;
		}
		declList<<";\n";   
		declInitList<<declInitList_temp.str()<<"={0};\n";
		declInitList_temp.str("");
	}
	else if(initNode->typ==Call){
		declInitList<<";"; 
	}
	else//������ڳ�ʼ�����ʼ��Ϊָ��ֵ
	{
		declInitList<<declInitList_temp.str();
		declInitList_temp.str("");
		Node* tempNode = from->u.decl.type;
		declList<<arrayType;
		while(tempNode->u.decl.type){
			dim = GetArrayDim(tempNode->u.adcl.dim);
			declList<<"["<<dim<<"]";
			tempNode = tempNode->u.adcl.type;
		}
		declList<<";\n";   
		List *tmp = initNode->u.initializer.exprs;
		int n = ListLength(tmp);
		if(n == 1) // �����ʼ������1����һֵʱ�����������г�Ա��ʼ��Ϊ��ֵ
		{
			Node *item = (Node *)FirstItem(tmp);
			if(item->typ == Const)
			{
				declInitList<<" = ";
				RecursiveAdclInit(tmp);
				declInitList<<";\n";
			}
		}
		else //��ʼ�ĸ���������ά��һ�£�����Բ�ȡ�����ĸ�ֵ��ʽ��val pp:Array[Int](1) = [1,2,3,4,5];
		{	
			declInitList<<" = ";
			RecursiveAdclInit(tmp);
			declInitList<<";\n";

		}
	}
}
void X86CodeGenerate::OutputConstant(Node *c, Bool with_name)
{
	int len = 0;
	const char *tmpString = c->u.Const.text;

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
		// declInitList<< c->u.Const.value.u;//splûָ�룬���Ժ���
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

void X86CodeGenerate::SPL2X86_Node(Node *node, int offset)
{
	if (node == NULL) return;

	if(node->parenthesized == TRUE) declInitList << "("; //�����ű�֤�߼���

#define CODE(name, node, union) SPL2X86_##name(node, union, offset)
	ASTSWITCH(node, CODE)
#undef CODE

	if(node->parenthesized == TRUE) declInitList << ")"; //�����ű�֤�߼���
}

void X86CodeGenerate::SPL2X86_List(List *list, int offset)
{
	ListMarker marker; 
	Node *item = NULL; 

	IterateList(&marker, list); 
	while (NextOnList(&marker, (GenericREF)&item))
	{ 
		string name = item->u.decl.name;
		if (strcmp(name.c_str(),"path")==0)
		{
			if(isInFileReader)
				infileName = item->u.decl.init->u.Const.text;
			if(isInFileWriter)
				outfileName = item->u.decl.init->u.Const.text;
			return;
		}
		SPL2X86_Node(item, offset);	
	}
}

/*******************************************************************

********************************************************************/
void X86CodeGenerate::SPL2X86_STRdcl(Node *node, strdclNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Const(Node *node, ConstNode *u, int offset)
{
	OutputConstant(node, TRUE);
}

void X86CodeGenerate::SPL2X86_Id(Node *node, idNode *u, int offset)
{
	declInitList << u->text;
}

void X86CodeGenerate::SPL2X86_Itco(Node *node, itcoNode *u, int offset)
{
	declInitList << u->text;
}

string X86CodeGenerate::GetOpType(OpType op)
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
	case ADDRESS:return " & ";
	default:
		FAIL("Unrecognized node type in CodeGeneration!"); break; 
	}
}
void X86CodeGenerate::SPL2X86_Binop(Node *node, binopNode *u, int offset)//�Էǽṹ�����͵�Stream���м�,lihe,2012-09-04
{
	//OutputTabs(offset);
	if (node->u.binop.left) 
		SPL2X86_Node(node->u.binop.left,offset); 
	string op = GetOpType(node->u.binop.op);
	declInitList << GetOpType(node->u.binop.op);
	if (node->u.binop.right) 
		SPL2X86_Node(node->u.binop.right, offset);
	
	//declInitList << ";\n";
}

void X86CodeGenerate::SPL2X86_Unary(Node *node, unaryNode *u, int offset)
{
	if (node->u.unary.op != POSTINC && node->u.unary.op != POSTDEC)
		declInitList << GetOpType(node->u.binop.op);
	if (node->u.unary.expr) 
		SPL2X86_Node(node->u.unary.expr, offset); 
	if(node->u.unary.op == POSTINC || node->u.unary.op == POSTDEC)
		declInitList << GetOpType(node->u.binop.op);
}
string X86CodeGenerate::GetPrimDataType(Node *from)//���Ͷ���
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
	return type;
}

//ȡ���ݳ�Ա�ĳ�ʼֵ
string X86CodeGenerate::GetDataInitVal(string type)
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
	}else//��������Ա�ǻ����������ʼ��Ϊnull����Ϊһ��δ֪��ĳ�ֵ�����磺�����ԱҲ�Ǹ�����
		s = "null";
	return s;
}
//ȡ�����ά��
string X86CodeGenerate::GetArrayDim(Node *from)
{
	string dim;
	if (from->typ == Const)//���ά���ڵ�����Ϊ����������a[10]
	{
		if(from->u.Const.text)dim = from->u.Const.text;
		else 
		{
			char *tmpdim = (char *)malloc(20);
			sprintf(tmpdim,"%d",from->u.Const.value.l);//20120322 zww���
			dim = tmpdim;
		}
	}
	else if (from->typ == Id)
	{
		dim = from->u.id.text;
	}
	else if(from->typ == ImplicitCast && from->u.implicitcast.value->typ ==Const)
	{
		if(from->u.implicitcast.value->u.Const.text)dim = from->u.implicitcast.value->u.Const.text;
		else 
		{
			char *tmpdim = (char *)malloc(20);
			sprintf(tmpdim,"%d",from->u.implicitcast.value->u.Const.value.l);
			dim = tmpdim;
		}
	}
	else if(from->typ == Binop)
	{
		string tmp = declInitList.str(); // ����
		stringstream tmp2;

		declInitList.str("");
		SPL2X86_Node(from, 0);
		tmp2 << "(" << declInitList.str() << ")";
		dim = tmp2.str();
		declInitList.str("");
		declInitList << tmp; // �ָ� 
	}
	else
		UNREACHABLE;

	return dim;
}

string X86CodeGenerate::GetArrayDataType(Node *node)
{
	string type;
	if (node->typ == Prim) //��������
	{
		type = GetPrimDataType(node);
	}
	else if (node->typ == Adcl) // Ҳ�Ǹ�������ݹ��������
	{
		stringstream ss;
		ss<<GetArrayDataType(node->u.adcl.type);
		type = ss.str();
	}
	else // �������ĳ�Ա�Ǹ������ͣ����д���չ
	{
		Warning(1,"this arrayDataType can not be handle!");
		type = "Any";// ��ʱ����һ��ͨ������
		UNREACHABLE;
	}
	return type;
}
void X86CodeGenerate::ExtractDeclVariables(Node *from)
{
	stringstream tempdeclList,tempdeclInitList;
	if (from->u.decl.type->typ == Prim) // ��������
	{
		Node *typeNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string type = GetPrimDataType(typeNode);
		string name = from->u.decl.name;
		char tempvalude[20];
		declList<<"\t"<<type<<" "<<name<<";\n";
		if (initNode) // ���ڳ�ʼ������г�ʼ��
		{
			declInitList << "\t\t"<<name<<" = ";
			SPL2X86_Node(initNode, 0);
			declInitList <<";\n";
		}

		if (isInParam)
		{
			parameterBuf<<type<<" "<<name<<", ";
			thisBuf << "\t\tthis." << name << " = " << name <<";\n";
		}
	}
	else if (from->u.decl.type->typ == Adcl) // ����, ��ദ���ά����, ��ά����չ
	{
		Node *tmpNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string name = from->u.decl.name;
		string arrayType = GetArrayDataType(tmpNode->u.adcl.type);
		string dim;
		if (initNode == NULL) //���û�г�ʼ�������������ͽ��г�ʼ��
		{
			string dim = GetArrayDim(tmpNode->u.adcl.dim);
			stringstream sdim;
			sdim<<dim;
			int dimNum;
			bool dynamicArray = false;
			
			if(sdim>>dimNum)		//Ϊ����
				declList<<"\t"<<arrayType<<" "<<name<<"["<<dim<<"]";
			else		//����ά����ȷ����ʹ��new��̬����
			{
				dynamicArray = true;
				declList<<"\t"<<arrayType<<"*"<<name;
				declInitList << "\t\t"<<name<<" = new "<<arrayType<<"["<<dim<<"]";
			}
			tmpNode = tmpNode->u.adcl.type;
			while(tmpNode->typ == Adcl)
			{
				dim = GetArrayDim(tmpNode->u.adcl.dim);
				declList<<"["<<dim<<"]";
				if (dynamicArray)
					declInitList<<"["<<dim<<"]";
				tmpNode = tmpNode->u.adcl.type;
			}
			declList<<";\n";
			if(dynamicArray)
				declInitList<<"();\n";
		}
		else//������ڳ�ʼ�����ʼ��Ϊָ��ֵ
		{
			declList<<"static "<<arrayType<<" "<<name;
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
			tempdeclList<<"::"<<name;
			while(tmpNode->typ == Adcl)
			{
				string dim = GetArrayDim(tmpNode->u.adcl.dim);
				declList<<"["<<dim<<"]";
				tempdeclList<<"["<<dim<<"]";
				tmpNode = tmpNode->u.adcl.type;
			}
			declList<<";\n";
			tempdeclList<<"=";
			List *tmp = initNode->u.initializer.exprs;
			int n = ListLength(tmp);
			if(n == 1) // �����ʼ������1����һֵʱ�����������г�Ա��ʼ��Ϊ��ֵ
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
			else //��ʼ�ĸ���������ά��һ�£�����Բ�ȡ�����ĸ�ֵ��ʽ��val pp:Array[Int](1) = [1,2,3,4,5];
			{	
				RecursiveAdclInit(tmp);
				tempdeclInitList<<tempdeclList.str()<<declInitList.str()<<";\n";
				declInitList.str("");
				

			}
			staticNameInit.push_back(tempdeclInitList.str());
		}

		if (isInParam)
		{
			//parameterBuf << name << " :Array[" << arrayType << "](1);\n";
			parameterBuf<<arrayType<<" "<<name<<"[];\n";
			thisBuf << "\t\tthis." << name << " = " << name <<";\n";
		}
	}
	else if (from->u.decl.type->typ == Ptr) // ָ�룬ֻ�ܳ�����param��
	{
		Node *typeNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string type;
		if(typeNode->u.ptr.type->typ==Prim)
			type = GetPrimDataType(typeNode->u.ptr.type);
		else if(typeNode->u.ptr.type->typ==Tdef)
			type = typeNode->u.ptr.type->u.tdef.name;
		else UNREACHABLE;			//�������͵��ݲ�֧��

		string name = from->u.decl.name;
		char tempvalude[20];
		declList<<"\t"<<type<<" "<<"*"<<name<<";\n";
		if (initNode) // ���ڳ�ʼ������г�ʼ��
		{
			declInitList << "\t\t"<<name<<" = ";
			SPL2X86_Node(initNode, 0);
			declInitList <<";\n";
		}

		if (isInParam)
		{
			parameterBuf<<type<<" "<<"*"<<name<<", ";
			thisBuf << "\t\tthis." << name << " = " << name <<";\n";
		}
	}
	else
		UNREACHABLE;
}

void X86CodeGenerate::SPL2X86_Cast(Node *node, castNode *u, int offset)
{
	if(node->u.cast.type->typ==Ptr&&node->u.cast.expr->typ==Const){//��Ӷ�NULL���ж�
		if(node->u.cast.expr->u.Const.value.i==0)
			declInitList<<"NULL";
	}
	else{
		declInitList<<"(";
		declInitList<<GetPrimDataType(node->u.cast.type)<<")";
		SPL2X86_Node(node->u.cast.expr,offset);
	}
}

void X86CodeGenerate::SPL2X86_Comma(Node *node, commaNode *u, int offset)
{
	isInComma = true;//�����ڶ��ű��ʽ��
	SPL2X86_List(u->exprs,offset);
	isInComma = false;
}

void X86CodeGenerate::SPL2X86_Ternary(Node *node, ternaryNode *u, int offset)
{
	SPL2X86_Node(u->cond,offset);
	declInitList<<" ? ";
	SPL2X86_Node(u->true_,offset);
	declInitList<<":";
	SPL2X86_Node(u->false_,offset);
}

void X86CodeGenerate::SPL2X86_Array(Node *node, arrayNode *u, int offset)
{
	SPL2X86_Node(u->name,offset);
	List *tmp = u->dims;
	while(tmp != NULL)//�����Ƕ�ά���飬��Ҫ����dim���list
	{
		declInitList<<"[";
		Node *item = (Node *)FirstItem(tmp);
		SPL2X86_Node(item,offset);
		declInitList<<"]";
		tmp = Rest(tmp);
	}
}

void X86CodeGenerate::SPL2X86_Call(Node *node, callNode *u, int offset)
{
	assert(u->name->typ == Id);
	{
		int flag=1;//��ʶ�Ƿ������
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
		else if (strcmp(ident,"println")==0) {declInitList<<"cout<<";flag=2;}
		else if (strcmp(ident,"printf")==0) {declInitList<<"cout<<";flag=3;}
		else if (strcmp(ident,"print")==0) {declInitList<<"cout<<";flag=4;}
		else //unkonwn methods
		{
			declInitList<<u->name->u.id.text;//���������
		}
		if (flag==1)
		{
			declInitList<<"(";
			OutputArgList(u->args,offset);//����
			declInitList<<")";
		}
		else if(flag==2)
		{
			OutputArgList(u->args,offset);//����
			declInitList<<"<<endl";
		}
		else
		{
			OutputArgList(u->args,offset);//����
		}
	}
}

void X86CodeGenerate::SPL2X86_Initializer(Node *node, initializerNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_ImplicitCast(Node *node, implicitcastNode *u, int offset)
{
	SPL2X86_Node(u->expr, offset);
}

void X86CodeGenerate::SPL2X86_Label(Node *node, labelNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Switch(Node *node, SwitchNode *u, int offset)
{
	declInitList<<"switch (";
	SPL2X86_Node(u->expr,offset);
	declInitList<<")";
	SPL2X86_Node(u->stmt,offset);
}

void X86CodeGenerate::SPL2X86_Case(Node *node, CaseNode *u, int offset)
{
	declInitList<<"case ";
	SPL2X86_Node(u->expr,offset);
	declInitList<<":";
	OutputCRSpaceAndTabs(offset+1);
	SPL2X86_Node(u->stmt,offset);
}

void X86CodeGenerate::SPL2X86_Default(Node *node, DefaultNode *u, int offset)
{
	declInitList<<"default: ";
	OutputCRSpaceAndTabs(offset);
	SPL2X86_Node(u->stmt,offset);
}

void X86CodeGenerate::SPL2X86_If(Node *node, IfNode *u, int offset)
{
	declInitList<<"if (";
	SPL2X86_Node(u->expr,offset);
	declInitList<<")";
	if (u->stmt->typ != Block) // ����Ƿ�block��㣬����Ҫ���ж���
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2X86_Node(u->stmt,offset+1);
	if(u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//����Ǳ��ʽ�������Ҫ��ĩλ��ӷֺű�ʾ����
		declInitList<<";";
}

void X86CodeGenerate::SPL2X86_IfElse(Node *node, IfElseNode *u, int offset)
{
	declInitList<<"if (";
	SPL2X86_Node(u->expr,offset);
	declInitList<<")";
	if (u->true_->typ != Block)//����Ƿ�block��㣬����Ҫ���ж���
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2X86_Node(u->true_,offset);
	if(u->true_->typ == Binop || u->true_->typ == Unary || u->true_->typ == Ternary || u->true_->typ == Call || u->true_->typ == Decl)//����Ǳ��ʽ�������Ҫ��ĩλ��ӷֺű�ʾ����
		declInitList<<";";
	OutputCRSpaceAndTabs(offset);
	declInitList<<"else ";
	if (u->false_->typ != Block && u->false_->typ != IfElse)//����Ƿ�block������ifelse��㣬����Ҫ���ж���
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2X86_Node(u->false_,offset);
	if(u->false_->typ == Binop || u->false_->typ == Unary || u->false_->typ == Ternary || u->false_->typ == Call || u->false_->typ == Decl)//����Ǳ��ʽ�������Ҫ��ĩλ��ӷֺű�ʾ����
		declInitList<<";";
}

void X86CodeGenerate::SPL2X86_While(Node *node, WhileNode *u, int offset)
{
	declInitList<<"while (";
	SPL2X86_Node(u->expr,offset);
	declInitList<<")";
	if (u->stmt->typ != Block)//����Ƿ�block��㣬����Ҫ���ж���
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2X86_Node(u->stmt,offset);
	if(u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//����Ǳ��ʽ�������Ҫ��ĩλ��ӷֺű�ʾ����
		declInitList<<";";
}

void X86CodeGenerate::SPL2X86_Do(Node *node, DoNode *u, int offset)
{
	declInitList<<"do";
	SPL2X86_Node(u->stmt,offset);
	declInitList<<"while (";
	SPL2X86_Node(u->expr,offset);
	declInitList<<");";
}

void X86CodeGenerate::SPL2X86_For(Node *node, ForNode *u, int offset)
{
	declInitList<<"for (";
	SPL2X86_Node(u->init,offset);
	declInitList<<";";
	SPL2X86_Node(u->cond,offset);
	declInitList<<";";
	SPL2X86_Node(u->next,offset);
	declInitList<<")";
	if (u->stmt->typ != Block)//����Ƿ�block��㣬����Ҫ���ж���
	{
		declInitList<<"\n";
	}
	OutputStmt(u->stmt,offset+1);
}

void X86CodeGenerate::SPL2X86_Goto(Node *node, GotoNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Continue(Node *node, ContinueNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Break(Node *node, BreakNode *u, int offset)
{
	declInitList<<"break;";
}

void X86CodeGenerate::SPL2X86_Return(Node *node, ReturnNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Block(Node *node, BlockNode *u, int offset)
{
	declInitList <<"{\n";
	//���decl
	SPL2X86_List(u->decl,  offset);
	declInitList<<"\n"; // ����һ��

	OutputStmtList(u->stmts, offset);

	OutputCRSpaceAndTabs(offset);
	declInitList<<"}\n"; // '}'��ռһ��
}

void X86CodeGenerate::SPL2X86_Prim(Node *node, primNode *u, int offset)
{
	if(isExternType)
		ExternTypeBuf<<GetPrimDataType(node)<<" ";
	else{
		declInitList << GetPrimDataType(node);
		globalvarbuf<<"extern "<<GetPrimDataType(node)<<" ";
	}
}

void X86CodeGenerate::SPL2X86_Tdef(Node *node, tdefNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Ptr(Node *node, ptrNode *u, int offset)
{
	if(u->type->typ == Tdef){
		declInitList<< u->type->u.tdef.name  <<"* ";
		globalvarbuf<<  u->type->u.tdef.name<<"* ";
	}else if(u->type->typ == Ptr){   
		declInitList << u->type->u.ptr.type->u.tdef.name << "** ";
		globalvarbuf << u->type->u.ptr.type->u.tdef.name << "** "; 
	}else{
		if(isExternType)
			ExternTypeBuf<< GetArrayDataType(node->u.adcl.type)<<"* ";
		else{
			declInitList<< GetArrayDataType(node->u.adcl.type)<<"* ";
			globalvarbuf<< GetArrayDataType(node->u.adcl.type)<<"* ";
		}
	}
}

void X86CodeGenerate::SPL2X86_Adcl(Node *node, adclNode *u, int offset)
{
	string arrayType = GetArrayDataType(node->u.adcl.type);
	if(isExternType)
		ExternTypeBuf<<arrayType<<" ";
	else{
		declInitList<<arrayType<<" ";
		globalvarbuf<<"extern "<<arrayType<<" ";
	}
}
void X86CodeGenerate::SPL2X86_Fdcl(Node *node, fdclNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Sdcl(Node *node, sdclNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Udcl(Node *node, udclNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Edcl(Node *node, edclNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Decl(Node *node, declNode *u, int offset)
{
	
	if (extractDecl)
	{
		ExtractDeclVariables(node);
		return;
	}
	else
	{
		if(strcmp(node->u.decl.name, "SPLExternType") ==0 && node->u.decl.type->typ== Sdcl)
		{
			isExternType = TRUE;
			needExternType = TRUE;
			ExternTypeBuf<<"typedef struct \n";
			ExternTypeBuf<<"{ \n";
			SPL2X86_Node(u->type, offset);
			ExternTypeBuf<<"}"<<node->u.decl.name<<";\n";
			isExternType = FALSE;
		}else{
			OutputCRSpaceAndTabs(offset);
			if(isCGGlobalVar && STORAGE_CLASS(node->u.decl.tq) == T_EXTERN)
			{
				declInitList<<"extern ";
				globalvarbuf<<"extern ";
			}
			SPL2X86_Node(u->type, offset);
			if(u->type->typ == Adcl)	//��ά����
			{
				Node *tempNode = u->type;
				globalvarbuf<<node->u.decl.name;
				if (flag_Global)			//�����GlobalVar�У�Ϊ�����������ȫ�ֱ���
					declInitList<<node->u.decl.name;
				else
					declInitList_temp<<node->u.decl.name;
				while(tempNode->typ == Adcl)
				{
					if(STORAGE_CLASS(node->u.decl.tq) == T_EXTERN){
						if (flag_Global)			
							declInitList<<"["<<"];";
						else
							declInitList_temp<<"["<<"];";
						globalvarbuf<<"["<<"]";
					}else{
						string dim = GetArrayDim(tempNode->u.adcl.dim);
						if (flag_Global)			
							declInitList<<"["<<dim<<"]";
						else
							declInitList_temp<<"["<<dim<<"]";
						globalvarbuf<<"["<<dim<<"]";
					}
					tempNode = tempNode->u.adcl.type;
				}
				temp_declInitList<<declInitList.str()<<";\n";
				globalvarbuf<<";\n";
			}
			else		//����
				{
					if(isExternType)
						ExternTypeBuf<<node->u.decl.name<<";\n";//��������
					else{
						declInitList<<" "<<node->u.decl.name;   //������
						temp_declInitList<<declInitList.str()<<";\n";
						globalvarbuf<<node->u.decl.name<<";\n";//��������
					}
			}

			if(u->type->typ == Adcl)
			{
				if(STORAGE_CLASS(node->u.decl.tq) != T_EXTERN)
					AdclInit(node,offset);		//��ʼ������
			}
			else
			{
				if (node->u.decl.init) 
				{
					if (u->type->typ == Prim && u->type->u.prim.basic == Char)//����Ǹ��ַ�����
					{
						declInitList<<" = "<<u->init->u.implicitcast.expr->u.Const.text;
					}
					else
					{
						declInitList<<" = ";			//��ʼ������ ��int a = 2;
						SPL2X86_Node(u->init,offset );
					}
				}
			}
			 if (u->type->typ != Adcl)		
				declInitList<<";";
		}
	}
}

void X86CodeGenerate::SPL2X86_Attrib(Node *node, attribNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Proc(Node *node, procNode *u, int offset)
{
}

void X86CodeGenerate::SPL2X86_Text(Node *node, textNode *u, int offset)
{
}
