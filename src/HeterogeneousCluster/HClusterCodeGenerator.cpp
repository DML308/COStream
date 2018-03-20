#include "HClusterCodeGenerator.h"

using namespace std;

/*记录异构集群间链接边的个数*/
static int netEdgeNum = 0;
static int threadNum = 0;

/*默认执行次数*/
const unsigned long long Default_Repeat_Count = 100000;

/*后端代码生成的入口函数*/
void HClusterCodeGenerator::HClusterCodeGeneration(const string &str)
{
	m_curDir += "\\HClusterCodeGeneration_Linux\\";
	m_curDir += str;
	m_curDir += "\\";

	//当前代码生成的目录
	cout << m_curDir << "******************************" << endl;
	
	//逐级创建不存在的目录
	int idx = 0, i = 0;
	while (true) {
		string::size_type pos = m_curDir.find("\\", idx);
		string tmpStr = m_curDir.substr(0, pos);
		if(pos == -1)
			break;
		else if(i > 0){
			_mkdir(tmpStr.c_str());
		}

		++i;
		idx = pos + 1;
	}//while

	cout << "------------生成异构集群后端OpenCL和C++混合代码----------" << endl;
	
	/*收集全局信息*/
	this->collectGlobalInfo();
	
	/*确定数据流类型和对应的结构体类型映射*/
	this->createStream2StructTypeMap();

	/*生成全局变量*/
	this->CGGlobalVar();
	/*生成全局变量声明式*/
	this->CGGlobalVarextern();

	/*为集群创建通信边信息*/
	this->CGGlobalStreamHeader();
	this->CGGlobalStreamCpp();

	/*生成进程信息*/
	this->CGProcesses();
	//为异构集群节点的每个进程生成线程信息
	this->CGThreads();

	//生成各节点信息
	this->CGActors();
	this->CGAllKernel();
	/*生成集群配置文件*/
	this->CGMachineFile();
	this->CGMain();//生成启动线程的main文件

	/*拷贝各个依赖文件到目标代码目录*/
	HClusterLibCopy hcLibCp;
	hcLibCp.run(m_curDir.c_str());
	cout << endl << "Code generation successful! Done!" << endl << endl;
	
}

int HClusterCodeGenerator::getCpuThreadNumOnHCluster(int hClusterNum)
{
	assert(hClusterNum < m_hcp->getHClusters());
	map<int, vector<FlatNode*>> tmpCore2FlatNodes;
	tmpCore2FlatNodes = m_hcp->getHClusterCore2FlatNodes(hClusterNum);
	return tmpCore2FlatNodes.size();
}

int HClusterCodeGenerator::getGpuThreadNumOnHCluster(int hClusterNum)
{
	assert(hClusterNum < m_hcp->getHClusters());
	map<int, vector<FlatNode*>> tmpGpu2FlatNodes;
	if (m_hcp->isGPUClusterNode(hClusterNum)) {
		tmpGpu2FlatNodes = m_hcp->getHClusterGpu2StatelessNodes(hClusterNum);
		return tmpGpu2FlatNodes.size();
	}
	else {
		return 0;
	}
}

int HClusterCodeGenerator::getNetCommEdgeSize(FlatNode * pflatNode, FlatNode * cflatNode, int pushValue, Bool type)
{
	//取pflatNode（生产者）到cflatNode（消费者）对应的缓冲区的大小，pushValue是生产者的push码率,type表示的是输入还是输出边
	long buffsize = 0;
	int curinitScheduleCount = m_sssg->GetInitCount(pflatNode);
	int cursteadyScheduleCount = m_hcp->getSteadyCount(pflatNode);
	long init_io = curinitScheduleCount * pushValue;
	long steady_io = cursteadyScheduleCount * pushValue;
	int c_cluster = m_hcp->getHClusterCoreNum(cflatNode).first;
	if (type == TRUE)
	{
		//生产者对应的输出边的缓冲区的大小——取初态或者稳态产生速率的最大值
		buffsize = steady_io > init_io ? steady_io : init_io;
	}
	else
	{
		//消费者对应的输入边的缓冲区的大小
		//集群节点间阶段划分的阶段差
		int flat_stageminus = m_hsa->getStageNum(cflatNode) - m_hsa->getStageNum(pflatNode);
		buffsize = (curinitScheduleCount + cursteadyScheduleCount * (flat_stageminus + 1)) * (pushValue);
	}

	return buffsize + 1;
}

//为SDF图每个actor节点生成类定义代码
void HClusterCodeGenerator::CGActors()
{
	stringstream out, buf;

	for (int i = 0; i < m_nActors; ++i)
	{
		/*判断当前节点划分到GPU混合架构服务器还是纯CPU服务器,CPU端节点或边界节点在CPU执行*/
		if (!m_hcp->isInGPUClusterNode(m_flatNodes[i]) || i == m_nActors - 1)
		{
			int len = ListLength(m_flatNodes[i]->contents->decl->u.decl.type->u.operdcl.outputs);
			int nOut = m_flatNodes[i]->nOut;
			OperatorType ot = m_flatNodes[i]->contents->ot;

			if (len == nOut)
			{
				buf << "#include \"" << m_flatNodes[i]->name << ".h\"\n";
				CGActor(m_flatNodes[i], ot);
			}//if
			else {
				// 一条边被多个actor共用, 暂不处理
				cout << "code generating error" << endl;
				UNREACHABLE;
			}//else
		}//if
		else {
			/*CurGpuFlag控制当前生成的代码版本*/
			if (CurGpuFlag && m_hcp->getHClusters() == 1)
			{
				buf << "#include \"" << m_flatNodes[i]->name << ".h\"\n";

				int len = ListLength(m_flatNodes[i]->contents->decl->u.decl.type->u.operdcl.outputs);
				int nOut = m_flatNodes[i]->nOut;
				OperatorType ot = m_flatNodes[i]->contents->ot;
				if (len == nOut)
				{
					int hClusterNum = m_hcp->getHClusterNumOfNode(m_flatNodes[i]);

					vector<FlatNode*> curStatelessNodes = m_hcp->getStatelessNodes(hClusterNum);
					assert(!curStatelessNodes.empty());
					vector<FlatNode *>::iterator fIter = find(curStatelessNodes.begin(), curStatelessNodes.end(), m_flatNodes[i]);
					if (fIter != curStatelessNodes.end())
					{
						m_kernelNodes.push_back(m_flatNodes[i]);
						CGActor_GPU(m_flatNodes[i], ot);
					}
					else
					{
						CGActor(m_flatNodes[i], ot);						
					}//if
				}//if
				else {
					// 一条边被多个actor共用, 暂不处理
					cout << "code generating error" << endl;
					UNREACHABLE;
				}//else
			}//if
			else {
				int len = ListLength(m_flatNodes[i]->contents->decl->u.decl.type->u.operdcl.outputs);
				int nOut = m_flatNodes[i]->nOut;
				OperatorType ot = m_flatNodes[i]->contents->ot;

				if (len == nOut)
				{
					int hClusterNum = m_hcp->getHClusterNumOfNode(m_flatNodes[i]);

					vector<FlatNode*> curStatelessNodes = m_hcp->getStatelessNodes(hClusterNum);
					assert(!curStatelessNodes.empty());
					vector<FlatNode *>::iterator fIter = find(curStatelessNodes.begin(), curStatelessNodes.end(), m_flatNodes[i]);
					if (fIter != curStatelessNodes.end())
					{
						m_kernelNodes.push_back(m_flatNodes[i]);
					}
					buf << "#include \"" << m_flatNodes[i]->name << ".h\"\n";
					CGActor(m_flatNodes[i], ot);
				}//if
				else {
					// 一条边被多个actor共用, 暂不处理
					cout << "code generating error" << endl;
					UNREACHABLE;
				}//else


			}//else
		}//else
		
	}//for


	out << m_curDir << "AllActorHeader.h";
	outputToFile(out.str(), buf.str());
}

void HClusterCodeGenerator::CGActor(FlatNode * actor, OperatorType ot)
{
	//将一个actor转换成一个类，(此处处理的是一个节点内部的actor——即与他通信的所有actor都在这个节点内部)
	stringstream buf, SRbuf, pushingBuf, popingBuf, buf1, buf2;
	stringstream logicInitBuf, workBuf, initWorkBuf;
	vector<string>::iterator iter;
	string className = actor->name, streamName = "Token_Temp";
	
	m_curActor = actor;
	/*当前处理的actor不能为空*/
	assert(actor != NULL);
	actor->SetIOStreams();

	buf << "#ifndef " << className << "_H\n" << "#define " << className << "_H\n";
	//类定义的注释
	buf << "/**\n * Class " << className << "\n*/\n";
	buf << "\n#include <mpi.h>\n";

	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"Producer.h\"\n";
	buf << "#include \"Consumer.h\"\n";
	buf << "#include \"globalstream.h\"\n";
	buf << "#include <iostream>\n";
	buf << "#include \"GlobalVar.h\"\n";
	buf << "#include \"NetConsumer.h\"\n";
	buf << "#include \"NetProducer.h\"\n";
	buf << "#include \"rdtsc.h\"\n";
	buf << "#include <ctime>\n";
	buf << "using namespace std;\n\n";

	for (map<string, List*>::iterator iter = m_struct2FieldList.begin(); iter != m_struct2FieldList.end(); ++iter)
	{
		buf << "extern " << "MPI::Datatype " << iter->first << "_mpitype" << ";\n";

	}
	buf << "extern " << "MPI::Intracomm comm;\n";
	buf << "\nclass " << className << "{\n"; // 类块开始
	// 写入发送、接收信息
	m_parameterBuf.str(""); // 清空parameterBuf内容
	m_thisBuf.str(""); // 清空thisBuf内容
	buf << SRbuf.str();

	// 写入循环次数信息
	buf << "private:\n";
	//写入边的私有变量
	CGEdgeParam(actor, buf);
	// 写入语法树成员变量信息（param, var, state）——只有state中的变量，param和var中的变量在常量传播完成后已经不存在了
	CGActorMemberVar(actor, ot, buf);
	// 写入 var, state 的初始化信息
	CGLogicStateInit(actor, ot, buf);
	// 写入 init 的初始化信息(考虑到init可以定义局部变量，所以不与上部分合并)
	CGLogicInit(actor, ot, buf);
	// 写入popToken函数
	CGPopToken(actor, buf, popingBuf.str());
	CGPushToken(actor, buf, pushingBuf.str());
	buf << "public:\n";
	// 写入构造函数信息
	CGThis(actor, ot, buf);
	//写入析构函数
	CGDestructor(actor, ot, buf);
	// 写入成员函数信息
	buf << "\t// ------------Member Methods------------\n";
	// 写入work函数
	CGWork(actor, ot, buf);
	//写入runInitScheduleWork函数
	CGRunInitScheduleWork(buf, actor);
	//写入runSteadyScheduleWork函数
	CGRunSteadyScheduleWork(buf, actor);
	//写入flush操作
	CGFlush(buf, actor);

	buf << "};\n\n";//类块结束
	buf << "\n\n#endif\n\n\n";
	buf1.str("");
	buf2.str("");
	buf2 << "#include \"" << actor->name << ".h\"\n";

	//输出到文件
	stringstream fileName, fileName1;
	fileName << m_curDir << className << ".h";
	fileName1 << m_curDir << className << ".cpp";

	outputToFile(fileName.str(), buf.str());
	if (m_array_staticMemberStream.str() != "")
	{
		//对静态成员变量、数组初始化++++++++
		buf2 << m_array_staticMemberStream.str() << "\n";
		outputToFile(fileName1.str(), buf2.str());
	}
	m_array_staticMemberStream.str("");
}

void HClusterCodeGenerator::CGActor_GPU(FlatNode * actor, OperatorType ot)
{
	//将一个actor转换成一个类，(此处处理的是一个节点内部的actor——即与他通信的所有actor都在这个节点内部)
	stringstream buf, SRbuf, pushingBuf, popingBuf, buf1, buf2;
	stringstream logicInitBuf, workBuf, initWorkBuf;
	vector<string>::iterator iter;
	string className = actor->name, streamName = "Token_Temp";

	m_curActor = actor;
	/*当前处理的actor不能为空*/
	assert(actor != NULL);
	actor->SetIOStreams();

	buf << "#ifndef " << className << "_H\n" << "#define " << className << "_H\n";
	//类定义的注释
	buf << "/**\n * Class " << className << "\n*/\n";
	buf << "\n#include <mpi.h>\n";

	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"Producer.h\"\n";
	buf << "#include \"Consumer.h\"\n";
	buf << "#include \"globalstream.h\"\n";
	buf << "#include <iostream>\n";
	buf << "#include \"GlobalVar.h\"\n";
	buf << "#include \"NetConsumer.h\"\n";
	buf << "#include \"NetProducer.h\"\n";
	buf << "#include \"rdtsc.h\"\n";
	buf << "#include <ctime>\n";
	buf << "#if defined(__APPLE__) || defined(__MACOSX)\n";
	buf << "#include <OpenCL/cl.hpp>\n";
	buf << "#else\n";
	buf << "#include <CL/cl.h>\n";
	buf << "#endif\n\n";

	buf << "using namespace std;\n\n";

	for (map<string, List*>::iterator iter = m_struct2FieldList.begin(); iter != m_struct2FieldList.end(); ++iter)
	{
		buf << "extern " << "MPI::Datatype " << iter->first << "_mpitype" << ";\n";

	}
	buf << "extern " << "MPI::Intracomm comm;\n\n";
	buf << "extern cl_program program;\n";
	buf << "extern cl_device_id *devices;\n";
	buf << "extern cl_context context;\n";
	buf << "extern cl_command_queue cQueue[3];\n";

	buf << "\nclass " << className << "{\n"; // 类块开始
											 // 写入发送、接收信息
	m_parameterBuf.str(""); // 清空parameterBuf内容
	m_thisBuf.str(""); // 清空thisBuf内容
	buf << SRbuf.str();

	// 写入循环次数信息
	buf << "private:\n";
	//写入边的私有变量
	CGEdgeParam(actor, buf);
	// 写入语法树成员变量信息（param, var, state）——只有state中的变量，param和var中的变量在常量传播完成后已经不存在了
	CGActorMemberVar(actor, ot, buf);
	// 写入 var, state 的初始化信息
	CGLogicStateInit(actor, ot, buf);
	// 写入 init 的初始化信息(考虑到init可以定义局部变量，所以不与上部分合并)
	CGLogicInit(actor, ot, buf);
	// 写入popToken函数
	CGPopToken(actor, buf, popingBuf.str());
	CGPushToken(actor, buf, pushingBuf.str());
	buf << "public:\n";
	// 写入构造函数信息
	CGThis(actor, ot, buf);
	//写入析构函数
	CGDestructor(actor, ot, buf);
	// 写入成员函数信息
	buf << "\t// ------------Member Methods------------\n";
	// 写入work函数
	CGWork_GPU(actor, ot, buf);
	//CGWork(actor, ot, buf);

	//写入runInitScheduleWork函数
	CGRunInitScheduleWork(buf, actor);
	//写入runSteadyScheduleWork函数
	CGRunSteadyScheduleWork(buf, actor);
	//写入flush操作
	CGFlush(buf, actor);

	buf << "};\n\n";//类块结束
	buf << "\n\n#endif\n\n\n";
	buf1.str("");
	buf2.str("");
	buf2 << "#include \"" << actor->name << ".h\"\n";

	//输出到文件
	stringstream fileName, fileName1;
	fileName << m_curDir << className << ".h";
	fileName1 << m_curDir << className << ".cpp";

	outputToFile(fileName.str(), buf.str());
	if (m_array_staticMemberStream.str() != "")
	{
		//对静态成员变量、数组初始化++++++++
		buf2 << m_array_staticMemberStream.str() << "\n";
		outputToFile(fileName1.str(), buf2.str());
	}
	m_array_staticMemberStream.str("");
}

/*将actor的上下边作为类的私有成员，生成相关代码*/
void HClusterCodeGenerator::CGEdgeParam(FlatNode *actor, stringstream &buf)
{
	buf << "\t//edge param\n";
	List *inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	vector<FlatNode *>tmpInFlatNode = actor->inFlatNodes;
	vector<FlatNode *>tmpOutFlatNode = actor->outFlatNodes;
	assert(tmpInFlatNode.size() == ListLength(inputList));
	assert(tmpOutFlatNode.size() == ListLength(outputList));

	int idx = 0;
	int hClusterNum = m_hcp->getHClusterNumOfNode(actor);

	/*处理入边*/
	IterateList(&input_maker, inputList);
	while (NextOnList(&input_maker, (GenericREF)&inputNode))
	{
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
	
		int hClusterNumInFlatNode = m_hcp->getHClusterNumOfNode(tmpInFlatNode[idx]);

		/*在同一个集群节点中*/
		if (hClusterNum == hClusterNumInFlatNode)
		{
			string tmpName = tmpInFlatNode[idx]->name + "_" + actor->name;
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			if (type_iter != m_stream2StructType.end())
				buf << "\tConsumer<" << type_iter->second << "> " << inputString << ";\n";
		}//if
		/*不在同一个集群节点中*/
		else {
			//不在同一个集群节点中
			string tmpName = tmpInFlatNode[idx]->name + "_" + actor->name + "_recvbuf";
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			if(type_iter != m_stream2StructType.end())
				buf << "\tNetConsumer<" << type_iter->second << "> " << inputString << ";\n";
		}//else
		++idx;
	}//while

	//处理出边
	idx = 0;
	IterateList(&output_maker, outputList);

	while (NextOnList(&output_maker, (GenericREF)&outputNode))
	{
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		int hClusterNumOutFlatNode = m_hcp->getHClusterNumOfNode(tmpOutFlatNode[idx]);
		if (hClusterNum == hClusterNumOutFlatNode)
		{//在同一个集群节点中
			string tmpName = actor->name + "_" + tmpOutFlatNode[idx]->name;
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			
			if(type_iter != m_stream2StructType.end())
				buf << "\tProducer<" << type_iter->second << "> " << outputString << ";\n";
		}//if
		/*不在同一个集群节点中*/
		else
		{
			string tmpName = actor->name + "_" + tmpOutFlatNode[idx]->name + "_sendbuf";
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			if (type_iter != m_stream2StructType.end())
				buf << "\tNetProducer<" << type_iter->second << "> " << outputString << ";\n";

		}
		++idx;
	}//while
}

/*生成actor内成员变量*/
void HClusterCodeGenerator::CGActorMemberVar(FlatNode * actor, OperatorType ot, stringstream &buf)
{
	List *state = NULL;
	List *param = NULL;
	assert(actor);

	buf << "\t//类内成员变量:\n";
	state = actor->contents->body->u.operBody.state;//在composite的param以及var中的变量可以全部移植到state中

	m_extractStateDecl = true;//提取state中的变量
	buf << "\t/* *****composite param***** */\n";
	buf << "\tint steadyScheduleCount;\t//稳态状态执行次数\n";
	buf << "\tint initScheduleCount;\t//初始状态执行次数\n";
	buf << "\n\t//经常量传播完成后state中的变量（包含原来的composite中param以及var中的变量）\n";
	m_stateDeclStream.str(""); // 清空declList内容
	CGDeclList(m_stateDeclStream, BlockDecl, state, 1, actor);//对于state中的变量将声明放在stateDeclStream中，初始化放在stateInitStream中
	buf << "\t/******logic state******/\n" << m_stateDeclStream.str()<<"\n";
	m_extractStateDecl = false;
}

void HClusterCodeGenerator::CGLogicStateInit(FlatNode * actor, OperatorType ot, stringstream &buf)
{
	buf << "\t//initState\n";
	buf << "\tvoid initState()\n\t{\n";
	buf << "\t\t/* State Init */\n";
	buf << m_stateInitStream.str();//打印state变量中的语句
	buf << "\n\t}\n\n";

	/*清空*/
	m_stateInitStream.str("");
}

void HClusterCodeGenerator::CGLogicInit(FlatNode * actor, OperatorType ot, stringstream &buf)
{
	buf << "\t//init\n";
	buf << "\tvoid init()";
	Node *init = actor->contents->body->u.operBody.init;

	stringstream logicInitStream("");
	if (init)
	{
		CGNode(logicInitStream, init, 1);
		buf << logicInitStream.str(); // init结构必须带"{}"
	}
	else
	{
		buf << "\n\t{\n";
		buf << "\t}\n\n"; // '}'独占一行
	}
}

void HClusterCodeGenerator::CGPopToken(FlatNode *actor, stringstream &buf, string popingBuf)
{
	buf << "\t//popToken\n";
	buf << "\tvoid popToken()\n\t{\n";
	List *inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	IterateList(&input_maker, inputList);
	vector<int>::iterator iter = actor->inPopWeights.begin();
	while (NextOnList(&input_maker, (GenericREF)&inputNode))
	{
		if (*iter == 0) continue;//pop为0 不生成update代码  20130119
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		buf << "\t\t" << inputString << ".updatehead" << "(" << *iter << ");\n";
		iter++;
	}
	buf << popingBuf;
	buf << "\t}\n\n"; // popToken方法结束
}

void HClusterCodeGenerator::CGPushToken(FlatNode *actor, stringstream &buf, string popingBuf)
{
	buf << "\t// pushToken\n";
	buf << "\tvoid pushToken()\n\t{\n";
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker output_maker;
	Node *outputNode = NULL;
	IterateList(&output_maker, outputList);
	vector<int>::iterator iter = actor->outPushWeights.begin();
	while (NextOnList(&output_maker, (GenericREF)&outputNode))
	{
		if (*iter == 0) continue;//push为0 不生成update代码  20130119
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		buf << "\t\t" << outputString << ".updatetail" << "(" << *iter << ");\n";
		iter++;
	}
	buf << popingBuf;
	buf << "\t}\n\n"; // popToken方法结束
}

