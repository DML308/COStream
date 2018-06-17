#include "X10CodeGenerate.h"

#ifndef WIN32
#include <sys/stat.h> 
#define mkdir(tmp) mkdir(tmp,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

using namespace std;

#define  Default_Repeat_Count 10
static bool isInComma = false;//表示当前处于逗号表达式中

X10CodeGenerate::X10CodeGenerate(SchedulerSSG *Sssg, int nCpucore, int buffer_size, const char *currentDir)
{
	sssg_ = Sssg;
	flatNodes_ = Sssg->GetFlatNodes();
	comName_ = "Main";
	nCpucore_ = nCpucore;
	nRepeatCount_ = Default_Repeat_Count;
	nActors_ = flatNodes_.size();
	// 取上限则可能出现空闲的place
	nPerThreads_ = (int)ceil((float)nActors_ / nCpucore_);
	dir_ = currentDir;
	buffer_size_ = buffer_size;
	currentNum_ = 0;


	strScheduler <<"\t\tfor(var i : Int = 0; i < initCount; ++i) // initSchedule\n";//初态调度
	strScheduler <<"\t\t\twork();\n";

	strScheduler <<"\t\tfor(var i : Int = 0; i < RepeatCount; ++i)\n";//稳态调度
	strScheduler <<"\t\t\tfor(var j : Int = 0; j < steadyCount; ++j) // steadySchedule\n";
	strScheduler <<"\t\t\t\twork();\n";
	
	strScheduler <<"\t\tflush();\n";


	int index = 0; 
	int i = 0; 
	while(1) 
	{ 
		string::size_type pos = dir_.find("\\", index); 
		string str1; 
		str1 = dir_.substr(0, pos); 

		if(pos == -1) break; 
		//else if(i > 0) _mkdir(str1.c_str()); 
#ifdef WIN32
		else if(i > 0) _mkdir(str1.c_str()); 
#else
		else if(i > 0) mkdir(str1.c_str()); 
#endif
		i++; 
		index = pos + 1; 
	} 

	string tmp = dir_ + "lib\\";
	//_mkdir(tmp.c_str());
#ifdef WIN32	
	_mkdir(tmp.c_str());
#else
	mkdir(tmp.c_str());
#endif 


	extractDecl = false;
	isInParam = false;
}

// 输出到文件
void X10CodeGenerate::OutputToFile(string fileName, string contents)
{
	ofstream fw;
	try{
		fw.open(fileName.c_str());
		fw<<contents;
		fw.close();
	}catch(...){
		cout<<"error:output to file"<<endl;
	}
}

// 对各节点进行简单的调度
void X10CodeGenerate::SimpleScheduler()
{
	for (int i = 0; i < nActors_; ++i) 
	{
		FlatNode *actor = flatNodes_[i];
		int place = i/nPerThreads_;
		mapFlatNode2Place.insert(make_pair(actor, place));
	}
}

void X10CodeGenerate::CGmain()
{
	stringstream buf;

	buf <<"import lib.*;\n"; // 相关库文件
	buf <<"public class "<<comName_<<" implements Interface "<<"{\n"; // 类块开始
	buf <<"\tstatic val DefaultRC:Int = "<<nRepeatCount_<<";// DefaultRepeatCount\n";//默认循环执行次数
	buf <<"\tvar rc:Int;\n";

	// 写入调度数据信息
	CGmainScheduleData(buf);
	
	// 对节点进行调度
	CGmainRun();
	buf << edgeBuf.str();

	buf <<"\t//methods\n";
	// 写入构造函数
	buf <<"\tpublic def this(i:Int) {\n\t\trc=i;\n\t}\n";
	// 写入run函数
	buf << runBuf.str();

	
	//main方法
	buf <<"\t// static main method\n";
	buf <<"\tpublic static def main(args: Array[String]) {\n";
	buf <<"\t\tif(Place.MAX_PLACES<"<<nCpucore_<<") Console.OUT.println(\"warning:at least need "<<nCpucore_<<" places!\");\n";
	buf <<"\t\telse {\n";
	buf <<"\t\t\tvar rc:Int;\n\t\t\tif(args.size==2 && args(0).compareTo(\"-i\") == 0) rc = Int.parse(args(1));\n"; 
	buf <<"\t\t\telse rc = DefaultRC;\n";
	buf <<"\t\t\tval stream = new "<<comName_<<"(rc);\n";
	buf <<"\t\t\tstream.run();\n";
	buf <<"\t\t}\n";
	buf <<"\t}\n";
	buf <<"}\n";//类块结束

	//输出到文件
	stringstream fileName;
	fileName<<dir_<<comName_<<".x10";
	OutputToFile(fileName.str(), buf.str());
}

void X10CodeGenerate::CGmainScheduleData(stringstream &buf)
{
	map<FlatNode *,int>::iterator pos;
	buf <<"\n\t// ScheduleData\n";
	buf <<"\tstatic val ";
	for (int i=0;i<nActors_;i++) // 初态调度次数
	{
		pos = sssg_->mapInitCount2FlatNode.find(flatNodes_[i]);
		if(i == nActors_-1) {
			buf <<flatNodes_[i]->name<<"_init = "<<pos->second<<";\n";
			break;
		}
		buf <<flatNodes_[i]->name<<"_init = "<<pos->second<<", ";
	}

	buf <<"\tstatic val ";
	for (int i=0;i<nActors_;i++) // 稳态调度次数
	{
		pos = sssg_->mapSteadyCount2FlatNode.find(flatNodes_[i]);
		if(i == nActors_-1) {
			buf <<flatNodes_[i]->name<<"_steady = "<<pos->second<<";\n";
			break;
		}
		buf <<flatNodes_[i]->name<<"_steady = "<<pos->second<<", ";
	}

}