/*根据actor的信息构造类*/
void HClusterCodeGenerator::CGThis(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	buf << "\t//构造函数\n";
	buf << "\t" << actor->name;
	buf << "( ";
	List *inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	vector<FlatNode *>tmpInFlatNode = actor->inFlatNodes;
	vector<FlatNode *>tmpOutFlatNode = actor->outFlatNodes;
	assert(tmpInFlatNode.size() == ListLength(inputList));
	assert(tmpOutFlatNode.size() == ListLength(outputList));

	int index = 0;
	int hClusterNum = m_hcp->getHClusterNumOfNode(actor);
	//打印构造函数
	map<string, FlatNode *> netConsumerBuf;//边与生产者的map
	map<string, FlatNode *> netProducerBuf;//边与消费者的map
	//参数列表++++++++++++++++++++++++++++++++++++++++++++++
	//输入
	IterateList(&input_maker, inputList);
	while (NextOnList(&input_maker, (GenericREF)&inputNode))
	{
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;

		int hClusterNumInFlatNode = m_hcp->getHClusterNumOfNode(tmpInFlatNode[index]);
		//在同一个集群节点中
		if (hClusterNum == hClusterNumInFlatNode)
		{
			string tmpName = tmpInFlatNode[index]->name + "_" + actor->name;
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			buf << "Buffer<" << type_iter->second << " >&" << inputString << ", ";
		}
		//不在同一个集群节点中
		else
		{
			//添加每一条网络边构造需要的信息
			string tmpStreamName = tmpInFlatNode[index]->name + "_" + actor->name + "_sendbuf";
			multimap<FlatNode*, string>::iterator up_pos = m_actor2OutEdge.end();
			for (up_pos = m_actor2OutEdge.begin(); up_pos != m_actor2OutEdge.end(); up_pos++)
			{//找边对应的生产者
				if (up_pos->second == tmpStreamName) break;
			}
			assert(up_pos != m_actor2OutEdge.end());
			netConsumerBuf.insert(make_pair(inputString, up_pos->first));
		}//else
		++index;
	}//while

	//输出
	index = 0;
	IterateList(&output_maker, outputList);
	while (NextOnList(&output_maker, (GenericREF)&outputNode))
	{
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		int hClusterNumOutFlatNode = m_hcp->getHClusterNumOfNode(tmpOutFlatNode[index]);
		//在同一个集群节点中
		if (hClusterNum == hClusterNumOutFlatNode)
		{
			string tmpName = actor->name + "_" + tmpOutFlatNode[index]->name;
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			buf << "Buffer<" << type_iter->second << " >&" << outputString << ", ";
		}
		//不在同一个集群节点中
		else
		{
			string tmpStreamName = actor->name + "_" + tmpOutFlatNode[index]->name + "_recvbuf";
			multimap<FlatNode*, string>::iterator down_pos = m_actor2InEdge.end();
			for (down_pos = m_actor2InEdge.begin(); down_pos != m_actor2InEdge.end(); down_pos++)
			{
				//找接受者
				if (down_pos->second == tmpStreamName) 
					break;
			}
			assert(down_pos != m_actor2InEdge.end());
			netProducerBuf.insert(make_pair(outputString, down_pos->first));
		}//else
		++index;
	}//while
	buf << m_parameterBuf.str();
	if (m_parameterBuf.str() == " ")
	{
		buf.seekp((int)buf.tellp() - 1);
	}
	else
	{
		buf.seekp((int)buf.tellp() - 2);
	}
	buf << "): ";
	//参数列表结束++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//初始化列表++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//输入
	index = 0;
	IterateList(&input_maker, inputList);
	while (NextOnList(&input_maker, (GenericREF)&inputNode))
	{
		//if(actor->inPopWeights[index] == 0) continue;//对于pop为0的边不需要构造 20130119
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		int hClusterNumInFlatNode = m_hcp->getHClusterNumOfNode(tmpInFlatNode[index]);
		//在同一个集群节点中
		if (hClusterNumInFlatNode == hClusterNum)
		{
			string tmpName = tmpInFlatNode[index]->name + "_" + actor->name;
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			buf << inputString << "(Consumer<" << type_iter->second << ">(" << inputString << ")), ";
		}
		//不在同一个集群节点中
		else
		{
			//对网络输入边构造（netconsumer）
			map<string, FlatNode *>::iterator con_iter1 = netConsumerBuf.find(inputString);
			stringstream message_Tag;
			int tag1 = findID(actor);
			int tag2 = findID(con_iter1->second);
			if (tag2)message_Tag << tag2 << tag1;
			else message_Tag << tag1;
			string edgename = con_iter1->second->name + "_" + actor->name + "_recvbuf";
			map<string, string>::iterator tagpos1 = m_stream2StructType.find(edgename);
			assert(tagpos1 != m_stream2StructType.end());
			int processSrc = m_hcp->getHClusterNumOfNode(con_iter1->second);

			int io = GetIORadio(actor, index, TRUE);
			buf << con_iter1->first << "(comm, " << tagpos1->second << "_mpitype, " << processSrc << ", " << message_Tag.str() << ", " << io;

			buf << ")," << endl;
		}//else
		++index;
	}//while

	//输出
	index = 0;
	IterateList(&output_maker, outputList);
	while (NextOnList(&output_maker, (GenericREF)&outputNode))
	{
		//if(actor->outPushWeights[index] == 0) continue;//对于push为0的边不需要构造  20130119
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		int hClusterNumOutFlatNode = m_hcp->getHClusterNumOfNode(tmpOutFlatNode[index]);
		//在同一个集群节点中
		if (hClusterNumOutFlatNode == hClusterNum)
		{
			string tmpName = actor->name + "_" + tmpOutFlatNode[index]->name;
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			buf << outputString << "(Producer<" << type_iter->second << ">(" << outputString << ")), ";
		}
		//不在同一个集群节点中
		else
		{
			//对网络输出边构造（netproduce）
			map<string, FlatNode *>::iterator pro_iter1 = netProducerBuf.find(outputString);
			stringstream message_Tag;
			int tag1 = findID(actor);
			int tag2 = findID(pro_iter1->second);
			if (tag1)message_Tag << tag1 << tag2;
			else message_Tag << tag2;
			int recv_len = 0;//具体数值有待计算
			string edgename = actor->name + "_" + pro_iter1->second->name + "_sendbuf";
			map<string, string>::iterator tagpos1 = m_stream2StructType.find(edgename);
			assert(tagpos1 != m_stream2StructType.end());
			
			int processDest = m_hcp->getHClusterNumOfNode(pro_iter1->second);

			int io = GetIORadio(actor, index, FALSE);
			buf << pro_iter1->first << "(comm, " << tagpos1->second << "_mpitype, " << processDest << ", " << message_Tag.str() << ", " << io;

			buf << ")," << endl;
		}
		++index;
	}//while

	//对稳态和初态的执行次数构造

	buf << "steadyScheduleCount(" << m_hcp->getSteadyCount(actor) << "),";
	buf << "initScheduleCount(" << m_sssg->GetInitCount(actor) << "),";
	buf.seekp((int)buf.tellp() - 1);
	//初始化列表结束++++++++++++++++++++++++++++++++++++++++++++++++++++
	//构造函数函数体++++++++++++++++++++++++++++++++++++++++++++++++++++
	buf << "\n\t{\n";
	buf << m_thisBuf.str();
	buf << "\t\tinitState();\n";// 初始化state中的变量
	buf << "\t}\n\n"; // init方法结束
	//构造函数函数体结束

}

void HClusterCodeGenerator::CGDestructor(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	//析构函数
	if (m_destructorStream.str() != "")
	{
		buf << "\t~" << actor->name << "()\n\t{\n";
		buf << m_destructorStream.str(); //打印state中变量的初始化语句
		buf << "\n\t}\n\n";
		m_destructorStream.str(""); // 清空
	}//if
}

void HClusterCodeGenerator::CGWork(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	/*当前actor分配在纯CPU服务器*/
	vector<string>::iterator iter;
	Node *work = actor->contents->body->u.operBody.work;
	stringstream Outputwork("");
	buf << "\t// work\n";
	buf << "\tvoid work()\n\t{";
	CGNode(Outputwork, work, 2);
	buf << Outputwork.str();
	buf << "\n\t\tpushToken();\n";
	buf << "\n\t\tpopToken();\n";
	buf << "\t}\n\n"; // work方法结束
}

void HClusterCodeGenerator::CGWork_GPU(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	/*如果当前actor分配在GPU混合架构服务器*/
	buf << "\t// work\n";
	buf << "\tvoid work()\n\t{";
	//buf << "\n\t\tcl_kernel kernel = clCreateKernel(program,\"kernel" << actor->name << "\",NULL);\n";
	//buf << "\t\tif(kernel == NULL){\n\t\t\t";
	//buf << "cout<<\"Error: "<<actor->name<<" failed to create kernel\"<<endl;\n\t\t\texit(1);\n\t\t}\n";

	//buf << "\t\tint status;\n";
	////记录当前actor的kernel的参数索引，设置参数
	//int argIdx = 0;

	//List *inputList = (actor)->contents->decl->u.decl.type->u.operdcl.inputs;
	//List *outputList = (actor)->contents->decl->u.decl.type->u.operdcl.outputs;
	//List *stateList = (actor)->contents->body->u.operBody.state;

	//ListMarker input_maker;
	//ListMarker output_maker;
	//ListMarker state_marker;
	//Node *inputNode = NULL;
	//Node *outputNode = NULL;
	//Node *stateNode = NULL;
	//vector<FlatNode *>tmpInFlatNode = (actor)->inFlatNodes;
	//vector<int>::iterator inIter = actor->inPopWeights.begin();
	//vector<FlatNode *>tmpOutFlatNode = (actor)->outFlatNodes;
	//vector<int>::iterator outIter = actor->outPushWeights.begin();
	//assert(tmpInFlatNode.size() == ListLength(inputList));
	//assert(tmpOutFlatNode.size() == ListLength(outputList));

	//int idx = 0, inArgs = 0, outArgs = 0;
	//int hClusterNum = m_hcp->getHClusterNumOfNode((actor));
	///*处理入边*/
	//IterateList(&input_maker, inputList);
	//while (NextOnList(&input_maker, (GenericREF)&inputNode))
	//{
	//	string inputString;
	//	if (inputNode->typ == Id)
	//		inputString = inputNode->u.id.text;
	//	else if (inputNode->typ == Decl)
	//		inputString = inputNode->u.decl.name;

	//	int hClusterNumInFlatNode = m_hcp->getHClusterNumOfNode(tmpInFlatNode[idx]);

	//	/*在同一个集群节点中*/
	//	if (hClusterNum == hClusterNumInFlatNode)
	//	{
	//		string tmpName = tmpInFlatNode[idx]->name + "_" + (actor)->name;
	//		map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
	//		if (type_iter != m_stream2StructType.end())
	//		{
	//			char c[20];
	//			sprintf(c, "%d", inArgs);
	//			string obj = "objIn" + string(c);
	//			buf << "\t\tcl_mem " << obj << " = clCreateBuffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,sizeof(" << inputString << "), " << "&" << inputString << ",NULL);\n";
	//			//设置kernel的参数
	//			buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(cl_mem)" << ", &" << obj << ");\n\n";
	//			++inArgs;
	//		}//if
	//	}//if
	//	else {
	//		//不在同一个集群节点中
	//		string tmpName = tmpInFlatNode[idx]->name + "_" + (actor)->name + "_recvbuf";
	//		map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
	//		if (type_iter != m_stream2StructType.end())
	//		{
	//			char c[20];
	//			sprintf(c, "%d", inArgs);
	//			string obj = "objIn" + string(c);
	//			buf << "\t\tcl_mem " << obj << " = clCreateBuffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,sizeof(" << inputString << "), " << "&" << inputString << ",NULL);\n";
	//			//设置kernel的参数
	//			buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(cl_mem)" << ", &" << obj << ");\n\n";
	//			++inArgs;
	//		}//if
	//	}//else
	//	++idx;
	//	++inIter;
	//}//while

	// //处理出边
	//idx = 0;
	//IterateList(&output_maker, outputList);
	//while (NextOnList(&output_maker, (GenericREF)&outputNode))
	//{
	//	string outputString;
	//	if (outputNode->typ == Id)outputString = outputNode->u.id.text;
	//	else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
	//	int hClusterNumOutFlatNode = m_hcp->getHClusterNumOfNode(tmpOutFlatNode[idx]);
	//	if (hClusterNum == hClusterNumOutFlatNode)
	//	{//在同一个集群节点中
	//		string tmpName = (actor)->name + "_" + tmpOutFlatNode[idx]->name;
	//		map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);

	//		if (type_iter != m_stream2StructType.end())
	//		{
	//			char c[20];
	//			sprintf(c, "%d", outArgs);
	//			string obj = "objOut" + string(c);
	//			buf << "\t\tcl_mem " << obj << " = clCreateBuffer(context,CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,sizeof(" << outputString << "), " << "&" << outputString << ",NULL);\n";
	//			//设置kernel的参数					
	//			buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(cl_mem)" << ", &" << obj << ");\n\n";
	//			++outArgs;
	//		}//if
	//	}//if
	//	 /*不在同一个集群节点中*/
	//	else
	//	{
	//		string tmpName = (actor)->name + "_" + tmpOutFlatNode[idx]->name + "_sendbuf";
	//		map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
	//		if (type_iter != m_stream2StructType.end())
	//		{
	//			char c[20];
	//			sprintf(c, "%d", outArgs);
	//			string obj = "objOut" + string(c);
	//			buf << "\t\tcl_mem " << obj << " = clCreateBuffer(context,CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,sizeof(" << outputString << "), " << "&" << outputString << ",NULL);\n";
	//			//设置kernel的参数					
	//			buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(cl_mem)" << ", &" << obj << ");\n\n";
	//			++outArgs;
	//		}//if
	//	}
	//	++idx;
	//	++outIter;
	//}//while

	////将该actor对应的param变量，添加为kernel参数
	//IterateList(&state_marker, stateList);
	//while (NextOnList(&state_marker, (GenericREF)&stateNode))
	//{
	//	string stateName, stateType;

	//	if (stateNode->typ == Decl) {
	//		stateName = stateNode->u.decl.name;
	//		/*普通变量声明*/
	//		if (stateNode->u.decl.type->typ == Prim)
	//		{
	//			stateType = GetPrimDataType(stateNode->u.decl.type);
	//			buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(" << stateType << "), &" << stateName << ");\n\n";
	//		}//if
	//		/*数组变量*/
	//		else if (stateNode->u.decl.type->typ == Adcl)
	//		{
	//			/*二维数组*/
	//			if (stateNode->u.decl.type->u.adcl.type->typ == Adcl) {
	//				string stateType = GetArrayDataType(stateNode->u.decl.type);
	//				stringstream sDim1, sDim2;
	//				int dim1, dim2;
	//				sDim1<< GetArrayDim(stateNode->u.decl.type->u.adcl.dim);
	//				sDim2<< GetArrayDim(stateNode->u.decl.type->u.adcl.type->u.adcl.dim);
	//				sDim1 >> dim1;
	//				sDim2 >> dim2;
	//				buf << "\t\t" << stateType << " _" << stateName << "[" << dim1*dim2 << "];\n";
	//				buf << "\t\tfor(int i=0;i<" << dim1*dim2 << ";++i){\n";
	//				buf << "\t\t\t_" << stateName << "[i] = " << stateName << "[i/" << dim2 << "][i%" << dim2 << "];\n\t\t}\n";
	//				buf << "\t\tcl_mem " << stateName << "_buf = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(" << stateType << ")*" << dim1 << "*" << dim2 << ", _" << stateName << ",NULL);\n";

	//				buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(cl_mem) , &" << stateName << "_buf);\n\n";
	//			}//if
	//			else {
	//				stringstream sDim1;
	//				int dim1;
	//				sDim1 << GetArrayDim(stateNode->u.decl.type->u.adcl.dim);
	//				sDim1 >> dim1;
	//				string stateType = GetArrayDataType(stateNode->u.decl.type);
	//				
	//				buf << "\t\tcl_mem " << stateName << "_buf = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(" << stateType << ")*" << dim1 << ", " << stateName << ",NULL);\n";

	//				buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(cl_mem) , &" << stateName << "_buf);\n\n";				}
	//		}//elif
	//		
	//	}//elif

	//}//while

	////将该actor对应的全局变量，添加为kernel参数
	//assert(m_globalVarName.size() == m_globalVarInfo.size() && m_globalVarInfo.size() == m_globalVarType.size());
	//if (!m_globalVarName.empty())
	//{
	//	for (vector<map<string, map<string, string>>>::iterator infoIter = m_globalVarInfo.begin(); infoIter != m_globalVarInfo.end(); ++infoIter)
	//	{
	//		for (map<string, map<string, string>>::iterator varIter = (*infoIter).begin(); varIter != (*infoIter).end(); ++varIter)
	//		{
	//			if (judgeKernelParameters(actor,varIter->first) && varIter->second.size() == 1)
	//			{
	//				stringstream sDim1, sDim2;
	//				int dim1, dim2;
	//				sDim1 << (varIter->second).begin()->first;
	//				sDim2 << (varIter->second).begin()->second;
	//				sDim1 >> dim1;
	//				sDim2 >> dim2;

	//				string varName = varIter->first;
	//				assert(varName == m_globalVarName[infoIter - m_globalVarInfo.begin()]);
	//				string varType = m_globalVarType[infoIter - m_globalVarInfo.begin()];
	//				
	//				//普通变量
	//				if (dim1 == 0)
	//				{
	//					buf << "\t\t" << varType << " _" << varName << ";\n";
	//					buf << "\t\t_" << varName << " = " << varName << ";\n";
	//					buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(" << varType << "), &_" << varName << ");\n\n";
	//				}//if
	//				else {
	//					/*一维数组*/
	//					if (dim2 == 0)
	//					{
	//						//buf << "\t\tcl_mem " << varName << " = clCreateBuffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,sizeof(" << varType << ")*"<<dim1<<", " << "&" << varName << ",NULL);\n";
	//						buf << "\t\t" << varType << " _" << varName << "[" << dim1 << "];\n";
	//						buf << "\t\tfor(int i=0;i<" << dim1 << ";i++){\n";
	//						buf << "\t\t\t_" << varName << "[i] = " << varName << "[i];\n\t\t}\n";
	//						//设置kernel的参数
	//						buf << "\t\tcl_mem " << varName << "_buf = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(" << varType << ")*" << dim1 << ", _" << varName << ",NULL);\n";

	//						buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(cl_mem) , &" << varName << "_buf);\n\n";
	//					}//if
	//					//二维数组
	//					else {
	//						//buf << "\t\tcl_mem " << varName << " = clCreateBuffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,sizeof(" << varType << ")*"<<dim1<<"*"<<dim2<<" , " << "&" << varName << ",NULL);\n";
	//						buf << "\t\t" << varType << " _" << varName << "[" << dim1 * dim2 << "];\n";
	//						buf << "\t\tfor(int i=0;i<" << dim1*dim2 << ";++i){\n";
	//						buf << "\t\t\t_" << varName << "[i] = " << varName << "[i/" << dim2 << "][i%" << dim2 << "];\n\t\t}\n";
	//						//设置kernel的参数
	//						buf << "\t\tcl_mem " << varName << "_buf = clCreateBuffer(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(" << varType << ")*" << dim1 << "*" << dim2 << ", _" << varName << ",NULL);\n";

	//						buf << "\t\tstatus = clSetKernelArg(kernel," << argIdx++ << ", sizeof(cl_mem) , &" << varName << "_buf);\n\n";
	//					}//else
	//				}//else

	//			}//if
	//		
	//		}//for
	//	}//for
	//}//if

	//
	//int inputSize = 0;
	//IterateList(&input_maker, inputList);
	//vector<int>::iterator iter = actor->inPopWeights.begin();
	//while (NextOnList(&input_maker, (GenericREF)&inputNode))
	//{
	//	if (*iter == 0) continue;//pop为0 不生成update代码  20130119
	//	inputSize += (*iter);
	//	iter++;
	//}//while

	//int gpuIdx = m_hcp->getHClusterGpuNum(actor).second;
	//assert(gpuIdx >= 0 && gpuIdx < 3);
	//buf << "\t\t// 将一个kernel 放入 command queue\n";
	////设置kernel的工作项数
	//buf << "\t\tsize_t global_work_size[] = { "<<inputSize<<" };\n";

	//buf << "\t\tsize_t local_work_size[] = { 1 }; \n";
	//buf << "\t\t//size_t globalNum = 1, localNum = 1;\n";
	//buf << "\t\tstatus = clEnqueueNDRangeKernel(cQueue["<< gpuIdx<<"], kernel,1, NULL, NULL, NULL , 0,NULL, NULL); \n";
	//buf << "\t\tif(status != CL_SUCCESS){\n\t\t\t";
	//buf << "cout<<endl<<\"Error: \"<<\"" << actor->name << " clEnqueueNDRangeKernel failed! status = \"<<status<<endl;\n\t\t\texit(1);\n\t\t}\n";

	//buf << "\t\tstatus = clEnqueueTask(cQueue["<< gpuIdx<<"], kernel,0, NULL, NULL); \n";
	//buf << "\t\tif(status != CL_SUCCESS){\n\t\t\t";
	//buf << "cout<<endl<<\"Error: \"<<\""<<actor->name<<" clEnqueueTask failed! status = \"<<status<<endl;\n\t\t\texit(1);\n\t\t}\n";

	
	//buf << "\t\t// 确认 command queue 中所有命令都执行完毕\n";
	//buf << "\t\tstatus = clFinish(cQueue[" << gpuIdx << "]);\n";
	//buf << "\t\tif(status != CL_SUCCESS){\n\t\t\t";
	//buf << "cout<<endl<<\"Error: \"<<\"" << actor->name <<" Enqueueing kernel \"<<status<<endl;\n\t\t\texit(1);\n\t\t}\n";
	
	//buf << "\n\t\t// 结果读回Host\n";
	//idx = 0;
	//outArgs = 0;
	//IterateList(&output_maker, outputList);
	//outIter = actor->outPushWeights.begin();
	//while (NextOnList(&output_maker, (GenericREF)&outputNode))
	//{
	//	string outputString;
	//	if (outputNode->typ == Id)outputString = outputNode->u.id.text;
	//	else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
	//	int hClusterNumOutFlatNode = m_hcp->getHClusterNumOfNode(tmpOutFlatNode[idx]);
	//	if (hClusterNum == hClusterNumOutFlatNode)
	//	{//在同一个集群节点中
	//		string tmpName = (actor)->name + "_" + tmpOutFlatNode[idx]->name;
	//		map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);

	//		if (type_iter != m_stream2StructType.end())
	//		{
	//			char c[20];
	//			sprintf(c, "%d", outArgs);
	//			string obj = "objOut" + string(c);
	//			buf << "\t\tclEnqueueReadBuffer(cQueue[" << gpuIdx << "], " << obj << ", CL_TRUE, 0 , sizeof(" << outputString << "), &" << outputString << ", 0, NULL, NULL);\n";
	//			buf << "\t\tclReleaseMemObject(" << obj << ");\n\n";
	//			++outArgs;
	//		}//if
	//	}//if
	//	 /*不在同一个集群节点中*/
	//	else
	//	{
	//		string tmpName = (actor)->name + "_" + tmpOutFlatNode[idx]->name + "_sendbuf";
	//		map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
	//		if (type_iter != m_stream2StructType.end())
	//		{
	//			char c[20];
	//			sprintf(c, "%d", outArgs);
	//			string obj = "objOut" + string(c);
	//			buf << "\t\tstatus = clEnququqReadBuffer(cQueue[" << gpuIdx << "], " << obj << ", CL_TRUE, 0 , sizeof(" << outputString << "), " << outputString << ", 0, NULL, NULL);\n";
	//			buf << "\t\tif(status != CL_SUCCESS){\n\t\t\t";
	//			buf << "cout<<\"Error: Reading buffer\"<<endl;\n\t\t\texit(1);\n\t\t}\n";

	//			buf << "\t\tclReleaseMemObject(" << obj << ");\n\n";
	//			++outArgs;
	//		}//if
	//	}
	//	++idx;
	//	++outIter;
	//}//while

	//为kernel函数添加函数体
	Node *work = actor->contents->body->u.operBody.work;
	stringstream Outputwork;
	CGNode(Outputwork, work, 2);
	/*保存原始work函数体*/
	string kernelString = Outputwork.str();
	buf << kernelString;

	//buf << "\n\t\t//处理入边,释放存储\n";
	//idx = 0;
	//inArgs = 0;
	//IterateList(&input_maker, inputList);
	//inIter = actor->inPopWeights.begin();	
	//while (NextOnList(&input_maker, (GenericREF)&inputNode))
	//{
	//	string inputString;
	//	if (inputNode->typ == Id)
	//		inputString = inputNode->u.id.text;
	//	else if (inputNode->typ == Decl)
	//		inputString = inputNode->u.decl.name;

	//	int hClusterNumInFlatNode = m_hcp->getHClusterNumOfNode(tmpInFlatNode[idx]);

	//	/*在同一个集群节点中*/
	//	if (hClusterNum == hClusterNumInFlatNode)
	//	{
	//		string tmpName = tmpInFlatNode[idx]->name + "_" + (actor)->name;
	//		map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
	//		if (type_iter != m_stream2StructType.end())
	//		{
	//			char c[20];
	//			sprintf(c, "%d", inArgs);
	//			string obj = "objIn" + string(c);
	//			buf << "\t\tclReleaseMemObject(" << obj << ");\n\n";
	//			++inArgs;
	//		}//if
	//	}//if
	//	else {
	//		//不在同一个集群节点中
	//		string tmpName = tmpInFlatNode[idx]->name + "_" + (actor)->name + "_recvbuf";
	//		map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
	//		if (type_iter != m_stream2StructType.end())
	//		{
	//			char c[20];
	//			sprintf(c, "%d", inArgs);
	//			string obj = "objIn" + string(c);
	//			buf << "\t\tclReleaseMemObject(" << obj << ");\n\n";
	//			++inArgs;
	//		}//if
	//	}//else
	//	++idx;
	//	++inIter;
	//}//while

	//IterateList(&state_marker, stateList);
	//while (NextOnList(&state_marker, (GenericREF)&stateNode))
	//{
	//	string stateName, stateType;

	//	if (stateNode->typ == Decl) {
	//		stateName = stateNode->u.decl.name;
	//		if (stateNode->u.decl.type->typ == Adcl)
	//		{
	//			buf << "\t\tclReleaseMemObject(" << stateName << "_buf);\n\n";
	//		}//elif

	//	}//elif
	//}//while
	//assert(m_globalVarName.size() == m_globalVarInfo.size() && m_globalVarInfo.size() == m_globalVarType.size());
	//if (!m_globalVarName.empty())
	//{
	//	for (vector<map<string, map<string, string>>>::iterator infoIter = m_globalVarInfo.begin(); infoIter != m_globalVarInfo.end(); ++infoIter)
	//	{
	//		for (map<string, map<string, string>>::iterator varIter = (*infoIter).begin(); varIter != (*infoIter).end(); ++varIter)
	//		{
	//			if (judgeKernelParameters(actor, varIter->first) &&  varIter->second.size() == 1)
	//			{
	//				stringstream sDim1, sDim2;
	//				int dim1, dim2;
	//				sDim1 << (varIter->second).begin()->first;
	//				sDim2 << (varIter->second).begin()->second;
	//				sDim1 >> dim1;
	//				sDim2 >> dim2;

	//				string varName = varIter->first;
	//				assert(varName == m_globalVarName[infoIter - m_globalVarInfo.begin()]);
	//				string varType = m_globalVarType[infoIter - m_globalVarInfo.begin()];

	//				//普通变量
	//				if (!dim1 == 0)
	//				{
	//					buf << "\t\tclReleaseMemObject(" << varName << "_buf);\n\n";	
	//				}//else

	//			}//if

	//		}//for
	//	}//for
	//}//if

	buf << "\n";
	buf << "\n\t\tpushToken();\n";
	buf << "\n\t\tpopToken();\n\n";

	//buf << "\t\tstatus = clReleaseKernel(kernel);\n";	
	
	buf << "\t}\n\n"; // work方法结束
}

void HClusterCodeGenerator::CGRunInitScheduleWork(stringstream &buf, FlatNode *actor)
{
	buf << "\t// runInitScheduleWork\n";
	buf << "\tvoid runInitScheduleWork()\n\t{\n";

	buf << "\t\tinit();\n"; // 调用init方法
	//加一次对数据的接受操作
	List *inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	ListMarker input_maker;
	Node *inputNode;
	vector<FlatNode *>tmpInFlatNode = actor->inFlatNodes;
	std::vector<int> inPopValues = actor->inPopWeights;
	assert(tmpInFlatNode.size() == ListLength(inputList));
	int index = 0;
	int hClusterNum = m_hcp->getHClusterNumOfNode(actor);
	IterateList(&input_maker, inputList);
	while (NextOnList(&input_maker, (GenericREF)&inputNode))
	{
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		int hClusterNumInFlatNode = m_hcp->getHClusterNumOfNode(tmpInFlatNode[index]);
		if (hClusterNum != hClusterNumInFlatNode)
		{//在同一个集群节点中
			string tmpName = tmpInFlatNode[index]->name + "_" + actor->name + "_recvbuf";
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			if (inPopValues[index])buf << "\t\t" << inputString << ".recv_items();\n";//在执行前提供一次接收操作，接受操作被隐藏在peek中			
		}
		++index;
	}
	
	buf << "\t\tfor(int i=0;i<initScheduleCount;i++)\n\t\t{\n";
	buf << "\t\t\twork();\n\t\t}\n\n";


	/*增加对发送数据的处理*/
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker output_maker;
	Node *outputNode;
	index = 0;
	vector<FlatNode *>tmpOutFlatNode = actor->outFlatNodes;
	vector<int > outPushValuse = actor->outPushWeights;
	assert(tmpOutFlatNode.size() == ListLength(outputList));
	IterateList(&output_maker, outputList);
	while (NextOnList(&output_maker, (GenericREF)&outputNode))
	{
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		int hClusterNumOutFlatNode = m_hcp->getHClusterNumOfNode(tmpOutFlatNode[index]);
		if (hClusterNumOutFlatNode != hClusterNum)
		{//在同一个集群节点中
			string tmpName = actor->name + "_" + tmpOutFlatNode[index]->name + "_sendbuf";
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			if (outPushValuse[index])buf << "\t\t" << outputString << ".initSchedule_flush();\n";
		}
		++index;
	}

	buf << "\t}\n\n"; // CGrunInitScheduleWork方法结束
}

void HClusterCodeGenerator::CGRunSteadyScheduleWork(stringstream &buf, FlatNode *actor)
{
	buf << "\t// CGrunSteadyScheduleWork\n";
	buf << "\tvoid runSteadyScheduleWork()\n\t{\n";

	List *inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	ListMarker input_maker;
	Node *inputNode;
	vector<FlatNode *>tmpInFlatNode = actor->inFlatNodes;
	std::vector<int> inPopValues = actor->inPopWeights;
	assert(tmpInFlatNode.size() == ListLength(inputList));
	int index = 0;
	int hClusterNum = m_hcp->getHClusterNumOfNode(actor);
	IterateList(&input_maker, inputList);
	while (NextOnList(&input_maker, (GenericREF)&inputNode))
	{
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;
		int hClusterNumInFlatNode = m_hcp->getHClusterNumOfNode(tmpInFlatNode[index]);
		if (hClusterNumInFlatNode != hClusterNum)
		{//在同一个集群节点中
			string tmpName = tmpInFlatNode[index]->name + "_" + actor->name + "_recvbuf";
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
			if (inPopValues[index])buf << "\t\t" << inputString << ".recv_items();\n";
		}
		++index;
	}
	
	buf << "\t\tfor(int i=0;i<steadyScheduleCount;i++)\n\t\t{\n";
	buf << "\t\t\twork();\n\t\t}\n\n";
	
	
	//加一次对数据的接受操作
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	ListMarker output_maker;
	Node *outputNode;
	index = 0;
	vector<FlatNode *>tmpOutFlatNode = actor->outFlatNodes;
	IterateList(&output_maker, outputList);
	while (NextOnList(&output_maker, (GenericREF)&outputNode))
	{
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
		int hClusterNumOutFlatNode = m_hcp->getHClusterNumOfNode(tmpOutFlatNode[index]);
		if (hClusterNumOutFlatNode != hClusterNum)
		{//在同一个集群节点中
			string tmpName = actor->name + "_" + tmpOutFlatNode[index]->name + "_sendbuf";
			map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
		}
		++index;
	}

	buf << "\t}\n\n"; // CGrunSteadyScheduleWork方法结束
}


void HClusterCodeGenerator::CGFlush(stringstream &buf, FlatNode *actor)
{//创建生产者的最后一次flush操作(在单个actor内)
	buf << "\t// flush\n";
	buf << "\tvoid flush()\n\t{\n";
	int hClusterNum = m_hcp->getHClusterNumOfNode(actor);//取当前节点所在的cluster号
															 //如果当前actor没有与其他的cluster中的actor通信，次actor的flush函数为空
	ListMarker output_maker;
	Node *outputNode = NULL;
	string outputString;
	int i = 0;
	//取每个actor的下端节点并判断其是否在同一个cluster内
	vector<FlatNode *>outputFlatNodes = actor->outFlatNodes;
	vector<int> outPushValuse = actor->outPushWeights;
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
	assert(ListLength(outputList) == outputFlatNodes.size());
	IterateList(&output_maker, outputList);
	while (NextOnList(&output_maker, (GenericREF)&outputNode) && i != outputFlatNodes.size())
	{
		int _tmpcluster = m_hcp->getHClusterNumOfNode(outputFlatNodes[i]);//取其所在的集群号
		if (_tmpcluster != hClusterNum)
		{//需要flush操作
			if (outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
			if (outPushValuse[i])buf << "\t\t" << outputString + ".flush();\n";
		}
		i++;
	}
	buf << "\t}\n\n"; // flush方法结束
}

void HClusterCodeGenerator::CGAllKernel()
{
	if (m_kernelNodes.empty())
		return;

	vector<FlatNode *> flatNodes = m_kernelNodes;
	int size = flatNodes.size();

	stringstream buf;
	buf << "#ifdef cl_khr_fp64\n";
	buf << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";
	buf << "#elif defined(cl_amd_fp64)\n";
	buf << "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n";
	buf << "#else\n";
	buf << "#error \"Double precision floating point not supported by OpenCL implementation.\"\n";
	buf << "#endif\n\n";

	for (map<string, string>::iterator iter1 = m_struct2StructFields.begin(); iter1 != m_struct2StructFields.end(); ++iter1)
	{
		buf << "typedef " << "struct " << "\n{\t" << iter1->first << "}" << iter1->second << ";\n";
	}//for

	/*为每个混合架构服务器上的actor生成kernel函数*/
	for (vector<FlatNode*>::iterator iter = flatNodes.begin(); iter != flatNodes.end(); ++iter)
	{
		buf << "\n__kernel void kernel" << (*iter)->name << "(";
		//为kernel函数添加参数
		List *inputList = (*iter)->contents->decl->u.decl.type->u.operdcl.inputs;
		List *outputList = (*iter)->contents->decl->u.decl.type->u.operdcl.outputs;
		List *stateList = (*iter)->contents->body->u.operBody.state;
		ListMarker input_maker;
		ListMarker output_maker;
		ListMarker state_marker;
		Node *inputNode = NULL;
		Node *outputNode = NULL;
		Node *stateNode = NULL;
		vector<FlatNode *>tmpInFlatNode = (*iter)->inFlatNodes;
		vector<FlatNode *>tmpOutFlatNode = (*iter)->outFlatNodes;
		assert(tmpInFlatNode.size() == ListLength(inputList));
		assert(tmpOutFlatNode.size() == ListLength(outputList));

		int idx = 0;
		int hClusterNum = m_hcp->getHClusterNumOfNode((*iter));
		/*处理入边*/
		IterateList(&input_maker, inputList);
		while (NextOnList(&input_maker, (GenericREF)&inputNode))
		{
			string inputString;
			if (inputNode->typ == Id)inputString = inputNode->u.id.text;
			else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;

			int hClusterNumInFlatNode = m_hcp->getHClusterNumOfNode(tmpInFlatNode[idx]);

			/*在同一个集群节点中*/
			if (hClusterNum == hClusterNumInFlatNode)
			{
				string tmpName = tmpInFlatNode[idx]->name + "_" + (*iter)->name;
				map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
				if (type_iter != m_stream2StructType.end())
					buf << "__global " << type_iter->second << "* " << inputString << ", ";
			}//if
			 /*不在同一个集群节点中*/
			else {
				//不在同一个集群节点中
				string tmpName = tmpInFlatNode[idx]->name + "_" + (*iter)->name + "_recvbuf";
				map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
				if (type_iter != m_stream2StructType.end())
					buf << "__global " << type_iter->second << "* " << inputString << ", ";
			}//else
			++idx;
		}//while

		 //处理出边
		idx = 0;
		IterateList(&output_maker, outputList);
		while (NextOnList(&output_maker, (GenericREF)&outputNode))
		{
			string outputString;
			if (outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
			int hClusterNumOutFlatNode = m_hcp->getHClusterNumOfNode(tmpOutFlatNode[idx]);
			if (hClusterNum == hClusterNumOutFlatNode)
			{//在同一个集群节点中
				string tmpName = (*iter)->name + "_" + tmpOutFlatNode[idx]->name;
				map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);

				if (type_iter != m_stream2StructType.end())
					buf << "__global " << type_iter->second << "* " << outputString << ", ";
			}//if
			 /*不在同一个集群节点中*/
			else
			{
				string tmpName = (*iter)->name + "_" + tmpOutFlatNode[idx]->name + "_sendbuf";
				map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
				if (type_iter != m_stream2StructType.end())
					buf << "__global " << type_iter->second << "* " << outputString << ", ";

			}
			++idx;
		}//while

		//将该actor对应的param变量，添加为kernel参数
		IterateList(&state_marker, stateList);
		while (NextOnList(&state_marker, (GenericREF)&stateNode))
		{
			string stateName, stateType;
			if (stateNode->typ == Decl) {
				stateName = stateNode->u.decl.name;
				/*普通变量声明*/
				if (stateNode->u.decl.type->typ == Prim)
				{
					stateType = GetPrimDataType(stateNode->u.decl.type);
					buf /*<< "__global " */<< stateType << /*"* _"*/" " << stateName << ", ";
				}//if
				 /*数组变量*/
				else if (stateNode->u.decl.type->typ == Adcl)
				{
					/*二维数组*/
					if (stateNode->u.decl.type->u.adcl.type->typ == Adcl) {
						string stateType = GetArrayDataType(stateNode->u.decl.type);
						buf << "__global " << stateType << "* _" << stateName << ", ";
					}//if
					else {
						string stateType = GetArrayDataType(stateNode->u.decl.type);
						buf << "__global " << stateType << "* " << stateName << ", ";
					}
				}//elif

			}//elif
		}//while

		//将该actor对应的全局变量，添加为kernel参数
		assert(m_globalVarName.size() == m_globalVarInfo.size() && m_globalVarInfo.size() == m_globalVarType.size());
		if (!m_globalVarName.empty())
		{
			for (vector<map<string, map<string, string>>>::iterator infoIter = m_globalVarInfo.begin(); infoIter != m_globalVarInfo.end(); ++infoIter)
			{
				for (map<string, map<string, string>>::iterator varIter = (*infoIter).begin(); varIter != (*infoIter).end(); ++varIter)
				{
					if (judgeKernelParameters(*iter, varIter->first) && varIter->second.size() == 1)
					{
						stringstream sDim1, sDim2;
						int dim1, dim2;
						sDim1 << (varIter->second).begin()->first;
						sDim2 << (varIter->second).begin()->second;
						sDim1 >> dim1;
						sDim2 >> dim2;

						string varName = varIter->first;
						assert(varName == m_globalVarName[infoIter - m_globalVarInfo.begin()]);
						string varType = m_globalVarType[infoIter - m_globalVarInfo.begin()];

						//普通变量
						if (dim1 == 0)
						{
							buf /*<< "__global "*/ << varType << /*"* _"*/" " << varName << ", ";
						}//if
						else {
							/*一维数组*/
							if (dim2 == 0)
							{
								//设置kernel的参数
								buf << "__global " << varType << "* " << varName << ", ";
							}//if
							 //二维数组
							else {
								//设置kernel的参数
								buf << "__global " << varType << "* _" << varName << ", ";
							}//else
						}//else

					}//if

				}//for
			}//for
		}//if

		//////////////////////////////////////////////////////////////
		buf.seekp((int)buf.tellp() - 2);
		buf << ")\n";

		buf << "{\n";
		
		
		if ((*iter)->outFlatNodes.size() > 0)
		{
			buf << "\tint id = get_global_id(0);\n";
		}

		/*work函数体映射至kernel函数体所需外部常量定义*/
		IterateList(&state_marker, stateList);
		while (NextOnList(&state_marker, (GenericREF)&stateNode))
		{
			string stateName, stateType;
			if (stateNode->typ == Decl) {
				stateName = stateNode->u.decl.name;
				///*普通变量声明*/
				//if (stateNode->u.decl.type->typ == Prim)
				//{
				//	stateType = GetPrimDataType(stateNode->u.decl.type);
				//	buf << "\t" << stateType << " " << stateName << ";\n";
				//	buf << "\t" << stateName << " = _" << stateName << "[0]; \n";
				//}//if
				 /*数组变量*/
				/*else */if (stateNode->u.decl.type->typ == Adcl)
				{
					/*二维数组*/
					if (stateNode->u.decl.type->u.adcl.type->typ == Adcl) {
						string stateType = GetArrayDataType(stateNode->u.decl.type);
						stringstream sDim1, sDim2;
						int dim1, dim2;
						sDim1 << GetArrayDim(stateNode->u.decl.type->u.adcl.dim);
						sDim2 << GetArrayDim(stateNode->u.decl.type->u.adcl.type->u.adcl.dim);
						sDim1 >> dim1;
						sDim2 >> dim2;
						buf << "\t" << stateType << " " << stateName << "[" << dim1 << "][" << dim2 << "];\n";
						buf << "\tfor(int i=0;i<" << dim1 << ";++i){\n";
						buf << "\t\tfor(int j=0;j<" << dim2 << ";++j){\n";
						buf << "\t\t\t" << stateName << "[i][j] = _" << stateName << "[i*" << dim1 << "+j];\n\t\t}\n\t}\n";

					}//if
				}//elif
			}//if
		}//while
		
		 //该actor对应的全局二维数组变量重新赋值
		assert(m_globalVarName.size() == m_globalVarInfo.size() && m_globalVarInfo.size() == m_globalVarType.size());
		if (!m_globalVarName.empty())
		{
			Node *work = (*iter)->contents->body->u.operBody.work;
			stringstream Outputwork;
			CGNode(Outputwork, work, 2);
			/*保存原始work函数体*/
			string kernelString = Outputwork.str();

			for (vector<map<string, map<string, string>>>::iterator infoIter = m_globalVarInfo.begin(); infoIter != m_globalVarInfo.end(); ++infoIter)
			{
				for (map<string, map<string, string>>::iterator varIter = (*infoIter).begin(); varIter != (*infoIter).end(); ++varIter)
				{
					if (judgeKernelParameters(*iter, varIter->first) && varIter->second.size() == 1)
					{
						stringstream sDim1, sDim2;
						int dim1, dim2;
						sDim1 << (varIter->second).begin()->first;
						sDim2 << (varIter->second).begin()->second;
						sDim1 >> dim1;
						sDim2 >> dim2;

						string varName = varIter->first;
						assert(varName == m_globalVarName[infoIter - m_globalVarInfo.begin()]);
						string varType = m_globalVarType[infoIter - m_globalVarInfo.begin()];

						////普通变量
						//if (dim1 == 0)
						//{
						//	buf << "\t" << varType << " " << varName << ";\n";
						//	buf << "\t" << varName << " = _" << varName << "[0]; \n";
						//}//if
						//else
						{
							/*二维数组*/
							if (dim2 != 0)
							{
								buf << "\t" << varType << " " << varName << "[" << dim1 << "][" << dim2 << "];\n";
								buf << "\tfor(int i=0;i<" << dim1 << ";++i){\n";
								buf << "\t\tfor(int j=0;j<" << dim2 << ";++j){\n";
								buf << "\t\t\t" << varName << "[i][j] = _" << varName << "[i*" << dim1 << "+j];\n\t\t}\n\t}\n";

							}//if
						}//else
					}//if
				}//for
			}//for
		}//if



		//为kernel函数添加函数体
		Node *work = (*iter)->contents->body->u.operBody.work;
		stringstream Outputwork;
		CGNode(Outputwork, work, 2);
		/*保存原始work函数体*/
		string kernelString = Outputwork.str();

		//处理work函数体，添加global id
		idx = 0;
		IterateList(&input_maker, inputList);
		while (NextOnList(&input_maker, (GenericREF)&inputNode))
		{
			string inputString;
			if (inputNode->typ == Id)inputString = inputNode->u.id.text;
			else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;

			int hClusterNumInFlatNode = m_hcp->getHClusterNumOfNode(tmpInFlatNode[idx]);

			/*在同一个集群节点中*/
			if (hClusterNum == hClusterNumInFlatNode)
			{
				string tmpName = tmpInFlatNode[idx]->name + "_" + (*iter)->name;
				map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
				if (type_iter != m_stream2StructType.end())
				{
					int pos1 = 0, pos2 = 0, pos3 = 0;
					string searchString = inputString + "[";
					while ((pos1 = kernelString.find(searchString, pos2)) != -1)
					{
						if ((pos1 == 0 && !((kernelString[pos1 + inputString.length()] >= 65 && kernelString[pos1 + inputString.length()] <= 90) || (kernelString[pos1 + inputString.length()] >= 97 && kernelString[pos1 + inputString.length()] <= 122)))||
							(pos1 != 0 && !((kernelString[pos1 - 1] >= 65 && kernelString[pos1 - 1] <= 90) || (kernelString[pos1 - 1] >= 97 && kernelString[pos1 - 1] <= 122))))
						{	
							string subStr;
							//处理括号匹配的问题
							stack<string> tmpStk;
							pos2 = kernelString.find("[", pos1);
							tmpStk.push("[");
							int tmpStartPos = pos2 + 1, tmpPosWhere;
							string findString = "[]";
							while ((tmpPosWhere = kernelString.find_first_of(findString, tmpStartPos)) != -1)
							{
								if (kernelString[tmpPosWhere] == '[')
								{
									tmpStk.push("[");
								}
								else {
									tmpStk.pop();
								}

								if (tmpStk.empty())
								{
									break;
								}

								tmpStartPos = tmpPosWhere + 1;
							}//while
							pos3 = tmpPosWhere;
							subStr = kernelString.substr(pos2 + 1, pos3 - pos2 - 1);

							subStr = subStr + " + (id)";

							kernelString.replace(pos2 + 1, pos3 - pos2 - 1, subStr);
							
							pos2 = pos3;
						}//if
						else {
							pos2 = pos1 + 1;
						}
					}//while				
				}//if
			}//if
			 /*不在同一个集群节点中*/
			else {
				//不在同一个集群节点中
				string tmpName = tmpInFlatNode[idx]->name + "_" + (*iter)->name + "_recvbuf";
				map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
				if (type_iter != m_stream2StructType.end())
				{
					int pos1 = 0, pos2 = 0, pos3 = 0;
					string searchString = inputString + "[";
					while ((pos1 = kernelString.find(searchString, pos2)) != -1)
					{
						if ((pos1 == 0 && !((kernelString[pos1 + inputString.length()] >= 65 && kernelString[pos1 + inputString.length()] <= 90) || (kernelString[pos1 + inputString.length()] >= 97 && kernelString[pos1 + inputString.length()] <= 122))) ||
							(pos1 != 0 && !((kernelString[pos1 - 1] >= 65 && kernelString[pos1 - 1] <= 90) || (kernelString[pos1 - 1] >= 97 && kernelString[pos1 - 1] <= 122))))
						{
							string subStr;
							//处理括号匹配的问题
							stack<string> tmpStk;
							pos2 = kernelString.find("[", pos1);
							tmpStk.push("[");
							int tmpStartPos = pos2 + 1, tmpPosWhere;
							string findString = "[]";
							while ((tmpPosWhere = kernelString.find_first_of(findString, tmpStartPos)) != -1)
							{
								if (kernelString[tmpPosWhere] == '[')
								{
									tmpStk.push("[");
								}
								else {
									tmpStk.pop();
								}

								if (tmpStk.empty())
								{
									break;
								}

								tmpStartPos = tmpPosWhere + 1;
							}//while
							pos3 = tmpPosWhere;
							subStr = kernelString.substr(pos2 + 1, pos3 - pos2 - 1);

							subStr = subStr + " + (id)";

							kernelString.replace(pos2 + 1, pos3 - pos2 - 1, subStr);

							pos2 = pos3;
						}//if
						else {
							pos2 = pos1 + 1;
						}
					}//while			
				}//if
			}//else
			++idx;
		}//while

		 //处理出边
		idx = 0;
		IterateList(&output_maker, outputList);
		while (NextOnList(&output_maker, (GenericREF)&outputNode))
		{
			string outputString;
			if (outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl) outputString = outputNode->u.decl.name;
			int hClusterNumOutFlatNode = m_hcp->getHClusterNumOfNode(tmpOutFlatNode[idx]);
			if (hClusterNum == hClusterNumOutFlatNode)
			{//在同一个集群节点中
				string tmpName = (*iter)->name + "_" + tmpOutFlatNode[idx]->name;
				map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);

				if (type_iter != m_stream2StructType.end())
				{
					int pos1 = 0, pos2 = 0, pos3 = 0;
					string searchString = outputString + "[";
					while ((pos1 = kernelString.find(searchString, pos2)) != -1)
					{
						if ((pos1 == 0 && !((kernelString[pos1 + outputString.length()] >= 65 && kernelString[pos1 + outputString.length()] <= 90) || (kernelString[pos1 + outputString.length()] >= 97 && kernelString[pos1 + outputString.length()] <= 122)))
							|| (pos1 != 0 && !((kernelString[pos1 - 1] >= 65 && kernelString[pos1 - 1] <= 90) || (kernelString[pos1 - 1] >= 97 && kernelString[pos1 - 1] <= 122))))
						{
							string subStr;
							//处理括号匹配的问题
							stack<string> tmpStk;
							pos2 = kernelString.find("[", pos1);
							tmpStk.push("[");
							int tmpStartPos = pos2 + 1, tmpPosWhere;
							string findString = "[]";
							while ((tmpPosWhere = kernelString.find_first_of(findString, tmpStartPos)) != -1)
							{
								if (kernelString[tmpPosWhere] == '[')
								{
									tmpStk.push("[");
								}
								else {
									tmpStk.pop();
								}

								if (tmpStk.empty())
								{
									break;
								}

								tmpStartPos = tmpPosWhere + 1;
							}//while
							pos3 = tmpPosWhere;
							subStr = kernelString.substr(pos2 + 1, pos3 - pos2 - 1);

							subStr = subStr + " + (id)";

							kernelString.replace(pos2 + 1, pos3 - pos2 - 1, subStr);

							pos2 = pos3;
						}//if
						else {
							pos2 = pos1 + 1;
						}
					}//while
				}//if
			}//if
			 /*不在同一个集群节点中*/
			else
			{
				string tmpName = (*iter)->name + "_" + tmpOutFlatNode[idx]->name + "_sendbuf";
				map<string, string>::iterator type_iter = m_stream2StructType.find(tmpName);
				if (type_iter != m_stream2StructType.end())
				{
					int pos1 = 0, pos2 = 0, pos3 = 0;
					string searchString = outputString + "[";
					while ((pos1 = kernelString.find(searchString, pos2)) != -1)
					{
						if ((pos1 == 0 && !((kernelString[pos1 + outputString.length()] >= 65 && kernelString[pos1 + outputString.length()] <= 90) || (kernelString[pos1 + outputString.length()] >= 97 && kernelString[pos1 + outputString.length()] <= 122)))
							|| (pos1 != 0 && !((kernelString[pos1 - 1] >= 65 && kernelString[pos1 - 1] <= 90) || (kernelString[pos1 - 1] >= 97 && kernelString[pos1 - 1] <= 122))))
						{
							string subStr;
							//处理括号匹配的问题
							stack<string> tmpStk;
							pos2 = kernelString.find("[", pos1);
							tmpStk.push("[");
							int tmpStartPos = pos2 + 1, tmpPosWhere;
							string findString = "[]";
							while ((tmpPosWhere = kernelString.find_first_of(findString, tmpStartPos)) != -1)
							{
								if (kernelString[tmpPosWhere] == '[')
								{
									tmpStk.push("[");
								}
								else {
									tmpStk.pop();
								}

								if (tmpStk.empty())
								{
									break;
								}

								tmpStartPos = tmpPosWhere + 1;
							}//while
							pos3 = tmpPosWhere;
							subStr = kernelString.substr(pos2 + 1, pos3 - pos2 - 1);

							subStr = subStr + " + (id)";

							kernelString.replace(pos2 + 1, pos3 - pos2 - 1, subStr);

							pos2 = pos3;
						}//if
						else {
							pos2 = pos1 + 1;
						}
					}//while
				
				}//if
			}//else
			++idx;
		}//while

		//输出新work函数体
		buf << kernelString;
		//kernel函数结束
		buf << "\n}\n";
	}//for

	/*将OpenCL的kernel函数写入文件*/
	stringstream fileName;
	fileName << m_curDir << "allKernel.cl";
	outputToFile(fileName.str(), buf.str());
}