void X10CodeGenerate::CGmainRun()
{
	runBuf <<"\t// run\n";
	runBuf << "\tpublic def run() {\n"; //run方法,类中最主要的方法
	runBuf << "\t\tfinish for (p in Place.places()) async at (p) {\n";
	runBuf << "\t\t\tswitch(p.id){\n";


	std::map<operatorNode *, string>::iterator pos;

	for (int i = 0; i < nActors_; ++i) //生成 各个 类实例
	{
		List *inputs = flatNodes_[i]->oldContents->decl->u.decl.type->u.operdcl.inputs;
		List *outputs =	flatNodes_[i]->oldContents->decl->u.decl.type->u.operdcl.outputs;
		int len = ListLength(outputs);
		int nOut = flatNodes_[i]->nOut;
		OperatorType ot = flatNodes_[i]->oldContents->ot;
		int place = 0;

		if (len == nOut)
		{
			CGmainActor(flatNodes_[i], ot, place);
			if (i % nPerThreads_ == 0)
			{
				runBuf << "\t\t\tcase " << i/nPerThreads_ << ":\n";
			}
			runBuf << MakeTabs(4) << "async {\n";
			runBuf << classInstances[i] ;
			runBuf << MakeTabs(4) << "}\n";
			if ((i+1) % nPerThreads_ == 0 || i == nActors_-1)
			{
				runBuf << MakeTabs(4) << "break;\n";
			}
		}
		else // 一条边被多个actor共用, 暂不处理
		{
			cout << "test" << endl;
			UNREACHABLE;
		}
		
	}
	runBuf << "\t\t\tdefault: break;\n";
	runBuf << "\t\t\t}\n";
	runBuf << "\t\t}\n";
	runBuf << "\t}\n"; // run方法结束
}

void X10CodeGenerate::CGmainActor(FlatNode *actor, OperatorType ot, int palce)
{
	static int count = 0;
	stringstream actorName;
	actorName << "actor_" << actor->name;
	int nPush, nPeek, nPop, number;
	List *inputs = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputs =	actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker marker; 
	Node *item = NULL; 
	
	std::map<FlatNode *, int>::iterator it;
	it = mapFlatNode2Place.find(actor);
	palce = it->second;

	stringstream recv, send;
	for(int i = 0; i < actor->nIn; ++i)
	{
		stringstream recvEdgeName;
		int num = 0;
		recv << "1, "; // 暂时处理每条边不共享的情形
		recv << actor->inPeekWeights[i] << ", " << actor->inPopWeights[i] << ", ";

		FlatNode *up = actor->inFlatNodes[i];
		for (num = 0; up->outFlatNodes[num] != actor; ++num);
		recvEdgeName << "edge_" << up->name << "_" << num;

		recv << recvEdgeName.str() << ", ";
	}

	for(int i = 0; i < actor->nOut; ++i)
	{
		stringstream sendEdgeName;
		send << actor->outPushWeights[i] << ", "; 
		sendEdgeName << "edge_" << actor->name << "_" << i;
		send << sendEdgeName.str() << ", ";
		stringstream tmpBuf, buf0, buf1, bufState0, bufState1;
		tmpBuf << actor->name << "_" << i;
		buf0 << "Buf_" << tmpBuf.str() << "_Dist_0";
		buf1 << "Buf_" << tmpBuf.str() << "_Dist_1";
		bufState0 << "Buf_" << tmpBuf.str() << "_State_0";
		bufState1 << "Buf_" << tmpBuf.str() << "_State_1";

		edgeBuf << "\tval " << buf0.str() << " = DistArray.make[Token_Temp](Dist.makeConstant(0..BUFFER_SIZE,Place.place(" << palce <<")),([i]:Point(1)) => null);\n";
		edgeBuf << "\tval " << buf1.str() << " = DistArray.make[Token_Temp](Dist.makeConstant(0..BUFFER_SIZE,Place.place(" << palce <<")),([i]:Point(1)) => null);\n";
		edgeBuf << "\tval " << bufState0.str() << " = DistArray.make[Int](Dist.makeConstant(0..1,Place.place(" << palce <<")),([i]:Point(1)) => 0);\n";
		edgeBuf << "\tval " << bufState1.str() << " = DistArray.make[Int](Dist.makeConstant(0..1,Place.place(" << palce <<")),([i]:Point(1)) => 0);\n";
		edgeBuf << "\tval " << sendEdgeName.str() << " = new Edge[Token_Temp](" << buf0.str() << ", " << buf1.str() << ", " << bufState0.str() << ", " << bufState1.str() << ");\n\n";
	}

	std::map<operatorNode *, string>::iterator pos;
	pos = mapOperator2ClassName.find(actor->oldContents);
	stringstream tmpNew, tmpRun, compositeArg, compositeParam; // compositeArg储存值
	
	declInitList.str("");
	Node *arg = actor->composite->body->u.comBody.param;
	if (arg)
	{
		SPL2X10_List(arg->u.param.parameters, 5);
		IterateList(&marker, arg->u.param.parameters); 
		while (NextOnList(&marker, (GenericREF)&item))
		{ 
			assert(item->typ == Decl);
			compositeArg << item->u.decl.name << ", ";
		}
	}
	compositeArg << "rc);\n";
	compositeParam << declInitList.str();
	tmpNew << "\n" << MakeTabs(5) << "val " << actorName.str() << " = new " << pos->second << "(" << recv.str() << send.str() << compositeArg.str();
	tmpRun << MakeTabs(5) << actorName.str() << ".run(" << actor->name << "_init, " << actor->name << "_steady, 1);\n";
	stringstream instanceBuf;
	instanceBuf << compositeParam.str() << tmpNew.str() << tmpRun.str();
	classInstances.push_back(instanceBuf.str());

	//string tmp = declInitList.str();
	//printf("%s---%s---%s\n", actor->name.c_str(), tmp.c_str(), compositeArg.str().c_str());
	++count;
}

string X10CodeGenerate::MakeTabs(int tabs)
{
	stringstream tmp;
	while (tabs--) tmp<<"\t";
	return tmp.str();
}