void HClusterCodeGenerator::CGMachineFile()
{
	stringstream buf;
	buf << "dell-4:1\n";
	buf << "dell-3:1\n";
	buf << "amax:1\n";
	stringstream fileName;
	fileName << m_curDir << "mf";
	outputToFile(fileName.str(), buf.str());
}


/*确定节点间通信边信息*/
void HClusterCodeGenerator::createStream2StructTypeMap()
{
	/*构造Stream与struct type的map*/

	//取所有边的field，并将各个域拼接成字符串存在一个临时map
	multimap<string, string> tmpFields2StreamName;
	map<string, List *> tmpStreamName2FieldList;

	string edgeName;
	for (int i = 0; i < m_hcp->getHClusters(); ++i)
	{
		vector<FlatNode *> tmpFlatNodes = m_hcp->getSubFlatNodes(i);

		//对当前服务器节点上的所有actor遍历
		for (vector<FlatNode*>::iterator iter1 = tmpFlatNodes.begin(); iter1 != tmpFlatNodes.end(); ++iter1)
		{
			for (vector<FlatNode*>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); ++iter2)
			{
				Node *outputNode = NULL;
				List *outputList = (*iter1)->contents->decl->u.decl.type->u.operdcl.outputs;
				ListMarker output_maker;
				StreamType *streamType = NULL;
	 			IterateList(&output_maker, outputList);

				while (NextOnList(&output_maker, (GenericREF)&outputNode))
				{
					if(outputNode->typ == Id)
						streamType = outputNode->u.id.decl->u.decl.type->u.strdcl.type;
					else if (outputNode->typ == Decl) 
						streamType = outputNode->u.decl.type->u.strdcl.type;
				}//while

				List *streamField = streamType->fields;
				string fieldsString = "";
				ListMarker streamField_maker;

				IterateList(&streamField_maker, streamField);

				Node *fieldNode = NULL;
				while (NextOnList(&streamField_maker, (GenericREF)&fieldNode))
				{
					//将各个field的类型和名称取出来，拼成一个string
					fieldsString += NodeDeclToString(fieldNode);
				}//while

				/*判断在同一台机器上*/
				if (m_hcp->getHClusterNumOfNode(*iter2) == i)
				{		
					edgeName = (*iter1)->name + "_" + (*iter2)->name;
					tmpStreamName2FieldList.insert(make_pair(edgeName, streamField));
					tmpFields2StreamName.insert(make_pair(fieldsString, edgeName));

					m_actor2InEdge.insert(make_pair(*iter2, edgeName));
					m_actor2OutEdge.insert(make_pair(*iter1, edgeName));
				}//if
				else {
					//在不同的机器上，创建发送缓存，iter1是生产者，iter2是消费者
					edgeName = (*iter1)->name + "_" + (*iter2)->name + "_sendbuf";
					tmpStreamName2FieldList.insert(make_pair(edgeName, streamField));
					tmpFields2StreamName.insert(make_pair(fieldsString, edgeName));

					//生产者与输出边
					m_actor2OutEdge.insert(make_pair(*iter1, edgeName));

					edgeName = (*iter1)->name + "_" + (*iter2)->name + "_recvbuf";
					tmpStreamName2FieldList.insert(make_pair(edgeName, streamField));
					tmpFields2StreamName.insert(make_pair(fieldsString, edgeName));

					//消费者与输入边
					m_actor2InEdge.insert(make_pair(*iter2, edgeName));

					++netEdgeNum;
				}//else

			}//for
		}//for
	}//for
	int count = 0;
	string countStr;
	for (multimap<string, string>::iterator iter3 = tmpFields2StreamName.begin(); iter3 != tmpFields2StreamName.end(); ++iter3)
	{
		char *tmpCountStr = (char *)malloc(sizeof(char) * 20);
		sprintf(tmpCountStr, "%d", count);

		countStr = tmpCountStr;
		//插入成功
		if ((m_struct2StructFields.insert(make_pair(iter3->first, "struct" + countStr))).second)
		{
			++count;
		}
	}

	assert(count == m_struct2StructFields.size());
	map<string, string>::iterator iter4;
	for (multimap<string, string>::iterator iter5 = tmpFields2StreamName.begin(); iter5 != tmpFields2StreamName.end(); ++iter5)
	{
		iter4 = m_struct2StructFields.find(iter5->first);
		assert(iter4 != m_struct2StructFields.end());
		m_stream2StructType.insert(make_pair(iter5->second, iter4->second));
	}

	for (map<string, string>::iterator iter6 = m_struct2StructFields.begin(); iter6 != m_struct2StructFields.end(); ++iter6)
	{
		map<string, List*>::iterator tmpIter;
		for (map<string, string>::iterator iter7 = m_stream2StructType.begin(); iter7 != m_stream2StructType.end(); ++iter7)
		{
			if (iter6->second == iter7->second)
			{
				tmpIter = tmpStreamName2FieldList.find(iter7->first);
				assert(tmpIter != tmpStreamName2FieldList.end());

				m_struct2FieldList.insert(make_pair(iter7->second, tmpIter->second));
				break;
			}
		}
	}
}

/*收集全局信息*/
void HClusterCodeGenerator::collectGlobalInfo()
{
	ListMarker marker;
	Node *item = NULL;
	IterateList(&marker, Program);

	//将全局信息加入到globalList中
	while (NextOnList(&marker, (GenericREF)&item))
	{
		if (item->typ != Composite)
		{
			m_globalList = AppendItem(m_globalList, item);
		}
	}//while
}
//判断收集每个actor中对应的kernel参数是否需要全局变量
bool HClusterCodeGenerator::judgeKernelParameters(FlatNode *actor, string varName)
{
	if (actor == NULL)
		return false;

	if (m_globalVarName.empty())
		return false;
	else {
		Node *work = actor->contents->body->u.operBody.work;
		stringstream Outputwork;
		CGNode(Outputwork, work, 2);
		/*保存原始work函数体*/
		string kernelString = Outputwork.str();
		int pos = 0, startPos = 0;
		while ((pos = kernelString.find(varName, startPos)) != -1)
		{
			/*判断是否真正出现当前全局变量*/
			if (!((((int)kernelString[pos + (varName).length()]) >= 65 && ((int)kernelString[pos + (varName).length()]) <= 90) || (((int)kernelString[pos + (varName).length()]) >= 97 && ((int)kernelString[pos + (varName).length()]) <= 122)) && !((((int)kernelString[pos - 1]) >= 65 && ((int)kernelString[pos - 1]) <= 90) || (((int)kernelString[pos - 1]) >= 97 && ((int)kernelString[pos - 1]) <= 122)))
				return true;
			startPos = pos + 1;
		}//while
		return false;
	}
}

int HClusterCodeGenerator::findID(FlatNode * flatnode)
{
	for (int i = 0; i != m_flatNodes.size(); i++)
		if (m_flatNodes[i] == flatnode)
			return i;
}
//string HClusterCodeGenerator::GetPrimDataType(Node * from)
//{
//	string type;
//	switch (from->u.prim.basic) {
//	case Sshort:
//	case Sint:
//		type = "int";;
//		break;
//	case Uint:
//	case Ushort:
//		type = "UInt";;
//		break;
//	case Slong:
//		type = "Long";;
//		break;
//	case Ulong:
//		type = "ULong";;
//		break;
//	case Float:
//		type = "float";
//		break;
//	case Double:
//		type = "double";
//		break;
//	case Char:
//	case Schar:
//	case Uchar:
//		type = "char";
//		break;
//	case Void:
//		type = "void";
//		break;
//	}
//	return type;
//}
//string HClusterCodeGenerator::GetArrayDataType(Node * node)
//{
//	//主要用来处理流中的单个元素中的成员（只可能是数组，或内置的简单类型）
//	string type;
//	if (node->typ == Prim) //基本类型
//	{
//		type = GetPrimDataType(node);
//	}
//	else if (node->typ == Adcl) // 也是个数组则递归查找类型
//	{
//		stringstream ss;
//		ss << GetArrayDataType(node->u.adcl.type);
//		type = ss.str();
//	}
//	else // 如果数组的成员是复杂类型，则有待扩展
//	{
//		Warning(1, "this arrayDataType can not be handle!");
//		type = "Any";// 暂时返回一种通用类型
//		UNREACHABLE;
//	}
//	return type;
//}
string HClusterCodeGenerator::NodeDeclToString(Node *node)
{
	//在数据流的结构所在的成员的类型是固定的（成员只能是Id或Array类型）
	assert(node->typ == Decl);
	string fieldStrings = "";
	stringstream tempdeclList, tempdeclInitList;
	if (node->u.decl.type->typ == Prim) // 基本类型
	{
		Node *typeNode = node->u.decl.type;
		string type = GetPrimDataType(typeNode);
		string name = node->u.decl.name;
		fieldStrings = type + " " + name + ";\n";
	}
	else if (node->u.decl.type->typ == Adcl) // 数组, 最多处理二维数组, 高维待扩展
	{
		Node *tmptype = node->u.decl.type;
		Node *tmpNode = node->u.decl.type;
		string name = node->u.decl.name;
		string arrayType = GetArrayDataType(tmpNode->u.adcl.type);
		fieldStrings = arrayType + " " + name;
		while (tmptype->typ == Adcl)
		{
			Node *tmpdim = tmptype->u.adcl.dim;
			assert(tmpdim->typ == Const);
			stringstream dimstream("");


			//CGNode(dimstream, tmpdim, 0);
			string dim = dimstream.str();
			fieldStrings += "[" + dim + "]";
		}
		fieldStrings += ";\n";
	}
	return fieldStrings;
}

string HClusterCodeGenerator::GetMPIDataType(Node *from)
{
	string mpi_type;
	assert(from != NULL && from->typ == Prim);
	switch (from->u.prim.basic) {
	case Sshort:
		mpi_type = "MPI::SHORT";
		break;
	case Sint:
		mpi_type = "MPI::INT";
		break;
	case Uint:
		mpi_type = "MPI::UNSIGNED";
		break;
	case Ushort:
		mpi_type = "MPI::UNSIGNED_SHORT";
		break;
	case Slong:
		mpi_type = "MPI::LONG";
		break;
	case Ulong:
		mpi_type = "MPI::UNSIGNED_LONG";;
		break;
	case Float:
		mpi_type = "MPI::FLOAT";
		break;
	case Double:
		mpi_type = "MPI::DOUBLE";
		break;
	case Char:
	case Schar:
		mpi_type = "MPI::CHAR";
		break;
	case Uchar:
		mpi_type = "MPI::UNSIGNED_CHAR";
		break;
	}
	return mpi_type;
}

/*向MPI提交类型*/
void HClusterCodeGenerator::CommitMPIDataType(stringstream & buf)
{
	for (map<string, List *>::iterator iter = m_struct2FieldList.begin(); iter != m_struct2FieldList.end(); iter++)
	{
		int len = ListLength(iter->second);
		buf << "\tMPI::Datatype " << iter->first << "_type[" << len << "] = {";
		ListMarker streamField_maker;
		IterateList(&streamField_maker, iter->second);
		Node *fieldNode = NULL;
		string fieldstypeString = "";
		while (NextOnList(&streamField_maker, (GenericREF)&fieldNode))
		{//将各个field的类型，拼成一个string,(暂时只处理流中的元素的成员不是数组的情况)
			fieldstypeString += GetMPIDataType(fieldNode->u.decl.type) + ",";
		}
		fieldstypeString.erase(fieldstypeString.length() - 1, 1);
		fieldstypeString += "};\n";
		buf << fieldstypeString;									
		//MPI_Datatype type1[4] = {MPI_INT, MPI_DOUBLE, MPI_CHAR};
		buf << "\tint " << iter->first << "_blocklen[" << len << "] ;\n";
		buf << "\tMPI::Aint " << iter->first << "_disp[" << len << "];\n"; //MPI_Aint disp1[4];
		for (int i = 0; i < len; i++)//int blocklen1[4] = {1, 1, 1, 1};
		{
			//流中的每个元素的类型都是一个简单的内置类型
			buf << "\t" << iter->first << "_blocklen[" << i << "]" << " = 1;\n";
		}
		Node *firstFieldNode = (Node *)FirstItem(iter->second);
		buf << "\t" << iter->first << "_disp[0] = (MPI::Aint) &" << iter->first << "_obj." << firstFieldNode->u.decl.name << ";\n";
		int count = 0;
		IterateList(&streamField_maker, iter->second);
		while (NextOnList(&streamField_maker, (GenericREF)&fieldNode))
		{
			if (count == 0) { count++; continue; }
			buf << "\t" << iter->first << "_disp[" << count << "] = (MPI_Aint &)" << iter->first << "_obj." << fieldNode->u.decl.name << " - " << iter->first << "_disp[" << count - 1 << "];\n";
		}
		buf << "\t" << iter->first << "_disp[0] = (MPI::Aint)0;\n";
		//构造MPI新类型
		buf << "\t" << iter->first << "_mpitype = MPI::Datatype::Create_struct( " << len << ", " << iter->first << "_blocklen, " << iter->first << "_disp, " << iter->first << "_type);\n";
		buf << "\t" << iter->first << "_mpitype.Commit();\n";
	}
}

int HClusterCodeGenerator::GetIORadio(FlatNode * flatNode, int edgeNo, Bool type)
{
	//取flatNode的第edgeNo边对应的输入输出码率，type表示的是对于flatNode是输入还是输出边（False表示输入，true表示输出）
	int ioRadio = 0;
	int curinitScheduleCount = m_sssg->GetInitCount(flatNode);
	int cursteadyScheduleCount = m_hcp->getSteadyCount(flatNode);
	int scheduleCount = cursteadyScheduleCount;
	if (type == TRUE)
	{
		//输入边
		int peekValue = flatNode->inPeekWeights[edgeNo] * scheduleCount;
		int popValue = flatNode->inPopWeights[edgeNo] * (scheduleCount + curinitScheduleCount);
	
		ioRadio = popValue;
	}
	else
	{//输出边
		ioRadio = flatNode->outPushWeights[edgeNo] * scheduleCount;
	}
	return ioRadio;
}

void HClusterCodeGenerator::OutputCRSpace(stringstream &out,int offset)
{
	out<<"\n";
	while (offset--) out<<"\t";
}

//输出信息到文件
void HClusterCodeGenerator::outputToFile(std::string fileName, std::string str)
{
	ofstream fw;
	try {
		fw.open(fileName.c_str());
		fw << str;
		fw.close();
	}
	catch (...) {
		cout << "error: output to file" << endl;
	}
}

void HClusterCodeGenerator::CGDeclList(stringstream &out, DeclKind k, List *lst, int offset, FlatNode *_actor)
{
	//_actor为空表明处理的不是_actor的成员变量
	ListMarker maker;
	Node *node;
	Bool firstp = TRUE;
	bool commalistp;

	if (lst == NULL)
		return;

	commalistp = (k == FormalDecl || k == EnumConstDecl);
	IterateList(&maker, lst);

	while (NextOnList(&maker, (GenericREF)&node))
	{
		if (node == NULL)
			continue;

		// if this declaration is from an included file, don't output it.
		if(k == TopDecl && node->coord.includedp)
			continue;

		/* output Text nodes without inserting delimiters (like ",") */
		if (node->typ == Text)
		{
			/* if text node is an #include, it must be at the top level for us to output it. */
			if(!strncmp(node->u.text.text,"#include",8) && k!=TopDecl)
				continue;
			CGTextNode(out, node, offset);
			continue;
		}//if

		if (firstp)
			firstp = FALSE;
		else if (commalistp)
			out << ",";


		switch (node->typ)
		{
		case Proc://.h文件放声明，.cpp放实现
			assert(k == TopDecl);
			CGDecl(out, k, node->u.proc.decl, offset, _actor);//out对应的是.cpp
			CGDecl(m_globalVarStream, TopDecl, node->u.proc.decl, offset, _actor);//在.h中放声明
			m_globalVarStream << ";\n";
			if (node->u.proc.body)
			{
				CGNode(out, node->u.proc.body, offset);
			}
			else {
				/* must output an empty pair of braces to distinguish this function definition (with an empty body) from a function prototype */
				out << "{}";
			}//else
			break;
		case Decl:
			CGDecl(out, k, node, offset, _actor);
			
			if (!commalistp)
			{
				TypeQual sc = NodeStorageClass(node);
				if (sc != T_TYPEDEF) out << ";\n";
				if ((m_extractStateDecl) && node->u.decl.init)
				{
					if (sc != T_STATIC || node->u.decl.type->typ != Adcl)
					{
						m_stateInitStream << ";\n";
					}
				}
				if (k == TopDecl)
					m_globalVarStream << ";\n";
			}
			break;
		case Prim:
		case Tdef:
		case Adcl:
		case Fdcl:
		case Ptr:
		case Sdcl:
		case Udcl:
		case Edcl:
			if (k == TopDecl)
			{
				CGType(m_globalVarStream, node, offset);
				m_globalVarStream << ";\n"; break;
			}//if
			CGType(out, node, offset);
			if (k != FormalDecl)
			{
				out << ";";
			}//if
			break;
		
			default:
				fprintf(stderr, "Internal error: unexpected node");
				PrintNode(stderr, node, 2);
				UNREACHABLE;
		}

	}//while
}

void HClusterCodeGenerator::CGTextNode(stringstream &out, Node *node, int offset)
{
	assert(node->typ == Text);
	if (node->u.text.text) out << node->u.text.text << "\n";
}

void HClusterCodeGenerator::CGNode(stringstream &out, Node *node, int offset)
{
	if (node == NULL)
	{
		out << ";";
		return;
	}
	switch (node->typ) {
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
		CGExpr(out, node, offset);
		out << ";";
		break;
	case Label:
		OutputCRSpace(out, offset);
		out << node->u.label.name;
		out << " : ";
		CGNode(out, node->u.label.stmt, offset);
		OutputCRSpace(out, offset + 1);
		break;
	case Switch:
		OutputCRSpace(out, offset);
		out << "switch (";
		CGExpr(out, node->u.Switch.expr, offset);
		out << ") ";
		CGNode(out, node->u.Switch.stmt, offset);
		break;
	case Case:
		OutputCRSpace(out, offset);
		out << "case ";
		CGExpr(out, node->u.Case.expr, offset);
		out << ": ";
		CGNode(out, node->u.Case.stmt, offset + 1);
		break;
	case Default:
		OutputCRSpace(out, offset);
		out << "default: ";
		CGNode(out, node->u.Default.stmt, offset);
		break;
	case If:
		OutputCRSpace(out, offset);
		out << "if (";
		CGExpr(out, node->u.If.expr, offset);
		out << ") ";
		if (node->u.If.stmt->typ != Block)
			OutputCRSpace(out, offset + 1);
		CGNode(out, node->u.If.stmt, offset + 1);
		break;
	case IfElse:
		out << "if (";
		CGExpr(out, node->u.IfElse.expr, offset);
		out << ") ";
		CGNode(out, node->u.IfElse.true_, offset);
		OutputCRSpace(out, offset);
		out << "else ";
		CGNode(out, node->u.IfElse.false_, offset);
		break;
	case While:
		OutputCRSpace(out, offset);
		out << "while (";
		CGExpr(out, node->u.While.expr, offset);
		out << ") ";
		CGNode(out, node->u.While.stmt, offset);
		break;
	case Do:
		OutputCRSpace(out, offset);
		out << "do";
		CGNode(out, node->u.Do.stmt, offset);
		OutputCRSpace(out, offset);
		out << "while (";
		CGExpr(out, node->u.Do.expr, offset);
		out << ");";
		break;
	case For:
		OutputCRSpace(out, offset);
		out << "for (";
		CGExpr(out, node->u.For.init, offset);
		out << "; ";
		CGExpr(out, node->u.For.cond, offset);
		out << "; ";
		CGExpr(out, node->u.For.next, offset);
		out << ") ";
		CGNode(out, node->u.For.stmt, offset);
		break;
	case Goto:
		break;
	case Continue:
		OutputCRSpace(out, offset + 1);
		out << "continue;";
		break;
	case Break:
		OutputCRSpace(out, offset + 1);
		out << "break;";
		break;
	case Return:
		OutputCRSpace(out, offset);
		out << "return ";
		if (node->u.Return.expr) {
			out << " ";
			CGExpr(out, node->u.Return.expr, offset);
		}
		out << ";";
		break;
	case Block:
		OutputCRSpace(out, offset);
		out << "{";
		CGDeclList(out, BlockDecl, node->u.Block.decl, offset + 1, NULL);
		CGStatementList(out, node->u.Block.stmts, offset + 1);
		OutputCRSpace(out, offset);
		out << "}";
		break;
	case Text:
		CGTextNode(out, node, offset);
		break;
	default:
		fprintf(stderr, "Internal error: unexpected node");
		PrintNode(stderr, node, 2);
		UNREACHABLE;
	}
}


void HClusterCodeGenerator::CGDecl(stringstream &out, DeclKind k, Node *node, int offset, FlatNode *_actor)
{
	assert(node != NULL);
	assert(node->typ == Decl);
	TypeQual sc = NodeStorageClass(node);
	if (k == TopDecl)
	{
		if (node->typ == Decl)
		{
			if (sc != T_TYPEDEF)
			{
				m_globalVarStream << "extern ";
			}
			CGPartialType(m_globalVarStream, Left, node->u.decl.type, sc, offset);
			if (node->u.decl.type->typ != Ptr)
				m_globalVarStream << " ";
			if (node->u.decl.name)
			{
				m_globalVarStream << node->u.decl.name;
				m_globalVarName.push_back(node->u.decl.name);
				/*普通全局变量*/
				if (node->u.decl.type->typ == Prim)
				{
					map<string, string> tmpDim;
					tmpDim.insert(make_pair("0", "0"));
					map<string, map<string,string>> tmpInfo;
					tmpInfo.insert(make_pair(node->u.decl.name, tmpDim));
					m_globalVarInfo.push_back(tmpInfo);
					string tmpType = GetPrimDataType(node->u.decl.type);
					m_globalVarType.push_back(tmpType);
				}
				/*数组类型全局变量*/
				else if (node->u.decl.type->typ == Adcl) {
					/*二维数组全局变量*/
					if (node->u.decl.type->u.adcl.type->typ == Adcl) {
						string dim1 = GetArrayDim(node->u.decl.type->u.adcl.dim);
						string dim2 = GetArrayDim(node->u.decl.type->u.adcl.type->u.adcl.dim);
						map<string, string> tmpDim;
						tmpDim.insert(make_pair(dim1, dim2));
						map<string, map<string, string>> tmpInfo;
						tmpInfo.insert(make_pair(node->u.decl.name, tmpDim));
						m_globalVarInfo.push_back(tmpInfo);
						string tmpType = GetArrayDataType(node->u.decl.type);
						m_globalVarType.push_back(tmpType);
					}//if
					/*否则为一维数组全局变量*/
					else {
						string dim1 = GetArrayDim(node->u.decl.type->u.adcl.dim);
						map<string, string> tmpDim;
						tmpDim.insert(make_pair(dim1, "0"));
						map<string, map<string, string>> tmpInfo;
						tmpInfo.insert(make_pair(node->u.decl.name, tmpDim));
						m_globalVarInfo.push_back(tmpInfo);
						string tmpType = GetArrayDataType(node->u.decl.type);
						m_globalVarType.push_back(tmpType);

					}//else
				}//elif			
			}//if
			CGPartialType(m_globalVarStream, Right, node->u.decl.type, sc, offset);
		}
	}
	if (sc == T_TYPEDEF && k == TopDecl)
		return;
	else if (k != EnumConstDecl)
	{
		//输类型、名称
		OutputCRSpace(out, offset);
		if (_actor != NULL && m_extractStateDecl && sc != T_STATIC &&  node->u.decl.type->typ == Adcl && node->u.decl.init != NULL)
		{//数组作为state中的变量且被初始化作为类的静态变量处理
			sc = T_STATIC;
		}
		else if (_actor != NULL && m_extractStateDecl && sc != T_STATIC &&  node->u.decl.type->typ == Adcl && node->u.decl.init == NULL)
		{//数组作为state中的变量且未被初始化作为指针来处理
			CGArrayPtr(out, node, offset);
			return;
		}
		CGPartialType(out, Left, node->u.decl.type, sc, offset);
		if (node->u.decl.type->typ != Ptr)out << " ";
		if (node->u.decl.name)
		{
			out << node->u.decl.name;
		}
		CGPartialType(out, Right, node->u.decl.type, sc, offset);
	}
	else {
		//处理枚举的成员
		out << node->u.decl.name;
	}

	//输出值(解决初始化问题)
	if (_actor == NULL)
	{
		//不是成员
		switch (k) {
		case SUFieldDecl:
			if (node->u.decl.bitsize != NULL)
			{
				out << ":";
				CGExpr(out, node->u.decl.bitsize, offset);
			}
			break;
		case TopDecl:
		case BlockDecl:
		case EnumConstDecl:
			if (IsSourceExpression(node->u.decl.init))
			{
				out << "=";
				CGExpr(out, node->u.decl.init, offset);
			}
			break;
		case FormalDecl:
			break;
		}
		if (node->u.decl.attribs) 
			CGAttribs(out, node->u.decl.attribs, offset);
	}
	else {
		//是成员
		if (m_extractStateDecl && node->u.decl.init != NULL)
		{//是成员且被初始化
			OutputCRSpace(m_stateInitStream, offset + 1);
			if (sc != T_STATIC &&  node->u.decl.type->typ != Adcl)
				m_stateInitStream << node->u.decl.name;
			else
			{
				if (sc == T_STATIC)CGPartialType(m_array_staticMemberStream, Left, node->u.decl.type, (TypeQual)(0), offset);
				else CGPartialType(m_array_staticMemberStream, Left, node->u.decl.type, sc, offset);
				if (node->u.decl.type->typ != Ptr)m_array_staticMemberStream << " ";
				m_array_staticMemberStream << _actor->name << "::" << node->u.decl.name;
				CGPartialType(m_array_staticMemberStream, Right, node->u.decl.type, sc, offset);
			}
			switch (k) {
			case SUFieldDecl:
				if (node->u.decl.bitsize != NULL)
				{
					m_stateInitStream << ":";
					CGExpr(m_stateInitStream, node->u.decl.bitsize, offset);
				}
				break;
			case TopDecl:
			case BlockDecl:
			case EnumConstDecl:
				if (IsSourceExpression(node->u.decl.init))
				{
					if (sc != T_STATIC &&  node->u.decl.type->typ != Adcl)
					{
						m_stateInitStream << "=";
						CGExpr(m_stateInitStream, node->u.decl.init, offset);
					}
					else
					{
						m_array_staticMemberStream << "=";
						CGExpr(m_array_staticMemberStream, node->u.decl.init, offset);
						m_array_staticMemberStream << ";";
					}
				}
				break;
			}
			if (node->u.decl.attribs && sc != T_STATIC) 
				CGAttribs(m_stateInitStream, node->u.decl.attribs, offset);
		}
	}

}

void HClusterCodeGenerator::CGExpr(stringstream &out, Node *node, int offset)
{
	CGInnerExpr(out, node, 0, Left, offset);
}

void HClusterCodeGenerator::CGInnerExpr(stringstream &out, Node *node, int enclosing_precedence, Context context, int offset)
{
	int my_precedence;
	Bool left_assoc;
	Bool parenthesize;
	if (node == NULL) return;
	my_precedence = Precedence(node, &left_assoc);//取优先级

												  /* determine whether node needs enclosing parentheses */
	if (node->parenthesized)
		parenthesize = TRUE;  /* always parenthesize if source did */
	else if (my_precedence < enclosing_precedence)
		parenthesize = TRUE;
	else if (my_precedence > enclosing_precedence)
		parenthesize = FALSE;
	else  /* my_precedence == enclosing_precedence */
		if ((left_assoc && context == Right) || (!left_assoc && context == Left))
			parenthesize = TRUE;
		else
			parenthesize = FALSE;

	if (parenthesize)
		out << "(";

	switch (node->typ) {
	case Block:
		out << "(";
		CGNode(out, node, offset);
		out << ")";
		break;
	case ImplicitCast:
		/* ignore implicitcasts inserted for typechecking */
		CGInnerExpr(out, node->u.implicitcast.expr, enclosing_precedence, context, offset);
		break;
	case Id:
		out << node->u.id.text;
		break;
	case Const:
		CGConst(out, node, offset);
		break;
	case Binop:
		CGBinop(out, node, my_precedence, offset);
		break;
	case Unary:
		CGUnary(out, node, my_precedence, offset);
		break;
	case Ternary:
		CGInnerExpr(out, node->u.ternary.cond, my_precedence, Left, offset);
		out << "?";
		CGInnerExpr(out, node->u.ternary.true_, my_precedence, Right, offset);
		out << ":";
		CGInnerExpr(out, node->u.ternary.false_, my_precedence, Right, offset);
		break;
	case Cast:
		out << "(";
		CGType(out, node->u.cast.type, offset);
		out << ")";
		CGInnerExpr(out, node->u.cast.expr, my_precedence, Right, offset);
		break;
	case Comma:
		CGCommaList(out, node->u.comma.exprs, offset);
		break;
	case Array:
		CGArray(out, node, my_precedence, offset);
		break;
	case Call:
		CGCall(out, node, my_precedence, offset);
		break;
	case Initializer:
		out << "{";
		CGCommaList(out, node->u.initializer.exprs, offset);
		out << "}";
		break;
	default:
		fprintf(stderr, "Internal error: unexpected node");
		PrintNode(stderr, node, 2);
		UNREACHABLE;
	}
	if (parenthesize)out << ')';

}

int HClusterCodeGenerator::Precedence(Node *node, Bool *left_assoc)
{//取运算符的优先级
	switch (node->typ) {
	case Unary:
		return OpPrecedence(Unary, node->u.unary.op, left_assoc);
	case Binop:
		return OpPrecedence(Binop, node->u.binop.op, left_assoc);
	case Cast:
		*left_assoc = FALSE;
		return 14;
	case Comma:
		*left_assoc = TRUE;
		return 1;
	case Ternary:
		*left_assoc = FALSE;
		return 3;
	default:
		*left_assoc = TRUE;
		return 20;  /* highest precedence */
	}
	/* unreachable */
}

void HClusterCodeGenerator::CGStatementList(stringstream &out, List *lst, int offset)
{
	ListMarker marker;
	Node *node;

	if (lst == NULL) return;

	IterateList(&marker, lst);
	while (NextOnList(&marker, (GenericREF)&node))
		CGNode(out, node, offset);
}

void HClusterCodeGenerator::CGConst(stringstream &out, Node *c, int offset)
{
	int len = 0;
	switch (c->u.Const.type->typ) {
	case Prim:
		switch (c->u.Const.type->u.prim.basic) {
		case Sint:
			if (c->u.Const.value.i < 0)out << "(";
			out << c->u.Const.value.i;
			if (c->u.Const.value.i < 0)out << ")";
			return;
			/*Manish 2/3 hack to print pointer constants */
		case Uint:
			out << c->u.Const.value.u;
			return;
		case Slong:
			if (c->u.Const.value.l < 0)out << "(";
			out << c->u.Const.value.l;
			if (c->u.Const.value.l < 0)out << ")";
			return;
		case Ulong:
			out << c->u.Const.value.ul;
			return;
		case Float:
			if (c->u.Const.value.f < 0)out << "(";
			out << c->u.Const.value.f;
			if (c->u.Const.value.f < 0)out << ")";
			return;
		case Double:
			if (c->u.Const.value.d < 0)out << "(";
			out << c->u.Const.value.d;
			if (c->u.Const.value.d < 0)out << ")";
			return;
		case Char:
		case Schar:
		case Uchar:
			OutputChar(out, c->u.Const.value.i);
			return;
		default:
			Fail(__FILE__, __LINE__, "");
			return;
		}

		/* Manish 2/3  Print Constant Pointers */
	case Ptr:
		out << c->u.Const.value.u; return;
		/* Used for strings */
	case Adcl:
		//out<<c->u.Const.value.s;
		out << "\"";
		while (*(c->u.Const.value.s) != 0)
		{
			OutputChar(out, *(c->u.Const.value.s)++);
		}
		out << "\"";
		return;

	default:
		assert(("Unrecognized constant type", TRUE));
		return;
	}
}

int HClusterCodeGenerator::OutputChar(stringstream &out, char val)
{
	switch (val)
	{
	case '\n': out << "\\n"; break;
	case '\t': out << "\\t"; break;
	case '\v': out << "\\v"; break;
	case '\b': out << "\\b"; break;
	case '\r': out << "\\r"; break;
	case '\f': out << "\\f"; break;
	case '\a': out << "\\a"; break;
	case '\\': out << "\\\\"; break;
	case '\?': out << "\\\?"; break;
	case '\"': out << "\\\""; break;
	case '\'': out << "\\\'"; break;
	default:
		if (isprint(val)) //判断val是否为可打印字符
		{
			out << (val);
		}
		else
		{
			out << "\\" << val;
		}
	}
	return 1;
}

void HClusterCodeGenerator::CGBinop(stringstream &out, Node *node, int my_precedence, int offset)
{
	assert(node->typ == Binop);
	CGInnerExpr(out, node->u.binop.left, my_precedence, Left, offset);
	out << GetOpType(node->u.binop.op);
	CGInnerExpr(out, node->u.binop.right, my_precedence, Right, offset);
}
string HClusterCodeGenerator::GetOpType(OpType op)
{
	switch (op)
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

void HClusterCodeGenerator::CGUnary(stringstream &out, Node *node, int my_precedence, int offset)
{
	assert(node->typ == Unary);
	switch (node->u.unary.op) {
	case SIZEOF:
		out << "sizeof(";
		if (IsType(node->u.unary.expr))
			CGType(out, node->u.unary.expr, offset);
		else CGExpr(out, node->u.unary.expr, offset);
		out << ")";
		break;
	case PREDEC:
	case PREINC:
		out << GetOpType(node->u.unary.op);
		CGInnerExpr(out, node->u.unary.expr, my_precedence, Right, offset);
		break;
	case POSTDEC:
	case POSTINC:
		CGInnerExpr(out, node->u.unary.expr, my_precedence, Left, offset);
		out << GetOpType(node->u.unary.op);
		break;
	default:
		out << GetOpType(node->u.unary.op);
		CGInnerExpr(out, node->u.unary.expr, my_precedence, Right, offset);
		break;
	}
}
void HClusterCodeGenerator::CGType(stringstream &out, Node *node, int offset)
{
	CGPartialType(out, Left, node, EMPTY_TQ, offset);
	CGPartialType(out, Right, node, EMPTY_TQ, offset);
}

void HClusterCodeGenerator::CGPartialType(stringstream &out, Context context, Node *node, TypeQual sc, int offset)
{
	if (node == NULL) return;

	switch (node->typ) {
		// base types: primitive, typedef, SUE 
	case Prim:
		if (context == Left) {
			char tmp_TQ[256];
			TQtoText(tmp_TQ, sc);
			out << tmp_TQ << " ";
			char tmp_PM[256];
			PrimToText(tmp_PM, node);
			out << tmp_PM << " ";
		}
		break;

	case Tdef:
		if (context == Left) {
			char tmp_TQ_1[256];
			TQtoText(tmp_TQ_1, sc);
			out << tmp_TQ_1 << " ";

			char tmp_TQ_2[256];
			TQtoText(tmp_TQ_2, node->u.tdef.tq);
			out << tmp_TQ_2 << " ";
			out << node->u.tdef.name;
		}
		break;

	case Sdcl:
	case Udcl:
	case Edcl:
		if (context == Left) {
			char tmp_TQ_1[256];
			TQtoText(tmp_TQ_1, sc);
			out << tmp_TQ_1 << " ";
			char tmp_TQ_2[256];
			TQtoText(tmp_TQ_2, (TypeQual)(node->u.sdcl.tq & ~T_SUE_ELABORATED));
			out << tmp_TQ_2 << " ";
			CGSUE(out, node->u.sdcl.type, SUE_ELABORATED(node->u.sdcl.tq), offset);
		}
		// no action in Right context 
		break;

		//  type operators: pointer, array, function.

#define IS_ARRAY_OR_FUNC(n)  (n->typ == Adcl || n->typ == Fdcl)

	case Ptr:
		if (context == Left) {
			CGPartialType(out, context, node->u.ptr.type, sc, offset);
			if (IS_ARRAY_OR_FUNC(node->u.ptr.type))out << "(";
			out << "*";
			char tmp_TQ[256];
			TQtoText(tmp_TQ, node->u.ptr.tq);
			out << tmp_TQ << " ";
		}
		else {
			if (IS_ARRAY_OR_FUNC(node->u.ptr.type)) out << ")";
			CGPartialType(out, context, node->u.ptr.type, sc, offset);
		}
		break;
	case Adcl:
		if (context == Left)
			CGPartialType(out, context, node->u.adcl.type, sc, offset);
		else {
			out << "[";
			if (IsSourceExpression(node->u.adcl.dim))	CGExpr(out, node->u.adcl.dim, offset);
			out << "]";
			CGPartialType(out, context, node->u.adcl.type, sc, offset);
		}
		break;
	case Fdcl:
		if (context == Left) {
			char tmp_TQ[256];
			TQtoText(tmp_TQ, node->u.fdcl.tq);
			out << tmp_TQ << " ";
			CGPartialType(out, context, node->u.fdcl.returns, sc, offset);
		}
		else
		{
			out << "(";
			CGDeclList(out, FormalDecl, node->u.fdcl.args, offset, NULL);
			out << ")";
			CGPartialType(out, context, node->u.fdcl.returns, sc, offset);
		}
		break;
	default:
		fprintf(stderr, "Internal error: unexpected node");
		PrintNode(stderr, node, 2);
		UNREACHABLE;
	}
}

void HClusterCodeGenerator::CGSUE(stringstream &out, SUEtype *sue, Bool elaboratedp, int offset)
{
	assert(sue != NULL);
	switch (sue->typ) {
	case Sdcl:
		out << "struct";
		break;
	case Udcl:
		out << "union";
		break;
	case Edcl:
		out << "enum";
		break;
	default:
		UNREACHABLE;
	}
	out << " ";
	if (sue->name) {
		out << sue->name;
		out << " ";
	}
	if (elaboratedp) {
		out << "{";
		CGDeclList(out,
			(sue->typ == Edcl) ? EnumConstDecl : SUFieldDecl,
			sue->fields, offset, NULL);
		out << "}";
	}
}

Bool HClusterCodeGenerator::IsSourceExpression(Node *node)
{
	while (node && node->typ == ImplicitCast)
		node = node->u.implicitcast.expr;

	return (node != NULL);
}

void HClusterCodeGenerator::CGCommaList(stringstream &out, List *list, int offset)
{
	if (list == NULL)
		return;
	ListMarker marker; Node *expr;
	IterateList(&marker, list);
	if (!NextOnList(&marker, (GenericREF)&expr))
		return;

	CGExpr(out, expr, offset);
	while (NextOnList(&marker, (GenericREF)&expr)) {
		out << ",";
		CGExpr(out, expr, offset);
	}
}
void HClusterCodeGenerator::CGArray(stringstream &out, Node *node, int my_precedence, int offset)
{
	CGInnerExpr(out, node->u.array.name, my_precedence, Left, offset);
	CGDimensions(out, node->u.array.dims, offset);
}

void HClusterCodeGenerator::CGDimensions(stringstream &out, List *dim, int offset)
{
	ListMarker marker; Node *expr;

	IterateList(&marker, dim);
	while (NextOnList(&marker, (GenericREF)&expr)) {
		out << "[";
		CGExpr(out, expr, offset);
		out << "]";
	}
}

void HClusterCodeGenerator::CGCall(stringstream &out, Node *node, int my_precedence, int offset)
{
	const char *ident = node->u.call.name->u.id.text;
	if (strcmp(ident, "println") == 0)
	{
		out << "\tcout<<";
		assert(ListLength(node->u.call.args) == 1);
		CGCommaList(out, node->u.call.args, offset);
		out << "<<endl";
	}
	else if (strcmp(ident, "printf") == 0)
	{
		out << "\tcout<<";
		CGCommaList(out, node->u.call.args, offset);
	}
	else if (strcmp(ident, "print") == 0)
	{
		out << "\tcout<<";
		CGCommaList(out, node->u.call.args, offset);
	}
	else
	{
		CGInnerExpr(out, node->u.call.name, my_precedence, Left, offset);
		out << "(";
		CGCommaList(out, node->u.call.args, offset);
		out << ")";
	}
}

void HClusterCodeGenerator::CGArrayPtr(stringstream &out, Node *node, int offset)
{
	assert(m_extractStateDecl && node->u.decl.type->typ == Adcl && node->u.decl.init == NULL);
	//在out中放类型
	CGPartialType(out, Left, node->u.decl.type, NodeStorageClass(node), offset);
	//将声明放在out流中（输出*）将对维数空间的开辟空间放在stateInitStream中
	Node *tmptype = node->u.decl.type;
	int dim = 0;
	while (tmptype->typ == Adcl)
	{
		out << "*"; dim++;
		tmptype = tmptype->u.adcl.type;
	}
	out << node->u.decl.name << ";\n";
	//向stateInitStream写入开辟空间的过程，以及destructorStream中释放空间的过程
	tmptype = node->u.decl.type;
	switch (dim) {
	case 1:
		m_stateInitStream << "\t" << node->u.decl.name << " = new ";
		CGPartialType(m_stateInitStream, Left, node->u.decl.type, NodeStorageClass(node), offset);
		m_stateInitStream << "[";
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_stateInitStream, tmptype->u.adcl.dim, offset);
		m_stateInitStream << "]";
		m_stateInitStream << ";\n";
		m_destructorStream << "\tdelete []" << node->u.decl.name << ";\n";
		break;
	case 2:
		//形如p[4][4];
		//形如 pp = new int * [4];
	{
		Node *_des_adcltype = node->u.decl.type;
		m_stateInitStream << "\t" << node->u.decl.name << " = new";
		CGPartialType(m_stateInitStream, Left, node->u.decl.type, NodeStorageClass(node), offset);
		m_stateInitStream << "*";
		m_stateInitStream << "[";
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_stateInitStream, tmptype->u.adcl.dim, offset);
		m_stateInitStream << "]";
		m_stateInitStream << ";\n";
		m_stateInitStream << "\tfor( int __i = 0 ;__i <";
		
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_stateInitStream, tmptype->u.adcl.dim, offset);
		m_stateInitStream << "; __i++) " << node->u.decl.name << "[__i] = new";
		CGPartialType(m_stateInitStream, Left, node->u.decl.type, NodeStorageClass(node), offset);
		m_stateInitStream << "[";
		tmptype = tmptype->u.adcl.type;
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_stateInitStream, tmptype->u.adcl.dim, offset);//第二维的长度
		m_stateInitStream << "]";
		m_stateInitStream << ";\n";
		//释放空间
		m_destructorStream << "\tfor( int __i = 0 ;__i <";
		if (IsSourceExpression(_des_adcltype->u.adcl.dim))	CGExpr(m_destructorStream, _des_adcltype->u.adcl.dim, offset);//释放第二维
		m_destructorStream << "; __i++) " << "delete[]" << node->u.decl.name << "[__i]";
		m_destructorStream << ";\n";
		m_destructorStream << "\tdelete []" << node->u.decl.name << ";\n";//释放第1维
		break;
	}

	case 3:
	{
		//形如p[4][4][4];
		Node *_des_adcltype = node->u.decl.type;
		m_stateInitStream << "\t" << node->u.decl.name << " = new";
		CGPartialType(m_stateInitStream, Left, node->u.decl.type, NodeStorageClass(node), offset);
		m_stateInitStream << "**";
		m_stateInitStream << "[";
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_stateInitStream, tmptype->u.adcl.dim, offset);//第一维
		m_stateInitStream << "]";
		m_stateInitStream << ";\n";
		m_stateInitStream << "\tfor( int __i = 0 ;__i <";
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_stateInitStream, tmptype->u.adcl.dim, offset);//第二维
		m_stateInitStream << "; __i++) " << node->u.decl.name << "[__i] = new";
		CGPartialType(m_stateInitStream, Left, node->u.decl.type, NodeStorageClass(node), offset);
		m_stateInitStream << "*[";
		tmptype = tmptype->u.adcl.type;
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_stateInitStream, tmptype->u.adcl.dim, offset);//第二维的长度
		m_stateInitStream << "]";
		m_stateInitStream << ";\n";

		m_stateInitStream << "\tfor( int __i = 0 ;__i <";
		if (IsSourceExpression(_des_adcltype->u.adcl.dim))	CGExpr(m_stateInitStream, _des_adcltype->u.adcl.dim, offset);//第三维
		m_stateInitStream << "; __i++) " << "\n\t\tfor( int __j = 0 ;__j <";
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_stateInitStream, tmptype->u.adcl.dim, offset);
		m_stateInitStream << "; __j++)\n\t\t\t ";
		m_stateInitStream << node->u.decl.name << "[__i][__j] = new";
		CGPartialType(m_stateInitStream, Left, node->u.decl.type, NodeStorageClass(node), offset);
		m_stateInitStream << "[";
		tmptype = tmptype->u.adcl.type;
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_stateInitStream, tmptype->u.adcl.dim, offset);
		tmptype = tmptype->u.adcl.type;
		m_stateInitStream << "]";
		m_stateInitStream << ";\n";

		//释放空间
		_des_adcltype = node->u.decl.type;
		tmptype = _des_adcltype->u.adcl.type;
		m_destructorStream << "\tfor( int __i = 0 ;__i <";
		if (IsSourceExpression(_des_adcltype->u.adcl.dim))	CGExpr(m_destructorStream, _des_adcltype->u.adcl.dim, offset);//第三维
		m_destructorStream << "; __i++) " << "\n\t\tfor( int __j = 0 ;__j <";
		if (IsSourceExpression(tmptype->u.adcl.dim))	CGExpr(m_destructorStream, tmptype->u.adcl.dim, offset);
		m_destructorStream << "; __j++)\n\t\t\t ";
		m_destructorStream << "delete[] " << node->u.decl.name << "[__i][__j]";
		m_destructorStream << ";\n";
		m_destructorStream << "\tfor( int __i = 0 ;__i <";
		if (IsSourceExpression(_des_adcltype->u.adcl.dim))	CGExpr(m_destructorStream, _des_adcltype->u.adcl.dim, offset);
		m_destructorStream << "; __i++) " << "delete[]" << node->u.decl.name << "[__i]";
		m_destructorStream << ";\n";
		m_destructorStream << "\tdelete []" << node->u.decl.name << ";\n";
		break;
	}

	default:
		cout << "数组维数大于3，暂不处理" << endl;
		UNREACHABLE;
		break;
	}
}