// ---------------------------------------------------------------------------
// 生成 stream 的数据成员
void X10CodeGenerate::CGclassMembers(Node *strdcl, stringstream &buf)
{
	List *classMembers = NULL;
	ListMarker marker;
	Node *classMember = NULL;
	stringstream buff,bufff;

	assert(strdcl && (strdcl->typ == Id || strdcl->typ == Decl));
	if (strdcl->typ == Id)
		strdcl = strdcl->u.id.decl;
	
	classMembers = strdcl->u.decl.type->u.strdcl.type->fields;//取出stream内的成员，设置为class的成员
	IterateList(&marker, classMembers );

	while ( NextOnList(&marker, (GenericREF) &classMember) )
	{
		string type, initValue;

		//如果stream内的成员全是prim类型则按如下处理，如果是复合类型待处理
		if(classMember->u.decl.type->typ == Prim)
		{
			type = GetPrimDataType(classMember->u.decl.type);
			initValue = GetDataInitVal(type);
		}
		buf << "\t\tpublic var " << classMember->u.decl.name << " : " << type << ";\n";
		bufff << "\t\t\t" << classMember->u.decl.name << " = " << initValue << ";\n";
	}
	buf << "\t\tpublic def this() {\n";
	buf << bufff.str();
	buf << "\t\t}\n";
	buf << "\t}\n\n";
}


// 生成接口：interface.x10
void X10CodeGenerate::CGinterface()
{
	stringstream buf;
	

	buf << "/**\n * Interface\n */\n";
	buf << "public interface Interface" << "\n{\n";
	// BUFFER_SIZE
	buf << "\tstatic val BUFFER_SIZE:Int = " << buffer_size_ << ";\n";

	// 对全局变量进行代码生成
	SPL2X10_List(gDeclList, 1);
	buf << declInitList.str() << "\n\n";
	declInitList.str("");

	List *outputs = sssg_->GetTopLevel()->oldContents->decl->u.decl.type->u.operdcl.outputs;
	assert(outputs && ListLength(outputs) == 1);
	string streamName = "Token_";
	char *tmp = new char[100];
	
	sprintf(tmp, "Temp");
	streamName += tmp;
	buf << "\tpublic static class " << streamName << " {\n";
	CGclassMembers((Node *)FirstItem(outputs), buf);

#if 0
	std::map<operatorNode *, compositeNode *>::iterator pos;
	// 考虑到stream的写法得重新设计，决定暂时关闭，12.26
	for (int i = 0; i < nActors_; ++i)
	{
		List *outputs = flatNodes_[i]->oldContents->decl->u.decl.type->u.operdcl.outputs;
		int len = ListLength(outputs);
		int nOut = flatNodes_[i]->nOut;
		ListMarker marker;
		Node *item = NULL;
		OperatorType ot = flatNodes_[i]->oldContents->ot;

		pos = mapOperator2ClassName.find(flatNodes_[i]->oldContents);
		if (pos == mapOperator2ClassName.end()) // 新的类模板进入了
		{
			mapOperator2ClassName.insert(make_pair(flatNodes_[i]->oldContents, flatNodes_[i]->composite));

			if (len == nOut)
			{
				IterateList(&marker, outputs);
				while (NextOnList(&marker, GenericREF(&item)))
				{
					string streamName = "Token_";
					char *tmp = new char[100];
					if (ot == Duplicate_)
					{
						sprintf(tmp, "Duplicate_%d", currentNum_++);
						streamName += tmp;
						buf << "\tpublic static class " << streamName << " {\n";
						CGclassMembers(item, buf);
						break;
					}
					else
					{
						sprintf(tmp, "%s_%d", item->u.decl.name, currentNum_++);
						streamName += tmp;
						buf << "\tpublic static class " << streamName << " {\n";
						CGclassMembers(item, buf);
					}

				}

			}
			else // 一条边被多个actor共用, 暂不处理
			{
				cout << "test" << endl;
			}
		}
	}
#endif

	buf << "}\n ";

	//输出到文件
	stringstream ss;
	ss<<dir_<<"Interface.x10";
	OutputToFile(ss.str(),  buf.str());
}

void X10CodeGenerate::CGactors()
{
	std::map<operatorNode *, string>::iterator pos;
	
	for (int i = 0; i < nActors_; ++i)//生成 各个 类模板
	{
		int len = ListLength(flatNodes_[i]->oldContents->decl->u.decl.type->u.operdcl.outputs);
		int nOut = flatNodes_[i]->nOut;
		OperatorType ot = flatNodes_[i]->oldContents->ot;

		pos = mapOperator2ClassName.find(flatNodes_[i]->oldContents);
		if (pos == mapOperator2ClassName.end()) // 新的类模板进入了
		{
			mapOperator2ClassName.insert(make_pair(flatNodes_[i]->oldContents, flatNodes_[i]->name));

			if (len == nOut)
			{
				CGactor(flatNodes_[i], ot);
			}
			else // 一条边被多个actor共用, 暂不处理
			{
				cout << "test" << endl;
				UNREACHABLE;
			}
		}
	}
	
}

//生成actor代码
void X10CodeGenerate::CGactor(FlatNode *actor, OperatorType ot)
{
	stringstream buf, SRbuf, pFlush, pushingBuf, popingBuf;
	stringstream initPushBuf, initPeekBuf, initWorkBuf;
	stringstream logicInitBuf, workBuf ;
	string classNmae = actor->name, streamName = "Token_Temp";

	assert(actor);
	actor->SetIOStreams();	
	buf <<"/**\n * Class "<<classNmae<<"\n */\n";
	buf <<"import lib.*;\n"; // 相关库文件
	buf <<"public class "<<classNmae<<" implements Interface "<<"{\n"; // 类块开始
	// 写入发送、接收信息
	parameterBuf.str(""); // 清空parameterBuf内容
	thisBuf.str(""); // 清空thisBuf内容
	// 接收边相关
	CGrecv(actor, ot, streamName, SRbuf, initPeekBuf, popingBuf, initWorkBuf);
	// 发送边相关
	CGsend(actor, ot, streamName, SRbuf, initPushBuf, pushingBuf, pFlush, initWorkBuf);
	buf << SRbuf.str();
	// 写入循环次数信息
	buf << "\tval RepeatCount:Int;\n";
	// 写入语法树成员变量信息（param, var, state）
	CGdeclList(actor, ot, buf);
	// 写入 var, state 的初始化信息
	CGinitVarAndState(actor, ot, buf);
	// 写入 init 的初始化信息(考虑到init可以定义局部变量，所以不与上部分合并)
	CGlogicInit(actor, ot, buf, logicInitBuf);
	// 写入构造函数信息
	CGthis(actor, ot, buf);
	// 写入成员函数信息
	buf <<"\t// ------------Member Mehods------------\n";
	// 写入flush函数
	CGflush(buf, pFlush.str());
	// 写入initWork函数
	CGinitWork(buf, initWorkBuf.str());
	// 写入initPeek函数
	CGinitPeek(buf, initPeekBuf.str());
	// 写入popToken函数
	CGpopToken(buf, popingBuf.str());
	// 写入initPush函数
	CGinitPush(buf, initPushBuf.str());
	// 写入pushToken函数
	CGpushToken(buf, pushingBuf.str());
	// 写入work函数
	CGwork(actor, ot, buf);
	// 写入run函数
	CGrun(buf, "initWork(nOut)");


	buf <<"}";//类块结束
	//输出到文件
	stringstream fileName;
	fileName<<dir_<<classNmae<<".x10";
	OutputToFile(fileName.str(),buf.str());
}

void X10CodeGenerate::CGrecv(FlatNode *actor, OperatorType ot, string streamName, stringstream &SRbuf
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
	
		initWorkBuf <<"\t\t"<<"if("<<peekNumber<<">0) "<<consumer0.str()<<".init();\n";
		initPeekBuf <<"\t\tfor(var i:Int=0;i<"<<peekNumber<<";i++) "<<peek0.str()<<"(i) = "<<consumer0.str()<<".peek(i);\n";
		popingBuf <<"\t\tfor(var i:Int=0;i<"<<popNumber<<";i++) "<<consumer0.str()<<".pop();\n";
	}
}

void X10CodeGenerate::CGsend(FlatNode *actor, OperatorType ot, string streamName, stringstream &SRbuf
	, stringstream &initPushBuf, stringstream &pushingBuf, stringstream &pFlush, stringstream &initWorkBuf)
{
	SRbuf <<"\t// sendBuffer\n";
	for (int i = 0; i < actor->nOut; ++i)
	{
		//if (actor->outPushWeights[i] == 0) continue; // 对 push(0) 操作不分配 sendBuffer
		
		string pushNumber = actor->outPushString[i];

		stringstream producer0, push0;
		producer0 <<"producer_"<<i;
		push0 << actor->pushString[i];

		// 发送边
		parameterBuf << pushNumber << " :Int, " << "edge_send_" << i << " :Edge[" << streamName << "], "; 
		thisBuf << "\t\tthis." << pushNumber << " = " << pushNumber <<";\n";

		SRbuf <<"\tval "<<producer0.str()<<" :Producer["<<streamName<<"];\n";
		SRbuf <<"\tval "<<pushNumber<<" :Int;\n";
		SRbuf <<"\tval "<<push0.str()<<" :Array["<<streamName<<"];\n\n";
		// 在构造函数里对发送边进行初始化（申请空间等）
		thisBuf<<"\t\t"<<producer0.str()<<" = new Producer["<<streamName<<"]("<<"BUFFER_SIZE, "<<"edge_send_"<<i<<");\n";
		thisBuf <<"\t\t"<<push0.str()<<" = new Array["<<streamName<<"](0..("<<pushNumber<<"-1),([i]:Point(1)) => null);\n\n";

		initWorkBuf <<"\t\t"<<producer0.str()<<".init(nOut);\n";
		initPushBuf <<"\t\tfor(var i:Int=0;i<"<<pushNumber<<";i++) "<<push0.str()<<"(i) = new "<<streamName<<"();\n";
		pushingBuf <<"\t\tfor(var i:Int=0;i<"<<pushNumber<<";i++) "<<producer0.str()<<".push("<<push0.str()<<"(i));\n";
		pFlush<<"\t\t"<<producer0.str()<<".flush();\n";
	}
}

void X10CodeGenerate::CGthis(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	buf <<"\t// Constructor\n";
	buf << "\tpublic def this (" << parameterBuf.str() << "rc :Int) {\n";
	
	buf << thisBuf.str();
	buf << "\t\tRepeatCount = rc;\n ";
	buf << "\t}\n"; // init方法结束
}

void X10CodeGenerate::CGflush(stringstream &buf, string pFlush)
{
	buf << "\t// flush\n";
	buf << "\tdef flush() {\n";

	buf << pFlush;

	buf << "\t}\n"; // flush方法结束
}

void X10CodeGenerate::CGinitWork(stringstream &buf, string initWorkBuf)
{
	buf << "\t// initWork\n";
	buf << "\tdef initWork(nOut : Int) {\n";
	buf << "\t\tinitVarAndState();\n";
	buf << "\t\tinit();\n";

	buf << initWorkBuf;

	buf << "\t}\n"; // initWork方法结束
}

void X10CodeGenerate::CGinitPeek(stringstream &buf, string initPeekBuf)
{
	buf <<"\t// initPeek\n";
	buf << "\tdef initPeek() {\n";

	buf << initPeekBuf;

	buf << "\t}\n"; // initPeek方法结束
}

void X10CodeGenerate::CGinitPush(stringstream &buf, string initPushBuf)
{
	buf <<"\t// initPush\n";
	buf << "\tdef initPush() {\n";

	buf << initPushBuf;

	buf << "\t}\n"; // initPush方法结束
}

void X10CodeGenerate::CGpopToken(stringstream &buf, string popingBuf)
{
	buf << "\t// popToken\n";
	buf << "\tdef popToken() {\n";

	buf << popingBuf;

	buf << "\t}\n"; // popToken方法结束
}

void X10CodeGenerate::CGpushToken(stringstream &buf, string pushingBuf)
{
	buf << "\t// pushToken\n";
	buf << "\tdef pushToken() {\n";

	buf << pushingBuf;

	buf << "\t}\n"; // pushToken方法结束
}

void X10CodeGenerate::CGwork(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	Node *work = actor->oldContents->body->u.operBody.work;
	stringstream tmpBuf;
	buf << "\t// work\n";
	buf << "\tdef work() {\n";
	buf << "\t\tinitPush();\n";
	buf << "\t\tinitPeek();\n\n\t\t";
	SPL2X10_Node(work, 2);
	buf << declInitList.str();
	buf << "\n\t\tpushToken();\n";
	buf << "\t\tpopToken();\n";
	buf << "\t}\n"; // work方法结束

	declInitList.str(""); // 清空
}