void HClusterCodeGenerator::CGAttribs(stringstream &out, List *attribs, int offset)
{
	out << " __attribute__((";
	while (attribs != NULL) {
		Node *attrib = (Node *)FirstItem(attribs);
		out << attrib->u.attrib.name;
		if (attrib->u.attrib.arg) {
			out << "(";
			CGExpr(out, attrib->u.attrib.arg, offset);
			out << ")";
		}
		attribs = Rest(attribs);
		if (attribs)out << ",";
	}
	out << "))";
}

/*对全局变量进行代码生成*/
void HClusterCodeGenerator::CGGlobalVar()
{
	stringstream buf;
	buf << "#include \"GlobalVar.h\"\n";
	stringstream outputGlobalVal("");

	CGDeclList(outputGlobalVal, TopDecl, m_globalList, 0, NULL);

	buf << outputGlobalVal.str() << "\n\n";
	//输出到文件
	stringstream out;
	out << m_curDir << "GlobalVar.cpp";
	outputToFile(out.str(), buf.str());
}

void HClusterCodeGenerator::CGGlobalVarextern()
{
	stringstream out;
	out << m_curDir << "GlobalVar.h";
	//对声明文件用宏进行保护
	stringstream outputMacroProtect("");
	outputMacroProtect << "#ifndef _GLOBAL_VAL_H_\n" <<
		"#define _GLOBAL_VAL_H_\n\n\n";
	outputMacroProtect << m_globalVarStream.str();
	outputMacroProtect << "\n#endif\n\n\n";

	outputToFile(out.str(), outputMacroProtect.str());
}

void HClusterCodeGenerator::CGGlobalStreamHeader()
{
	stringstream buf;
	buf << "#ifndef _GLOBAL_STREAM_H_\n";
	buf << "#define _GLOBAL_STREAM_H_\n\n\n";
	buf << "/*全局变量，用于存储边的信息*/\n";
	buf << "/*边的命名规则，A_B，其中A->B*/\n\n";
	buf << "\n#include <mpi.h>\n";

	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"MathExtension.h\"\n";
	if (Win)
	{
		buf << "#define nRepeatCount 10\n";
	}

	for (map<string, string>::iterator iter1 = m_struct2StructFields.begin(); iter1 != m_struct2StructFields.end(); ++iter1)
	{
		buf << "typedef " << "struct " << "\n{\t" << iter1->first << "}" << iter1->second << ";\n";
	}//for 

	for (map<string, string>::iterator iter1 = m_struct2StructFields.begin(); iter1 != m_struct2StructFields.end(); ++iter1)
	{
		buf << "extern " << "MPI::Datatype " << iter1->second << "_mpitype" << ";\n";
	}//for

	for (map<string, List*>::iterator iter = m_struct2FieldList.begin(); iter != m_struct2FieldList.end(); ++iter)
	{
		buf << "extern " << iter->first << " " << iter->first << "_obj;\n";
	}//for

	//对于在同一台服务器上的节点分配共享缓冲区，对于边界节点分配接受缓冲区和发送缓冲区
	for (int i = 0; i < m_hcp->getHClusters(); ++i)
	{
		if (m_hcp->isGPUClusterNode(i))
		{
			//得到当前服务器节点上分配的actors
			vector<FlatNode*> tmpFlatNodes = m_hcp->getSubFlatNodes(i);
			for (vector<FlatNode*>::iterator iter1 = tmpFlatNodes.begin(); iter1 != tmpFlatNodes.end(); ++iter1)
			{
				/*当前节点分配到缓和架构的GPU端*/
				if (m_hcp->isNodeInGPU(i, *iter1))
				{
					pair<int, int> ret_0 = m_hcp->getHClusterGpuNum(*iter1);
					//判断当前节点iter1的划分映射结果
					assert(i == ret_0.first);
					for (vector<FlatNode *>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); ++iter2)
					{
						int tmpHClusterNum = m_hcp->getHClusterNumOfNode(*iter2);
						/*在同一台服务器上，但是不在同一个GPU上*/
						if (tmpHClusterNum == i)
						{
							if (m_hcp->isNodeInGPU(i, *iter2))
							{
								/*在同一台服务器，不在同一个GPU*/
								pair<int, int> ret_1 = m_hcp->getHClusterGpuNum(*iter2);
								if (ret_0.second != ret_1.second)
								{
									string edgename = (*iter1)->name + "_" + (*iter2)->name;
									map<string, string>::iterator iter2 = m_stream2StructType.find(edgename);
									buf << "extern Buffer<" << iter2->second << ">" << iter2->first << ";\n";
								}
							}//if
							/*在同一台服务器，一个位于GPU，一个位于CPU*/
							else {
								string edgename = (*iter1)->name + "_" + (*iter2)->name;
								map<string, string>::iterator iter2 = m_stream2StructType.find(edgename);
								buf << "extern Buffer<" << iter2->second << ">" << iter2->first << ";\n";

							}//else
						}//if
					}//for
				}//if
				 /*当前节点分配到缓和架构的CPU端*/
				else {
					pair<int, int> ret_0 = m_hcp->getHClusterCoreNum(*iter1);
					assert(i == ret_0.first);

					for (vector<FlatNode *>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); ++iter2)
					{
						int tmpHClusterNum = m_hcp->getHClusterNumOfNode(*iter2);
						/*在同一台服务器上*/
						if (tmpHClusterNum == i)
						{
							if (m_hcp->isNodeInCPU(i, *iter2))
							{
								/*在同一台服务器，不在同一个CPU核*/
								pair<int, int> ret_1 = m_hcp->getHClusterCoreNum(*iter2);
								if (ret_0.second != ret_1.second)
								{
									string edgename = (*iter1)->name + "_" + (*iter2)->name;
									map<string, string>::iterator iter2 = m_stream2StructType.find(edgename);
									buf << "extern Buffer<" << iter2->second << ">" << iter2->first << ";\n";
								}//if
							}//if
							else {
								string edgename = (*iter1)->name + "_" + (*iter2)->name;
								map<string, string>::iterator iter2 = m_stream2StructType.find(edgename);
								buf << "extern Buffer<" << iter2->second << ">" << iter2->first << ";\n";
							}//else
						}//if
					}//for
				}//else
			}//for
		}//if
		else {
			//得到当前服务器节点上分配的actors
			vector<FlatNode*> tmpFlatNodes = m_hcp->getSubFlatNodes(i);
			for (vector<FlatNode*>::iterator iter1 = tmpFlatNodes.begin(); iter1 != tmpFlatNodes.end(); ++iter1)
			{
				pair<int, int> ret_0 = m_hcp->getHClusterCoreNum(*iter1);
				//判断当前节点iter1的划分映射结果
				assert(i == ret_0.first);
				for (vector<FlatNode *>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); ++iter2)
				{
					int tmpHClusterNum = m_hcp->getHClusterNumOfNode(*iter2);
					/*在同一台服务器上*/
					if (tmpHClusterNum == i)
					{
						pair<int, int> ret_1 = m_hcp->getHClusterCoreNum(*iter2);

						/*在同一台服务器上，但是不在同一个核上*/
						if (ret_1.first == i && ret_1.second != ret_0.second)
						{
							//在同一台机器上并且在同一个核上，作为线程内部的缓冲区分配，在一个进程不在同一个核上，则在作为全局分配
							string edgename = (*iter1)->name + "_" + (*iter2)->name;
							map<string, string>::iterator iter2 = m_stream2StructType.find(edgename);
							buf << "extern Buffer<" << iter2->second << ">" << iter2->first << ";\n";

						}//if
					}//if
				}//for
			}//for
		}//else
	}//for

	buf << "#endif\n\n";
	//写入文件
	stringstream out;
	out << m_curDir << "globalstream.h";
	outputToFile(out.str(), buf.str());

}

void HClusterCodeGenerator::CGGlobalStreamCpp()
{
	stringstream buf;
	string edgename;
	vector<int>::iterator iter;
	/*缓冲区大小*/
	int size;

	/*保存两个节点分配的阶段差值*/
	int stageDiff;

	buf << "/*cpp文件，全局变量，用于存储边的信息*/\n";
	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"globalstream.h\"\n";

	//对于在同一台服务器上的节点分配共享缓冲区，对于边界节点分配接受缓冲区和发送缓冲区
	for (int i = 0; i < m_hcp->getHClusters(); ++i)
	{
		if (m_hcp->isGPUClusterNode(i))
		{
			//得到当前服务器节点上分配的actors
			vector<FlatNode*> tmpFlatNodes = m_hcp->getSubFlatNodes(i);
			for (vector<FlatNode*>::iterator iter1 = tmpFlatNodes.begin(); iter1 != tmpFlatNodes.end(); ++iter1)
			{
				/*当前节点分配到缓和架构的GPU端*/
				if (m_hcp->isNodeInGPU(i, *iter1))
				{
					pair<int, int> ret_0 = m_hcp->getHClusterGpuNum(*iter1);
					//判断当前节点iter1的划分映射结果
					assert(i == ret_0.first);

					iter = (*iter1)->outPushWeights.begin();
					for (vector<FlatNode *>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); ++iter2)
					{
						int tmpHClusterNum = m_hcp->getHClusterNumOfNode(*iter2);

						/*在同一台服务器上，但是不在同一个GPU上*/
						if (tmpHClusterNum == i)
						{
							if (m_hcp->isNodeInGPU(i, *iter2))
							{
								/*在同一台服务器，不在同一个GPU*/
								pair<int, int> ret_1 = m_hcp->getHClusterGpuNum(*iter2);
								if (ret_0.second != ret_1.second)
								{
									//在同一台机器上
									edgename = (*iter1)->name + "_" + (*iter2)->name;
									stageDiff = m_hsa->getStageNum(*iter2) - m_hsa->getStageNum(*iter1);
									size = (m_sssg->GetInitCount(*iter1) + m_hcp->getSteadyCount(*iter1) * (stageDiff + 1)) * (*iter);
									iter++;
									map<string, string>::iterator iter4 = m_stream2StructType.find(edgename);
									assert(iter4 != m_stream2StructType.end());
									buf << "Buffer<" << iter4->second << "> " << edgename << "(" << size << ");\n";

								}//if
							}//if
							else {
								/*在同一台服务器，一个位于GPU，一个位于CPU*/
								edgename = (*iter1)->name + "_" + (*iter2)->name;
								stageDiff = m_hsa->getStageNum(*iter2) - m_hsa->getStageNum(*iter1);
								size = (m_sssg->GetInitCount(*iter1) + m_hcp->getSteadyCount(*iter1) * (stageDiff + 1)) * (*iter);
								iter++;
								map<string, string>::iterator iter4 = m_stream2StructType.find(edgename);
								assert(iter4 != m_stream2StructType.end());
								buf << "Buffer<" << iter4->second << "> " << edgename << "(" << size << ");\n";
							}//else
						}//if
					}//for
				}//if
				else {
					pair<int, int> ret_0 = m_hcp->getHClusterCoreNum(*iter1);
					assert(i == ret_0.first);

					iter = (*iter1)->outPushWeights.begin();
					for (vector<FlatNode *>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); ++iter2)
					{
						int tmpHClusterNum = m_hcp->getHClusterNumOfNode(*iter2);
						/*在同一台服务器上*/
						if (tmpHClusterNum == i)
						{
							if (m_hcp->isNodeInCPU(i, *iter2))
							{
								/*在同一台服务器，不在同一个CPU核*/
								pair<int, int> ret_1 = m_hcp->getHClusterCoreNum(*iter2);
								if (ret_0.second != ret_1.second)
								{
									//在同一台机器上
									edgename = (*iter1)->name + "_" + (*iter2)->name;
									stageDiff = m_hsa->getStageNum(*iter2) - m_hsa->getStageNum(*iter1);
									size = (m_sssg->GetInitCount(*iter1) + m_hcp->getSteadyCount(*iter1) * (stageDiff + 1)) * (*iter);
									iter++;
									map<string, string>::iterator iter4 = m_stream2StructType.find(edgename);
									assert(iter4 != m_stream2StructType.end());
									buf << "Buffer<" << iter4->second << "> " << edgename << "(" << size << ");\n";

								}//if
							}//if
							else {
								/*在同一台服务器，一个位于GPU，一个位于CPU*/
								edgename = (*iter1)->name + "_" + (*iter2)->name;
								stageDiff = m_hsa->getStageNum(*iter2) - m_hsa->getStageNum(*iter1);
								size = (m_sssg->GetInitCount(*iter1) + m_hcp->getSteadyCount(*iter1) * (stageDiff + 1)) * (*iter);
								iter++;
								map<string, string>::iterator iter4 = m_stream2StructType.find(edgename);
								assert(iter4 != m_stream2StructType.end());
								buf << "Buffer<" << iter4->second << "> " << edgename << "(" << size << ");\n";

							}//else
						}//if
					}//for
				}//else
			}//for
		}//if
		else {
			//得到当前服务器节点上分配的actors
			vector<FlatNode*> tmpFlatNodes = m_hcp->getSubFlatNodes(i);
			for (vector<FlatNode*>::iterator iter1 = tmpFlatNodes.begin(); iter1 != tmpFlatNodes.end(); ++iter1)
			{
				pair<int, int> ret_0 = m_hcp->getHClusterCoreNum(*iter1);
				//判断当前节点iter1的划分映射结果
				assert(i == ret_0.first);

				iter = (*iter1)->outPushWeights.begin();
				for (vector<FlatNode *>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); ++iter2)
				{
					int tmpHClusterNum = m_hcp->getHClusterNumOfNode(*iter2);
					/*在同一台服务器上*/
					if (tmpHClusterNum == i)
					{
						pair<int, int> ret_1 = m_hcp->getHClusterCoreNum(*iter2);
						/*在同一台服务器上，但是不在同一个核上*/
						if (ret_1.first == i && ret_1.second != ret_0.second)
						{
							//在同一台机器上
							edgename = (*iter1)->name + "_" + (*iter2)->name;
							stageDiff = m_hsa->getStageNum(*iter2) - m_hsa->getStageNum(*iter1);
							size = (m_sssg->GetInitCount(*iter1) + m_hcp->getSteadyCount(*iter1) * (stageDiff + 1)) * (*iter);
							iter++;
							map<string, string>::iterator iter4 = m_stream2StructType.find(edgename);
							assert(iter4 != m_stream2StructType.end());
							buf << "Buffer<" << iter4->second << "> " << edgename << "(" << size << ");\n";
						}//if
					}//if

				}//for

			}//for
		}//else
	}//for

	stringstream out;
	out << m_curDir << "globalstream.cpp";
	outputToFile(out.str(), buf.str());
}

/*为异构集群每个服务器节点生成多线程*/
void HClusterCodeGenerator::CGThreads()
{
	string threadName;
	stringstream buf;	
	char a[20], b[20];
	string processTag, threadTag; //存储进程和线程标记

	assert(m_hcp->getHClusters() == m_hcp->getHClusterNum2FlatNodes().size());
	for (int i = 0; i < m_hcp->getHClusters(); ++i)
	{
		if (m_hcp->isGPUClusterNode(i))
		{
			if (CurGpuFlag)
			{
				itoa(i, a, 20);
				processTag = a;
				int nCore = getCpuThreadNumOnHCluster(i);
				int nGpu = getGpuThreadNumOnHCluster(i);

				for (int j = 0; j < nCore+ nGpu; ++j)
				{
					buf << "/*该文件定义各thread的入口函数，在函数内部完成软件流水迭代*/\n";
					stringstream out;
					itoa(j, b, 20);
					threadTag = b;
					/*构造线程函数名*/
					threadName = "thread_" + processTag + "_" + threadTag;
					out << m_curDir << threadName << ".cpp";
					/*构造进程i上的第j个线程函数代码*/
					CGThread(i, nCore, nGpu, j, buf);

					outputToFile(out.str(), buf.str());
					out.str("");
					buf.str("");
				}//for			
			}//if
		}//if
		else {
			itoa(i, a, 20);
			processTag = a;
			int nPartitions = 0;
			int nCore = getCpuThreadNumOnHCluster(i);
			int nGpu = getGpuThreadNumOnHCluster(i);
			assert(nGpu == 0);

			for (int j = 0; j < nCore; ++j)
			{
				buf << "/*该文件定义各thread的入口函数，在函数内部完成软件流水迭代*/\n";
				stringstream out;
				itoa(j, b, 20);
				threadTag = b;
				/*构造线程函数名*/
				threadName = "thread_" + processTag + "_" + threadTag;
				out << m_curDir << threadName << ".cpp";
				/*构造进程i上的第j个线程函数代码*/
				CGThread(i, nCore, nGpu ,j, buf);

				outputToFile(out.str(), buf.str());
				out.str("");
				buf.str("");
			}//for			
		}//else
		
	}//for
}