void X10CodeGenerate::CGrun(stringstream &buf, string initFun)
{
	buf <<"\t// run\n";
	buf << "\tpublic def run(initCount : Int, steadyCount : Int, nOut : Int) {\n";//run方法,类中最主要的方法

	buf << "\t\t" << initFun << ";\n"; // 调用init方法

	buf << strScheduler.str();

	buf << "\t}\n"; // run方法结束
}

// 写入语法树成员变量信息（param, var, state）
void X10CodeGenerate::CGdeclList(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	List *state = NULL;
	Node *var = NULL, *param = NULL;

	assert(actor);
	buf <<"\t// AST Variables\n";
	state = actor->oldContents->body->u.operBody.state;
	param = actor->oldComposite->body->u.comBody.param;

	extractDecl = true;
	declList.str(""); // 清空declList内容
	if(param)
	{
		isInParam = true;
		SPL2X10_List(param->u.param.parameters, 0);
		isInParam = false;
	}
	buf << "\t/* *****composite param***** */\n" << declList.str();

	declList.str(""); // 清空declList内容
	
	buf << "\t/* *****composite var***** */\n" << declList.str();

	stringstream tmp;
	tmp << declInitList.str();
	declInitList.str(""); // 清空declInitList内容
	declList.str(""); // 清空declList内容
	SPL2X10_List(state, 0);
	buf << "\t/* *****logic state***** */\n" << declList.str();
	extractDecl = false;
	
	stateInit << declInitList.str();
	declInitList.str(""); // 清空declInitList内容
	declInitList << tmp.str();
}

void X10CodeGenerate::CGinitVarAndState(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	buf << "\t// initVarAndState\n";
	buf << "\tdef initVarAndState() {\n";
	
	declInitList << "\n";

	buf << declInitList.str();
	buf << "\n\t\t/**** State Init ****/\n";
	buf << stateInit.str();
	buf << "\n\t}\n"; // initVarAndState方法结束

	declInitList.str(""); // 清空
	stateInit.str(""); // 清空
}

void X10CodeGenerate::CGlogicInit(FlatNode *actor, OperatorType ot, stringstream &buf, stringstream &logicInit)
{
	declInitList.str(""); // 清空
	buf << "\t// init\n";
	buf << "\tdef init()";
	Node *init = actor->oldContents->body->u.operBody.init;
	
	if (init)
	{
		SPL2X10_Node(init, 2);
		buf << declInitList.str(); // init结构必须带"{}"
	}
	else
	{
		buf << " {\n";
		buf << "\t}\n"; // '}'独占一行
	}
	

	declInitList.str(""); // 清空
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
//提取类成员变量信息
void X10CodeGenerate::ExtractDeclVariables(Node *from)
{
	if (from->u.decl.type->typ == Prim) // 基本类型
	{
		Node *typeNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string type = GetPrimDataType(typeNode);
		string name = from->u.decl.name;
		declList << "\tvar "<< name << " :"<<type<<";\n"; // 建立变量声明

		if (initNode) // 存在初始化则进行初始化
		{
			declInitList << "\t\t"<<name<<" = ";
			SPL2X10_Node(initNode, 0);
			declInitList <<";\n";
		}

		if (isInParam)
		{
			parameterBuf << name << " :"<<type<<", ";
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
		declList << "\tvar "<< name << " :Array[" << arrayType << "](1);\n";

		if (initNode == NULL) //如果没有初始化，则按数组类型进行初始化
		{
			declInitList << "\t\t"<<name<<" = new Array["<<arrayType<<"](0..("<<dim<<"-1), ";
			GetArrayDataInitVal(tmpNode, declInitList); //数组初始化
			declInitList << ");\n";//一个数组声明加初始化
		}
		else//如果存在初始化则初始化为指定值
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
					declInitList <<name<<" :Array["<<arrayType<<"](1) = new Array["<<arrayType<<"](0..("<<dim<<"-1), "<<ss.str()<<");\n";
				}
			}
			else //初始的个数和数组维数一致，则可以采取这样的赋值形式：val pp:Array[Int](1) = [1,2,3,4,5];
			{	
				declInitList<<"\t\t"<<name<<" = ";
				RecursiveAdclInit(tmp);
				declInitList<<";\n";

			}
		}

		if (isInParam)
		{
			parameterBuf << name << " :Array[" << arrayType <<", ";
			thisBuf << "\t\tthis." << name << " = " << name <<";\n";
		}
	}
	else if (from->u.decl.type->typ == Ptr) // 指针，只能出现在param中
	{
		assert(isInParam);
		Node *arrayNode = from->u.decl.type;
		Node *tmpNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string name = from->u.decl.name;
		string ptrType = GetPtrDataType(tmpNode->u.ptr.type);
		
		declList << "\tvar "<< name << " :Array[" << ptrType << "](1);\n";
		parameterBuf << name << " :Array[" << ptrType << "](1)" <<", ";
		thisBuf << "\t\tthis." << name << " = " << name <<";\n";
	}
	else
		UNREACHABLE;
}

//取得操作类型
string X10CodeGenerate::GetOpType(OpType op)
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

//取基本数据成员的类型
string X10CodeGenerate::GetPrimDataType(Node *from)
{
	string type;

	switch(from->u.prim.basic){
		 case Sshort:
		 case Sint:
			 type = "Int";;
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
			 type = "Float";
			 break;
		 case Double:
			 type = "Double";
			 break;
		 case Char:
		 case Schar:
		 case Uchar:
			type = "Char";
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

//取数组成员的类型
string X10CodeGenerate::GetArrayDataType(Node *node)
{
	string type;
	if (node->typ == Prim) //基本类型
	{
		type = GetPrimDataType(node);
	}
	else if (node->typ == Adcl) // 也是个数组则递归查找类型
	{
		stringstream ss;
		ss<<"Array["<<GetArrayDataType(node->u.adcl.type)<<"]";
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

//取指针指向的类型
string X10CodeGenerate::GetPtrDataType(Node *node)
{
	string type;
	if (node->typ == Prim) //基本类型
	{
		type = GetPrimDataType(node);
	}
	else if (node->typ == Adcl) // 也是个指针则递归查找类型
	{
		stringstream ss;
		ss<<"Array["<<GetPtrDataType(node->u.adcl.type)<<"]";
		type = ss.str();
	}
	else // 如果数组的成员是复杂类型，则有待扩展
	{
		Warning(1,"this ptrDataType can not be handle!");
		type = "Any";// 暂时返回一种通用类型
		UNREACHABLE;
	}
	return type;
}

//取数组成员的初始值
void X10CodeGenerate::GetArrayDataInitVal(Node *node, stringstream &strInit)
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
string X10CodeGenerate::GetDataInitVal(string type)
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
string X10CodeGenerate::GetArrayDim(Node *from)
{
	string dim;
	//PrintNode(stdout, from, 0);
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
		SPL2X10_Node(from, 0);
		tmp2 << "(" << declInitList.str() << ")";
		dim = tmp2.str();
		declInitList.str("");
		declInitList << tmp; // 恢复 
	}
	else
		UNREACHABLE;

	return dim;
}

//数组初始化过程（递归）
void X10CodeGenerate::RecursiveAdclInit(List *init)
{
	//多维数组则init是一个多维的链表
	ListMarker marker;
	Node *item;
	int i=1;
	int len = ListLength(init);
	IterateList(&marker, init);
	while ( NextOnList(&marker, (GenericREF) &item) ) 
	{
		if(i==1) declInitList<<"[";
		if (item->typ == Unary)
		{
			if(i!=1) declInitList<<", ";  // lxx.2012.02.23
			declInitList<<GetOpType(item->u.unary.op);
			SPL2X10_Node(item->u.unary.expr,0);
		}
		else if(item->typ == Const) // 如果数组成员是基本类型
		{
			if(i!=1) declInitList<<", ";
			SPL2X10_Node(item,0);
			//if(i == 1) declInitList<<" as "<<ArrayDT;//为x10-2.2作出的调整
		}
		else if (item->typ == Initializer)//如果数组成员是一个数组则递归
		{
			RecursiveAdclInit(item->u.initializer.exprs);
			if(i != len)
			{
				declInitList<<", ";
				OutputCRSpaceAndTabs(4);
			}
		}
		else if (item->typ == ImplicitCast)//基本类型的隐式转换
		{
			if(i!=1) declInitList<<", ";
			SPL2X10_Node(item->u.implicitcast.value,0);
		}
		i++;
	}
	declInitList<<"]";
}

//数组初始化过程
void X10CodeGenerate::AdclInit(Node *from, int offset)
{
	Node *arrayNode = from->u.decl.type;
	Node *tmpNode = from->u.decl.type;
	Node *initNode = from->u.decl.init;
	string name = from->u.decl.name;
	string arrayType = GetArrayDataType(tmpNode->u.adcl.type);
	string dim = GetArrayDim(tmpNode->u.adcl.dim);
	declList << "\tvar "<< name << " :Array[" << arrayType << "](1);\n";

	if (initNode == NULL) //如果没有初始化，则按数组类型进行初始化
	{
		declInitList << "\t"<<" = new Array["<<arrayType<<"](0..("<<dim<<"-1), ";
		GetArrayDataInitVal(tmpNode, declInitList); //数组初始化
		declInitList << ")";//一个数组声明加初始化
	}
	else//如果存在初始化则初始化为指定值
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
				declInitList <<name<<" :Array["<<arrayType<<"](1) = new Array["<<arrayType<<"](0..("<<dim<<"-1), "<<ss.str()<<");\n";
			}
		}
		else //初始的个数和数组维数一致，则可以采取这样的赋值形式：val pp:Array[Int](1) = [1,2,3,4,5];
		{	
			declInitList<<"\t"<<" = ";
			RecursiveAdclInit(tmp);
			//declInitList<<";\n";

		}
	}
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
void X10CodeGenerate::OutputConstant(Node *c, Bool with_name)
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
			if(c->u.Const.value.d==0.0)
				declInitList<<"0.0";
			else
				declInitList<< std::fixed << c->u.Const.value.d;
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

int X10CodeGenerate::OutputString(const char *s)
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

int X10CodeGenerate::OutputChar(char val)
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
		if (isprint(val)) 
		{
			declInitList<<(val);
		} else 
		{
			declInitList<<"\\"<<val;
		}
	}

	return 1;
}

void X10CodeGenerate::OutputCRSpaceAndTabs(int tabs)
{ 
	declInitList<<"\n"; 
	while (tabs--) declInitList<<"\t";
}

void X10CodeGenerate::OutputTabs(int tabs)
{
	while (tabs--) declInitList<<"\t";
}

void X10CodeGenerate::AddSemicolon()
{
	
}