void HClusterCodeGenerator::CGThread(int hClusterIdx, int totalCores, int totalGpus, int threadIdx, stringstream &buf)
{
	char a[20];
	itoa(threadIdx, a, 20);
	string idxName = a;
	int maxStageNum = m_hsa->getMaxStageNum(hClusterIdx);

	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"Producer.h\"\n";
	buf << "#include \"Consumer.h\"\n";
	buf << "#include \"NetProducer.h\"\n";
	buf << "#include \"NetConsumer.h\"\n";
	buf << "#include \"globalstream.h\"\n";
	buf << "#include \"GlobalVar.h\"\n";
	buf << "#include \"AllActorHeader.h\"\t//包含所有actor的头文件\n";
	buf << "#include \"lock_free_barrier.h\"\t//包含barrier函数\n";
	buf << "#include \"rdtsc.h\"\n";
	buf << "\n#include <mpi.h>\n"; //mpi的库

	buf << "#include <sstream>\n";
	buf << "#include <fstream>\n";
	

	 /*判断当前进程对应的服务器节点结构，若是纯CPU服务器*/
	if (!m_hcp->isGPUClusterNode(hClusterIdx))
	{

		buf << "\nusing namespace std;\n";

		buf << "//extern pthread_barrier_t barrier;\n";
		buf << "extern int nRepeatCount;\n";


		//先声明MPI的类型（由结构体转换而来的）
		for (map<string, List *>::iterator iter1 = m_struct2FieldList.begin(); iter1 != m_struct2FieldList.end(); iter1++)
		{
			buf << "extern " << "MPI::Datatype " << iter1->first << "_mpitype" << ";\n";
		}//for

		map<int, set<int>> tmpCore2Stage;
		map<int, set<int>>::iterator iter;

		tmpCore2Stage = m_hCluster2CoreNum2Stage.find(hClusterIdx)->second;
		iter = tmpCore2Stage.find(threadIdx);
		//判错
		assert(iter != tmpCore2Stage.end());
		set<int> *tmpStages = &iter->second;
		vector<FlatNode*> tmpFlatNodes = m_hcp->getHClusterCore2FlatNodes(hClusterIdx)[threadIdx];
		buf << "void thread_" << hClusterIdx << "_" << idxName << "_fun()\n{\n";

		/*定义线程内部节点间的通信缓冲区*/
		for (vector<FlatNode*>::iterator iter2 = tmpFlatNodes.begin(); iter2 != tmpFlatNodes.end(); ++iter2)
		{
			pair<int, int> ret_0 = m_hcp->getHClusterCoreNum(*iter2);
			vector<int>::iterator iter3 = (*iter2)->outPushWeights.begin();

			for (vector<FlatNode*>::iterator iter4 = (*iter2)->outFlatNodes.begin(); iter4 != (*iter2)->outFlatNodes.end(); ++iter4)
			{
				int tmpHClusterNum = m_hcp->getHClusterNumOfNode(*iter4);
				if (tmpHClusterNum == ret_0.first)
				{
					/*找到当前出边节点所在的集群节点编号*/
					pair<int, int> ret_1 = m_hcp->getHClusterCoreNum(*iter4);

					/*如果在同一台服务器上 而且在同一核上*/
					if (ret_0.first == ret_1.first && ret_0.second == ret_1.second)
					{
						string edgename = (*iter2)->name + "_" + (*iter4)->name;
						int stagediff = m_hsa->getStageNum(*iter4) - m_hsa->getStageNum(*iter2);
						int size = (m_sssg->GetInitCount(*iter2) + m_hcp->getSteadyCount(*iter2) * (stagediff + 1)) * (*iter3);

						++iter3;

						map<string, string>::iterator iter5 = m_stream2StructType.find(edgename);
						assert(iter5 != m_stream2StructType.end());

						buf << "\tBuffer<" << iter5->second << ">" << edgename << "(" << size << ");\n";

					}//if
				}//if
			}//for
		}//for
		/*对每个线程中的actor构造对象actor_obj*/
		for (vector<FlatNode*>::iterator iter1 = tmpFlatNodes.begin(); iter1 != tmpFlatNodes.end(); ++iter1)
		{
			string bufstr = "";
			buf << "\t" << (*iter1)->name << " " << (*iter1)->name << "_obj";
			bufstr += "(";
			/*找出每个actor的输入输出边，注意边之间的对应顺序*/
			vector<FlatNode*> tmpInFlatNodes = (*iter1)->inFlatNodes;
			int curHClusterNum = m_hcp->getHClusterNumOfNode(*iter1);
			for (vector<FlatNode*>::iterator inIter = tmpInFlatNodes.begin(); inIter != tmpInFlatNodes.end(); ++inIter)
			{
				int inHClusterNum = m_hcp->getHClusterNumOfNode(*inIter);
				/*在同一个集群节点中*/
				if (inHClusterNum == curHClusterNum)
				{
					bufstr += ((*inIter)->name + "_" + (*iter1)->name + ",");
				}//if
			}//for
			vector<FlatNode*> tmpOutFlatNodes = (*iter1)->outFlatNodes;
			for (vector<FlatNode*>::iterator outIter = tmpOutFlatNodes.begin(); outIter != tmpOutFlatNodes.end(); ++outIter)
			{
				int outHClusterNum = m_hcp->getHClusterNumOfNode(*outIter);
				if (outHClusterNum == curHClusterNum)
				{
					bufstr += ((*iter1)->name + "_" + (*outIter)->name + ",");
				}
			}//for

			if (bufstr.length() > 1)
			{
				buf << bufstr;
				buf.seekp((int)buf.tellp() - 1);

				buf << ");\n";
			}//if
			else {
				buf << ";\n";
			}//else

		}//for

		buf << "\tchar stage[" << maxStageNum << "]={0};\n";
		buf << "\tstage[0]=1;\n\n";

		if (threadIdx)
			buf << "\tworkerSync(" << threadIdx << ");\n\n";
		else
			buf << "\tmasterSync(" << totalCores + totalGpus<< ");\n\n";

		buf << "\tfor(int stageNum=0;stageNum<" << maxStageNum << ";stageNum++)\n";
		buf << "\t{\n";

		/*打印初态执行*/
		set<int>::iterator endIter = tmpStages->end();
		for (int i = maxStageNum - 1; i >= 0; --i)
		{
			set<int>::iterator stageIter = tmpStages->find(i);
			/*该stage在当前线程上*/
			if (stageIter != endIter)
			{
				buf << "\t\tif(" << i << "==stageNum)\n\t\t{\n";
				vector<FlatNode*> curFlatNodes = m_hsa->getFlatNodesOfHClusterAndStage(hClusterIdx, i);
				for (vector<FlatNode*>::iterator iter1 = curFlatNodes.begin(); iter1 != curFlatNodes.end(); ++iter1)
				{
					pair<int, int> ret_0 = m_hcp->getHClusterCoreNum(*iter1);
					if (ret_0.first == hClusterIdx && ret_0.second == threadIdx)
					{
						map<FlatNode *, Bool>::iterator haveClusterCommIter;
						buf << "\t\t\t" << (*iter1)->name << "_obj.runInitScheduleWork();\n";
					}//if
				}//for
				buf << "\t\t}\n\n";
			}//if
		}//for


		if (threadIdx)
			buf << "\t\n\t\tworkerSync(" << threadIdx << ");\n";
		else
			buf << "\t\n\t\tmasterSync(" << totalCores + totalGpus << ");\n";

		buf << "\t\n\t//pthread_barrier_wait (&barrier);\n\t}\n\n";

		//稳态
		buf << "\tfor(unsigned long long stageNum=0;stageNum<" << maxStageNum << "+nRepeatCount-1;stageNum++)\n\t{\n";
		//打印稳态执行
		for (int i = maxStageNum - 1; i >= 0; --i)
		{
			set<int>::iterator stageIter = tmpStages->find(i);
			if (stageIter != endIter)
			{
				buf << "\t\tif(stage[" << i << "])\n\t\t{\n";
				vector<FlatNode*> curFlatNodes = m_hsa->getFlatNodesOfHClusterAndStage(hClusterIdx, i);
				for (vector<FlatNode*>::iterator iter1 = curFlatNodes.begin(); iter1 != curFlatNodes.end(); ++iter1)
				{
					pair<int, int> ret_0 = m_hcp->getHClusterCoreNum(*iter1);
					if (ret_0.first == hClusterIdx && ret_0.second == threadIdx)
					{
						buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
					}//if
				}//for
				buf << "\t\t}\n\n";
			}//if
		}//for


		buf << "\t\tfor(int index=" << maxStageNum - 1 << "; index>= 1; --index)\n";
		buf << "\t\t\tstage[index] = stage[index-1];\n\t\tif(stageNum == (nRepeatCount - 1))\n\t\t{\n\t\t\tstage[0]=0;\n\t\t}\n\n";

		if (threadIdx)
			buf << "\t\n\t\tworkerSync(" << threadIdx << ");\n";
		else
			buf << "\t\n\t\tmasterSync(" << totalCores + totalGpus << ");\n";

		buf << "\t\n\t//pthread_barrier_wait (&barrier);\n\t}\n\n";

		//对该线程内部的所有actor产生flush操作，发送完缓冲区中的所有数据，但如果actor没有与其他cluster上的actor通信此flush执行空操作
		for (vector<FlatNode*>::iterator iter1 = tmpFlatNodes.begin(); iter1 != tmpFlatNodes.end(); ++iter1)
		{
			buf << "\t" << (*iter1)->name << "_obj.flush();\n";
		}//for


		buf << "\n}\n\n";
	}
	/*GPU混合架构服务器*/
	else {
		buf << "#if defined(__APPLE__) || defined(__MACOSX)\n";
		buf << "#include <OpenCL/cl.hpp>\n";
		buf << "#else\n";
		buf << "#include <CL/cl.h>\n";
		buf << "#endif\n\n";

		buf << "\nusing namespace std;\n";

		buf << "//extern pthread_barrier_t barrier;\n";
		buf << "extern int nRepeatCount;\n";

		//先声明MPI的类型（由结构体转换而来的）
		for (map<string, List *>::iterator iter1 = m_struct2FieldList.begin(); iter1 != m_struct2FieldList.end(); iter1++)
		{
			buf << "extern " << "MPI::Datatype " << iter1->first << "_mpitype" << ";\n";
		}//for

		map<int, set<int>> tmpCore2Stage,tmpGpu2Stage;
		map<int, set<int>>::iterator cIter,gIter;

		tmpCore2Stage = m_hCluster2CoreNum2Stage.find(hClusterIdx)->second;
		tmpGpu2Stage = m_hCluster2GpuNum2Stage.find(hClusterIdx)->second;
		vector<FlatNode*> tmpFlatNodes;
		set<int> *tmpStages;
		if (threadIdx < totalGpus)
		{
			gIter = tmpGpu2Stage.find(threadIdx);
			//判错
			assert(gIter != tmpGpu2Stage.end());
			tmpStages = &gIter->second;

			tmpFlatNodes = m_hcp->getHClusterGpu2StatelessNodes(hClusterIdx)[threadIdx];
			buf << "\nextern cl_program program;\n";
			buf << "extern cl_device_id *devices;\n";
			buf << "extern cl_context context;\n\n";
			/*buf << "extern cl_command_queue cQueue_0_0;\n";
			buf << "extern cl_command_queue cQueue_0_1;\n";
			buf << "extern cl_command_queue cQueue_1_0;\n";
			buf << "extern cl_command_queue cQueue_1_1;\n";
			buf << "extern cl_command_queue cQueue_2_0;\n";
			buf << "extern cl_command_queue cQueue_2_1;\n";*/
			buf << "extern cl_command_queue cQueue[3];\n";

			buf << "void thread_" << hClusterIdx << "_" << idxName << "_fun()\n{\n";

			/*定义线程内部节点间的通信缓冲区*/
			for (vector<FlatNode*>::iterator iter2 = tmpFlatNodes.begin(); iter2 != tmpFlatNodes.end(); ++iter2)
			{
				pair<int, int> ret_0 = m_hcp->getHClusterGpuNum(*iter2);
				vector<int>::iterator iter3 = (*iter2)->outPushWeights.begin();

				for (vector<FlatNode*>::iterator iter4 = (*iter2)->outFlatNodes.begin(); iter4 != (*iter2)->outFlatNodes.end(); ++iter4)
				{
					/*找到当前出边节点所在的集群节点编号*/
					int hClusterNum = m_hcp->getHClusterNumOfNode(*iter4);
					if (ret_0.first == hClusterNum)
					{
						if (m_hcp->isNodeInGPU(hClusterNum, *iter4))
						{
							//如果在同一服务器且在同一GPU
							pair<int, int> ret_1 = m_hcp->getHClusterGpuNum(*iter4);
							if (ret_0.second == ret_1.second)
							{
								string edgename = (*iter2)->name + "_" + (*iter4)->name;
								int stagediff = m_hsa->getStageNum(*iter4) - m_hsa->getStageNum(*iter2);
								int size = (m_sssg->GetInitCount(*iter2) + m_hcp->getSteadyCount(*iter2) * (stagediff + 1)) * (*iter3);

								++iter3;

								map<string, string>::iterator iter5 = m_stream2StructType.find(edgename);
								assert(iter5 != m_stream2StructType.end());

								buf << "\tBuffer<" << iter5->second << ">" << edgename << "(" << size << ");\n";

							}//if
						}//if
					}//if
				}//for
			}//for	
		}//if
		else {
			cIter = tmpCore2Stage.find(threadIdx-totalGpus);
			//判错
			assert(cIter != tmpCore2Stage.end());
			tmpStages = &cIter->second;

			tmpFlatNodes = m_hcp->getHClusterCore2FlatNodes(hClusterIdx)[threadIdx-totalGpus];

			buf << "void thread_" << hClusterIdx << "_" << idxName << "_fun()\n{\n";

			/*定义线程内部节点间的通信缓冲区*/
			for (vector<FlatNode*>::iterator iter2 = tmpFlatNodes.begin(); iter2 != tmpFlatNodes.end(); ++iter2)
			{
				pair<int, int> ret_0 = m_hcp->getHClusterCoreNum(*iter2);
				vector<int>::iterator iter3 = (*iter2)->outPushWeights.begin();

				for (vector<FlatNode*>::iterator iter4 = (*iter2)->outFlatNodes.begin(); iter4 != (*iter2)->outFlatNodes.end(); ++iter4)
				{
					/*找到当前出边节点所在的集群节点编号*/
					int hClusterNum = m_hcp->getHClusterNumOfNode(*iter4);
					if (ret_0.first == hClusterNum)
					{
						if (m_hcp->isNodeInCPU(hClusterNum, *iter4))
						{
							//如果在同一服务器且在同一核
							pair<int, int> ret_1 = m_hcp->getHClusterCoreNum(*iter4);
							if (ret_0.second == ret_1.second)
							{
								string edgename = (*iter2)->name + "_" + (*iter4)->name;
								int stagediff = m_hsa->getStageNum(*iter4) - m_hsa->getStageNum(*iter2);
								int size = (m_sssg->GetInitCount(*iter2) + m_hcp->getSteadyCount(*iter2) * (stagediff + 1)) * (*iter3);

								++iter3;

								map<string, string>::iterator iter5 = m_stream2StructType.find(edgename);
								assert(iter5 != m_stream2StructType.end());

								buf << "\tBuffer<" << iter5->second << ">" << edgename << "(" << size << ");\n";

							}//if
						}//if
					}//if
				}//for
			}//for

		}//else

		


		 /*对每个线程中的actor构造对象actor_obj*/
		for (vector<FlatNode*>::iterator iter1 = tmpFlatNodes.begin(); iter1 != tmpFlatNodes.end(); ++iter1)
		{
			string bufstr = "";
			buf << "\t" << (*iter1)->name << " " << (*iter1)->name << "_obj";
			bufstr += "(";
			/*找出每个actor的输入输出边，注意边之间的对应顺序*/
			vector<FlatNode*> tmpInFlatNodes = (*iter1)->inFlatNodes;
			int curHClusterNum = m_hcp->getHClusterNumOfNode(*iter1);
			for (vector<FlatNode*>::iterator inIter = tmpInFlatNodes.begin(); inIter != tmpInFlatNodes.end(); ++inIter)
			{
				int inHClusterNum = m_hcp->getHClusterNumOfNode(*inIter);
				/*在同一个集群节点中*/
				if (inHClusterNum == curHClusterNum)
				{
					bufstr += ((*inIter)->name + "_" + (*iter1)->name + ",");
				}//if
			}//for


			vector<FlatNode*> tmpOutFlatNodes = (*iter1)->outFlatNodes;
			for (vector<FlatNode*>::iterator outIter = tmpOutFlatNodes.begin(); outIter != tmpOutFlatNodes.end(); ++outIter)
			{
				int outHClusterNum = m_hcp->getHClusterNumOfNode(*outIter);
				if (outHClusterNum == curHClusterNum)
				{
					bufstr += ((*iter1)->name + "_" + (*outIter)->name + ",");
				}
			}//for

			if (bufstr.length() > 1)
			{
				buf << bufstr;
				buf.seekp((int)buf.tellp() - 1);

				buf << ");\n";
			}//if
			else {
				buf << ";\n";
			}//else
		}//for

		buf << "\tchar stage[" << maxStageNum << "]={0};\n";
		buf << "\tstage[0]=1;\n\n";

		if (threadIdx)
			buf << "\tworkerSync(" << threadIdx << ");\n\n";
		else
			buf << "\tmasterSync(" << totalCores + totalGpus << ");\n\n";

		buf << "\tfor(int stageNum=0;stageNum<" << maxStageNum << ";stageNum++)\n";
		buf << "\t{\n";

		/*打印初态执行*/
		set<int>::iterator endIter = tmpStages->end();
		for (int i = maxStageNum - 1; i >= 0; --i)
		{
			set<int>::iterator stageIter = tmpStages->find(i);
			/*该stage在当前线程上*/
			if (stageIter != endIter)
			{
				buf << "\t\tif(" << i << "==stageNum)\n\t\t{\n";
				vector<FlatNode*> curFlatNodes = m_hsa->getFlatNodesOfHClusterAndStage(hClusterIdx, i);
				for (vector<FlatNode*>::iterator iter1 = curFlatNodes.begin(); iter1 != curFlatNodes.end(); ++iter1)
				{
					pair<int, int> ret_0;
					if (m_hcp->isNodeInGPU(hClusterIdx, *iter1))
					{
						ret_0 = m_hcp->getHClusterGpuNum(*iter1);
						if (ret_0.first == hClusterIdx && ret_0.second == threadIdx)
						{
							map<FlatNode *, Bool>::iterator haveClusterCommIter;
							buf << "\t\t\t" << (*iter1)->name << "_obj.runInitScheduleWork();\n";
						}//if
					}//if
					else {
						ret_0 = m_hcp->getHClusterCoreNum(*iter1);
						if (ret_0.first == hClusterIdx && ret_0.second == threadIdx - totalGpus)
						{
							map<FlatNode *, Bool>::iterator haveClusterCommIter;
							buf << "\t\t\t" << (*iter1)->name << "_obj.runInitScheduleWork();\n";
						}//if
					}//else		 

				}//for
				buf << "\t\t}\n\n";
			}//if
		}//for
		
		


		if (threadIdx)
			buf << "\t\n\t\tworkerSync(" << threadIdx << ");\n";
		else
			buf << "\t\n\t\tmasterSync(" << totalCores+totalGpus << ");\n";

		buf << "\t\n\t//pthread_barrier_wait (&barrier);\n\t}\n\n";

		//稳态
		buf << "\tfor(unsigned long long stageNum=0;stageNum<" << maxStageNum << "+nRepeatCount-1;stageNum++)\n\t{\n";
		//打印稳态执行
		for (int i = maxStageNum - 1; i >= 0; --i)
		{
			set<int>::iterator stageIter = tmpStages->find(i);
			if (stageIter != endIter)
			{
				buf << "\t\tif(stage[" << i << "])\n\t\t{\n";
				vector<FlatNode*> curFlatNodes = m_hsa->getFlatNodesOfHClusterAndStage(hClusterIdx, i);
				for (vector<FlatNode*>::iterator iter1 = curFlatNodes.begin(); iter1 != curFlatNodes.end(); ++iter1)
				{
					pair<int, int> ret_0;
					if (m_hcp->isNodeInGPU(hClusterIdx, *iter1))
					{
						ret_0 = m_hcp->getHClusterGpuNum(*iter1);
						if (ret_0.first == hClusterIdx && ret_0.second == threadIdx)
						{
							map<FlatNode *, Bool>::iterator haveClusterCommIter;
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
						}//if
					}//if
					else {
						ret_0 = m_hcp->getHClusterCoreNum(*iter1);
						if (ret_0.first == hClusterIdx && ret_0.second == threadIdx - totalGpus)
						{
							map<FlatNode *, Bool>::iterator haveClusterCommIter;
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
						}//if
					}//else		
				}//for
				buf << "\t\t}\n\n";
			}//if
		}//for


		buf << "\t\tfor(int index=" << maxStageNum - 1 << "; index>= 1; --index)\n";
		buf << "\t\t\tstage[index] = stage[index-1];\n\t\tif(stageNum == (nRepeatCount - 1))\n\t\t{\n\t\t\tstage[0]=0;\n\t\t}\n\n";

		if (threadIdx)
			buf << "\t\n\t\tworkerSync(" << threadIdx << ");\n";
		else
			buf << "\t\n\t\tmasterSync(" << totalCores + totalGpus << ");\n";

		buf << "\t\n\t//pthread_barrier_wait (&barrier);\n\t}\n\n";

		//对该线程内部的所有actor产生flush操作，发送完缓冲区中的所有数据，但如果actor没有与其他cluster上的actor通信此flush执行空操作
		for (vector<FlatNode*>::iterator iter1 = tmpFlatNodes.begin(); iter1 != tmpFlatNodes.end(); ++iter1)
		{
			buf << "\t" << (*iter1)->name << "_obj.flush();\n";
		}//for

		//buf << "\tcout<<endl<<\"thread_"<< idxName <<" end~\"<<endl;\n";

		buf << "\n}\n\n";

	}//else
}