void X10CodeGenerate::CharToText(char *str, unsigned char val){ 
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

//遍历函数调用中的参数项
void X10CodeGenerate::OutputArgList(List *list, int offset)
{ 
	ListMarker marker;
	Node *item;
	int i=0;
	IterateList(&marker, list);
	while (NextOnList(&marker, (GenericREF) &item))
	{
		if(i!=0) declInitList<<", ";
		SPL2X10_Node(item, offset);
		i++;
	}
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------

void X10CodeGenerate::SPL2X10_Node(Node *node, int offset)
{
	if (node == NULL) return;

	if(node->parenthesized == TRUE) declInitList << "("; //加括号保证逻辑性

#define CODE(name, node, union) SPL2X10_##name(node, union, offset)
	ASTSWITCH(node, CODE)
#undef CODE

	if(node->parenthesized == TRUE) declInitList << ")"; //加括号保证逻辑性
}

void X10CodeGenerate::OutputStmt(Node *node, int offset)
{
	if (node->typ != Block)
		OutputTabs(offset);
	if (node == NULL)
	{
		/* empty statement */
		declInitList << ";";
		return;
	}
	SPL2X10_Node(node, offset);
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

void X10CodeGenerate::OutputStmtList(List *list, int offset)
{
	ListMarker marker; 
	Node *item = NULL; 

	IterateList(&marker, list); 
	while (NextOnList(&marker, (GenericREF)&item))
	{ 
		OutputStmt(item, offset);
	}
}

void X10CodeGenerate::SPL2X10_List(List *list, int offset)
{
	ListMarker marker; 
	Node *item = NULL; 
	
	IterateList(&marker, list); 
	while (NextOnList(&marker, (GenericREF)&item))
	{ 
		SPL2X10_Node(item, offset);
	}
}

/*******************************************************************

********************************************************************/
void X10CodeGenerate::SPL2X10_STRdcl(Node *node, strdclNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Const(Node *node, ConstNode *u, int offset)
{
	OutputConstant(node, TRUE);
}

void X10CodeGenerate::SPL2X10_Id(Node *node, idNode *u, int offset)
{
	declInitList << u->text;
}

void X10CodeGenerate::SPL2X10_Binop(Node *node, binopNode *u, int offset)
{
	//OutputTabs(offset);
	if (node->u.binop.left) 
		SPL2X10_Node(node->u.binop.left,offset); 
	declInitList << GetOpType(node->u.binop.op);
	if (node->u.binop.right) 
		SPL2X10_Node(node->u.binop.right, offset);
	
	//declInitList << ";\n";
}

void X10CodeGenerate::SPL2X10_Unary(Node *node, unaryNode *u, int offset)
{
	if (node->u.unary.op != POSTINC && node->u.unary.op != POSTDEC)
		declInitList << GetOpType(node->u.binop.op);
	if (node->u.unary.expr) 
		SPL2X10_Node(node->u.unary.expr, offset); 
	if(node->u.unary.op == POSTINC || node->u.unary.op == POSTDEC)
		declInitList << GetOpType(node->u.binop.op);
}

void X10CodeGenerate::SPL2X10_Cast(Node *node, castNode *u, int offset)
{
	declInitList<<"( ";
	SPL2X10_Node(node->u.cast.expr,offset);
	declInitList<<" as "<<GetPrimDataType(node->u.cast.type)<<" )";
}

void X10CodeGenerate::SPL2X10_Comma(Node *node, commaNode *u, int offset)
{
	isInComma = true;//正处于逗号表达式中
	SPL2X10_List(u->exprs,offset);
	isInComma = false;
}

void X10CodeGenerate::SPL2X10_Ternary(Node *node, ternaryNode *u, int offset)
{
	SPL2X10_Node(u->cond,offset);
	declInitList<<" ? ";
	SPL2X10_Node(u->true_,offset);
	declInitList<<":";
	SPL2X10_Node(u->false_,offset);
}

void X10CodeGenerate::SPL2X10_Array(Node *node, arrayNode *u, int offset)
{
	SPL2X10_Node(u->name,offset);
	List *tmp = u->dims;
	while(tmp != NULL)//可能是多维数组，需要遍历dim这个list
	{
		declInitList<<"(";
		Node *item = (Node *)FirstItem(tmp);
		SPL2X10_Node(item,offset);
		declInitList<<")";
		tmp = Rest(tmp);
	}
}

void X10CodeGenerate::SPL2X10_Call(Node *node, callNode *u, int offset)
{
	assert(u->name->typ == Id);
	{
		const char *ident = u->name->u.id.text;
		if (strcmp(ident,"acos") == 0) declInitList<<"Math.acos";
		else if (strcmp(ident,"acosh")==0) declInitList<<"Math.acosh";
		else if (strcmp(ident,"acosh")==0) declInitList<<"Math.acosh";
		else if (strcmp(ident,"asin")==0) declInitList<<"Math.asin";
		else if (strcmp(ident,"asinh")==0) declInitList<<"Math.asinh";
		else if (strcmp(ident,"atan")==0) declInitList<<"Math.atan";
		else if (strcmp(ident,"atan2")==0) declInitList<<"Math.atan2";
		else if (strcmp(ident,"atanh")==0) declInitList<<"Math.atanh";
		else if (strcmp(ident,"ceil")==0) declInitList<<"Math.ceil";
		else if (strcmp(ident,"cos")==0) declInitList<<"Math.cos";
		else if (strcmp(ident,"cosh")==0) declInitList<<"Math.cosh";
		else if (strcmp(ident,"exp")==0) declInitList<<"Math.exp";
		else if (strcmp(ident,"expm1")==0) declInitList<<"Math.expm1";
		else if (strcmp(ident,"floor")==0) declInitList<<"Math.floor";
		else if (strcmp(ident,"fmod")==0) declInitList<<"Math.fmod";
		else if (strcmp(ident,"frexp")==0) declInitList<<"Math.frexp";
		else if (strcmp(ident,"log")==0) declInitList<<"Math.log";
		else if (strcmp(ident,"log10")==0) declInitList<<"Math.log10";
		else if (strcmp(ident,"log1p")==0) declInitList<<"Math.log1p";
		else if (strcmp(ident,"modf")==0) declInitList<<"Math.modf";
		else if (strcmp(ident,"pow")==0) declInitList<<"Math.pow";
		else if (strcmp(ident,"sin")==0) declInitList<<"Math.sin";
		else if (strcmp(ident,"sinh")==0) declInitList<<"Math.sinh";
		else if (strcmp(ident,"sqrt")==0) declInitList<<"Math.sqrt";
		else if (strcmp(ident,"tan")==0) declInitList<<"Math.tan";
		else if (strcmp(ident,"tanh")==0) declInitList<<"Math.tanh";
		// not from profiling: round(x) is currently macro for floor((x)+0.5)
		else if (strcmp(ident,"round")==0) declInitList<<"Math.round";
		// not from profiling: just stuck in here to keep compilation of gmti 
		// from spewing warnings.
		else if (strcmp(ident,"abs")==0) declInitList<<"Math.abs";
		else if (strcmp(ident,"max")==0) declInitList<<"Math.max";
		else if (strcmp(ident,"min")==0) declInitList<<"Math.min";
		else if (strcmp(ident,"println")==0) declInitList<<"Console.OUT.println";
		else if (strcmp(ident,"printf")==0) declInitList<<"Console.OUT.printf";
		else if (strcmp(ident,"print")==0) declInitList<<"Console.OUT.print";
		else //unkonwn methods
		{
			;
		}
		declInitList<<"(";
		OutputArgList(u->args,offset);//参数
		declInitList<<")";
	}
}

void X10CodeGenerate::SPL2X10_Initializer(Node *node, initializerNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_ImplicitCast(Node *node, implicitcastNode *u, int offset)
{
	SPL2X10_Node(u->expr, offset);
}

void X10CodeGenerate::SPL2X10_Label(Node *node, labelNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Switch(Node *node, SwitchNode *u, int offset)
{
	declInitList<<"switch (";
	SPL2X10_Node(u->expr,offset);
	declInitList<<")";
	SPL2X10_Node(u->stmt,offset);
}

void X10CodeGenerate::SPL2X10_Case(Node *node, CaseNode *u, int offset)
{
	declInitList<<"case ";
	SPL2X10_Node(u->expr,offset);
	declInitList<<":";
	OutputCRSpaceAndTabs(offset+1);
	SPL2X10_Node(u->stmt,offset);
}

void X10CodeGenerate::SPL2X10_Default(Node *node, DefaultNode *u, int offset)
{
	declInitList<<"default: ";
	OutputCRSpaceAndTabs(offset);
	SPL2X10_Node(u->stmt,offset);
}

void X10CodeGenerate::SPL2X10_If(Node *node, IfNode *u, int offset)
{
	declInitList<<"if (";
	SPL2X10_Node(u->expr,offset);
	declInitList<<")";
	if (u->stmt->typ != Block) // 如果是非block结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2X10_Node(u->stmt,offset+1);
	if(u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList<<";";
}

void X10CodeGenerate::SPL2X10_IfElse(Node *node, IfElseNode *u, int offset)
{
	declInitList<<"if (";
	SPL2X10_Node(u->expr,offset);
	declInitList<<")";
	if (u->true_->typ != Block)//如果是非block结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2X10_Node(u->true_,offset);
	if(u->true_->typ == Binop || u->true_->typ == Unary || u->true_->typ == Ternary || u->true_->typ == Call || u->true_->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList<<";";
	OutputCRSpaceAndTabs(offset);
	declInitList<<"else ";
	if (u->false_->typ != Block && u->false_->typ != IfElse)//如果是非block结点或者ifelse结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2X10_Node(u->false_,offset);
	if(u->false_->typ == Binop || u->false_->typ == Unary || u->false_->typ == Ternary || u->false_->typ == Call || u->false_->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList<<";";
}

void X10CodeGenerate::SPL2X10_While(Node *node, WhileNode *u, int offset)
{
	declInitList<<"while (";
	SPL2X10_Node(u->expr,offset);
	declInitList<<")";
	if (u->stmt->typ != Block)//如果是非block结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset+1);
	}
	SPL2X10_Node(u->stmt,offset);
	if(u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList<<";";
}

void X10CodeGenerate::SPL2X10_Do(Node *node, DoNode *u, int offset)
{
	declInitList<<"do";
	SPL2X10_Node(u->stmt,offset);
	declInitList<<"while (";
	SPL2X10_Node(u->expr,offset);
	declInitList<<")";
}

void X10CodeGenerate::SPL2X10_For(Node *node, ForNode *u, int offset)
{
	declInitList<<"for (";
	SPL2X10_Node(u->init,offset);
	declInitList<<";";
	SPL2X10_Node(u->cond,offset);
	declInitList<<";";
	SPL2X10_Node(u->next,offset);
	declInitList<<")";
	if (u->stmt->typ != Block)//如果是非block结点，则需要换行对齐
	{
		declInitList<<"\n";
	}
	OutputStmt(u->stmt,offset+1);
}

void X10CodeGenerate::SPL2X10_Goto(Node *node, GotoNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Continue(Node *node, ContinueNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Break(Node *node, BreakNode *u, int offset)
{
	declInitList<<"break";
}

void X10CodeGenerate::SPL2X10_Return(Node *node, ReturnNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Block(Node *node, BlockNode *u, int offset)
{
	declInitList <<"{\n";

	//OutputCRSpaceAndTabs(offset + 1);
	SPL2X10_List(u->decl,  offset);

	declInitList<<"\n"; // 另起一行
	OutputStmtList(u->stmts, offset);

	OutputCRSpaceAndTabs(offset);
	declInitList<<"}\n"; // '}'独占一行
}

void X10CodeGenerate::SPL2X10_Prim(Node *node, primNode *u, int offset)
{
	declInitList << GetPrimDataType(node);
}

void X10CodeGenerate::SPL2X10_Tdef(Node *node, tdefNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Ptr(Node *node, ptrNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Adcl(Node *node, adclNode *u, int offset)
{
#if 0
	/*X10-2.2后多维的数组需要去掉具体类型。例如
		 val kkey=[[14,4,13,1],[2,5,67,80]];
		 让x10编译器自己判断类型。
	*/
	if((node->u.adcl.type)->typ != Adcl){ 
		string arrayType = getArrayDataType(node->u.adcl.type);
		declInitList<<"Array["<<arrayType<<"](1)";
	}
#endif

#if 1 //X10-2.1数组处理方式
	string arrayType = GetArrayDataType(node->u.adcl.type);
	declInitList<<"Array["<<arrayType<<"](1)";
#endif
}

void X10CodeGenerate::SPL2X10_Fdcl(Node *node, fdclNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Sdcl(Node *node, sdclNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Udcl(Node *node, udclNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Edcl(Node *node, edclNode *u, int offset)
{
}


void X10CodeGenerate::SPL2X10_Decl(Node *node, declNode *u, int offset)
{
	if (extractDecl)
	{
		ExtractDeclVariables(node);
		return;
	}
	else
	{
		OutputCRSpaceAndTabs(offset);
		//declInitList << "\n";
		if(u->type->typ == Adcl && ((u->type)->u.adcl.type)->typ== Adcl)//二维数组
			declInitList <<"var "<<node->u.decl.name<<" :";
		else
			declInitList <<"var "<<node->u.decl.name<<" :";
	}

	SPL2X10_Node(u->type, offset);
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
				SPL2X10_Node(u->init,offset );
			}
		}
	}
	declInitList<<";";
}

void X10CodeGenerate::SPL2X10_Attrib(Node *node, attribNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Proc(Node *node, procNode *u, int offset)
{
}

void X10CodeGenerate::SPL2X10_Text(Node *node, textNode *u, int offset)
{
}