//生成进程文件
void HClusterCodeGenerator::CGProcess(int cluster, stringstream & buf)
{
	char str[20];
	itoa(cluster, str, 20);
	string processTag = str;

	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"Producer.h\"\n";
	buf << "#include \"Consumer.h\"\n";
	buf << "#include \"globalstream.h\"\n";
	buf << "#include \"AllActorHeader.h\"\n";
	buf << "#include \"lock_free_barrier.h\"\n";
	buf << "#include \"rdtsc.h\"\n";
	buf << "#include <mpi.h>\n";
	buf << "#include <pthread.h>\n";
	buf << "#include <fstream>\n";
	buf << "#include <iostream>\n";
	buf << "#include <string.h>\n";
	buf << "#include \"setcpu.h\"\n\n";
	
	/*判断当前服务器节点是否含有GPU*/
	if (!m_hcp->isGPUClusterNode(cluster))
	{
		buf << "using namespace std;\n";
		buf << "//extern int nRepeatCount 重复执行次数\n";
		buf << "extern int nRepeatCount;\n";

		int nCores = getCpuThreadNumOnHCluster(cluster);
		/*得到当前进程中的所有线程函数声明*/
		for (int i = 0; i < nCores; ++i)
		{
			itoa(i, str, 20);
			string threadTag = str;

			buf << "extern void thread_" << cluster << "_" << threadTag << "_fun();\n";
		}//for

		 /*当前进程中的所有线程函数入口函数声明*/
		for (int i = 1; i < nCores; ++i)
		{
			buf << "void* thread_" << cluster << "_" << i << "_fun_start(void*)\n{";
			buf << "\n\t" << "set_cpu(" << i << ");\n";
			buf << "\tthread_" << cluster << "_" << i << "_fun();\n";
			buf << "\tpthread_exit((void*)" << threadNum++ << ");\n}\n";
		}//for

		 /*进程函数名*/
		buf << "void process_" << processTag << "_handler()\n{";

		//buf << "\n\tcout<<endl<<\"process_\"<<" << processTag << "<<\" begin~\"<<endl;\n";

		/*barrier用的数组*/
		buf << "\n\tallocBarrier(" << nCores << ");\n";
		buf << "\tpthread_t tid[" << nCores - 1 << "];\n";
		buf << "\tvoid *tRet;\n";
		buf << "\tset_cpu(0);\n";

		for (int i = 1; i < nCores; ++i)
		{
			buf << "\tpthread_create(&tid[" << i - 1 << "],NULL,thread_" << cluster << "_" << i << "_fun_start,(void*)NULL);\n";
		}//for

		buf << "\tthread_" << cluster << "_0_fun();\n";

		for (int i = 1; i < nCores; ++i)
		{
			buf << "\tpthread_join(tid[" << i - 1 << "],&tRet);\n";
		}//for

		 //buf << "\n\tcout<<endl<<\"process_\"<<"<<processTag<<"<<\" end~\"<<endl;\n";


		buf << "\n}\n";
	}//if
	 //GPU混合架构服务器节点上运行的进程代码
	else {
		buf << "#if defined(__APPLE__) || defined(__MACOSX)\n";
		buf << "#include <OpenCL/cl.hpp>\n";
		buf << "#else\n";
		buf << "#include <CL/cl.h>\n";
		buf << "#endif\n\n";

		buf << "using namespace std;\n";
		buf << "//重复执行次数\n";
		buf << "extern int nRepeatCount;\n\n";

		/*生成OpenCL初始化工作*/
		buf << "size_t deviceListSize;\n";
		buf << "cl_uint numPlatforms;\n";
		buf << "cl_platform_id *platforms,platform;\n";
		buf << "cl_int status0,status1,status2,status3,status4,status5,status6;\n";
		buf << "cl_context context;\n";
		buf << "cl_device_id *devices;\n";
		buf << "cl_program program;\n";
		/*buf << "cl_command_queue cQueue_0_0;\n";
		buf << "cl_command_queue cQueue_0_1;\n";
		buf << "cl_command_queue cQueue_1_0;\n";
		buf << "cl_command_queue cQueue_1_1;\n";
		buf << "cl_command_queue cQueue_2_0;\n";
		buf << "cl_command_queue cQueue_2_1;\n";*/
		buf << "cl_command_queue cQueue[3];\n";
	
		int nCore = getCpuThreadNumOnHCluster(cluster);
		int nGpu = getGpuThreadNumOnHCluster(cluster);
		/*得到当前进程中的所有线程函数声明*/
		for (int i = 0; i < nCore; ++i)
		{
			itoa(i, str, 20);
			string threadTag = str;

			buf << "extern void thread_" << cluster << "_" << threadTag << "_fun();\n";
		}//for

		for(int i=nCore;i<nGpu+nCore;++i)	
		{ 
			itoa(i, str, 20);
			string threadTag = str;
			buf << "extern void thread_" << cluster << "_" << threadTag << "_fun();\n";
		}//for

		 /*当前进程中的所有线程函数入口函数声明*/
		for (int i = 1; i < nCore; ++i)
		{
			buf << "void* thread_" << cluster << "_" << i << "_fun_start(void*)\n{";
			buf << "\n\t//" << "set_cpu(" << i << ");\n";
			buf << "\tthread_" << cluster << "_" << i << "_fun();\n";
			buf << "\tpthread_exit((void*)" << threadNum++ << ");\n}\n";
		}//for

		for (int i = nCore; i < nCore + nGpu; ++i)
		{
			buf << "void* thread_" << cluster << "_" << i << "_fun_start(void*)\n{";
			buf << "\n\t//" << "set_cpu(" << i << ");\n";
			buf << "\tthread_" << cluster << "_" << i << "_fun();\n";
			buf << "\tpthread_exit((void*)" << threadNum++ << ");\n}\n";
		}

		 /*进程函数名*/
		buf << "void process_" << processTag << "_handler()\n{";

		//buf << "\n\tcout<<endl<<\"process_\"<<" << processTag << "<<\" begin~\"<<endl;\n";
		buf << "\n\t//(1)得到并选择可用平台\n";
		buf << "\tstatus0 = clGetPlatformIDs(0,NULL,&numPlatforms);\n";
		buf << "\tif(status0 != CL_SUCCESS || numPlatforms<=0){\n\t\t";
		buf << "cout<<\"Error: First clGetPlatformIDs failed!\"<<endl;\n\t\texit(1);\n\t}\n";
		buf << "\tplatforms= (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));\n";
		buf << "\tstatus0 = clGetPlatformIDs(numPlatforms, platforms, NULL);\n";
		buf << "\tif(status0 != CL_SUCCESS || platforms == NULL){\n\t\t";
		buf << "cout<<\"Error: Second clGetPlatformIDs failed!\"<<endl;\n\t\texit(1);\n\t}\n";

		buf << "\tfor(unsigned int i=0;i<numPlatforms;++i)\n\t{\n\t";
		buf << "\tchar pbuff[100];\n\t";
		buf << "\tstatus0=clGetPlatformInfo(platforms[i],CL_PLATFORM_VENDOR,sizeof(pbuff), pbuff, NULL);\n\t";
		buf << "\tplatform = platforms[i];\n\t";
		buf << "\t//cout<<pbuff<<endl;\n\t";
		buf << "\tif(!strcmp(pbuff, \"NVIDIA Corporation\")){\n\t\t\t//cout<<\"find!\"<<endl;\n\t\t\tbreak;}\n\t";
		buf << "}\n";
		buf << "\tdelete platforms;\n";

		buf << "\t//(2)如果我们能找到相应平台，就使用它，否则返回NULL\n";
		buf << "\tcl_context_properties cps[3] = {CL_CONTEXT_PLATFORM,(cl_context_properties)platform,0};\n";
		buf << "\tcl_context_properties *cprops = (NULL == platform) ? NULL : cps;\n";

		buf << "\t//(3)生成 context\n";
		buf << "\tcontext = clCreateContextFromType(cprops,CL_DEVICE_TYPE_GPU,NULL,NULL,&status1);\n";
		buf << "\tif(status1 != CL_SUCCESS){\n\t\t";
		buf << "cout<<\"Error: clCreateContextFromType Failed!\"<<endl;\n\t\texit(1);\n\t}\n";
		
		buf << "\t//(4)寻找OpenCL设备,首先,得到设备列表的长度\n";
		buf << "\tstatus2= clGetContextInfo(context,CL_CONTEXT_DEVICES,0,NULL,&deviceListSize);\n";
		buf << "\t//cout<<endl<<\"deviceListSize = \"<<deviceListSize<<endl;\n";
		buf << "\tif(status2 != CL_SUCCESS){\n\t\t";
		buf << "cout<<\"Error: clGetContextInfo Failed!\"<<endl;\n\t\texit(1);\n\t}\n";
		
		buf << "\t//(5)寻找OpenCL设备,然后，得到设备列表\n";
		buf << "\tdevices = (cl_device_id *)malloc(deviceListSize);\n";
		buf << "\tstatus3 = clGetContextInfo(context,CL_CONTEXT_DEVICES,deviceListSize,devices,NULL);\n";
		buf << "\tif(devices == 0){\n\t\t";
		buf << "cout<<\"Error: No devices found!\"<<endl;\n\t\texit(1);\n\t}\n";
		buf << "\tif(status3 != CL_SUCCESS){\n\t\t";
		buf << "cout<<\"Error: clGetContextInfo Failed!\"<<endl;\n\t\texit(1);\n\t}\n";
		
		buf << "\t//(6)读取内核文件，装载内核程序\n";
		buf << "\tifstream programFile(\"allKernel.cl\");\n";
		buf << "\tstring kernelSource(istreambuf_iterator<char>(programFile),(istreambuf_iterator<char>()));\n";
		buf << "\tconst char *source = kernelSource.c_str();\n";
		buf << "\tsize_t sourceSize[] = {kernelSource.length()};\n";
		buf << "\tprogram = clCreateProgramWithSource(context,1,&source,sourceSize,&status4);\n";

		buf << "\tif(status4 != CL_SUCCESS){\n\t\t";
		buf << "cout<<\"Error: clCreateProgramWithSource Failed!\"<<endl;\n\t\texit(1);\n\t}\n";
		
		buf << "\t//(7)编译CL program\n";
		buf << "\tstatus5 = clBuildProgram(program, 3, devices, NULL, NULL, NULL);\n\n";
		buf << "\tif(status5 != CL_SUCCESS){\n\t\t";
		buf << "cout<<\"Error: clBuildProgram Failed!\"<<endl;";
		buf << "\n\t\tsize_t len;\n\t\tchar *log;\n\t\tcl_int errCode = clGetProgramBuildInfo(program, *devices, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);";
		buf << "\n\t\tif(errCode){\n\t\t\tprintf(\"clGetProgramBuildInfo failed at line %d\\n\", __LINE__);\n\t\t\texit(-1);\n\t\t}";
		buf << "\n\t\tlog = (char *)malloc(len);";
		buf << "\n\t\tif(!log){\n\t\t\tprintf(\"malloc failed at line %d\\n\", __LINE__);\n\t\t\texit(-2);\n\t\t}";

		buf << "\n\t\terrCode= clGetProgramBuildInfo(program, *devices, CL_PROGRAM_BUILD_LOG, len, log, NULL);";
		buf << "\n\t\tif(errCode){\n\t\t\tprintf(\"clGetProgramBuildInfo failed at line %d\\n\", __LINE__);\n\t\t\texit(-3);\n\t\t}";

		buf << "\n\t\tfprintf(stderr,\"Build log : \\n%s\\n\", log);";
		buf << "\n\t\tfree(log);";
		buf << "\n\t\tfprintf(stderr,\"clBuildProgram failed\\n\");";
		buf << "\n\t\texit(EXIT_FAILURE);\n\t}\n";

		buf << "\t//(8)创建一个OpenCL command queue\n";
		/*buf << "\tcQueue_0_0 = clCreateCommandQueue(context, devices[0], 0, &status6);\n";
		buf << "\tcQueue_0_1 = clCreateCommandQueue(context, devices[0], 0, &status6);\n";
		buf << "\tif(cQueue_0_0 == NULL || cQueue_0_1 == NULL){\n\t\t";
		buf << "cout<<\"Error: \"<<\" Failed to create cQueue_0_0 or cQueue_0_1.\"<<endl;\n\t\texit(1);\n\t}\n";
		buf << "\tcQueue_1_0 = clCreateCommandQueue(context, devices[1], 0, &status6);\n";
		buf << "\tcQueue_1_1 = clCreateCommandQueue(context, devices[1], 0, &status6);\n";
		buf << "\tif(cQueue_1_0 == NULL || cQueue_1_1 == NULL){\n\t\t";
		buf << "cout<<\"Error: \"<<\" Failed to create cQueue_1_0 or cQueue_1_1.\"<<endl;\n\t\texit(1);\n\t}\n";
		buf << "\tcQueue_2_0 = clCreateCommandQueue(context, devices[2], 0, &status6);\n";
		buf << "\tcQueue_2_1 = clCreateCommandQueue(context, devices[2], 0, &status6);\n";
		buf << "\tif(cQueue_2_0 == NULL || cQueue_2_1 == NULL){\n\t\t";
		buf << "cout<<\"Error: \"<<\" Failed to create cQueue_2_0 or cQueue_2_1.\"<<endl;\n\t\texit(1);\n\t}\n";
*/
		buf << "\tcQueue[0] = clCreateCommandQueue(context, devices[0], 0, &status6);\n";
		buf << "\tif(cQueue[0] == NULL){\n\t\t";
		buf << "cout<<\"Error: \"<<\" Failed to create cQueue[0].\"<<endl;\n\t\texit(1);\n\t}\n";

		buf << "\tcQueue[1] = clCreateCommandQueue(context, devices[1], 0, &status6);\n";
		buf << "\tif(cQueue[1] == NULL){\n\t\t";
		buf << "cout<<\"Error: \"<<\" Failed to create cQueue[1].\"<<endl;\n\t\texit(1);\n\t}\n";

		buf << "\tcQueue[2] = clCreateCommandQueue(context, devices[2], 0, &status6);\n";
		buf << "\tif(cQueue[2] == NULL){\n\t\t";
		buf << "cout<<\"Error: \"<<\" Failed to create cQueue[2].\"<<endl;\n\t\texit(1);\n\t}\n";


		/*barrier用的数组*/
		buf << "\n\tallocBarrier(" << nCore+nGpu << ");\n";
		buf << "\tpthread_t tid[" << nCore + nGpu - 1 << "];\n";
		buf << "\tvoid *tRet;\n";
		buf << "\t//set_cpu(0);\n";

		for (int i = 1; i < nCore+nGpu; ++i)
		{
			buf << "\tpthread_create(&tid[" << i - 1 << "],NULL,thread_" << cluster << "_" << i << "_fun_start,(void*)NULL);\n";
		}//for

		buf << "\tthread_" << cluster << "_0_fun();\n";

		for (int i = 1; i <  nCore + nGpu; ++i)
		{
			buf << "\tpthread_join(tid[" << i - 1 << "],&tRet);\n";
		}//for

		buf << "\tclReleaseProgram(program);\n";
		buf << "\tclReleaseContext(context);\n";
		/*	buf << "\tclReleaseCommandQueue(cQueue_0_0);\n";
			buf << "\tclReleaseCommandQueue(cQueue_0_1);\n";
			buf << "\tclReleaseCommandQueue(cQueue_1_0);\n";
			buf << "\tclReleaseCommandQueue(cQueue_1_1);\n";
			buf << "\tclReleaseCommandQueue(cQueue_2_0);\n";
			buf << "\tclReleaseCommandQueue(cQueue_2_1);\n";*/
		buf << "\tclReleaseCommandQueue(cQueue[0]);\n";
		buf << "\tclReleaseCommandQueue(cQueue[1]);\n";
		buf << "\tclReleaseCommandQueue(cQueue[2]);\n";
		buf << "\tfree(devices);\n";

		//buf << "\n\tcout<<endl<<\"process_\"<<" << processTag << "<<\" end~\"<<endl;\n";
		buf << "\n}\n";
	}//else
}

/*为每个异构集群节点创建独立的进程*/
void HClusterCodeGenerator::CGProcesses()
{
	string processName, processTag;
	stringstream buf;
	char str[20];
	assert(m_hcp->getHClusters() == m_hcp->getHClusterNum2FlatNodes().size());

	for (int i = 0; i < m_hcp->getHClusters(); ++i)
	{	
		if (m_hcp->isGPUClusterNode(i))
		{
			/*对于GPU的进程文件，当目标代码版本标志为true时，生成相应代码文件*/
			if (CurGpuFlag)
			{
				buf << "/*每个服务器节点对应的进程文件*/";
				stringstream out;
				//将当前进程编号转化为字符串形式
				itoa(i, str, 20);
				processTag = str;

				processName = "process_" + processTag;
				out << m_curDir << processName + ".cpp";

				//生成进程内容文件
				CGProcess(i, buf);

				outputToFile(out.str(), buf.str());
				out.str("");
				buf.str("");
			}//if
		}
		else {
			buf << "/*每个服务器节点对应的进程文件*/";
			stringstream out;
			//将当前进程编号转化为字符串形式
			itoa(i, str, 20);
			processTag = str;

			processName = "process_" + processTag;
			out << m_curDir << processName + ".cpp";

			//生成进程内容文件
			CGProcess(i, buf);

			outputToFile(out.str(), buf.str());
			out.str("");
			buf.str("");
		}
		
	}//for
}

/*生成主程序*/
void HClusterCodeGenerator::CGMain()
{
	stringstream buf, out;
	char str[20];

	/*生成包含标准库文件的语句*/
	buf << "#include <iostream>\n";
	buf << "#include <stdio.h>\n";
	buf << "#include <stdlib.h>\n";
	buf << "#include <string.h>\n";
	buf << "#include <mpi.h>\n";
	buf << "#include <vector>\n";
	buf << "#include <sstream>\n";
	buf << "#include <fstream>\n";

	/*生成包含统计时间所需要的标准库文件的语句*/
	if (RUNTIME)
	{
		buf << "#include \"rdtsc.h\"\n";
	}

	//生成包含依赖文件的语句
	buf << "#include \"globalstream.h\"\n";
	buf << "#include \"lock_free_barrier.h\"\t//包含barrier函数\n\n";
	buf << "using namespace std;\n";

	/*各个进程函数声明语句*/
	for (int i = 0; i < m_hcp->getHClusters(); ++i)
	{
		itoa(i, str, 20);
		string processTag = str;
		if (m_hcp->isGPUClusterNode(i))
		{
			if(CurGpuFlag)
				buf << "extern void process_" << processTag << "_handler();\n";
			else
				buf << "//extern void process_" << processTag << "_handler();\n";
		}
		else {
			buf << "extern void process_" << processTag << "_handler();\n";
		}
		
	}//for

	/*MPI类型声明（由结构体转换而来）*/
	for (map<string, List*>::iterator iter = m_struct2FieldList.begin(); iter != m_struct2FieldList.end(); ++iter)
	{
		buf << "MPI::Datatype " << iter->first << "_mpitype" << ";\n";
	}//for

	for (map<string, List*>::iterator iter = m_struct2FieldList.begin(); iter != m_struct2FieldList.end(); ++iter)
	{
		buf << iter->first << " " << iter->first << "_obj;\n";
	}//for

	/*声明默认执行次数的变量*/
	buf << "unsigned long long nRepeatCount = " << Default_Repeat_Count << ";//程序默认执行次数\n";

	//定义MPI通信变量
	buf << "MPI::Intracomm comm;\n";
	//mpi变量 rank，size定义
	buf << "int rank = -1;\nint size = -1;\n\n";

	//main入口函数语句
	buf << "int main(int argc, char **argv)\n{\n";
	buf << "\tint n;\n";
	buf << "\twhile((n=getopt(argc,argv,\"i:\"))!=-1)\n";
	buf << "\t{\n";
	buf << "\t\tswitch(n)\n\t\t{\n";
	buf << "\t\t\tcase 'i':\n";
	buf << "\t\t\t\tnRepeatCount==atoi(optarg);\n";
	buf << "\t\t\t\tbreak;\n";
	buf << "\t\t}\n\n";
	buf << "\t}\n\n";

	buf << "\n\t/*MPI函数*/\n";
	//初始化MPI主进程
	buf << "\tint provided;\n";
	buf << "\tcomm = MPI::COMM_WORLD;\n";
	buf << "\tprovided = MPI::Init_thread(argc, argv, MPI_THREAD_MULTIPLE);\n";
	buf << "\tif(provided != MPI_THREAD_MULTIPLE)\n\t{\n\t\tcout<<\"MPI do not Support Multiple thread.\"<<endl;\n\t\texit(0);\n\t}\n\n";
	buf << "\trank = MPI::COMM_WORLD.Get_rank();\n";
	buf << "\tsize = MPI::COMM_WORLD.Get_size();\n";

	// 向MPI提交由结构体类型的转换的MPIDatatype
	CommitMPIDataType(buf);

	//统计时间
	if (RUNTIME)
	{
		buf << "\ttsc_counter main_Begin" << ";\n";
		buf << "\ttsc_counter main_End" << ";\n";
		buf << "\tRDTSC(main_Begin);\n";

	}//if

	/*执行相应的进程*/
	buf << "\tswitch(rank)\n\t{\n";
	for (int i = 0; i < m_hcp->getHClusters(); ++i)
	{
		itoa(i, str, 20);
		string processTag = str;
		stringstream processStream;
		if (m_hcp->isGPUClusterNode(i))
		{
			if (CurGpuFlag)
			{
				buf << "\t\tcase " << i << ":\n";
				//processStream << "\t\t\tcout<<\"process_\"<<rank<<\" begin~\"<<endl;\n";
				processStream << "\t\t\tprocess_" + processTag + "_handler();\n";
				processStream << "\t\t\tcout<<\"process_\"<<rank<<\" end~\"<<endl;\n";
				processStream << "\t\t\tbreak; \n";
			}//else
			else {
				buf << "\t\t//case " << i << ":\n";
				//processStream << "\t\t\tcout<<\"process_\"<<rank<<\" begin~\"<<endl;\n";
				processStream << "\t\t\t//process_" + processTag + "_handler();\n";
				processStream << "\t\t\t//cout<<\"process_\"<<rank<<\" end~\"<<endl;\n";
				processStream << "\t\t\t//break; \n";
			}//else
		}
		else {
			buf << "\t\tcase " << i << ":\n";
			//processStream << "\t\t\tcout<<\"process_\"<<rank<<\" begin~\"<<endl;\n";
			processStream << "\t\t\tprocess_" + processTag + "_handler();\n";
			processStream << "\t\t\tcout<<\"process_\"<<rank<<\" end~\"<<endl;\n";
			processStream << "\t\t\tbreak; \n";
		}
		
		buf << processStream.str();
	}//for
	buf << "\t\tdefault: \t\tbreak;\n";
	buf << "\t}\n\n";

	for (map<string, List *>::iterator iter = m_struct2FieldList.begin(); iter != m_struct2FieldList.end(); iter++)
	{
		buf << "\t" << iter->first << "_mpitype.Free();\n";
	}
	buf << "\tMPI::Finalize();\n";

	//关于统计时间的语句
	if (RUNTIME)
	{
		buf << "\tRDTSC(main_End);\n";
		buf << "\tunsigned long long MaincomputeTime = main_End.int64 -  main_Begin.int64;\n";
		buf << "\tcout<<\"The total time = \"<<MaincomputeTime<<endl;\n";
		//将runtimeInfoStream中的内容写入到文件中
		string runtimeFileName = "runtime.txt";
		buf << "\n\tstring runtimeFileName = \"" << runtimeFileName << "\";\n";
		buf << "\tstringstream runtimeStream;\n";
		buf << "\n\truntimeStream<<\"MainComputeTime = \"<<" << "MaincomputeTime " << "* nRepeatCount" << "<<\"\\n\";\n";
		buf << "\tofstream fw;\n\ttry\n\t{\n\t\tfw.open(runtimeFileName.c_str());\n\t\tfw<<runtimeStream.str();\n\t\tfw.close();\n\t}\n\tcatch(...)\n\t{\n\t\tcout<<\"error:output to file\"<<endl;\n\t}\n";
	}


	buf << "\treturn 0;\n";
	buf << "\n}\n";

	/*在当前目录下生成main.cpp文件，并写入内容*/
	out << m_curDir << "main.cpp";
	outputToFile(out.str(), buf.str());
}