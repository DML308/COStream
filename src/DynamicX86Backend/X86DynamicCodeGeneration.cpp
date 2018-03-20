#include "X86DynamicCodeGeneration.h"
static bool isInComma = false;//表示当前处于逗号表达式中
static bool isEnumVar = false;
static bool isstructVar = false;//表示该变量为结构体内变量
static bool isUnionVar = false;
static bool isCGGlobalVar = false;//表示目前正处于生成全局变量
static int flag_Global = 0;//标识是否在输出GlobalVarCpp文件，初始化为0
static bool Filerw = 0;//标识是否有文件读写actor
static bool FileR = 0; // 标识是否有文件读 actor
static bool FileW = 0; // 标识是否有文件写 actor
static bool isInFileReader = false;//表示当前处于FileReader中
static bool isInFileWriter = false;//表示当前处于FileWriter中
static int batch = 100;		//batching
static bool isExternType = false; //表示目前正处于生成嵌入式的结构文件ExternType中
static bool needExternType = false; //表示时候需要生成接口文件
#ifdef WIN32
#define mkdir(tmp) _mkdir(tmp.c_str());
#else
#define mkdir(tmp) mkdir(tmp.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif	

DynamicX86CodeGenerate::DynamicX86CodeGenerate(DividedStaticGraph*dsg, int nCpucore, const char*currentDir, vector<StageAssignment*>assignmentlist, vector<MetisPartiton*>_mplist)
{
	n_fdcl = 0;
	declInitLen = 0;
	flag = 0;
	psalist = assignmentlist;
	mplist = _mplist;
	dsg_= dsg;
	flatNodes_ = dsg->getAllFlatNodes();
//	pEdgeInfo = new ActorEdgeInfo(Sssg);//这个edgeinfo一会再改
	nCpucore_ = nCpucore;
	nActors_ = flatNodes_.size();
	readerActor = writerActor = NULL;
	dir_ = currentDir;
	pEdgeInfo = new ActorEdgeInfo(dsg);
	//建立代码生成目标文件的目录

	int index = 0;
	int i = 0;
	while (1)
	{
		string::size_type pos = dir_.find("\\", index);
		string tmp;
		tmp = dir_.substr(0, pos);

		if (pos == -1) break;
		else if (i > 0) mkdir(tmp);
		i++;
		index = pos + 1;
	}
	
	//建立mapActor2InEdge，mapActor2OutEdge以及readActor，writeActor
	vector<FlatNode*>::iterator iter1, iter2, iter3;		//iter1表示当前结点，iter2用于遍历其输入节点，iter3用于遍历其输出节点
	for (iter1 = flatNodes_.begin(); iter1 != flatNodes_.end(); iter1++)
	{
		//设置节点输入边信息

		
		for (iter2 = (*iter1)->inFlatNodes.begin(); iter2 != (*iter1)->inFlatNodes.end(); ++iter2)
		{
			string tmpString = (*iter2)->name + "_" + (*iter1)->name;		//得到形如A_B的边
			mapActor2InEdge.insert(make_pair((*iter1), tmpString));
		}
		for (iter3 = (*iter1)->outFlatNodes.begin(); iter3 != (*iter1)->outFlatNodes.end(); ++iter3)
		{
			string tmpString = (*iter1)->name + "_" + (*iter3)->name; 
			mapActor2OutEdge.insert(make_pair((*iter1), tmpString));
		}

		if ((*iter1)->name.find("FileReader") != -1)		//当前结点名字中出现FileReader
		{
			readerActor = (*iter1);
			FileR = 1;					//存在文件读操作
		}
		if ((*iter1)->name.find("FileWriter") != -1)		//当前结点名字中出现FileWriter
		{
			writerActor = (*iter1);
			FileW = 1;				//存在文件写
		}
	}
	//构造每个线程上的stage集合列表
	//按子图数目分组
	vector<FlatNode*>tempactors;
	vector<FlatNode*>::iterator iter;
	for (int i = 0; i < dsg->getCount(); i++)		//子图循环
	{
		//每一个子图创建一个map，然后插入vector
		map<int, set<int> > tmp;	
		for (int j = 0; j < nCpucore; j++)			//子图内核循环,一共有如下阶段
		{
			set<int>tempstageset;		//临时set
			tempstageset.clear();
			tempactors = mplist[i]->findNodeSetInPartition(j);//找到对应核上的算子组合
			for (iter = tempactors.begin(); iter != tempactors.end(); iter++)
			{
				tempstageset.insert(psalist[i]->FindStage(*iter));
			}
			tmp.insert(make_pair(j, tempstageset));
		}
		mapNum2StageList.push_back(tmp);//一个每个子图的每个线程对应的stage列表
	}
	//这两个变量不知道什么作用
	extractDecl = false;
	isInParam = false;

	threadNum = 0;
	vector<CombineState>::iterator iterThreadNum = dsg_->combineList.begin();
	while (iterThreadNum != dsg_->combineList.end())
	{
		if ((*iterThreadNum) == FULLCORE || (*iterThreadNum) == NOTMERGE)	//使用8个核或者合并效率低
			threadNum += nCpucore_;
		else if ((*iterThreadNum) == MERGEONE || (*iterThreadNum) == MERGEDOWNONE || (*iterThreadNum) == MERGEUPONE)
		{
			threadNum += nCpucore_;
			iterThreadNum++;//跳过下一个
		}
		iterThreadNum++;
	}
	setGraphindex2coreNum();
}
void DynamicX86CodeGenerate::DYMain()
{
	stringstream buf, ss;
	char a[10];
	string Tab;
	if (CallModelEmbed){}
	else
	{
		buf << "#include \"iostream\"\n";
		buf << "#include \"stdlib.h\"\n";
		buf << "#include \"stdio.h\"\n";
		buf << "#include <pthread.h>\n";
		buf << "#include \"setCpu.h\"\n";
		buf << "#include \"lock_free_barrier.h\"	//包含barrier函数\n";
		buf << "#include \"global.h\"\n";
		if (gfrtaCallList != NULL){
			buf << "#include \"frta.h\"\n";
			buf << "#include \"GlobalVar.h\"\n";
		}
		buf << "using namespace std;\n";
	}
	buf << Tab << "int MAX_ITER=1;//默认的执行次数是1\n";

	//线程声明，主要包含如下几部分，1，各静态子图的线程启动，2.压缩/合并子图的线程启动
	//不合并的子图每个需要8个线程，合并的子图两者只需要8个线程

	//"thread_" << i / nCpucore_ << "_" << i%nCpucore_ << ".cpp";
	//buf << "void thread_" << index / nCpucore_ << "_" << index%nCpucore_ << "_fun()\n{\n";
	//thread_15/8_15%8_fun()
	//thread_1_7_fun()
	for (int i = 0; i < threadNum; i++)
	{
		buf << Tab << "extern void thread_" << i/nCpucore_<<"_"<<i%nCpucore_<< "_fun();\n";
	}
	for (int i = 0; i < threadNum; i++)
	{
		buf << Tab << "void* thread_" << i/nCpucore_<<"_"<<i%nCpucore_<< "_fun_start(void *)\n";
		buf << Tab << "{\n\t" << Tab << "set_cpu(";

		//由于线程数目足够多，所以setCpu需要修改
		buf << i%nCpucore_;
		buf<<");\n\t" << Tab << "thread_" << i/nCpucore_<<"_"<<i%nCpucore_<< "_fun();\n\t" << Tab << "return 0;\n" << Tab << "}\n";
	}
	if (CallModelEmbed){}
	else				//独立模式，生成main
	{
		buf << "int main(int argc,char **argv)\n{\n";
		buf << "\tvoid setRunIterCount(int,char**);\n";
		if (gfrtaCallList != NULL){
			COSX86_List(gfrtaCallList, 0);
			buf << "\t";
			buf << declInitList.str();
			buf << "\n";
			declInitList.str(""); // 清空declInitList内容
			CGFrta();
		}
		buf << "\tsetRunIterCount(argc,argv);\n";
		//buf << "\tset_cpu(0);\n";
		//buf << "\tallocBarrier(" << nCpucore_ << ");\n";
		//线程启动前完成全局变量初始化
		for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
		{
			buf << "sem_init(&zu" << i << ",0,0);\n";
		}
		for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
		{
			buf << "sem_init(&exchange" << i << ",0,0);\n";
		}
		for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
		{
			//buf << "pthread_barrier_init(&barrier" << i << ",NULL," << (dsg_->ssg2coreNum[i]).second << ");\n";
		}
		buf << "\tallocBarrier(" << nCpucore_ << ");\n";

		//锁的处理
		bool hasDSG = false;
		for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
		{
			if (dsg_->combineList[i] == MERGEDOWNONE || dsg_->combineList[i] == MERGEONE || dsg_->combineList[i] == MERGEUPONE)
			{
				hasDSG = true;
			}
		}
		if (hasDSG == true)
		{
			for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
			{
				if (dsg_->combineList[i] == MERGEONE || dsg_->combineList[i] == MERGEUPONE || dsg_->combineList[i] == MERGEDOWNONE)
				{
					//设置对应i子图的锁
					buf << "pthread_mutex_init(&mutex" << i << ",NULL);\n";
				}
			}
		}


		buf << "\tpthread_t tid[" << threadNum << "];\n";
		for (int i = 0; i < threadNum; i++)
		{
			buf << "\tpthread_create (&tid[" << i << "], NULL, thread_" << i/nCpucore_<<"_"<<i%nCpucore_ << "_fun_start, (void*)NULL);\n";
		}
		//此处需要修改
		//buf << "\tthread_0_fun();\n";
		/*for (int i = 0; i < threadNum; i++)
		{
			if (i % 8 == 0)
			{
				buf << "\tthread_";
				buf << i;
				buf << "_fun(); \n";
			}
		}*/

		//main要等待各子图主线程结束
		//map<int, int>::iterator iterForCore = graphIndex2coreNum.begin();
		int usecore = 0;
		while (usecore < threadNum)
		{
			buf << "\tpthread_join(tid["<<usecore << "],NULL);\n";
			usecore++;
		}
		/*while (iterForCore != graphIndex2coreNum.end())
		{
			
			
			buf << "\tpthread_join(tid[" << usecore << "],NULL);\n";
			usecore += iterForCore->second;
			iterForCore++;
		}*/

		buf << "\treturn 0;\n";
		buf << "\n}\n";
		buf << "void setRunIterCount(int argc,char **argv)\n{\n";
		buf << "\tint oc;\n";
		buf << "\tchar *b_opt_arg;\n";
		buf << "\twhile((oc=getopt(argc,argv,\"i:\"))!=-1)\n";
		buf << "\t{\n";
		buf << "\t\tswitch(oc)\n";
		buf << "\t\t{\n";
		buf << "\t\t\tcase 'i':\n";
		buf << "\t\t\t\tMAX_ITER=atoi(optarg);\n";
		buf << "\t\t\t\tbreak;\n";
		buf << "\t\t}\n";
		buf << "\t}\n";
		buf << "}\n";
		ss << dir_ << "main.cpp";
		
	}
	OutputToFile(ss.str(), buf.str());
	if (CallModelEmbed){}
}








//public 生成缓冲区声明函数，这个需要修改,需要把动态输出边在这里直接定义出来
void DynamicX86CodeGenerate::DYGlobalHeader()
{
	stringstream buf;
	string edgename;
	ListMarker marker;
	Node *node;
	vector<FlatNode *>::iterator iter_1, iter_2;
	buf << "#ifndef _GLOBAL_H\n";
	buf << "#define _GLOBAL_H\n";
	buf << "/*全局变量，用于存储边的信息*/\n";
	buf << "/*边的命名规则：A_B,其中A->B*/\n\n";
	buf << "/*压缩算子动态边需求2条*/\n";
	buf << "/*动态边全部在这里定义*/\n";

	if (FileR || FileW)		//存在文件读写操作
	{
		buf << "#include <sstream>\n";
		buf << "#include <fstream>\n";
		buf << "#include \"tinystr.h\"\n";
		buf << "#include \"tinyxml.h\"\n";
	}
	IterateList(&marker, Program);
	while (NextOnList(&marker, (GenericREF)&node)) {
		if (node == NULL)
			continue;

		if (node->coord.includedp)
			continue;

		if (node->typ == Text) {

			buf << node->u.text.text << endl;
			continue;
		}
	}
	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"MathExtension.h\"\n";
	buf << "#include <semaphore.h>\n";
	buf << "#include <string>\n";
	buf << "#include \"DSG.h\"\n";
	buf << "using namespace std;\n";
	if (Win)//Win默认是false
	{
		buf << "#define MAX_ITER 10\n";
	}
	if (CallModelEmbed)
		buf << "namespace COStream{\n";//名字空间
	//输出stream结构体类型定义 
	pEdgeInfo->DeclEdgeType(buf);

	//添加枚举状态类型
	buf << "enum State\n";
	buf << "{\n";
	buf << "\tRUNNING,STOP,WAITING,FULL\n";
	buf << "};\n";

	//信号量声明
	//组内控制
	buf << "extern sem_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "zu"<<i <<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//去掉最后一个逗号
	buf << ";\n";

	//组间控制
	buf << "extern sem_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "exchange" <<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//去掉最后一个逗号
	buf << ";\n";

	//屏障
	buf << "extern pthread_barrier_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "barrier"<< i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//去掉最后一个逗号
	buf << ";\n";

	//状态标签
	buf << "extern int ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "label"<<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//去掉最后一个逗号
	buf << ";\n";

	//流水线停止变量
	buf << "extern volatile bool ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "pipeline"<<i <<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//去掉最后一个逗号
	buf << ";\n";

	//压缩算子上游锁
	//判断有压缩算子
	bool hasDSG = false;
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		if (dsg_->combineList[i] == MERGEDOWNONE || dsg_->combineList[i] == MERGEONE || dsg_->combineList[i] == MERGEUPONE)
		{
			hasDSG = true;
		}
	}
	if (hasDSG == true)
	{
		buf << "extern pthread_mutex_t ";
		for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
		{
			if (dsg_->combineList[i] == MERGEONE || dsg_->combineList[i] == MERGEUPONE || dsg_->combineList[i] == MERGEDOWNONE)
				buf << "mutex"<<i <<",";
		}
		buf.seekp((int)buf.tellp() - 1);		//去掉最后一个逗号
		buf << ";\n";
	}
	

	//输出全局变量Buffer的声明
	vector<FlatNode *>::iterator iter;
	for (iter = flatNodes_.begin(); iter != flatNodes_.end(); ++iter)//遍历所有结点,只看输出边
	{
		//如果是压缩子图，则设置2个Buffer，需要一个根据flatNode获得所属子图下标的函数
		for (iter_2 = (*iter)->outFlatNodes.begin(); iter_2 != (*iter)->outFlatNodes.end(); iter_2++)	//iter_2为该算子的输出边
		{
			int graphindex = dsg_->flatNode2graphindex.find(*iter)->second;
			if (dsg_->combineList[graphindex] == MERGEONE || dsg_->combineList[graphindex] == MERGEDOWNONE || dsg_->combineList[graphindex] == MERGEUPONE)
			{
				//压缩算子输出边动态的
				//增加一组动态边的定义,生成2个动态边

				//一个压缩算子生成两个Buffer，生成两个动态边
				if ((dsg_->getActorPosition(*iter) == LAST || dsg_->getActorPosition((*iter)) == BOTH) && !dsg_->isSink(*iter))
				{
					//该算子处于子图的最后，其输出边为动态边
					string edgename1 = (*iter)->name + "_" + (*iter_2)->name + "_w";//上游动态Buffer
					string edgename2 = (*iter)->name + "_" + (*iter_2)->name + "_r";//下游动态Buffer
					buf << "extern Buffer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << edgename1 << ";\n";
					buf << "extern Buffer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << edgename2 << ";\n";

					//产生全局Producer和Consumer
					string dsgconsumerName = (*iter)->name + "_" + (*iter_2)->name + "_READ";
					string dsgproducerName = (*iter)->name + "_" + (*iter_2)->name + "_WRITE";
					buf << "extern DSGProducer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << dsgproducerName << ";\n";
					buf << "extern DSGConsumer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << dsgconsumerName << ";\n";

				}
				else
				{
					//该算子处于压缩子图内，但是不是动态边算子
					string edgename = (*iter)->name + "_" + (*iter_2)->name;
					buf << "extern Buffer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << edgename << ";\n";
				}
			}
			else
			{
				//还需要判断actor是不是LAST节点
				if ((dsg_->getActorPosition(*iter) == LAST ||dsg_->getActorPosition((*iter))==BOTH)&& !dsg_->isSink(*iter))
				{
					//动态边算子
					string edgename = (*iter)->name + "_" + (*iter_2)->name;
					buf << "extern Buffer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << "> " << edgename << ";\n";
					//增加一组动态边的定义，生成2个动态边
					string dsgconsumerName = (*iter)->name + "_" + (*iter_2)->name + "_C";
					buf << "extern DSGConsumer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << dsgconsumerName << ";\n"; 
					string dsgproducerName = (*iter)->name + "_" + (*iter_2)->name + "_P";
					buf << "extern DSGProducer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << dsgproducerName << ";\n";
				}
				else
				{
					string edgename = (*iter)->name + "_" + (*iter_2)->name;
					buf << "extern Buffer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << "> " << edgename << ";\n" ;
				}
				
			}
			
		}
	}

	

	if (readerActor)		//读文件Actor
	{
		string typeName = pEdgeInfo->getEdgeInfo(readerActor, readerActor->outFlatNodes[0]).typeName;
		buf << "extern " << typeName << "* source;\n";
		buf << "int FileReader(string path,int mode);\n";
		buf << "istream& operator>>(istream& is, " << typeName << " &object);\n";
	}
	if (writerActor)
	{
		string typeName = pEdgeInfo->getEdgeInfo(writerActor->inFlatNodes[0], writerActor).typeName;
		buf << "extern " << typeName << "* sink;\n";
		buf << "void FileWriter(string path,int mode);\n";
		buf << "ostream& operator<<(ostream& os, const " << typeName << " &object);\n";
		buf << "extern int outPutCount;\n";
	}
	if (CallModelEmbed)
		buf << "}\n";//名字空间结束

	buf << "\n";
	buf << "#endif\n";
	//输出到文件
	stringstream ss;
	ss << dir_ << "global.h";
	OutputToFile(ss.str(), buf.str());
}
void DynamicX86CodeGenerate::DYGlobalCpp()
{
	//创建所有缓冲区

	//对于子图间缓冲区，分为两种，1.普通子图间动态缓冲区，大小初始为下游子图稳态需求数据量的10倍
	//2.压缩子图间动态缓冲区，分为read和write，分别代表consumer和producer，大小初始为下游子图稳态需求数据量的10倍，它的命名和其他缓冲区不同
	stringstream buf;
	buf << "/*cpp文件，全局变量，用于存储边的信息*/\n";

	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"global.h\"\n";
	buf << "#include \"DSG.h\"\n";
	buf << "#include <vector>\n";
	buf << "using namespace std;\n";
	if (CallModelEmbed)
		buf << "namespace COStream{\n";//名字空间
	//全局控制变量初始化

	//组内控制信号量
	buf << "sem_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "zu" <<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//去掉最后一个逗号
	buf << ";\n";
	
	

	//组间控制信号
	buf << "sem_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "exchange"<<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//去掉最后一个逗号
	buf << ";\n";


	//屏障
	buf << "pthread_barrier_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "barrier"<<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);
	buf << ";\n";
	
	//线程标签
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "int label"<<i<<"=WAITING;\n";
	}

	//流水线排空
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "volatile bool pipeline"<<i<<"=true;\n";
	}

	//锁
	bool hasDSG = false;
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		if (dsg_->combineList[i] == MERGEDOWNONE || dsg_->combineList[i] == MERGEONE || dsg_->combineList[i] == MERGEUPONE)
		{
			hasDSG = true;
		}
	}
	if (hasDSG == true)
	{
		for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
		{
			if (dsg_->combineList[i] == MERGEONE || dsg_->combineList[i] == MERGEUPONE || dsg_->combineList[i] == MERGEDOWNONE)
			{
				//设置对应i子图的锁
				buf << "pthread_mutex_t mutex"<<i<<";\n";
				
			}
		}
	}



	int init1, init2;//发送actor和接受actor初态调度产生和接受的数据量
	for (vector<FlatNode*>::iterator iter1 = flatNodes_.begin(); iter1 != flatNodes_.end(); ++iter1)
	{
		//对于算子，要判断它是否在压缩子图中
		//其次要判断它是否是LAST，有动态边
		int graphindex = dsg_->flatNode2graphindex.find(*iter1)->second;		//获得子图编号
		//使用输出边
		if (dsg_->combineList[graphindex] == MERGEONE || dsg_->combineList[graphindex] == MERGEUPONE
			|| dsg_->combineList[graphindex] == MERGEDOWNONE)
		{
			//只有这三种压缩子图的输出是双Buffer，其他都是单Buffer
			if ((dsg_->getActorPosition(*iter1) == LAST || dsg_->getActorPosition((*iter1)) == BOTH) && !dsg_->isSink(*iter1))
			{
				//子图的最下节点
				for (vector<FlatNode*>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); iter2++)
				{
					//循环所有输出边，每个输出边构造Buffer
					int size;
					int copySize = 0;
					int copyStartPos = 0;
					string edgenameRead = (*iter1)->name + "_" + (*iter2)->name + "_r";//下游子图读取buffer
					string edgenameWrite = (*iter1)->name + "_" + (*iter2)->name + "_w";//上游子图写入buffer
					string dsgproducerName = (*iter1)->name + "_" + (*iter2)->name + "_WRITE";

					string dsgconsumerName = (*iter1)->name + "_" + (*iter2)->name + "_READ";

					//必须获取接受算子的pop值,要计算一次稳态需求的数据，需要这个值乘以稳态次数
					int bianhao;
					for (int j = 0; j < (*iter2)->inFlatNodes.size(); j++)
					{
						if ((*iter2)->inFlatNodes.at(j) == (*iter1))//找到编号
						{
							bianhao = j;
						}
					}
					int perWorkPopCount = (*iter2)->inPopWeights[bianhao];//iter2需求数据数量
					//这里得获取iter2在下个子图中的稳态调度次数
					int SteadyWorkNum = dsg_->scheStaticChildGraph[graphindex + 1]->GetSteadyCount(*iter2);
					size = batch*SteadyWorkNum*perWorkPopCount;

					buf << "Buffer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << "> " << edgenameWrite <<
						"(" << size << "," << copySize << "," << copyStartPos << ");\n";
					buf << "Buffer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << "> " << edgenameRead <<
						"(" << size << "," << copySize << "," << copyStartPos << ");\n";
					buf << "DSGProducer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << ">" << dsgproducerName <<
						"(" << edgenameWrite << "," << SteadyWorkNum*perWorkPopCount << "," << batch << ");\n";
					buf << "DSGConsumer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << ">" << dsgconsumerName <<
						"(" << edgenameRead << "," << SteadyWorkNum*perWorkPopCount << ");\n";
				}
			}
			else
			{
				//压缩子图中actor，其输出边为普通通信边
				for (vector<FlatNode*>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); iter2++)
				{
					//循环所有输出边，每个输出边构造Buffer
					int stageminus;//普通通信边两个actor有阶段差
					int size;		//缓冲区大小
					string edgename = (*iter1)->name + "_" + (*iter2)->name;//A_B
					stageminus = psalist[graphindex]->FindStage(*iter2) - psalist[graphindex]->FindStage(*iter1);//发送算子与接受算子阶段差
					
					int edgePos = iter2 - (*iter1)->outFlatNodes.begin();//iter2在iter1输出边中的序号
					int perSteadyPushCount = dsg_->scheStaticChildGraph[graphindex]->GetSteadyCount(*iter1)*(*iter1)->outPushWeights.at(edgePos);
					//获得稳态push个数
					if (NoCheckBuffer)
					{
						int copySize = 0, copyStartPos = 0;//拷贝的数据大小，copy存放的开始位置
						for (int inEdgeIndex = 0; inEdgeIndex < (*iter2)->inFlatNodes.size(); inEdgeIndex++)//接受算子的输入边数目遍历
						{
							int perWorkPeekCount = (*iter2)->inPeekWeights[inEdgeIndex];//接受边peek值
							int perWorkPopCount = (*iter2)->inPopWeights[inEdgeIndex];//接受边pop值
							//如果没有初态，这两个值应该相等
							init1 = dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter1)*(*iter1)->outPushWeights.at(edgePos);
							//发送算子一次initwork产生数据量，目前这里必须是0
							init2 = dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter2)*perWorkPopCount;//接受方初态调度需要消耗的
							//数据量，目前也必须为0
							size = init1 + perSteadyPushCount*(stageminus + 2);
							if (perWorkPopCount == perWorkPeekCount)//没有初态调度
							{
								if (perSteadyPushCount)
								{
									copySize = (init1 - init2)*perSteadyPushCount;
									copyStartPos = init2%perSteadyPushCount;
								}
							}
							else
							{
								SyntaxError("gosh,you do not know that dynamic can not use init function?");
								int leftnum = ((init1 - init2) % perSteadyPushCount + perSteadyPushCount - (perWorkPeekCount - perWorkPopCount) % perSteadyPushCount) % perSteadyPushCount;
								copySize = leftnum + perWorkPeekCount - perWorkPopCount;
								int addtime = copySize%perSteadyPushCount ? copySize / perSteadyPushCount + 1 : copySize / perSteadyPushCount;
								copyStartPos = init2%perSteadyPushCount;
								size += addtime*perSteadyPushCount;
							}
							buf << "Buffer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << ">" << edgename <<
								"(" << size << "," << copySize << "," << copyStartPos << ");\n";
						}
					}
					else//使用边界检查.目前不支持，不能打开
					{
						size = (dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter1) + dsg_->scheStaticChildGraph[graphindex]
							->GetSteadyCount(*iter1)*(stageminus + 1))*(*iter1)->outPushWeights.at(edgePos);
						int tempSize = 1;
						while (size > tempSize)
						{
							tempSize <<= 1;
						}
						size = tempSize;
						buf << "Buffer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << ">" << edgename << "(" << size << ");\n";
					}


				}
			}
		}
		else	//非压缩子图
		{
			if ((dsg_->getActorPosition(*iter1) == LAST || dsg_->getActorPosition((*iter1)) == BOTH) && !dsg_->isSink(*iter1))
			{
				for (vector<FlatNode*>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); iter2++)
				{
					string edgename = (*iter1)->name + "_" + (*iter2)->name;
					string dsgconsumerName = (*iter1)->name + "_" + (*iter2)->name + "_C";
					string dsgproducerName = edgename + "_P";
					//循环所有输出边，每个输出边构造Buffer,非压缩算子动态边，只构建一个Buffer，一个dsgConsumer一个dsgproducer
					int copySize = 0;
					int copyStartPos = 0;
					int size;//缓冲区大小
					//必须获取接受算子的pop值,要计算一次稳态需求的数据，需要这个值乘以稳态次数
					int bianhao;
					for (int j = 0; j < (*iter2)->inFlatNodes.size(); j++)
					{
						if ((*iter2)->inFlatNodes.at(j) == (*iter1))//找到编号
						{
							bianhao = j;
						}
					}
					int perWorkPopCount = (*iter2)->inPopWeights[bianhao];//iter2需求数据数量
					//这里得获取iter2在下个子图中的稳态调度次数
					int SteadyWorkNum = dsg_->scheStaticChildGraph[graphindex+1]->GetSteadyCount(*iter2);
					size = batch*SteadyWorkNum*perWorkPopCount;
					buf << "Buffer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << "> " << edgename <<
						"(" << size << "," << copySize << "," << copyStartPos << ");\n";


					buf << "DSGProducer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << ">" << dsgproducerName <<
						"(" << edgename << "," << SteadyWorkNum*perWorkPopCount << "," << batch << ");\n";


					buf << "DSGConsumer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << ">" << dsgconsumerName<<
						"(" << edgename << "," << SteadyWorkNum*perWorkPopCount << ");\n";

					
				}
			}
			else
			{
				//非压缩子图中actor，其输出边为普通通信边
				//压缩子图中actor，其输出边为普通通信边
				//如果是sink，直接不会有iter2
				for (vector<FlatNode*>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); iter2++)
				{
					//循环所有输出边，每个输出边构造Buffer
					int stageminus;//普通通信边两个actor有阶段差
					int size;		//缓冲区大小
					string edgename = (*iter1)->name + "_" + (*iter2)->name;//A_B
					stageminus = psalist[graphindex]->FindStage(*iter2) - psalist[graphindex]->FindStage(*iter1);//发送算子与接受算子阶段差

					int edgePos = iter2 - (*iter1)->outFlatNodes.begin();//iter2在iter1输出边中的序号
					int perSteadyPushCount = dsg_->scheStaticChildGraph[graphindex]->GetSteadyCount(*iter1)*(*iter1)->outPushWeights.at(edgePos);
					//获得稳态push个数
					if (NoCheckBuffer)
					{
						int copySize = 0, copyStartPos = 0;//拷贝的数据大小，copy存放的开始位置
						for (int inEdgeIndex = 0; inEdgeIndex < (*iter2)->inFlatNodes.size(); inEdgeIndex++)//接受算子的输入边数目遍历
						{
							if ((*iter2)->inFlatNodes.at(inEdgeIndex) == (*iter1))
							{

								int perWorkPeekCount = (*iter2)->inPeekWeights[inEdgeIndex];//接受边peek值
								int perWorkPopCount = (*iter2)->inPopWeights[inEdgeIndex];//接受边pop值
								//如果没有初态，这两个值应该相等
								init1 = dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter1)*(*iter1)->outPushWeights.at(edgePos);
								//发送算子一次initwork产生数据量，目前这里必须是0
								init2 = dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter2)*perWorkPopCount;//接受方初态调度需要消耗的
								//数据量，目前也必须为0
								size = init1 + perSteadyPushCount*(stageminus + 2);
								if (perWorkPopCount == perWorkPeekCount)//没有初态调度
								{
									if (perSteadyPushCount)
									{
										copySize = (init1 - init2)*perSteadyPushCount;
										copyStartPos = init2%perSteadyPushCount;
									}
								}
								else
								{
									SyntaxError("gosh,you do not know that dynamic can not use init function?");
									int leftnum = ((init1 - init2) % perSteadyPushCount + perSteadyPushCount - (perWorkPeekCount - perWorkPopCount) % perSteadyPushCount) % perSteadyPushCount;
									copySize = leftnum + perWorkPeekCount - perWorkPopCount;
									int addtime = copySize%perSteadyPushCount ? copySize / perSteadyPushCount + 1 : copySize / perSteadyPushCount;
									copyStartPos = init2%perSteadyPushCount;
									size += addtime*perSteadyPushCount;
								}
								buf << "Buffer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << ">" << edgename <<
									"(" << size << "," << copySize << "," << copyStartPos << ");\n";
							}
						}
					}
					else//使用边界检查.目前不支持，不能打开
					{
						size = (dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter1) + dsg_->scheStaticChildGraph[graphindex]
							->GetSteadyCount(*iter1)*(stageminus + 1))*(*iter1)->outPushWeights.at(edgePos);
						int tempSize = 1;
						while (size > tempSize)
						{
							tempSize <<= 1;
						}
						size = tempSize;
						buf << "Buffer<" << pEdgeInfo->getEdgeInfo((*iter1), (*iter2)).typeName << ">" << edgename << "(" << size << ");\n";
					}
				}
			}
		}
	}
	if (CallModelEmbed)
	{
		buf << "}\n";
	}
	//输出到文件
	stringstream ss;
	ss << dir_ << "global.cpp";
	OutputToFile(ss.str(), buf.str());

}
void DynamicX86CodeGenerate::DYGlobalVarExtern()		//全局变量的声明
{
	stringstream ss;
	ss << dir_ << "GlobalVar.h";
	OutputToFile(ss.str(), globalvarbuf.str());
}

void DynamicX86CodeGenerate::DYGlobalVar()		//全局变量的定义
{
	isCGGlobalVar = true;
	stringstream buf;
	// 对全局变量进行代码生成
	flag_Global = 1;
	COSX86_List(gDeclList, 1);
	buf << declInitList.str() << "\n\n";
	declInitList.str("");
	flag_Global = 0;
	//输出到文件
	stringstream ss;
	isCGGlobalVar = false;
	ss << dir_ << "GlobalVar.cpp";
	OutputToFile(ss.str(), buf.str());
}


void DynamicX86CodeGenerate::DYGlobalActors()
{

}
void DynamicX86CodeGenerate::DYinitVarAndState(FlatNode*actor, OperatorType ot,stringstream& buf)
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
void DynamicX86CodeGenerate::DYlogicInit(FlatNode*actor, OperatorType ot, stringstream&buf)
{
	declInitList.str(""); // 清空
	buf << "\t// init\n";
	buf << "\tvoid init()";
	Node *init = actor->contents->body->u.operBody.init;
	if (init)
	{
		COSX86_Node(init, 2);
		buf << declInitList.str(); // init结构必须带"{}"
	}
	else
	{
		buf << " {\n";
		buf << "\t}\n"; // '}'独占一行
	}
	declInitList.str(""); // 清空
}
void DynamicX86CodeGenerate::DYdeclList(FlatNode*actor, OperatorType ot, stringstream&buf)
{
	List *state = NULL;
	Node  *param = NULL;
	assert(actor);
	buf << "\t// AST Variables\n";
	//state，var，param被常量传播消除
	state = actor->contents->body->u.operBody.state;


	extractDecl = true;//标志生成decl
	declList.str(""); // 清空declList内容
	buf << "\tint steadyScheduleCount;\t//稳态时一次迭代的执行次数\n";
	buf << "\tint initScheduleCount;\n";
	

	//state
	COSX86_List(state, 0);//输出state
	buf << "\t/* *****logic state***** */\n\t" << declList.str();
	
	extractDecl = false;
	//state init
	stateInit << declInitList.str();
	declInitList.str(""); // 清空declInitList内容
}
void DynamicX86CodeGenerate::DYinitPeek(stringstream &buf, string initPeekBuf)		//基本不使用
{
	buf << "\t// initPeek\n";
	buf << "\tdef initPeek() {\n";

	buf << initPeekBuf;

	buf << "\t}\n"; // initPeek方法结束
}


void DynamicX86CodeGenerate::DYinitWork(stringstream &buf)		//初始化函数
{
	buf << "\t// initWork\n";
	buf << "\tvoid initWork() {\n";
	buf << "\t\tinitVarAndState();\n";
	buf << "\t\tinit();\n";

	buf << "\t}\n"; // initWork方法结束
}


void DynamicX86CodeGenerate::DYwork(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	vector<string>::iterator iter;
	Node *work = actor->contents->body->u.operBody.work;
	string actorname = actor->name;
	stringstream tmpBuf;
	buf << "\t// work\n";
	buf << "\tvoid work() {\n";
	COSX86_Node(work, 2);		//访问work将work中的内容输出到declInitList
	buf << declInitList.str();
	buf << "\n\t\tpushToken();\n";
	buf << "\n\t\tpopToken();\n";
	buf << "\t}\n"; // work方法结束
	declInitList.str(""); // 清空
}



/*-------------------------

工具函数

----------------------------*/
//根据dsg的SSSG图的index和combinelist的情况使用mp划分得到使用核数
void DynamicX86CodeGenerate::setGraphindex2coreNum()
{
	//赋值graphIndex2coreNum，该成员被getThreadIndex2SSSGindex使用
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); i++)
	{
		int maxPartition = 0;
		map<FlatNode*, int>::iterator iter1;
		for (iter1 = mplist[i]->FlatNode2PartitionNum.begin(); iter1 != mplist[i]->FlatNode2PartitionNum.end(); iter1++)
		{
			if (iter1->second>maxPartition)
			{
				maxPartition = iter1->second;
			}
		}
		if (dsg_->combineList[i] == MERGEDOWNTWO || dsg_->combineList[i] == MERGEUPTWO)
		{
			//压缩算子无论是上压缩还是下压缩，都使用nCpucore-上游算子使用核数
			graphIndex2coreNum.insert(pair<int, int>(i, nCpucore_ - graphIndex2coreNum.find(i - 1)->second));
		}
		else if (dsg_->combineList[i] == MERGETWO)
		{
			//该算子使用的核数是nCpucore-上游算子使用核数
			graphIndex2coreNum.insert(pair<int, int>(i, nCpucore_-graphIndex2coreNum.find(i-1)->second));
		}
		else if (dsg_->combineList[i] == MERGEONE || dsg_->combineList[i] == MERGEUPONE || dsg_->combineList[i]==MERGEDOWNONE)
		{
			//这些算子使用的核数，也是划分的最大编号加1，如果使用5个核，则maxPartition=4，insert的值为maxPartition+1
			graphIndex2coreNum.insert(pair<int, int>(i, maxPartition + 1));
		}
		else
		{
			//非压缩算子使用线程数目为nCpucore
			graphIndex2coreNum.insert(pair<int, int>(i, nCpucore_));
		}
		
	}

}

//根据线程编号获得其在本子图中的编号
int DynamicX86CodeGenerate::getindexinGraph(int index)
{
	int wholeIndex = 0;
	int graphidex = getThreadIndex2SSSGindex(index);
	for (int i = 0; i < graphidex; i++)
	{
		wholeIndex += graphIndex2coreNum.find(i)->second;
	}
	return index - wholeIndex;
}

//根据线程的index获得对应SSSG图的index
int DynamicX86CodeGenerate::getThreadIndex2SSSGindex(int index)
{
	int wholeIndex = 0;
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); i++)
	{
		wholeIndex += graphIndex2coreNum.find(i)->second;
		if (index < wholeIndex)
		{
			return i;
		}
	}
}
//根据线程编号判断是否是子图的主线程
bool DynamicX86CodeGenerate::isMainThreadInGraph(int index)
{
	int bianhao = getindexinGraph(index);		//获得线程在子图内编号
	/*int graphindex = getThreadIndex2SSSGindex(index);	//获得在的子图编号
	if (dsg_->combineList[graphindex] == MERGETWO || dsg_->combineList[graphindex] == MERGEUPTWO ||
		dsg_->combineList[graphindex] == MERGEDOWNTWO)
	{
		//只有压缩算子的下游算子其主线程不为核数的倍数
		int UseCorePos = graphIndex2coreNum.find(graphindex - 1)->second;
		//如果上游算子使用5个，则实际线程编号为0-4，下游子图的主线程编号为5
		if (bianhao == UseCorePos)
			return true;
		else return false;
	}
	else*/
	{
		if (bianhao%nCpucore_ == 0)
			return true;
		else return false;
	}
}

//获得一个子图的输入边的名称
string DynamicX86CodeGenerate::getGraphInName(int graphindex)
{
	assert(graphindex != 0);//首子图没有输入边
	string inName="";
	FlatNode*firstNode = dsg_->staticChildGraph[graphindex]->flatNodes[0];

	pair<multimap<FlatNode*, string>::iterator, multimap<FlatNode*, string>::iterator>pos1;

	pos1 = mapActor2InEdge.equal_range(firstNode); //[pos1.first,pos1.second)区间内每个key都等于firstNode，参数是key

	while (pos1.first != pos1.second)
	{
		inName= pos1.first->second;
		++pos1.first;
	}
	//这里结果是A_B，要怎么获得A_B_C或者A_B_P
	if (dsg_->combineList[graphindex] == NOTMERGE || dsg_->combineList[graphindex] == NOTJUDGE || dsg_->combineList[graphindex] == FULLCORE)
	{
		inName += "_C";
	}
	else if (dsg_->combineList[graphindex] == MERGEONE || dsg_->combineList[graphindex] == MERGEUPONE ||
		dsg_->combineList[graphindex] == MERGEDOWNONE)
	{
		inName += "_C";
	}
	else
	{
		inName += "_READ";
	}
	
	return inName;
}
//获得一个子图的输出边的名称
string DynamicX86CodeGenerate::getGraphOutName(int graphindex)
{
	if (graphindex == (dsg_->scheStaticChildGraph.size() - 1))//尾子图没有输出边
	{
		return "";
	}
	string outName = "";
	int numOfActor = dsg_->staticChildGraph[graphindex]->flatNodes.size();
	FlatNode*lastNode = dsg_->staticChildGraph[graphindex]->flatNodes[numOfActor - 1];

	pair<multimap<FlatNode*, string>::iterator, multimap<FlatNode*, string>::iterator>pos1;
	
	pos1 = mapActor2OutEdge.equal_range(lastNode);
	while (pos1.first != pos1.second)
	{
		outName = pos1.first->second;
		++pos1.first;
	}
	if (dsg_->combineList[graphindex] == MERGEDOWNONE || dsg_->combineList[graphindex] == MERGEONE || dsg_->combineList[graphindex] == MERGEUPONE)
	{
		outName += "_WRITE";
	}
	else
	{
		outName += "_P";
	}
	return outName;
}


void DynamicX86CodeGenerate::DYThreads()		//线程生成文件入口
{
	string threadname;
	stringstream buf;
	//如果有nCpucore个核，N个子图，就需要判断子图的MERGE情况
	//如果是FULLCORE，就是nCpucore，如果是MERGE或者MERGEUP,MERGEDOWN就合并为一组
	
	
	for (int i = 0; i < threadNum; ++i)
	{
		buf << "/*所有线程函数在此定义*/\n";
		stringstream ss;
		ss << dir_ << "thread_" << i / nCpucore_ << "_" << i%nCpucore_ << ".cpp";
		DYThread(i, buf);	//
		OutputToFile(ss.str(), buf.str());
		ss.str("");
		buf.str("");//清空输入
	}


}
void DynamicX86CodeGenerate::DYThreadFaker(int index, stringstream&buf)		//线程生成代码
{
	//必须使用pSAlist来获取每个子图的阶段划分
	//根据dsg内部的SSSG图下标与psalist和mplist中的下标的MetisPartition和StageAssignment对应
	//而给定的是index，因此必须有一个通过index获取其对应静态子图下标的函数
	int graphIndex = getThreadIndex2SSSGindex(index);//获得图下标
	int bigstagenum = psalist[graphIndex]->MaxStageNum();//获得阶段划分编号最大值
	//如果3个算子，8核，对于编号为1的划分，这里index值为14，在当前子图中编号为1，graphNum就为1
	int graphNum = getindexinGraph(index);		//获得该index在该子图中的编号
	//如果一个压缩子图使用4个核，其阶段划分编号最大值就是4
	pair<multimap<FlatNode*, string>::iterator, multimap<FlatNode*, string>::iterator>pos1, pos2;
	//添加头文件
	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"Producer.h\"\n";
	buf << "#include \"DSG.h\"\n";
	buf << "#include \"Consumer.h\"\n";
	
	buf << "#include \"global.h\"\n";
	buf << "#include \"AllActorHeader.h\"\t//包含所有actor的头文件\n";
	buf << "#include \"lock_free_barrier.h\"\t\n";
	buf << "#include \"pthread.h\"\n";
	buf << "#include \"rdtsc.h\"\n";
	buf << "#include <fstream>\n";
	if (MakeProfile)		//不需要制作profile文件，但是加上好了
	{
		buf << "#include <sys/stat.h>\n";
		buf << "#include <sys/types.h>\n";
	}
	if (CHECKEACHACTORTIME || MakeProfile)//测试稳态每个actor的steadywork时间
	{
		buf << "#include <sstream>";
	}
	if (CallModelEmbed)	//嵌入式
	{
		buf << "namespace COStream{\n";
	}
	if (Linux)
	{
		buf << "extern int MAX_ITER;\n";		//linux下的迭代次数
	}
	if (TRACE)
	{
		buf << "extern double (*deltatime)[" << threadNum << "][2];\n";//如果为trace的话，则记录每个线程每次迭代的同步和计算时间长度
		buf << "#define MEASURE_RUNS 100000\n";//MEASURE_RUNS为时钟长度单位，为10M时钟周期
	}
	else if (CALRATIO)		//测试每个线程计算同步比
	{
		buf << "#define MEASURE_RUNS 100000\n";
	}

	

	buf << "void thread_" << index / nCpucore_ << "_" << index%nCpucore_ << "_fun()\n{\n";
	
	if (TRACE || CALRATIO)//测试每个线程计算同步比
	{
		buf << "\ttsc_counter c0,c1;\n";
	}
	if (CALRATIO)
		buf << "\tdouble ca1=0,total = 0;\n";
	if (CHECKEACHACTORTIME)
	{
		buf << "\tpfstream txtfw;\n\tstringstream ss;\n";
		buf << "\tss<<\"该文件记录该线程上每个actor的稳态执行的时间，第一列为actor的名称，第二列为该次稳态执行的时钟周期数（单位为10^6）\"<<endl;\n";
		buf << "\ttsc_counter c2,c3;\n";
	}
	else if (CALRATIO)
	{
		buf << "\tofstream txtfw;\n";
	}

	if (MakeProfile)
	{
		buf << "\tofstream txtpf;\n\tstringstream sst;\n";
		buf << "\ttsc_counter cc2,cc3;\n";
	}

	//主线程并不一定包含的算子就是该子图的第一个算子和最后一个算子
	//接下来如果该线程函数是中间子图并且该index为主线程的话，必须进行一次上游算子是否停止执行判断
	//主要处理起始数据不足的情况
	if (graphIndex != 0&&isMainThreadInGraph(index))
	{
		//主线程
		buf << "\tif(label" << graphIndex - 1 << "==STOP)\n";
		buf << "\t{\n";
		//接下来要获取该子图对应的动态输入边名字，
		//并且要判断是否是压缩子图，压缩子图使用不同的通信边命名
		if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
			dsg_->combineList[graphIndex] != NOTJUDGE)
		{
			buf << "\t\t" << getGraphInName(graphIndex) << ".append(" << getGraphOutName(graphIndex - 1) << ");\n";

		}
		
		buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
		buf << "\t\t{";
		buf << "\t\t//数据量不足\n";
		
		buf << "\t\t\tlabel" << graphIndex << "=STOP;\n";
		buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
		buf << "\t\t\treturn ;\n";
		buf << "\t\t}\n";
		buf << "\t}\n\n";


	}

	//接下来要获取对应index的全部算子
	//就需要判断当前index对应线程所在子图，然后根据子图下标来获取对应metis划分
	//同时由于metis划分基于划分编号，所以必须根据index获取到该index在子图中的划分编号
	//对于压缩算子的下游子图，其index不能用index%nCpucore来计算
	vector<FlatNode*> tmpactorset;//对应当前划分的算子集合
	if (dsg_->combineList[graphIndex] == MERGETWO || dsg_->combineList[graphIndex] == MERGEUPTWO
		|| dsg_->combineList[graphIndex] == MERGEDOWNTWO)
	{
		//必须获得算子在本子图中的编号
		
		tmpactorset = mplist[graphIndex]->findNodeSetInPartition(graphNum);
	}
	else
		tmpactorset = mplist[graphIndex]->findNodeSetInPartition(index%nCpucore_);
	

	//这里确定主线程与从属线程
	//只生成到第一次同步
	//需要一个工具函数，根据传入的index来判断是否是某个子图的主线程函数为0，8，或者压缩算子的某一个
	if (isMainThreadInGraph(index))
	{
		//主线程

		buf << "\t//从属线程信号量需求启动\n";
		if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
			dsg_->combineList[graphIndex] != NOTJUDGE)
		{
			if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEUPONE
				|| dsg_->combineList[graphIndex] == MERGEDOWNONE)
			{
				//对于压缩算子的上游算子，如果压缩子图总核数小于nCpucore，由下游算子补齐
				//假设上游算子有5个，这里bigstagenum就等于5，从0到4
				buf << "\tfor(int i = 0;i<"<<bigstagenum<<";++i)\n";
				buf << "\t{\n" << "sem_post(zu" << graphIndex << ");\n";
				buf << "\t}\n";
				buf << "masterSync(" << bigstagenum << ");\n";
			}
			else
			{
				//压缩算子的下游算子假设有2个，则减法结果是3，应该是012
				buf << "\tfor(int i = 0;i<" << (nCpucore_ - psalist[graphIndex - 1]->MaxStageNum()) << ";++i)\n";
				buf << "\t{\n" << "sem_post(zu" << graphIndex << ");\n";
				buf << "\t}\n";
				buf << "masterSync(" << (nCpucore_ - psalist[graphIndex - 1]->MaxStageNum()) << ");\n";
			}
			
		}
		else
		{
			//非压缩算子主线程
			buf << "\tfor(int i = 0;i<" << nCpucore_ << ";++i)\n";
			buf << "\t{\n" << "sem_post(zu" << graphIndex << ");\n";
			buf << "\t}\n";
			buf << "masterSync(" << nCpucore_ << ");\n";
		}
		
	}
	else
	{
		//从属线程
		buf << "sem_wait(&zu" << graphIndex << ");\n";
		buf << "workerSync(" << getindexinGraph(index) << ");\n";
	}



	//构造函数调用
	for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
	{
		buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
		//mapActor2InEdge和mapActor2OutEdge存放该actor所对应的输入输出边buffer的名称
		//获取对应string区间
		pos1 = mapActor2InEdge.equal_range(*iter);
		pos2 = mapActor2OutEdge.equal_range(*iter);
		while (pos2.first != pos2.second)		//首先是输出边
		{
			buf << pos2.first->second << ",";
			++pos2.first;
		}
		while (pos1.first != pos1.second)		//输入边
		{
			buf << pos1.first->second << ",";
			++pos1.first;
		}
		buf.seekp((int)buf.tellp() - 1);		//去掉最后一个逗号
		buf << "):\n";
		if (MakeProfile)		//做profile，累加执行时间
		{
			buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
		}
	}
	//stage表示阶段号数组，初始除0外都为0
	buf << "\tchar stage[" << bigstagenum << "]={0};\n";
	buf << "\tstage[0]=1;\n";
	//这里要根据子图判断，来添加一个流水线排空控制变量
	if (graphIndex != 0 && isMainThreadInGraph(index))
	{
		//非首子图并且是主线程
		buf << "\tbool out = true;\n";
		//从属线程不需要流水线结束，只需要挂起，然后等待main函数结束即可
	}
	
	//初始化
	//直接调用runInitScheduleWork
	//循环所有划分本线程算子
	for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
	{
		buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
	}


	
	//稳态调度前同步
	if (!isMainThreadInGraph(index))
	{
		//不是主线程，从属线程
		buf << "\t\n\t\tworkerSync(" <<getindexinGraph(index) << ");\n";
	}
	else
	{
		//这个masterSync需要对压缩子图进行判断，因为压缩子图后，同步的线程数量不再是nCpucore
		
		if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
			dsg_->combineList[graphIndex] != NOTJUDGE)
		{
			//压缩子图主线程
			if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEUPONE
				|| dsg_->combineList[graphIndex] == MERGEDOWNONE)
			{
				//上游算子
				buf << "\t\n\t\tmasterSync(" << bigstagenum << ");\n";
			}
			else
			{
				//下游算子
				buf << "\t\n\t\tmasterSync(" << (nCpucore_ - psalist[graphIndex - 1]->MaxStageNum()) << ");\n";
			}
			
		}
		else
			buf << "\t\n\t\tmasterSync(" << nCpucore_ << ");\n";
	}


	//稳态调度
	//要获得对应index编号的阶段号，并且一个线程可能执行多个阶段
	set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second; 
	//这个mapNum2Stage在哪里初始化的？
	set<int>::iterator endIter = ptempstagenum.end();
	//首子图和其他子图不同，迭代次数确定
	if (isMainThreadInGraph(index))//主线程必须是nCpucore的倍数
	{
		if (graphIndex == 0)
		{
			//首子图,也得判断是否是压缩子图
			//分成压缩子图与非压缩子图
			if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
				dsg_->combineList[graphIndex] != NOTJUDGE)
			{
				//压缩算子的首子图，第一个算子为压缩算子上游
			}
			else
			{
				buf << "\tfor(int _stageNum = " << bigstagenum << ";_stageNum<2*" << bigstagenum << "+MAX_ITER-1;_stageNum++)\n";
				buf << "\t{\n";
				for (int i = bigstagenum - 1; i >= 0; i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenum为该线程需要执行的阶段
					//下面找到各阶段执行的算子
					if (stageiter != endIter)
					{
						buf << "\t\tif(stage[" << i << "])\n\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
						vector<FlatNode*>::iterator iter1;
						for (iter1 = flatVec.begin(); iter1 != flatVec.end(); iter1++)
						{
							if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
							{
								//当前线程序号等于该算子划分的编号，表示该算子确定在该线程中执行
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
							}
						}
						buf << "\t\t}\n";
					}

				}
				//开始数组回滚
				buf << "\t\tfor(int index = " << bigstagenum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";
				buf << "\t\tif(_stageNum==(MAX_ITER-1+" << bigstagenum << "))\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0]=0;\n";
				buf << "\t\t}\n";
				buf << "\t\n\t\tmasterSync(" << nCpucore_ << ");\n";

				//缓存判定
				buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())//输出缓存满\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");//唤醒下游算子\n";
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");//进入等待\n";
				//稳态开始下一轮循环,分发信号量
				//这里可能需要每一个子线程一个信号量，这个等以后再考虑
				buf << "\t\tfor(int i = 0;i<" << nCpucore_ << ";i++)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";
				buf << "\t\n\t\tmasterSync(" << nCpucore_ << ");\n";
				buf << "\t}\n";
			}
		

		}
		else
		{
			//其他子图
			//分成压缩子图与非压缩子图
			if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
				dsg_->combineList[graphIndex] != NOTJUDGE)
			{
				//压缩算子
				//分为上游和下游
				if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEUPONE
					|| dsg_->combineList[graphIndex]);
			}
			else
			{
				//非压缩算子
				//稳态调度
				buf << "while(1)\n";
				buf << "\t{\n";
				for (int i = bigstagenum - 1; i >= 0; i++)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenum是在该线程上执行的所有阶段集合
					if (stageiter != endIter)//找到表示该stage在该thread上
					{
						buf << "\t\tif(stage[" << i << "])\n";
						buf << "\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);//对应该阶段的算子
						vector<FlatNode*>::iterator iter1;
						for (iter1 == flatVec.begin(); iter1 != flatVec.end(); iter1++)
						{
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadySchuduleWork();\n";
						}
						buf << "\t\t}\n";
					}

				}
				//数组回滚
				buf << "\t\tfor(int index=" << bigstagenum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";
				buf << "\t\tmasterSync(" << nCpucore_ << ");\n";


				//上游算子交互
				buf << "\t\t//开始处理线程同步，首先处理与上游算子的交互\n";
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&label" << graphIndex - 1 <<
					"==STOP&&stage[0]!=0)\n";
				buf << "\t\t//流水线开始排空\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0]=0;\n";
				buf << "\t\t}\n";
				
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t//稳态数据不足\n";
				buf << "\t\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t\t\t//等待唤醒\n";
				buf << "\t\t}\n";

				//唤醒后检查
				buf << "\t\t//唤醒后还要一次检查，判断数据是否够一次稳态，不够表示上游算子已经结束，开始排空\n";
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0] = 0;\n";
				buf << "\t\t}\n";

				//输出交互
				if (graphIndex == dsg_->scheStaticChildGraph.size() - 1)//最后一个子图
				{
					;
				}
				else
				{
					buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
					buf << "\t\t{\n";
					buf << "\t\t//输出满\n";
					buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
					buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
					buf << "\t\t}\n";
				}

				//通知其他从属线程开始流水线排空
				buf << "\t\tif(stage[0]==0&&pipeline" << graphIndex << "==true)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpipeline" << graphIndex << "=false\n";
				buf << "\t\t}\n";

				//流水线停止判断
				buf << "\t\tfor(int i= 0;i<" << bigstagenum << ";i++)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tif(stage[i]==1)\n";
				buf << "\t\t\t{\n";
				buf << "out = false;\n";
				buf << "\t\t\t}\n";
				buf << "\t\t}\n";
				buf << "\t\tif(out==false);\n";
				buf << "\t\telse break;\n";
				
				//稳态循环信号量
				buf << "\t\tfor(int i = 0;i<" << nCpucore_ << ";i++)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";

				buf << "\t\tmasterSync(" << nCpucore_ << ");\n";
				buf << "\t}\n";
			}
		}
	}
	else
	{
		//非主线程
		if (graphIndex == 0)
		{
			//首线程的从属线程，这是最类似静态的线程了
			buf << "\tfor(int _stageNum=" << bigstagenum << ";_stageNum<2*" << bigstagenum << "+MAX_ITER-1;_stageNum++)\n\t{\n";
			for (int i = bigstagenum - 1; i >= 0; i--)
			{
				set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenum为该线程需要执行的阶段集合
				//如果找到就表示对应阶段i在本线程中执行
				if (stageiter != endIter)
				{
					buf << "\t\tif(stage[" << i << "])\n\t\t{\n";
					vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
					vector<FlatNode*>::iterator iter1;
					for (iter1 = flatVec.begin(); iter1 != flatVec.end(); iter1++)
					{
						if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
						{
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
						}
					}
					buf << "\t\t}\n";
				}
			}
			//开始数组回滚
			buf << "\t\tfor(int index=" << bigstagenum - 1 << ";index>=1;--index)\n";
			buf << "\t\t\tstage[index]=stage[index-1];\n";
			buf << "\t\tif(_stageNum==(MAX_ITER-1+" << bigstagenum << "))\n";
			buf << "\t\t\tstage[0] = 0;\n";
			buf << "\t\t}\n";
			buf << "workerSync(" << graphNum << ");\n";
			buf << "sem_wait(&zu" << graphIndex << ")\n";
			buf << "workerSync(" << graphNum << ");\n";
			buf << "\t}\n";
			buf << "}\n";
		}
		else
		{
			buf << "\twhile(1)\n";
			buf << "\t{\n";
			buf << "\t\t//从属线程不需要结束，只要等主进程main函数等待主线程执行完毕后自动清理，如果上游算子已经";
			buf << "停止就会一直挂起";
			//接下来要获得在该index的线程上执行的阶段
			for (int i = bigstagenum - 1; i >= 0; i--)	//迭代stage
			{
				set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenum是一个该线程
				//需要执行的阶段集合。
				//如果stageiter能找到就说明该阶段在本线程中执行
				if (stageiter != endIter)
				{
					buf << "\t\tif(stage[" << i << "])\n";
					buf << "\t\t{\n";

					vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
					vector<FlatNode*>::iterator iter1;
					for (iter1 = flatVec.begin(); iter1 != flatVec.end(); iter1++)
					{
						//开始稳态函数调用
						if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
						{
							//表示在划分中，该阶段对应的该算子划分到本线程
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
						}
					}
					buf << "\t\t}\n";
				}
			}
			


			//开始进行数组回滚
			buf << "\t\tfor(int index=" << bigstagenum - 1 << ";index>=1;--index)\n";
			buf << "\t\t\tstage[index] = stage[index-1];\n";

			//稳态结束，开始同步
			buf << "\t\tworkerSync(" << graphNum << ")\n";

			//等待下一次执行的信号量
			buf << "\t\tsem_wait(&zu" << graphIndex << ");\n";


			//流水线排空

			buf << "\t\tif(pipeline" << graphIndex << "==false&&stage[0]!=0)\n";
			buf << "\t\t{\n";
			buf << "\t\t//主线程设置了pipeline标记，其他线程获取信号量后只能读取该标记\n";
			buf << "\t\t//并且如果此时stage[0]为1就令其为0开始排空，如果为0就跳过";
			buf << "\t\t\tstage[0] = 0;\n";
			buf << "\t\t}\n";
			//再次同步
			buf << "\t\tworkerSync(" << graphNum << ")\n";

			buf << "\t}\n";
			buf << "}\n";
		}
	}
	//到了流水线排空阶段
	//这里分为压缩子图上游和其他子图还有最后子图
	//这里只有主线程需要进行流水线排空处理
	if (isMainThreadInGraph(index))
	{
		buf << "稳态调度结束，流水线排空\n\t设置结束状态，唤醒下游子图\n";
		if (graphIndex == dsg_->scheStaticChildGraph.size() - 1)	//最后子图
		{
			buf << "\tlabel" << graphIndex << "=STOP";
		}
		else
		{
			buf << "\tlabel" << graphIndex << "=STOP\n";
			buf << "\t//数据的剩余部分拷贝，留到下游算子，不然上游算子拷贝需要获得下游算子状态\n";
			buf << "\t//会严重增加线程同步量\n";
			buf << "\tsem_post(&exchange" << graphIndex + 1 << ");\n";
			buf << "\t}\n";
		}
	}


}
void DynamicX86CodeGenerate::DYThread(int index, stringstream&buf)
{
	//必须使用pSAlist来获取每个子图的阶段划分
	//根据dsg内部的SSSG图下标与psalist和mplist中的下标的MetisPartition和StageAssignment对应
	//而给定的是index，因此必须有一个通过index获取其对应静态子图下标的函数
	int graphIndex = getThreadIndex2SSSGindex(index);//获得图下标
	int stageMaxNum = psalist[graphIndex]->MaxStageNum();
	int graphNum = getindexinGraph(index);//index在子图中编号
	int useCoreNum = graphIndex2coreNum.find(graphIndex)->second;
	//dsg类的ssg2coreNum是一个FlatNode2Corenum的成员，这里的graphIndex2coreNum是index2coreNum的成员
	//如果一个压缩子图使用4个核，其metis划分编号最大值就是4
	//共享文件
	pair<multimap<FlatNode*, string>::iterator, multimap<FlatNode*, string>::iterator>pos1, pos2;
	//添加头文件
	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"Producer.h\"\n";
	
	buf << "#include \"Consumer.h\"\n";
	buf << "#include \"DSG.h\"\n";
	buf << "#include \"global.h\"\n";
	buf << "#include \"AllActorHeader.h\"\t//包含所有actor的头文件\n";
	buf << "#include \"lock_free_barrier.h\"\t\n";
	buf << "#include \"pthread.h\"\n";
	buf << "#include \"rdtsc.h\"\n";
	buf << "#include <fstream>\n";
	if (MakeProfile)		//不需要制作profile文件，但是加上好了
	{
		buf << "#include <sys/stat.h>\n";
		buf << "#include <sys/types.h>\n";
	}
	if (CHECKEACHACTORTIME || MakeProfile)//测试稳态每个actor的steadywork时间
	{
		buf << "#include <sstream>";
	}
	if (CallModelEmbed)	//嵌入式
	{
		buf << "namespace COStream{\n";
	}
	if (Linux)
	{
		buf << "extern int MAX_ITER;\n";		//linux下的迭代次数
	}
	if (TRACE)
	{
		buf << "extern double (*deltatime)[" << threadNum << "][2];\n";//如果为trace的话，则记录每个线程每次迭代的同步和计算时间长度
		buf << "#define MEASURE_RUNS 100000\n";//MEASURE_RUNS为时钟长度单位，为10M时钟周期
	}
	else if (CALRATIO)		//测试每个线程计算同步比
	{
		buf << "#define MEASURE_RUNS 100000\n";
	}
	buf << "void thread_" << index / nCpucore_ << "_" << index%nCpucore_ << "_fun()\n{\n";

	if (TRACE || CALRATIO)//测试每个线程计算同步比
	{
		buf << "\ttsc_counter c0,c1;\n";
	}
	if (CALRATIO)
		buf << "\tdouble ca1=0,total = 0;\n";
	if (CHECKEACHACTORTIME)
	{
		buf << "\tpfstream txtfw;\n\tstringstream ss;\n";
		buf << "\tss<<\"该文件记录该线程上每个actor的稳态执行的时间，第一列为actor的名称，第二列为该次稳态执行的时钟周期数（单位为10^6）\"<<endl;\n";
		buf << "\ttsc_counter c2,c3;\n";
	}
	else if (CALRATIO)
	{
		buf << "\tofstream txtfw;\n";
	}

	if (MakeProfile)
	{
		buf << "\tofstream txtpf;\n\tstringstream sst;\n";
		buf << "\ttsc_counter cc2,cc3;\n";
	}

	//接下来，根据是否是主线程，是不是第一组分开
	if (graphIndex == 0)
	{
		//首子图
		if (isMainThreadInGraph(index))
		{
			//主线程
			if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEUPONE
				|| dsg_->combineList[graphIndex] == MERGEDOWNONE)
			{
				//压缩子图
				//首子图的压缩子图没有WAITING状态
				buf << "\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";
				buf << "\tlabel" << graphIndex << "=RUNNING;\n";
				//屏障
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" <<nCpucore_<< ",true,true,"<<dsg_->ssg2coreNum[graphIndex].second<<");\n";
				//构造函数调用
				vector<FlatNode*> tmpactorset;
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(graphIndex);

				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//首子图没有动态输入边
					if (dsg_->getActorPosition(*iter) == LAST)
					{
						//最多一个输出边
						string outName = getGraphOutName(graphIndex);
						pos1 = mapActor2InEdge.equal_range(*iter);
						if (outName != "")
						{
							buf << outName << ",";
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}


					}
					else
					{
						//子图中其他算子
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter的输入边列表
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter的输出边列表
						while (pos2.first != pos2.second)
						{
							buf << pos2.first->second << ",";
							++pos2.first;
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}
					}
					buf.seekp((int)buf.tellp() - 1);
					buf << ");\n";
					if (MakeProfile)		//做profile，累加执行时间
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}
			
				//流水调度控制数据
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//这里是执行阶段数目
				//stage表示阶段号数组，初始除0外都为0
				buf << "\tstage[0]=1;\n";

				//初始化
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name + "_obj.runInitScheduleWork();\n";
				}

				//稳态调度
				//获取本线程使用的阶段数目
				set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
				set<int>::iterator endIter = ptempstagenum.end();

				buf << "\tfor(int _stageNum = " << stageMaxNum << ";_stageNum<2*" << stageMaxNum << "+MAX_ITER-1;_stageNum++)\n";
				buf << "\t{\n";
				for (int i = stageMaxNum - 1; i >= 0; i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);
					if (stageiter != endIter)
					{
						buf << "\t\tif(stage[" << i << "])\n";
						buf << "\t\t{\n";
						vector<FlatNode*>flatVec = psalist[graphIndex]->FindActor(i);
						vector<FlatNode*>::iterator iter1;
						for (iter1 = flatVec.begin(); iter1 != flatVec.end(); ++iter1)
						{
							if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
							{
								//当前线程序号等于该算子被划分的序号
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";

							}
						}
						buf << "\t\t}\n";
					}
				}
				//数组回滚
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";
				buf << "\t\tif(_stageNum==(MAX_ITER-1+" << stageMaxNum << "))\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0]=0;\n";
				buf << "\t\t}\n";

				//二级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,true," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//没有输入,由于是压缩算子，所以必然有输出边
				//下游交互
				buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
					//输出满
				buf << "\t\t{\n";
				buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//加锁
				buf << "\t\t\tlabel" << graphIndex << "=FULL;\n";	//设置状态
				buf << "\t\t\tif(label" << graphIndex + 1 << "==WAITING||label" << graphIndex + 1 << "==FULL)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t//下游子图已经挂起\n";
				buf << "\t\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "\t\t\t}\n";

				buf << "\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";//解锁
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//加锁，修改状态
				buf << "\t\t\tlabel" << graphIndex << "=RUNNING;\n";
				buf << "\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";
				buf << "\t\t\t" << getGraphOutName(graphIndex) << ".resetWrite();\n";
				buf << "\t\t}\n";
				
				//控制从属线程
				buf << "\t\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";

				//三级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\t}\n";

				//停止标志
				buf << "\tlabel" << graphIndex << "=STOP;\n";
				buf << "\tsem_post(&exchange" << graphIndex + 1 << ");\n";

				buf << "}\n";



			}
			else
			{
				//首子图非压缩子图主线程

				
				buf << "\tfor(int i = 1;i<" << nCpucore_ << ";++i)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";

				//屏障
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ");\n";
				//构造函数调用
				vector<FlatNode*> tmpactorset;
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(graphIndex);

				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter!= tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//首子图没有动态输入边
					if (dsg_->getActorPosition(*iter) == LAST)
					{
						//最多一个输出边
						string outName = getGraphOutName(graphIndex);
						pos1 = mapActor2InEdge.equal_range(*iter);
						if (outName != "")
						{
							buf << outName << ",";
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}


					}
					else
					{
						//子图中其他算子
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter的输入边列表
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter的输出边列表
						while (pos2.first != pos2.second)
						{
							buf << pos2.first->second << ",";
							++pos2.first;
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}
					}
					buf.seekp((int)buf.tellp() - 1);
					buf << ");\n";
					if (MakeProfile)		//做profile，累加执行时间
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}

				//流水调度控制数据
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//这里是执行阶段数目
				//stage表示阶段号数组，初始除0外都为0
				buf << "\tstage[0]=1;\n";

				//初始化
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name + "_obj.runInitScheduleWork();\n";
				}
				//稳态调度
				//获得在该线程内执行的阶段
				set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
				set<int>::iterator endIter = ptempstagenum.end();

				buf << "\tfor(int _stageNum = " << stageMaxNum << ";_stageNum<2*" << stageMaxNum << "+MAX_ITER-1;_stageNum++)\n";
				buf << "\t{\n";
				for (int i = stageMaxNum - 1; i >= 0; i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenum为该线程需要执行的阶段
					//下面找到各阶段执行的算子
					if (stageiter != endIter)
					{
						buf << "\t\tif(stage[" << i << "])\n\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
						vector<FlatNode*>::iterator iter1;
						for (iter1 = flatVec.begin(); iter1 != flatVec.end(); iter1++)
						{
							if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
							{
								//当前线程序号等于该算子划分的编号，表示该算子确定在该线程中执行
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
							}
						}
						buf << "\t\t}\n";
					}
				}

				//数组回滚
				//开始数组回滚
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";
				buf << "\t\tif(_stageNum==(MAX_ITER-1+" << stageMaxNum << "))\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0]=0;\n";
				buf << "\t\t}\n";

				//二级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ");\n";
				//下游子图处理
				if (graphIndex != (dsg_->scheStaticChildGraph.size() - 1))
				{
					buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
					buf << "\t\t{\n";
					buf << "\t\t\tsem_post(&exchange" << graphIndex+1 << ");\n";
					buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
					
					buf << "\t\t"<<getGraphOutName(graphIndex) << ".resetTail();\n";
					buf << "\t\t}\n";
				}

				//从属线程驱动执行稳态
				buf << "\t\tfor(int i = 1;i<" << nCpucore_ << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";

				//三级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\t}\n";

				//设置STOP
				buf << "\tlabel" << graphIndex << "=STOP;\n";
				if (graphIndex != (dsg_->scheStaticChildGraph.size() - 1))
				{
					buf << "\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				}
				buf << "}\n";
			}

			
		}
		else
		{
			//首子图从属线程

			buf << "\tsem_wait(&zu" << graphIndex << ");\n";

			//屏障
			//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\tworkerSync(" << index%nCpucore_ << ");\n";
			//构造函数调用
			vector<FlatNode*> tmpactorset;
			tmpactorset = mplist[graphIndex]->findNodeSetInPartition(index%nCpucore_);

			for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
			{
				buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
				//首子图没有动态输入边
				if (dsg_->getActorPosition(*iter) == LAST)
				{
					//最多一个输出边
					string outName = getGraphOutName(graphIndex);
					pos1 = mapActor2InEdge.equal_range(*iter);
					if (outName != "")
					{
						buf << outName << ",";
					}
					while (pos1.first != pos1.second)
					{
						buf << pos1.first->second << ",";
						++pos1.first;
					}


				}
				else
				{
					//子图中其他算子
					pos1 = mapActor2InEdge.equal_range(*iter);		//iter的输入边列表
					pos2 = mapActor2OutEdge.equal_range(*iter);		//iter的输出边列表
					while (pos2.first != pos2.second)
					{
						buf << pos2.first->second << ",";
						++pos2.first;
					}
					while (pos1.first != pos1.second)
					{
						buf << pos1.first->second << ",";
						++pos1.first;
					}
				}
				buf.seekp((int)buf.tellp() - 1);
				buf << ");\n";
				if (MakeProfile)		//做profile，累加执行时间
				{
					buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
				}
			}
			
			//流水调度数据
			buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//这里是执行阶段数目
			//stage表示阶段号数组，初始除0外都为0
			buf << "\tstage[0]=1;\n";
			
			//初始化
			for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
			{
				buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
			}

			//稳态调度
			set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;//核与阶段的关系
			//ptempstagenum中是某个核上阶段的集合
			set<int>::iterator enditer = ptempstagenum.end();


			buf << "\tfor(int _stageNum =" << stageMaxNum << ";_stageNum<2*" << stageMaxNum << "+MAX_ITER-1;_stageNum++)\n";
			buf << "\t{\n";
			for (int i = stageMaxNum - 1; i >= 0; i--)
			{
				set<int>::iterator stageiter = ptempstagenum.find(i);
				//阶段从StageMaxNum到0，必须确定哪些阶段在这个核上执行
				if (stageiter != enditer)
				{
					//在该核上
					buf << "\t\tif(stage[" << i << "])\n";
					buf << "\t\t{\n";
					vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
					//找到对应该阶段的所有算子
					vector<FlatNode*>::iterator iter1;

					for (iter1 = flatVec.begin(); iter1 != flatVec.end(); ++iter1)
					{
						if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
						{
							//这个判断有必要吗
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
						}
					}
					buf << "\t\t}\n";
				}
			}

			//数组回滚
			buf << "\t\tfor(int index=" << stageMaxNum - 1 << ";index>=1;--index)\n";
			buf << "\t\t\tstage[index]=stage[index-1];\n";
			buf << "\t\tif(_stageNum==(MAX_ITER-1+" << stageMaxNum << "))\n";
			buf << "\t\t{\n";
			buf << "\t\t\tstage[0] = 0;\n";
			buf << "\t\t}\n";

			//二次屏障
			//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\tworkerSync(" << index%nCpucore_ << ");\n";
			//等待信号量
			buf << "\tsem_wait(&zu" << graphIndex << ");\n";

			//首线程的从属线程不需要判断pipeline来决定是否排空
			//因为有固定的迭代次数

			//三次屏障
			//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\t}\n";
			buf << "}";
		}
	}
	else
	{
		//其他子图
		if (isMainThreadInGraph(index))
		{
			//主线程
			if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEDOWNONE || dsg_->combineList[graphIndex]
				== MERGEUPONE)
			{
				//压缩子图
				//上游子图主线程
				//上游子图处理
				buf << "\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\tlabel" << graphIndex << "=RUNNING;\n";
				buf << "\tif(label" << graphIndex - 1 << "==STOP)\n";
				buf << "\t{\n";
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t\tlabel" << graphIndex << "=STOP;\n";
				if (graphIndex != (dsg_->staticChildGraph.size() - 1))
					buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "\t\t\treturn ;\n";
				buf << "\t\t}\n";
				buf << "\t}\n";

				//启动从属线程,个数从成员变量graphIndex2CoreNum中获取
				buf << "\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";

				//屏障
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,true," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//构造函数,获得全部算子
				vector<FlatNode*> tmpactorset;
				//获得组内编号
				int bianhao = getindexinGraph(index);
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(bianhao);

				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//判断是否要用DSG构造
					if (dsg_->getActorPosition(*iter) == FIRST)
					{
						//只有一个输入边
						string inName = getGraphInName(graphIndex);

						pos2 = mapActor2OutEdge.equal_range(*iter);
						while (pos2.first != pos2.second)
						{
							buf << pos2.first->second << ",";
							++pos2.first;
						}
						buf << inName << ",";

					}
					else if (dsg_->getActorPosition(*iter) == LAST)
					{
						//最多一个输出边
						string outName = getGraphOutName(graphIndex);
						pos1 = mapActor2InEdge.equal_range(*iter);
						if (outName != "")
						{
							buf << outName << ",";//没输出边就不会执行这一句
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}


					}
					else
					{
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter的输入边列表
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter的输出边列表
						while (pos2.first != pos2.second)
						{
							buf << pos2.first->second << ",";
							++pos2.first;
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}
					}
					buf.seekp((int)buf.tellp() - 1);		//去掉一个逗号
					buf << ");\n";
					if (MakeProfile)		//做profile，累加执行时间
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}


				//流水调度数据
				//stage表示阶段号数组，初始除0外都为0
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//这里是执行阶段数目
				buf << "\tstage[0]=1;\n";
				buf << "\tbool out = true;\n";

				//初始化
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
				}


				//稳态调度
				set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
				set<int>::iterator endIter = ptempstagenum.end();

				buf << "\twhile(1)\n";
				buf << "\t{\n";
				for (int i = stageMaxNum - 1; i >= 0; i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);//在该线程执行的阶段中寻找
					if (stageiter != endIter)
					{
						buf << "\t\tif(stage[" << i << "])\n";
						buf << "\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
						vector<FlatNode*>::iterator iter1;
						for (iter1 = flatVec.begin(); iter1 != flatVec.end(); ++iter1)
						{
							if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
							{
								//当前线程为划分中该算子被划分的线程
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
							}
						}
						buf << "\t\t}\n";
					}
				}

				
				//开始数组回滚
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";

				//二级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,true," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//处理STOP
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&label" << graphIndex - 1 << "==STOP&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t//上游数据不足，开始排空\n";
				buf << "\t\t\tstage[0] = 0;\n";
				buf << "\t\t}\n";

				//上游交互
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t//必须与下游一起停止才能唤醒上游\n";
				buf << "\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//加锁
				buf << "\t\t\tif(label" << graphIndex + 1 << "==WAITING||label" << graphIndex + 1 << "==FULL)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t//下游停止\n";
				buf << "\t\t\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t\t\t}\n";
				buf << "\t\t\tlabel" << graphIndex << "=WAITING;\n";
				buf << "\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t\t\t//唤醒后判断\n";
				buf << "\t\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t\t{\n";
				
				buf << "\t\t\t\tstage[0] = 0;\n";
				buf << "\t\t\t}\n";
				buf << "\t\t\telse\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//加锁，修改状态为running
				buf << "\t\t\t\tlabel" << graphIndex << "=RUNNING;\n";
				buf << "\t\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";//解锁
				buf << "\t\t\t\t" << getGraphInName(graphIndex) << ".resetHead();\n";
				buf << "\t\t\t}\n";
				buf << "\t\t}\n";

				//下游交互
				//出现MERGEONE一定有下游子图
				buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//加锁
				buf << "\t\t\tlabel" << graphIndex << "=FULL;\n";
				buf << "\t\t\tif(label" << graphIndex + 1 << "==WAITING||label" << graphIndex + 1 << "==FULL)\n";
				buf <<"\t\t\t{\n";
				buf << "\t\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "\t\t\t}\n";
				buf << "\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";	//解锁
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";
				buf << "\t\t\tlabel" << graphIndex << "=RUNNING;\n";
				buf << "\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";
				buf << "\t\t\t" << getGraphOutName(graphIndex) << ".resetWrite();\n";
				buf << "\t\t}\n";
				buf << "\t\telse\n";
				buf << "\t\t{\n";
				buf << "\t\t\t;\n";
				buf << "\t\t}\n";

				//从属流水线判断空
				buf << "\t\tif(stage[0]==0&&pipeline" << graphIndex << "==true)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpipeline" << graphIndex << "=false;\n";
				buf << "\t\t}\n";

				//流水线停止判断
				buf << "\t\t\tout = true;\n";
				buf << "\t\tfor(int i = 0;i<" << stageMaxNum << ";++i)\n";
				buf << "\t\t{\n";
				
				buf << "\t\t\tif(stage[i]==1)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\tout=false;\n";
				buf << "\t\t\t}\n";
				buf << "\t\t}\n";
				buf << "\t\tif(out==false);\n";
				buf << "\t\telse break;\n";

				//控制其他线程
				buf << "\t\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";

				//三级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\t}\n";
				buf << "\tcout<<\"" << graphIndex << "_" << getindexinGraph(index) << "\"<<endl;\n";
				buf << "\tlabel" << graphIndex << "=STOP;\n";
				buf << "\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "}\n";
					

			}
			else if (dsg_->combineList[graphIndex] == MERGEDOWNTWO || dsg_->combineList[graphIndex] == MERGETWO || dsg_->combineList[graphIndex]
				== MERGEUPTWO)
			{
				//下游子图主线程
				//压缩子图
				//上游子图处理
				buf << "\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\tlabel" << graphIndex<< "=RUNNING;\n";
				buf << "\tif(label" << graphIndex-1 << "!=STOP)\n";
				buf << "\t{\n";
				buf << "\t\t" << getGraphInName(graphIndex) << ".copy(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t}\n";
				buf << "\telse\n";
				buf << "\t{\n";
				buf << "\t\t//上游子图数据不足,拷贝剩余数据\n";
				buf << "\t\t" << getGraphInName(graphIndex) << ".append(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t{\n";
				buf << "\t\t\tlabel" << graphIndex << "=STOP;\n";
				if (graphIndex != (dsg_->staticChildGraph.size() - 1))
					buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "\t\t\treturn ;\n";
				buf << "\t\t}\n";
				buf << "\t}\n";


				//启动从属线程
				buf << "\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";

				//屏障
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,false," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//构造函数调用
				//找到全部算子
				vector<FlatNode*> tmpactorset;
				//获取编号
				int bianhao = getindexinGraph(index);
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(bianhao);

				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//判断是否要用DSG构造
					if (dsg_->getActorPosition(*iter) == FIRST)
					{
						//只有一个输入边
						string inName = getGraphInName(graphIndex);

						pos2 = mapActor2OutEdge.equal_range(*iter);
						while (pos2.first != pos2.second)
						{
							buf << pos2.first->second << ",";
							++pos2.first;
						}
						buf << inName << ",";

					}
					else if (dsg_->getActorPosition(*iter) == LAST)
					{
						//最多一个输出边
						string outName = getGraphOutName(graphIndex);
						pos1 = mapActor2InEdge.equal_range(*iter);
						if (outName != "")
						{
							buf << outName << ",";
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}


					}
					else
					{
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter的输入边列表
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter的输出边列表
						while (pos2.first != pos2.second)
						{
							buf << pos2.first->second << ",";
							++pos2.first;
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}
					}
					buf.seekp((int)buf.tellp() - 1);		//去掉一个逗号
					buf << ");\n";
					if (MakeProfile)		//做profile，累加执行时间
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}


				//流水调度数据
				//stage表示阶段号数组，初始除0外都为0
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//这里是执行阶段数目
				buf << "\tstage[0]=1;\n";
				buf << "\tbool out = true;\n";


				//初始化
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
				}

				//稳态调度
				//获取阶段
				set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
				set<int>::iterator endIter = ptempstagenum.end();

				buf << "\twhile(1)\n";
				buf << "\t{\n";
				for (int i = stageMaxNum - 1; i >= 0; i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);

					//找到在该核上执行的阶段
					if (stageiter != endIter)
					{
						buf << "\t\tif(stage[" << i << "])\n";
						buf << "\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
						//找到在该阶段中执行的算子
						vector<FlatNode*>::iterator iter1;
						for (iter1 = flatVec.begin(); iter1 != flatVec.end(); ++iter1)
						{
							if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
							{
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
							}
						}
						buf << "\t\t}\n";
					}
				}

				////开始数组回滚
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";

				//二级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,false," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//数据STOP处理
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&label" << graphIndex - 1 << "==STOP&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t\t" << getGraphInName(graphIndex) << ".append(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t\t\tstage[0] = 0;\n";
				buf << "\t\t}\n";


				//上游交互
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0&&label"<<graphIndex-1<<"!=STOP)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex-1<< ");\n";//加锁
				//FULL
				buf << "\t\t\tif(label" << graphIndex - 1 << "==FULL)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\t//上游停止运行\n";
				buf << "\t\t\t\t" << getGraphInName(graphIndex) << ".copy(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\t\t\t" << getGraphInName(graphIndex) << ".resetHead();\n";
				buf << "\t\t\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t\t\t\tpthread_mutex_unlock(&mutex" << graphIndex - 1 << ");\n";
				buf << "\t\t\t}\n";
				//WAITING
				buf << "\t\t\telse if(label" << graphIndex - 1 << "==WAITING)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\tlabel" << graphIndex << "=WAITING;\n";
				buf << "\t\t\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t\t\t\tpthread_mutex_unlock(&mutex" << graphIndex - 1 << ");\n";
				buf << "\t\t\t\tsem_wait(&exchange" << graphIndex << ");\n";

				buf << "\t\t\t\tif(label" << graphIndex - 1 << "==STOP)\n";
				buf << "\t\t\t\t{\n";
				buf << "\t\t\t\t\t" << getGraphInName(graphIndex) << ".append(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\t\t\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t\t\t\t\tstage[0]=0;\n";
				buf << "\t\t\t\t}\n";
				buf << "\t\t\t\telse\n";
				buf << "\t\t\t\t{\n";
				buf << "\t\t\t\t\t" << getGraphInName(graphIndex) << ".copy(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\t\t\t\t" << getGraphInName(graphIndex) << ".resetHead();\n";
				buf << "\t\t\t\t\tlabel" << graphIndex << "=RUNNING;\n";
				buf << "\t\t\t\t\tsem_post(&exchange" << graphIndex << ");\n";
				buf << "\t\t\t\t}\n";
				buf << "\t\t\t}\n";

				//RUNNING
				buf <<"\t\t\telse\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\tlabel" << graphIndex << "=WAITING;\n";
				buf << "\t\t\t\tpthread_mutex_unlock(&mutex" << graphIndex - 1 << ");\n";
				buf << "\t\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t\t\t\tif(label" << graphIndex - 1 << "==STOP)\n";
				buf << "\t\t\t\t{\n";
				buf << "\t\t\t\t\t" << getGraphInName(graphIndex) << ".append(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\t\t\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t\t\t\t\tstage[0]=0;\n";
				buf << "\t\t\t\t}\n";
				buf << "\t\t\t\telse\n";
				buf << "\t\t\t\t{\n";
				buf << "\t\t\t\t\t" << getGraphInName(graphIndex) << ".copy(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\t\t\t\t" << getGraphInName(graphIndex) << ".resetHead();\n";
				buf << "\t\t\t\t\tlabel" << graphIndex << "=RUNNING;\n";
				buf << "\t\t\t\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";

				buf << "\t\t\t\t}\n";
				buf << "\t\t\t}\n";
				buf << "\t\t}\n";

				//下游交互
				if (graphIndex == (dsg_->scheStaticChildGraph.size() - 1))
				{
					//没有下游;
				}
				else
				{
					buf << "\t\tif(" << getGraphOutName(graphIndex) << "_obj.full())\n";
					buf << "\t\t{\n";
					buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex - 1 << ");\n";//加锁
					buf << "\t\t\tif(label" << graphIndex - 1 << "==RUNNING)\n";
					buf << "\t\t\t\tlabel" << graphIndex << "=FULL;\n";
					buf << "\t\t\t\tpthread_mutex_unlock(&mutex" << graphIndex - 1 << ");\n";
					buf << "\t\t\t\tsem_wait(&exchange" << graphIndex << ");\n"; 
					buf << "\t\t\t}\n";

					buf << "\t\t\telse{\n";
					buf << "\t\t\t\tpthread_mutex_unlock(&mutex" << graphIndex - 1 << ");\n";
					buf << "\t\t\t}\n";

					buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
					buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
					buf << "\t\t\t" << getGraphOutName(graphIndex) << ".resetTail();\n";
					buf << "\t\t}\n";

				}

				//从属线程控制
				buf << "\t\tif(stage[0]==0&&pipeline" << graphIndex << "==true)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpipeline" << graphIndex << "=false;\n";
				buf << "\t\t}\n";

				//流水线停止判断
				buf << "\t\t\tout = true;\n";
				buf << "\t\tfor(int i = 0;i<" << stageMaxNum << ";++i)\n";
				buf << "\t\t{\n";
				
				buf << "\t\t\tif(stage[i]==1)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\tout = false;\n";
				buf << "\t\t\t}\n";
				buf << "\t\t}\n";
				buf << "\t\tif(out==false);\n";
				buf << "\t\telse break;\n";

				//从属线程信号量
				buf << "\t\tfor(int i = 0;i<" << stageMaxNum << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";
				
				//三级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\t}\n";

				//停止
				buf << "\tlabel" << graphIndex << "=STOP;\n";
				buf << "\tcout<<\"" << graphIndex << "_" << getindexinGraph(index) << "\"<<endl;\n";
				buf << "}\n";

			}
			else
			{
				//非压缩子图主线程
				//上游子图处理
				buf << "\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t//判断上游子图是否已经停止\n";
				buf << "\tif(label" << graphIndex - 1 << "==STOP)\n";
				buf << "{\n";
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t{\n";
				buf << "\t\t\tlabel" << graphIndex << "=STOP;\n";
				if (graphIndex != (dsg_->staticChildGraph.size() - 1))
					buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "\t\t\treturn ;\n";
				buf << "\t\t}\n";
				buf << "\t}\n";


				//启动从属线程
				buf << "\tfor(int i = 1;i<" << nCpucore_ << ";i++)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";

				//屏障
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ");\n";
				//构造函数调用
				//如果是first或者last，就需要调整构造函数


				//找到对应当前index的全部算子
				vector<FlatNode*> tmpactorset;
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(index%nCpucore_);
				
				//构造函数调用
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//判断是否要用DSG构造
					if (dsg_->getActorPosition(*iter) == FIRST)
					{
						//只有一个输入边
						string inName = getGraphInName(graphIndex);
						
						pos2 = mapActor2OutEdge.equal_range(*iter);
						while (pos2.first != pos2.second)
						{
							buf << pos2.first->second << ",";
							++pos2.first;
						}
						buf << inName <<",";

					}
					else if (dsg_->getActorPosition(*iter) == LAST)
					{
						//最多一个输出边
							string outName = getGraphOutName(graphIndex);
							pos1 = mapActor2InEdge.equal_range(*iter);
							if (outName != "")
							{
								buf << outName << ",";
							}
							while (pos1.first != pos1.second)
							{
								buf << pos1.first->second << ",";
								++pos1.first;
							}
							

					}
					else
					{
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter的输入边列表
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter的输出边列表
						while (pos2.first != pos2.second)
						{
							buf << pos2.first->second << ",";
							++pos2.first;
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}
					}
					buf.seekp((int)buf.tellp() - 1);		//去掉一个逗号
					buf << ");\n";
					if (MakeProfile)		//做profile，累加执行时间
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}


				//流水调度数据
				//stage表示阶段号数组，初始除0外都为0
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//这里是执行阶段数目
				buf << "\tstage[0]=1;\n";
				buf << "\tbool out = true;\n";


				//初始化
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
				{
					buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
				}

				//稳态调度前同步，屏障
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";

				set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
				set<int>::iterator endIter = ptempstagenum.end();

				buf << "\twhile(1)\n";
				buf << "\t{\n";
				for (int i = stageMaxNum - 1; i >= 0;i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);
					if (stageiter != endIter)
					{
						buf << "\t\tif(stage[" << i << "])\n";
						buf << "\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);//对应阶段的算子
						vector<FlatNode*>::iterator iter1;//用于循环该阶段算子
						for (iter1 = flatVec.begin(); iter1 != flatVec.end(); ++iter1)
						{
							if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
							{
								//当前线程序号等于该算子划分的编号，表示该算子确定在该线程中执行
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
							}
						}
						buf << "\t\t}\n";
					}
				}

				//开始数组回滚
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";

				//二级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ");\n";
				//上游子图同步
				//stop判断
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&label" << graphIndex - 1 << "==STOP&&stage[0]!=0)\n";
				buf << "\t\t\tstage[0] = 0;\n";
				

				//上游子图判断
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";

				buf << "\t\t\t//等待唤醒\n";
				buf << "\t\t\t\t" << getGraphInName(graphIndex) << ".resetHead();\n";
				buf << "\t\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\tstage[0] = 0;\n";
				buf << "\t\t\t}\n";
				buf << "\t\t\telse\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t;\n";
				buf << "\t\t\t}\n";
				buf << "\t\t}\n";

				//下游子图判断，如果有下游
				if (graphIndex == (dsg_->scheStaticChildGraph.size() - 1))
				{
					;
				}
				else
				{
					//有下游
					buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
					buf << "\t\t{\n";
					buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
					buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
					buf << "\t\t\t" << getGraphOutName(graphIndex) << ".resetTail();\n";
					buf << "\t\t}\n";
				}

				//流水线排空通知
				buf << "\t\tif(stage[0]==0&&pipeline" << graphIndex << "==true)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpipeline" << graphIndex << "=false;\n";
				buf << "\t\t}\n";
					
				//流水线停止判断
				buf << "\t\t\tout = true;\n";
				buf << "\t\tfor(int i = 0;i<" << stageMaxNum << ";++i)\n";
				buf << "\t\t{\n";
				
				buf << "\t\t\tif(stage[i]==1)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\tout=false;\n";
				buf << "\t\t\t}\n";
				buf << "\t\t}\n";
				buf << "\t\tif(out==false);\n";
				buf << "\t\telse break;\n";


				//线程同步
				buf << "\t\tfor(int i = 1;i<" << nCpucore_ << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";

				//三级屏障
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";

				buf << "\t}\n";
				buf << "\tcout<<\"" << graphIndex << "_" << getindexinGraph(index) << "\"<<endl;\n";
				buf << "\tlabel" << graphIndex << "=STOP;\n";
				if (graphIndex != (dsg_->scheStaticChildGraph.size() - 1))
					buf << "\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "}\n";

			}
		}
		else
		{
			//非首子图从属线程
			buf << "\tsem_wait(&zu" << graphIndex << ");\n";

			//屏障
			//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\tworkerSync(" << index%nCpucore_ << ");\n";
			//构造函数
			//如果包含动态边，就需要注意，即本线程中算子为该子图的first或者last
			//找到对应当前index的全部算子
			vector<FlatNode*> tmpactorset;
			//tmpactorset = mplist[graphIndex]->findNodeSetInPartition(index%nCpucore_);
			tmpactorset = mplist[graphIndex]->findNodeSetInPartition(getindexinGraph(index));

			for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
			{
				buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
				if (dsg_->getActorPosition(*iter) == FIRST)
				{
					//只有一个输入边
					string inName = getGraphInName(graphIndex);

					pos2 = mapActor2OutEdge.equal_range(*iter);
					while (pos2.first != pos2.second)
					{
						buf << pos2.first->second << ",";
						++pos2.first;
					}
					buf << inName << ",";

				}
				else if (dsg_->getActorPosition(*iter) == LAST)
				{
					//最多一个输出边
					string outName = getGraphOutName(graphIndex);
					pos1 = mapActor2InEdge.equal_range(*iter);
					if (outName != "")
					{
						buf << outName << ",";
					}
					while (pos1.first != pos1.second)
					{
						buf << pos1.first->second << ",";
						++pos1.first;
					}


				}
				else
				{
					pos1 = mapActor2InEdge.equal_range(*iter);		//iter的输入边列表
					pos2 = mapActor2OutEdge.equal_range(*iter);		//iter的输出边列表
					while (pos2.first != pos2.second)
					{
						buf << pos2.first->second << ",";
						++pos2.first;
					}
					while (pos1.first != pos1.second)
					{
						buf << pos1.first->second << ",";
						++pos1.first;
					}
				}
				buf.seekp((int)buf.tellp() - 1);		//去掉一个逗号
				buf << ");\n";

				if (MakeProfile)		//做profile，累加执行时间
				{
					buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
				}
			}
			//流水调度数据
			//stage表示阶段号数组，初始除0外都为0
			buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//这里是执行阶段数目
			buf << "\tstage[0]=1;\n";
			buf << "\tbool out = true;\n";


			//初始化
			for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
			{
				buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
			}

			

			//获得在该核上执行的阶段集合
			set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
			set<int>::iterator endIter = ptempstagenum.end();
			buf << "\twhile(1)\n";
			buf << "\t{\n";
			for (int i = stageMaxNum - 1; i >= 0; i--)
			{
				//先判断该阶段是否在该核上
				set<int>::iterator stageiter = ptempstagenum.find(i);
				if (stageiter != endIter)
				{

					//找到在该阶段中执行的算子
					buf << "\t\tif(stage[" << i << "])\n";
					buf << "\t\t{\n";
					vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
					vector<FlatNode*>::iterator iter1;
					for (iter1 = flatVec.begin(); iter1 != flatVec.end(); ++iter1)
					{

						if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
						{
							//当前线程序号等于该算子划分的编号，表示该算子确定在该线程中执行
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
						}
					}
					buf << "\t\t}\n";
					}
			}

			//数组回滚
			buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
			buf << "\t\t\tstage[index]=stage[index-1];\n";
				
			
			//二级屏障
			//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\tworkerSync(" << index%nCpucore_ << ");\n";
			//流水线停止判断
			buf << "\t\t\tout = true;\n";
			buf << "\t\tfor(int i = 0;i<" << stageMaxNum << ";++i)\n";
			buf << "\t\t{\n";

			buf << "\t\t\tif(stage[i]==1)\n";
			buf << "\t\t\t{\n";
			buf << "\t\t\t\tout=false;\n";
			buf << "\t\t\t}\n";
			buf << "\t\t}\n";
			buf << "\t\tif(out==false);\n";
			buf << "\t\telse break;\n";

			buf << "\t\tsem_wait(&zu" << graphIndex << ");\n";

			//流水线排空判断
			buf << "\t\tif(pipeline" << graphIndex << "==false&&stage[0]!=0)\n";
			buf << "\t\t{\n";
			buf << "\t\t\tstage[0] = 0;\n";
			buf << "\t\t}\n";
			//三级屏障
			//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			
			buf << "\t}\n";
			buf << "\tcout<<\"" << graphIndex << "_" << getindexinGraph(index) << "\"<<endl;\n";
			buf << "}\n";
		}
		

	}
}
void DynamicX86CodeGenerate::DYAllActorHeader()		//生成所有算子文件头文件的集合文件
{
	vector<FlatNode*>::iterator iter;
	stringstream buf, ss;
	buf << "/*包含所有actor的头文件，主要是为了方便主文件包含*/\n\n";
	for (iter = flatNodes_.begin(); iter != flatNodes_.end(); iter++)
	{
		buf << "#include \"" << (*iter)->name << ".h\"\n";
	}
	ss << dir_ << "AllActorHeader.h";
		OutputToFile(ss.str(), buf.str());
}

void DynamicX86CodeGenerate::DYactors()
{
	stringstream ss, buf;
	std::map<operatorNode*, string>::iterator pos;
	for (int i = 0; i < nActors_; ++i)
	{
		int len = ListLength(flatNodes_[i]->oldContents->decl->u.decl.type->u.operdcl.outputs);		//输出边数目
		int nOut = flatNodes_[i]->nOut;
		//这里有一个无用判断，按道理，len是等于nOut的
		OperatorType ot = flatNodes_[i]->oldContents->ot;
		//这个ot代表了算子的类型
		//原始的这里的pos获取没有意义
		string name = flatNodes_[i]->name;
		int index = name.find_first_of("_");
		string tmp = name.substr(0, index);//获得算子名字

		mapOperator2ClassName.insert(make_pair(flatNodes_[i]->oldContents,flatNodes_[i]->name));

		buf << "#include \"" << flatNodes_[i]->name << ".h\"\n";
		if (strcmp(tmp.c_str(), "FileReader") == 0)
			isInFileReader = true;
		if (strcmp(tmp.c_str(), "FileWriter") == 0)
			isInFileWriter = true;
		curactor = flatNodes_[i];
		DYactor(curactor, ot);
	}
	ss << dir_ << "AllActorHeader.h";
	OutputToFile(ss.str(), buf.str());
}
void DynamicX86CodeGenerate::DYEdgeParam(FlatNode *actor, stringstream &buf)		//该函数生成缓冲区成员变量
{
	buf << "\t// make output datamember\n";
	vector<FlatNode*>::iterator iter, end;	//生成producer与consumer

	//获取输出边
	iter = actor->outFlatNodes.begin();
	end = actor->outFlatNodes.end();

	//获取输出边名称
	List* outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;		//输出边的列表
	ListMarker output_maker;
	Node*outputNode = NULL;
	IterateList(&output_maker, outputList);
	while (iter != end)
	{
		NextOnList(&output_maker, (GenericREF)&outputNode);		//转换为void**
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl)outputString = outputNode->u.decl.name;
		if (dsg_->getActorPosition(actor) == LAST)//使用动态缓冲
		{
			//处于First或者last的位置，对于输出边只有LAST需要使用动态缓冲，但是需要判断是否是Sink
			//如果是First，则要判断是不是压缩算子的下游，还有判断是否是sink
			//为last判断是否是Sink
			if (dsg_->isSink(actor))	//是sink
				buf << "\tProducer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << ">" << outputString << ";\n";
			else
				buf << "\tDSGProducer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << "> &" << outputString << ";\n";
		}
		else
			buf << "\tProducer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << ">" << outputString << ";\n";
		iter++;
	}


	buf << "\t// make input datamember\n";
	//获取输入边
	iter = actor->inFlatNodes.begin();
	end = actor->inFlatNodes.end();
	Node*inputNode = NULL;
	List*inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	ListMarker input_maker;
	IterateList(&input_maker, inputList);
	while (iter != end)
	{
		NextOnList(&input_maker, (GenericREF)&inputNode);
		string inputString;
		if (inputNode->typ == Id) inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl) inputString = inputNode->u.decl.name;

		if (dsg_->getActorPosition(actor) == FIRST)		//子图的首算子
		{
			//判断是否是source
			if (dsg_->isSource(actor))
				buf << "\tConsumer<" << pEdgeInfo->getEdgeInfo((*iter), actor).typeName << ">" << inputString << ";\n";
			else
				buf << "\tDSGConsumer<" << pEdgeInfo->getEdgeInfo((*iter), actor).typeName << "> &" << inputString << ";\n";
		}
		else
			buf << "\tConsumer<" << pEdgeInfo->getEdgeInfo((*iter), actor).typeName << ">" << inputString << ";\n";;
		iter++;
	}
}
void DynamicX86CodeGenerate::DYactor(FlatNode*actor,OperatorType ot)
{
	//生成actor变化，根据CombineState和算子位置来判断需要使用什么样的缓冲区
	//同时include的库也要增加
	stringstream buf;//存放输出到actor头文件的字符串流
	stringstream srcBuf;//存放输出到actor源文件的字符流，实际基本用不到
	vector<string>::iterator iter;
	string className = actor->name;
	assert(actor);
	cout << actor->name << "      " << endl;
	actor->SetIOStreams();		//该函数设置nPeek/nPop/nPushString

	//头文件
	buf << "/**\n*Class " << className << "\n*/\n";

	buf << "#include \"Buffer.h\"\n";

	buf << "#include \"DSG.h\"\n";
	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"Consumer.h\"\n";
	buf << "#include \"Producer.h\"\n";
	buf << "#include \"global.h\"\n";
	buf << "#include <string>\n";
	buf << "#include \"iostream\"\n";
	buf << "#include \"GlobalVar.h\"\n";
	buf << "using namespace std;\n";

	if (CallModelEmbed)
		buf << "namespace COStream{\n";//添加名字空间,这部分目前不考虑
	//类定义开始
	buf << "class "<< className << "{\n";

	//通信边定义
	DYEdgeParam(actor, buf);
	if (actor == readerActor)
	{

	}
	else if (actor == writerActor)
	{

	}
	else
	{
		//开始写入类私有成员
		parameterBuf.str("");		//清空parammeterBuf,thisBuf
		thisBuf.str("");
		buf << "private:\n";

		DYdeclList(actor, ot, buf);		//写入语法书变量信息，其实主要是定义初稳态调度次数
		DYinitVarAndState(actor, ot, buf);//写入在work外的一些变量的声明

		DYlogicInit(actor, ot, buf);	//init函数，没init函数会创建一个空函数	

		DYpopToken(actor, buf);			//写pop函数，消费者

		DYpushToken(actor, buf);		//写push函数，生产者

		//写入initwork函数
		DYinitWork(buf);

		//写入work函数
		DYwork(actor, ot, buf);

	}
	
	//生成每个actor的成员函数，包括构造函数，初态调度，稳态调度
	buf << "public:\n";
	//构造函数
	DYthis(actor, ot, buf);
	//初态调度函数生成
	DYrunInitScheduleWork(actor,buf);
	//稳态调度函数生成
	DYrunSteadyScheduleWork(actor, buf);
	buf << "};\n";

	
	if (CallModelEmbed)
		buf << "}\n";//名字空间块结束
	//构造源文件内容
	srcBuf << "#include \"" << actor->name << ".h\"\n";
	for (iter = staticNameInit.begin(); iter != staticNameInit.end(); ++iter)
		srcBuf << (*iter);
	//输出到文件
	stringstream headerFileName, srcFileName;	//分别保存头文件名和源文件名，源文件定义了该actor的静态成员
	headerFileName << dir_ << className << ".h";
	srcFileName << dir_ << className << ".cpp";
	OutputToFile(headerFileName.str(), buf.str());	//头文件
	if (staticNameInit.size() != 0)		//若包含静态成员，则生成源文件初始化类成员
		OutputToFile(srcFileName.str(), srcBuf.str());
	staticNameInit.clear();
}

void DynamicX86CodeGenerate::DYpopToken(FlatNode*actor, stringstream&buf)
{
	//消费者
	//如果actor位置不是first，就是正常的poptoken，否则判断是否source，然后就调用DSGComsumer的函数
	buf << "\t//popToken\n";
	buf << "\tvoid popToken(){";
	//注意，对于动态算子通信边，它必须是唯一的输出边，如果出现多输出就是错的。
	List*inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	

	ListMarker input_maker;
	
	Node *inputNode = NULL;
	
	//输入边的数据是固定的，所以不存在变化

	IterateList(&input_maker, inputList);
	vector<int>::iterator iter = actor->inPopWeights.begin();
	while (NextOnList(&input_maker, (GenericREF)&inputNode))
	{
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl)inputString = inputNode->u.decl.name;
		buf << "\n\t" << inputString << ".updatehead" << "(" << *iter << ");\n";
	}
	buf << "\t}\n";
}

void DynamicX86CodeGenerate::DYpushToken(FlatNode*actor, stringstream&buf)
{
	//生产者
	//如果actor位置不是last，就是正常的pushtoken，否则判断是否sink，然后就调用DSGProducer的函数
	buf << "\t// pushToken\n";
	buf << "\tvoid pushToken(){";
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;		//输出列表
	ListMarker output_maker;
	Node* outputNode = NULL;
	IterateList(&output_maker, outputList);
	vector<int>::iterator iter = actor->outPushWeights.begin();		//输出边的窗口迭代器

	while (NextOnList(&output_maker, (GenericREF)&outputNode))
	{
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl)outputString = outputNode->u.decl.name;
		if (dsg_->getActorPosition(actor) == LAST)
		{
			if (dsg_->isSink(actor))
			{
				buf << "\n\t" << outputString << ".updatetail" << "(" << *iter << ");\n";
			}
			else
			{
				assert(ListLength(outputList) == 1);
				buf << "\n\t" << outputString << ".updatetail" << "();\n";
			}
		}
		else
			buf << "\n\t" << outputString << ".updatetail" << "(" << *iter << ");\n";
		iter++;
	}
	buf << "\t}\n";

	/*if (dsg_->getActorPosition(actor) == LAST)		//为子图最后算子
	{
		if (dsg_->isSink(actor))		//不是sink，正常产生输出边Producer
		{
			while (NextOnList(&output_maker,(GenericREF)outputNode))
			{
				string outputString;
				if (outputNode->typ == Id)outputString = outputNode->u.id.text;
				else if (outputNode->typ == Decl)outputString = outputNode->u.decl.name;
				buf << "\n\t" << outputString << ".updatetail" << "(" << *iter << ");\n";
			}
		}
		else
		{
			assert(ListLength(outputList) == 1);//作为静态子图的LAST必须保证只有一个输出边
			//DSGProducer

		}
	}
	else	//子图中非LAST的其他actor
	{
		while (NextOnList(&output_maker, (GenericREF)&outputNode))
		{
			string outputString;
			if (outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl)outputString = outputNode->u.decl.name;
			buf << "\n\t" << outputString << ".updatetail" << "(" << *iter << ");\n";
			iter++;//下一个窗口
		}
	}
		
		*/
}

void DynamicX86CodeGenerate::DYthis(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	//判断actor位置，如果是FIRST或者LAST并且不是source或者sink，就要调用相应新缓冲区构造函数并添加参数
	//构造函数不同
	//DSGProducer和DSGConsumer还是使用原始Buffer，但是使用的参数是全局变量
	buf << "\t //Constructor\n";
	buf << "\t" << actor->name << "(";//构造函数名称
	vector<FlatNode*>::iterator iter, end;

	//构造函数参数
	//遍历输出边
	iter = actor->outFlatNodes.begin();
	end = actor->outFlatNodes.end();

	List*outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;

	
	ListMarker output_maker;
	Node *outputNode = NULL;
	IterateList(&output_maker, outputList);
	while (iter != end)		//循环输出边
	{
		NextOnList(&output_maker, (GenericREF)&outputNode);
		string outputString;
		if (outputNode->typ == Id) outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl)outputString = outputNode->u.decl.name;//获得输出边的名称
		
		//需要加判断是否是子图的尾或者首算子
		if (dsg_->getActorPosition(actor) == LAST||dsg_->getActorPosition(actor)==BOTH)
		{
			buf << "DSGProducer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << ">&" << outputString << ",";
		}
		else
			buf << "Buffer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << ">&" << outputString << ",";
		iter++;
	}


		//遍历输入边
	iter = actor->inFlatNodes.begin();
	end = actor->inFlatNodes.end();

	//获取输入边名称
	List*inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	ListMarker input_maker;
	Node* inputNode = NULL;
	IterateList(&input_maker, inputList);
	
	while (iter != end)
	{
		NextOnList(&input_maker, (GenericREF)&inputNode);
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl)inputString = inputNode->u.decl.name;//结果是类似PStream1_06_0这种的
		if (dsg_->getActorPosition(actor) == FIRST || dsg_->getActorPosition(actor) == BOTH)
		{
			buf << "DSGConsumer<" << pEdgeInfo->getEdgeInfo((*iter),actor).typeName << ">&" << inputString << ",";
		}
		else
			buf << "Buffer<"<<pEdgeInfo->getEdgeInfo((*iter), actor).typeName << ">&" << inputString << ",";
		iter++;
	}
	//以上部分生成了构造函数的（）内的形参，现在要生成默认初始化列表

	//这部分需要检验
	buf << parameterBuf.str();
	if (parameterBuf.str() == "")
	{
		buf.seekp((int)buf.tellp() - 1);//设置输出文件流的指针位置，tellp用于输出流返回流中指针位置
		//这里变化的原因是因为前面填充输入边的时候，多填了一个，
	}
	else
	{
		buf.seekp((int)buf.tellp() - 2);
	}
	buf << "):";

	//开始生成默认初始化列表
	//输出边，对于LAST节点要特殊设置
	IterateList(&output_maker, outputList);
	while (NextOnList(&output_maker, (GenericREF)&outputNode))
	{
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ = Decl) outputString = outputNode->u.decl.name;
		//判断//关键是，对于LAST节点，并且是sink，根本不会进入这个地方，不过为了防止有人故意写，还是写上好了
		if (dsg_->getActorPosition(actor) == LAST || dsg_->getActorPosition(actor) == BOTH)
		{
			if (dsg_->isSink(actor))		//SDF的最后一个
			{
				buf << outputString << "(" << outputString << "),";
			}
			else
			{
				assert(ListLength(outputList) == 1);
				//输出边是动态，需要获取下一子图的起始actor的数据需求量
				int dataInUse = dsg_->getNextGraphFirstNodePop(actor);
				//buf << outputString << "(" << outputString << "," << dataInUse << "," << batch << "),";
				buf << outputString << "(" << outputString << "),";
			}

		}
		else
			buf << outputString << "("<<outputString << "),";
	}
	
	//输入边，对于FIRST节点要特殊设置,对于source，没有可能产生输入边，它根本不会进入循环
	IterateList(&input_maker, inputList);
	while (NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl)inputString = inputNode->u.decl.name;
		//判断,输入边，如果是FIRST还不是source就需要使用新缓冲区
		if (dsg_->getActorPosition(actor) == FIRST || dsg_->getActorPosition(actor) == BOTH)
		{
			//首先要获取本算子的数据需求数量
			int dataInUse = dsg_->getThisNodePop(actor);
			//buf << inputString << "(" << inputString << "," << dataInUse << ")";
			buf << inputString << "(" << inputString << "),";
		}
		else
			buf << inputString << "(" << inputString << "),";
	}
	buf.seekp((int)buf.tellp() - 1);
	/*if (ListLength(inputList) == 0)
	{
		//没有输入边
		buf.seekp((int)buf.tellp() - 1);
		//去一个逗号
	}*/
	buf << "{\n";
	buf << thisBuf.str();

	buf << "\t\tsteadyScheduleCount = " << dsg_->getSteadyCount(actor) << ";\n";
	buf << "\t}\n";


}
void DynamicX86CodeGenerate::DYrunInitScheduleWork(FlatNode*actor, stringstream &buf)
{
	//判断actor位置，如果是FIRST或者LAST并且不是source或者sink，对于特殊缓冲区其初态不一样'
	//对于动态，其初态的意义就是一些变量的初始化，并不进行work
	buf << "\t//runInitScheduleWork\n";
	buf << "\tvoid runInitScheduleWork(){\n";
	buf << "\t\tinitWork();\n";
	buf << "\t}\n"; // CGrunInitScheduleWork方法结束

}
//对于动态的Producer，每次work调用[]和updatetail，稳态一次并不执行resetTail，当缓冲区满的时候会在[]中扩大
//对于动态的Consumer，每次稳态执行N次updatehead，但是数据满足与否在线程函数中执行，所以也没必要执行resetHead，只要下游
//算子被唤醒发现有数据读，才会resethead
void DynamicX86CodeGenerate::DYrunSteadyScheduleWork(FlatNode*actor, stringstream &buf)
{
	//判断actor位置，如果是FIRST或者LAST并且不是source或者sink，对于特殊缓冲区其稳态不一样
	buf << "\t//runSteadyScheduleWork\n";
	buf << "\tvoid runSteadyScheduleWork(){\n";
	if (AmplifySchedule)
	{
		buf << "\t\tfor(int j=0;i<AMPLIFYFACTOR;j++)\n\t\t{\n";
	}
	buf << "\t\tfor(int i = 0;i<steadyScheduleCount;i++)\n\t\twork();\n";
	if (NoCheckBuffer)
	{
		//出边
		List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;
		ListMarker output_maker;
		Node *outputNode = NULL;
		IterateList(&output_maker, outputList);
		while (NextOnList(&output_maker, (GenericREF)&outputNode))
		{
			string outputString;
			if (outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl)outputString = outputNode->u.decl.name;

			if (dsg_->getActorPosition(actor) == LAST || dsg_->getActorPosition(actor) == BOTH)
			{
				//动态边不需要resetTail;
			}
			else
				buf << "\t\t" << outputString << ".resetTail();\n";
		}

		//入边
		List *inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
		ListMarker input_maker;
		Node* inputNode = NULL;
		IterateList(&input_maker, inputList);
		while (NextOnList(&input_maker, (GenericREF)&inputNode))
		{
			string inputString;
			if (inputNode->typ == Id) inputString = inputNode->u.id.text;
			else if (inputNode->typ == Decl)inputString = inputNode->u.decl.name;
			if (dsg_->getActorPosition(actor) == FIRST || dsg_->getActorPosition(actor) == BOTH)
			{
				;
			}
			else
				buf << "\t\t" << inputString << ".resetHead();\n";
		}
	}
	if (AmplifySchedule)
		buf << "\t\t}\n";
	buf << "\t}\n"; // // CGrunSteadyScheduleWork方法结束
}










//工具函数
int DynamicX86CodeGenerate::OutpusString(const char *s)
{

	int len = 0;

	declInitList << "\"";
	while (*s != 0)
	{
		len += OutputChar(*s++);
	}
	declInitList << "\"";

	return len + 2;
}
int DynamicX86CodeGenerate::OutputChar(char val)
{
	switch (val)
	{
	case '\n': declInitList << "\\n"; break;
	case '\t': declInitList << "\\t"; break;
	case '\v': declInitList << "\\v"; break;
	case '\b': declInitList << "\\b"; break;
	case '\r': declInitList << "\\r"; break;
	case '\f': declInitList << "\\f"; break;
	case '\a': declInitList << "\\a"; break;
	case '\\': declInitList << "\\\\"; break;
	case '\?': declInitList << "\\\?"; break;
	case '\"': declInitList << "\\\""; break;
	case '\'': declInitList << "\\\'"; break;
	default:
		if (isprint(val)) //判断val是否为可打印字符
		{
			declInitList << (val);
		}
		else
		{
			declInitList << "\\" << val;
		}
	}

	return 1;
}

void DynamicX86CodeGenerate::CGFrta()
{

	stringstream buf;
	int n = nDeclDim.size() - 1;
	buf << "#include <string>\n";
	buf << "#include \"fstream\"\n\n";
	buf << "using namespace std;\n\n";
	buf << "int frta(double cc";
	for (int i = 0; i<n; i++)
		buf << "[" << nDeclDim[i + 1] << "]";
	buf << ", string path){\n";
	assert(n <= 3);
	if (n>0)
		buf << "\tint i";
	if (n > 1)
		buf << ",j";
	if (n > 2)
		buf << ",k";
	buf << ";\n";
	buf << "\tifstream fin;\n";
	buf << "\tfin.open(path.c_str());\n";
	buf << "\tif(!fin.is_open()){\n";
	buf << "\t\texit(EXIT_FAILURE);\n\t}\n";
	if (n > 0)
		buf << "\tfor(i=0;i<" << nDeclDim[1] << ";i++)\n";
	if (n > 1)
		buf << "\t\tfor(j=0;j<" << nDeclDim[2] << ";j++)\n";
	if (n > 2)
		buf << "\t\t\tfor(k=0;k<" << nDeclDim[3] << ";k++)\n";
	for (int i = 0; i <= n; i++)
		buf << "\t";
	buf << "fin>>cc";
	if (n > 0)
		buf << "[i]";
	if (n > 1)
		buf << "[j]";
	if (n > 2)
		buf << "[k]";
	buf << ";\n";
	buf << "\tfin.close();\n";
	buf << "\treturn 0;\n}\n";


	//输出到文件
	stringstream ss;
	ss << dir_ << "frta.h";
	OutputToFile(ss.str(), buf.str());
}
void DynamicX86CodeGenerate::OutputToFile(string fileName, string oldContents)
{
	ofstream fw;
	try{
		fw.open(fileName.c_str());
		fw << oldContents;
		fw.close();
	}
	catch (...){
		cout << "error:output to file" << endl;
	}
}
//string DynamicX86CodeGenerate::GetPrimDataType(Node *from)//类型定义
//{
//	string type;
//
//	switch (from->u.prim.basic){
//	case Sshort:
//	case Sint:
//		type = "int";;
//		break;
//		/*Manish 2/3 hack to print pointer constants */
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
//	default: type = "Any";
//		break;
//	}
//	return type;
//}
void DynamicX86CodeGenerate::AdclInit(Node *from, int offset)
{
	Node *arrayNode = from->u.decl.type;
	Node *tmpNode = from->u.decl.type;
	Node *initNode = from->u.decl.init;//变量初始化node
	string name = from->u.decl.name;
	string arrayType = GetArrayDataType(tmpNode->u.adcl.type);
	string dim = GetArrayDim(tmpNode->u.adcl.dim);
	bool isGlobal = FALSE;
	if (initNode == NULL) //如果没有初始化，则按数组类型进行初始化
	{
		if (from->u.decl.tq == T_TOP_DECL){
			nDeclDim.push_back(name);
			isGlobal = TRUE;
		}
		Node* tempNode = from->u.decl.type;
		declList << arrayType;
		while (tempNode->u.decl.type){
			dim = GetArrayDim(tempNode->u.adcl.dim);
			if (isGlobal)
				nDeclDim.push_back(dim);
			declList << "[" << dim << "]";
			tempNode = tempNode->u.adcl.type;
		}
		declList << ";\n";
		declInitList << declInitList_temp.str() << "={0};\n";
		declInitList_temp.str("");
	}
	else if (initNode->typ == Call){
		declInitList << ";";
	}
	else//如果存在初始化则初始化为指定值
	{
		declInitList << declInitList_temp.str();
		declInitList_temp.str("");
		Node* tempNode = from->u.decl.type;
		declList << arrayType;
		while (tempNode->u.decl.type){
			dim = GetArrayDim(tempNode->u.adcl.dim);
			declList << "[" << dim << "]";
			tempNode = tempNode->u.adcl.type;
		}
		declList << ";\n";
		List *tmp = initNode->u.initializer.exprs;
		int n = ListLength(tmp);
		if (n == 1) // 如果初始化个数1个单一值时，则将数组所有成员初始化为该值
		{
			Node *item = (Node *)FirstItem(tmp);
			if (item->typ == Const)
			{
				declInitList << " = ";
				RecursiveAdclInit(tmp);
				declInitList << ";\n";
			}
		}
		else //初始的个数和数组维数一致，则可以采取这样的赋值形式：val pp:Array[Int](1) = [1,2,3,4,5];
		{
			declInitList << " = ";
			RecursiveAdclInit(tmp);
			declInitList << ";\n";

		}
	}
}
void DynamicX86CodeGenerate::OutputArgList(List *list, int offset)
{
	ListMarker marker;
	Node *item;
	int i = 0;
	IterateList(&marker, list);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		if (i != 0) declInitList << ", ";
		COSX86_Node(item, offset);
		i++;
	}
}

void DynamicX86CodeGenerate::OutputCRSpaceAndTabs(int tabs)
{
	declInitList << "\n";
	while (tabs--) declInitList << "\t";
}

void DynamicX86CodeGenerate::OutputTabs(int tabs)
{
	while (tabs--) declInitList << "\t";
}

void DynamicX86CodeGenerate::OutputStmt(Node *node, int offset)
{
	if (node->typ != Block)
		OutputTabs(offset);
	if (node == NULL)
	{
		declInitList << ";";
		return;
	}
	//遍历语句node
	COSX86_Node(node, offset);
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
	{
				   declInitList << "return";
				   OutputStmt(node->u.Return.expr, offset);
				   break;
	}
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
void DynamicX86CodeGenerate::OutputConstant(Node *c, Bool with_name)
{
	int len = 0;
	const char *tmpString = c->u.Const.text;

	switch (c->u.Const.type->typ)
	{
	case Prim:
		switch (c->u.Const.type->u.prim.basic)
		{
		case Sint:
			declInitList << c->u.Const.value.i;
			break;
		case Uint:
			declInitList << c->u.Const.value.u;
			break;
		case Slong:
			declInitList << c->u.Const.value.l << "L";
			break;
		case Ulong:
			declInitList << c->u.Const.value.ul << "UL";
			break;
		case Float:
			if (c->u.Const.value.d == 0.0) declInitList << "0.0";
			else
				declInitList << c->u.Const.value.f;
			break;
		case Double:
			if (c->u.Const.value.d == 0.0) declInitList << "0.0";
			else if (c->u.Const.value.d - (int)c->u.Const.value.d == 0)
				//declInitList<<c->u.Const.value.d<<".0";
				declInitList << c->u.Const.value.d;
			else
				declInitList << c->u.Const.value.d;
			break;
		case Char:
		case Schar:
		case Uchar:
			OutputChar(c->u.Const.value.i);
			break;

		default:
			Fail(__FILE__, __LINE__, "");
			return;
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
		return;
	}
}
int DynamicX86CodeGenerate::OutputString(const char *s)
{
	int len = 0;

	declInitList << "\"";
	while (*s != 0)
	{
		len += OutputChar(*s++);
	}
	declInitList << "\"";

	return len + 2;
}
void DynamicX86CodeGenerate::OutputStmtList(List *list, int offset)
{
	ListMarker marker;
	Node *item = NULL;

	IterateList(&marker, list);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		OutputStmt(item, offset);
	}
}

//string DynamicX86CodeGenerate::GetArrayDataType(Node *node)
//{
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


void DynamicX86CodeGenerate::ExtractDeclVariables(Node *from)
{
	stringstream tempdeclList, tempdeclInitList;
	if (from->u.decl.type->typ == Prim) // 基本类型
	{
		Node *typeNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string type = GetPrimDataType(typeNode);
		string name = from->u.decl.name;
		char tempvalude[20];
		declList << "\t" << type << " " << name << ";\n";
		if (initNode) // 存在初始化则进行初始化
		{
			declInitList << "\t\t" << name << " = ";
			COSX86_Node(initNode, 0);
			declInitList << ";\n";
		}

		if (isInParam)
		{
			parameterBuf << type << " " << name << ", ";
			thisBuf << "\t\tthis." << name << " = " << name << ";\n";
		}
	}
	else if (from->u.decl.type->typ == Adcl) // 数组, 最多处理二维数组, 高维待扩展
	{
		Node *tmpNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string name = from->u.decl.name;
		string arrayType = GetArrayDataType(tmpNode->u.adcl.type);
		string dim;
		if (initNode == NULL) //如果没有初始化，则按数组类型进行初始化
		{
			string dim = GetArrayDim(tmpNode->u.adcl.dim);
			stringstream sdim;
			sdim << dim;
			int dimNum;
			bool dynamicArray = false;

			if (sdim >> dimNum)		//为数组
				declList << "\t" << arrayType << " " << name << "[" << dim << "]";
			else		//数组维数不确定，使用new动态生成
			{
				dynamicArray = true;
				declList << "\t" << arrayType << "*" << name;
				declInitList << "\t\t" << name << " = new " << arrayType << "[" << dim << "]";
			}
			tmpNode = tmpNode->u.adcl.type;
			while (tmpNode->typ == Adcl)
			{
				dim = GetArrayDim(tmpNode->u.adcl.dim);
				declList << "[" << dim << "]";
				if (dynamicArray)
					declInitList << "[" << dim << "]";
				tmpNode = tmpNode->u.adcl.type;
			}
			declList << ";\n";
			if (dynamicArray)
				declInitList << "();\n";
		}
		else//如果存在初始化则初始化为指定值
		{
			declList << "static " << arrayType << " " << name;
			tempdeclList << arrayType << " " << curactor->name << "::" << name;
			while (tmpNode->typ == Adcl)
			{
				string dim = GetArrayDim(tmpNode->u.adcl.dim);
				declList << "[" << dim << "]";
				tempdeclList << "[" << dim << "]";
				tmpNode = tmpNode->u.adcl.type;
			}
			declList << ";\n";
			tempdeclList << "=";
			List *tmp = initNode->u.initializer.exprs;
			int n = ListLength(tmp);
			if (n == 1) // 如果初始化个数1个单一值时，则将数组所有成员初始化为该值
			{
				Node *item = (Node *)FirstItem(tmp);
				if (item->typ == Const)
				{
					stringstream ss;
					ss << item->u.Const.text;
					if (ss.str() == "NULL")
					{
						declInitList << arrayType << " " << name << "[" << dim << "];\n";
					}
					else
					{
						declInitList << arrayType << " " << name << "[" << dim << "]={" << ss.str() << "};\n";
					}
				}
			}
			else //初始的个数和数组维数一致，则可以采取这样的赋值形式：val pp:Array[Int](1) = [1,2,3,4,5];
			{
				RecursiveAdclInit(tmp);
				tempdeclInitList << tempdeclList.str() << declInitList.str() << ";\n";
				declInitList.str("");


			}
			staticNameInit.push_back(tempdeclInitList.str());
		}

		if (isInParam)
		{
			//parameterBuf << name << " :Array[" << arrayType << "](1);\n";
			parameterBuf << arrayType << " " << name << "[];\n";
			thisBuf << "\t\tthis." << name << " = " << name << ";\n";
		}
	}
	else if (from->u.decl.type->typ == Ptr) // 指针，只能出现在param中
	{
		Node *typeNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string type;
		if (typeNode->u.ptr.type->typ == Prim)
			type = GetPrimDataType(typeNode->u.ptr.type);
		else if (typeNode->u.ptr.type->typ == Tdef)
			type = typeNode->u.ptr.type->u.tdef.name;
		else UNREACHABLE;			//其他类型的暂不支持

		string name = from->u.decl.name;
		char tempvalude[20];
		declList << "\t" << type << " " << "*" << name << ";\n";
		if (initNode) // 存在初始化则进行初始化
		{
			declInitList << "\t\t" << name << " = ";
			COSX86_Node(initNode, 0);
			declInitList << ";\n";
		}

		if (isInParam)
		{
			parameterBuf << type << " " << "*" << name << ", ";
			thisBuf << "\t\tthis." << name << " = " << name << ";\n";
		}
	}
	else
		UNREACHABLE;
}

string DynamicX86CodeGenerate::GetOpType(OpType op)
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
	case ADDRESS:return " & ";
	default:
		FAIL("Unrecognized node type in CodeGeneration!"); break;
	}
}
//多维数组初始化过程（递归）
void DynamicX86CodeGenerate::RecursiveAdclInit(List *init)
{
	//多维数组则init是一个多维的链表
	ListMarker marker;
	Node *item;
	int i = 1;
	int len = ListLength(init);
	IterateList(&marker, init);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		if (i == 1) declInitList << "{";
		if (item->typ == Unary)
		{
			if (i != 1) declInitList << ",";
			declInitList << GetOpType(item->u.unary.op);
			COSX86_Node(item->u.unary.expr, 0);
		}
		else if (item->typ == Const) // 如果数组成员是基本类型
		{
			if (i != 1) declInitList << ",";
			COSX86_Node(item, 0);
		}
		else if (item->typ == Initializer)//如果数组成员是一个数组则递归
		{
			RecursiveAdclInit(item->u.initializer.exprs);
			if (i != len)
			{
				declInitList << ",";
				OutputCRSpaceAndTabs(4);
			}
		}
		else if (item->typ == ImplicitCast)//基本类型的隐式转换
		{
			if (i != 1) declInitList << ",";
			COSX86_Node(item->u.implicitcast.value, 0);
		}
		i++;
	}
	declInitList << "}";
}


//取数组的维数
string DynamicX86CodeGenerate::GetArrayDim(Node *from)
{
	string dim;
	if (from->typ == Const)//如果维数节点类型为常量，例如a[10]
	{
		if (from->u.Const.text)dim = from->u.Const.text;
		else
		{
			char *tmpdim = (char *)malloc(20);
			sprintf(tmpdim, "%d", from->u.Const.value.l);//20120322 zww添加
			dim = tmpdim;
		}
	}
	else if (from->typ == Id)
	{
		dim = from->u.id.text;
	}
	else if (from->typ == ImplicitCast && from->u.implicitcast.value->typ == Const)
	{
		if (from->u.implicitcast.value->u.Const.text)dim = from->u.implicitcast.value->u.Const.text;
		else
		{
			char *tmpdim = (char *)malloc(20);
			sprintf(tmpdim, "%d", from->u.implicitcast.value->u.Const.value.l);
			dim = tmpdim;
		}
	}
	else if (from->typ == Binop)
	{
		string tmp = declInitList.str(); // 保存
		stringstream tmp2;

		declInitList.str("");
		COSX86_Node(from, 0);
		tmp2 << "(" << declInitList.str() << ")";
		dim = tmp2.str();
		declInitList.str("");
		declInitList << tmp; // 恢复 
	}
	else
		UNREACHABLE;

	return dim;
}













//\----------------------------------------------------------------------

void DynamicX86CodeGenerate::COSX86_Node(Node *node, int offset)
{
	if (node == NULL) return;

	if (node->parenthesized == TRUE) declInitList << "("; //加括号保证逻辑性

#define CODE(name, node, union) COSX86_##name(node, union, offset)
	ASTSWITCH(node, CODE)
#undef CODE

	if (node->parenthesized == TRUE) declInitList << ")"; //加括号保证逻辑性
}

void DynamicX86CodeGenerate::COSX86_List(List *list, int offset)
{
	ListMarker marker;
	Node *item = NULL;

	IterateList(&marker, list);
	while (NextOnList(&marker, (GenericREF)&item))
	{
		string name = item->u.decl.name;
		if (strcmp(name.c_str(), "path") == 0)
		{
			if (isInFileReader)
				infileName = item->u.decl.init->u.Const.text;
			if (isInFileWriter)
				outfileName = item->u.decl.init->u.Const.text;
			return;
		}
		COSX86_Node(item, offset);
	}
}
void DynamicX86CodeGenerate::COSX86_Id(Node *node, idNode *u, int offset)
{
	declInitList << u->text;
}
void DynamicX86CodeGenerate::COSX86_Binop(Node *node, binopNode *u, int offset)//对非结构体类型的Stream进行简化,lihe,2012-09-04
{
	//OutputTabs(offset);
	if (node->u.binop.left)
		COSX86_Node(node->u.binop.left, offset);
	string op = GetOpType(node->u.binop.op);
	declInitList << GetOpType(node->u.binop.op);
	if (node->u.binop.right)
		COSX86_Node(node->u.binop.right, offset);

	//declInitList << ";\n";
}
void DynamicX86CodeGenerate::COSX86_Unary(Node *node, unaryNode *u, int offset)
{
	if (node->u.unary.op != POSTINC && node->u.unary.op != POSTDEC)
		declInitList << GetOpType(node->u.binop.op);
	if (node->u.unary.expr)
		COSX86_Node(node->u.unary.expr, offset);
	if (node->u.unary.op == POSTINC || node->u.unary.op == POSTDEC)
		declInitList << GetOpType(node->u.binop.op);
}
void DynamicX86CodeGenerate::COSX86_Cast(Node *node, castNode *u, int offset)
{
	if (node->u.cast.type->typ == Ptr&&node->u.cast.expr->typ == Const){//添加对NULL的判断
		if (node->u.cast.expr->u.Const.value.i == 0)
			declInitList << "NULL";
	}
	else{
		declInitList << "(";
		declInitList << GetPrimDataType(node->u.cast.type) << ")";
		COSX86_Node(node->u.cast.expr, offset);
	}
}
void DynamicX86CodeGenerate::COSX86_STRdcl(Node *node, strdclNode *u, int offset)
{
}
void DynamicX86CodeGenerate::COSX86_Const(Node *node, ConstNode *u, int offset)
{
	OutputConstant(node, TRUE);
}
void DynamicX86CodeGenerate::COSX86_Comma(Node *node, commaNode *u, int offset)
{
	isInComma = true;//正处于逗号表达式中
	COSX86_List(u->exprs, offset);
	isInComma = false;
}

void DynamicX86CodeGenerate::COSX86_Ternary(Node *node, ternaryNode *u, int offset)
{
	COSX86_Node(u->cond, offset);
	declInitList << " ? ";
	COSX86_Node(u->true_, offset);
	declInitList << ":";
	COSX86_Node(u->false_, offset);
}

void DynamicX86CodeGenerate::COSX86_Array(Node *node, arrayNode *u, int offset)
{
	COSX86_Node(u->name, offset);
	List *tmp = u->dims;
	while (tmp != NULL)//可能是多维数组，需要遍历dim这个list
	{
		declInitList << "[";
		Node *item = (Node *)FirstItem(tmp);
		COSX86_Node(item, offset);
		declInitList << "]";
		tmp = Rest(tmp);
	}
}

void DynamicX86CodeGenerate::COSX86_Call(Node *node, callNode *u, int offset)
{
	assert(u->name->typ == Id);
	{
		int flag = 1;//标识是否加括号
		const char *ident = u->name->u.id.text;
		if (strcmp(ident, "acos") == 0) declInitList << "acos";
		else if (strcmp(ident, "acosh") == 0) declInitList << "acosh";
		else if (strcmp(ident, "acosh") == 0) declInitList << "acosh";
		else if (strcmp(ident, "asin") == 0) declInitList << "asin";
		else if (strcmp(ident, "asinh") == 0) declInitList << "asinh";
		else if (strcmp(ident, "atan") == 0) declInitList << "atan";
		else if (strcmp(ident, "atan2") == 0) declInitList << "atan2";
		else if (strcmp(ident, "atanh") == 0) declInitList << "atanh";
		else if (strcmp(ident, "ceil") == 0) declInitList << "ceil";
		else if (strcmp(ident, "cos") == 0) declInitList << "cos";
		else if (strcmp(ident, "cosh") == 0) declInitList << "cosh";
		else if (strcmp(ident, "exp") == 0) declInitList << "exp";
		else if (strcmp(ident, "expm1") == 0) declInitList << "expm1";
		else if (strcmp(ident, "floor") == 0) declInitList << "floor";
		else if (strcmp(ident, "fmod") == 0) declInitList << "fmod";
		else if (strcmp(ident, "frexp") == 0) declInitList << "frexp";
		else if (strcmp(ident, "log") == 0) declInitList << "log";
		else if (strcmp(ident, "log10") == 0) declInitList << "log10";
		else if (strcmp(ident, "log1p") == 0) declInitList << "log1p";
		else if (strcmp(ident, "modf") == 0) declInitList << "modf";
		else if (strcmp(ident, "pow") == 0) declInitList << "pow";
		else if (strcmp(ident, "sin") == 0) declInitList << "sin";
		else if (strcmp(ident, "sinh") == 0) declInitList << "sinh";
		else if (strcmp(ident, "sqrt") == 0) declInitList << "sqrt";
		else if (strcmp(ident, "tan") == 0) declInitList << "tan";
		else if (strcmp(ident, "tanh") == 0) declInitList << "tanh";
		// not from profiling: round(x) is currently macro for floor((x)+0.5)
		else if (strcmp(ident, "round") == 0) declInitList << "round";
		// not from profiling: just stuck in here to keep compilation of gmti 
		// from spewing warnings.
		else if (strcmp(ident, "abs") == 0) declInitList << "abs";
		else if (strcmp(ident, "max") == 0) declInitList << "max";
		else if (strcmp(ident, "min") == 0) declInitList << "min";
		else if (strcmp(ident, "frta") == 0) declInitList << "frta";
		else if (strcmp(ident, "println") == 0) { declInitList << "cout<<"; flag = 2; }
		else if (strcmp(ident, "printf") == 0) { declInitList << "cout<<"; flag = 3; }
		else if (strcmp(ident, "print") == 0) { declInitList << "cout<<"; flag = 4; }
		else //unkonwn methods
		{
			declInitList << u->name->u.id.text;//输出函数名
		}
		if (flag == 1)
		{
			declInitList << "(";
			OutputArgList(u->args, offset);//参数
			declInitList << ")";
			if (strcmp(ident, "frta") == 0)
				declInitList << ";";
		}
		else if (flag == 2)
		{
			OutputArgList(u->args, offset);//参数
			declInitList << "<<endl";
		}
		else
		{
			OutputArgList(u->args, offset);//参数
		}
	}
}

void DynamicX86CodeGenerate::COSX86_Initializer(Node *node, initializerNode *u, int offset)
{

}

void DynamicX86CodeGenerate::COSX86_ImplicitCast(Node *node, implicitcastNode *u, int offset)
{
	if (isEnumVar == true)
	{
		/*删除Enum类型未初始化时的“=”*/
		string orignal = globalvarbuf.str();
		//清空declInitList
		globalvarbuf.str("");
		orignal = orignal.substr(0, orignal.size() - 3);

		globalvarbuf << orignal;
	}

	COSX86_Node(u->expr, offset);
}

void DynamicX86CodeGenerate::COSX86_Label(Node *node, labelNode *u, int offset)
{

}

void DynamicX86CodeGenerate::COSX86_Switch(Node *node, SwitchNode *u, int offset)
{
	declInitList << "switch (";
	COSX86_Node(u->expr, offset);
	declInitList << ")";
	COSX86_Node(u->stmt, offset);
}

void DynamicX86CodeGenerate::COSX86_Case(Node *node, CaseNode *u, int offset)
{
	declInitList << "case ";
	COSX86_Node(u->expr, offset);
	declInitList << ":";
	OutputCRSpaceAndTabs(offset + 1);
	COSX86_Node(u->stmt, offset);
}

void DynamicX86CodeGenerate::COSX86_Default(Node *node, DefaultNode *u, int offset)
{
	declInitList << "default: ";
	OutputCRSpaceAndTabs(offset);
	COSX86_Node(u->stmt, offset);
}

void DynamicX86CodeGenerate::COSX86_If(Node *node, IfNode *u, int offset)
{
	declInitList << "if (";
	COSX86_Node(u->expr, offset);
	declInitList << ")";
	if (u->stmt->typ != Block) // 如果是非block结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset + 1);
	}
	COSX86_Node(u->stmt, offset + 1);
	if (u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList << ";";
}

void DynamicX86CodeGenerate::COSX86_IfElse(Node *node, IfElseNode *u, int offset)
{
	declInitList << "if (";
	COSX86_Node(u->expr, offset);
	declInitList << ")";
	if (u->true_->typ != Block)//如果是非block结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset + 1);
	}
	COSX86_Node(u->true_, offset);
	if (u->true_->typ == Binop || u->true_->typ == Unary || u->true_->typ == Ternary || u->true_->typ == Call || u->true_->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList << ";";
	OutputCRSpaceAndTabs(offset);
	declInitList << "else ";
	if (u->false_->typ != Block && u->false_->typ != IfElse)//如果是非block结点或者ifelse结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset + 1);
	}
	COSX86_Node(u->false_, offset);
	if (u->false_->typ == Binop || u->false_->typ == Unary || u->false_->typ == Ternary || u->false_->typ == Call || u->false_->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList << ";";
}

void DynamicX86CodeGenerate::COSX86_While(Node *node, WhileNode *u, int offset)
{
	declInitList << "while (";
	COSX86_Node(u->expr, offset);
	declInitList << ")";
	if (u->stmt->typ != Block)//如果是非block结点，则需要换行对齐
	{
		OutputCRSpaceAndTabs(offset + 1);
	}
	COSX86_Node(u->stmt, offset);
	if (u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//如果是表达式结点则需要在末位添加分号表示结束
		declInitList << ";";
}

void DynamicX86CodeGenerate::COSX86_Do(Node *node, DoNode *u, int offset)
{
	declInitList << "do";
	COSX86_Node(u->stmt, offset);
	declInitList << "while (";
	COSX86_Node(u->expr, offset);
	declInitList << ");";
}

//该函数处理源COStream程序中涉及到for循环的部分，转换为C++文件中的for循环
void DynamicX86CodeGenerate::COSX86_For(Node *node, ForNode *u, int offset)
{
	declInitList << "for (";
	COSX86_Node(u->init, offset);
	declInitList << " ; ";
	COSX86_Node(u->cond, offset);
	declInitList << " ; ";
	COSX86_Node(u->next, offset);
	declInitList << ")";
	if (u->stmt->typ != Block)//如果是非block结点，则需要换行对齐
	{
		declInitList << "\n";
	}
	//declInitList<<"\t{\n ";
	OutputStmt(u->stmt, offset + 1);
	//declInitList<<" \n\t}\n ";
}

void DynamicX86CodeGenerate::COSX86_Goto(Node *node, GotoNode *u, int offset)
{
	declInitList << "goto ";
}

void DynamicX86CodeGenerate::COSX86_Continue(Node *node, ContinueNode *u, int offset)
{
	declInitList << "continue;";
}

void DynamicX86CodeGenerate::COSX86_Break(Node *node, BreakNode *u, int offset)
{
	declInitList << "break;";
}

void DynamicX86CodeGenerate::COSX86_Return(Node *node, ReturnNode *u, int offset)
{
	cout << "return 函数被调用*********************************************" << endl;
}

void DynamicX86CodeGenerate::COSX86_Block(Node *node, BlockNode *u, int offset)
{
	declInitList << "\t\t{\n";
	//输出decl
	COSX86_List(u->decl, offset);
	declInitList << "\n"; // 另起一行

	OutputStmtList(u->stmts, offset);

	//OutputCRSpaceAndTabs(offset);
	declInitList << "\t\t}\n"; // '}'独占一行
}

void DynamicX86CodeGenerate::COSX86_Prim(Node *node, primNode *u, int offset)
{
	if (!isstructVar && !isUnionVar && !isEnumVar)
		declInitList << GetPrimDataType(node);
	if (!isstructVar && !isUnionVar && !isEnumVar)//使结构体变量不输出extern
		globalvarbuf << "extern ";
	if (isEnumVar == false)
		globalvarbuf << GetPrimDataType(node) << " ";
}

void DynamicX86CodeGenerate::COSX86_Tdef(Node *node, tdefNode *u, int offset)
{
	cout << "typedef 函数被调用*********************************************" << endl;

}

void DynamicX86CodeGenerate::COSX86_Ptr(Node *node, ptrNode *u, int offset)
{
	declInitList << GetArrayDataType(node->u.adcl.type) << "* ";
	globalvarbuf << GetArrayDataType(node->u.adcl.type) << "* ";
}


void DynamicX86CodeGenerate::COSX86_Adcl(Node *node, adclNode *u, int offset)
{
	string arrayType = GetArrayDataType(node->u.adcl.type);
	if (!isstructVar && !isUnionVar && !isEnumVar)
		declInitList << arrayType << " ";
	if (!isstructVar && !isUnionVar && !isEnumVar)//使结构体变量不输出extern
		globalvarbuf << "extern ";
	globalvarbuf << arrayType << " ";
}

//2015-05-08添加函数声明代码生成模块实现
void DynamicX86CodeGenerate::COSX86_Fdcl(Node *node, fdclNode *u, int offset)
{
	//（1）更新函数声明个数记录
	n_fdcl++;
	//（2）获取函数声明返回值类型
	string funType = GetArrayDataType(node->u.fdcl.returns);
	//（3）获取函数声明参数列表
	ListMarker argList;
	IterateList(&argList, node->u.fdcl.args);
	Node *item = NULL;
	//（4）将类型与参数信息写入globalvarbuf流
	globalvarbuf << "extern " << funType << " " << "(";
	while (NextOnList(&argList, (GenericREF)&item))
	{
		string argType = "";
		if (node->u.decl.type->typ == Prim)
		{
			argType = GetPrimDataType(node->u.decl.type);
		}
		else{
			argType = GetArrayDataType(node->u.decl.type);
		}
		globalvarbuf << argType << " " << item->u.decl.name << " , ";
	}
	globalvarbuf << ");\n";
}

void DynamicX86CodeGenerate::COSX86_Sdcl(Node *node, sdclNode *u, int offset)
{
	string sdclType = u->type->name;
	if (isCGGlobalVar)
	{
		globalvarbuf << "struct ";
		globalvarbuf << sdclType << "{\n";

		isstructVar = true;//控制变量的输出

		COSX86_List(u->type->fields, 0);

		globalvarbuf << "};";
		//declInitList<<sdclType<<" ";//输出到函数中
		isstructVar = false;

	}
	else
		declInitList << sdclType;
}

void DynamicX86CodeGenerate::COSX86_Udcl(Node *node, udclNode *u, int offset)
{
	string udclType = u->type->name;
	if (isCGGlobalVar)
	{
		globalvarbuf << "union ";
		globalvarbuf << udclType << "{\n";
		isUnionVar = true;//控制变量的输出
		COSX86_List(u->type->fields, 0);
		isUnionVar = false;
		globalvarbuf << "};";
	}
	else
		declInitList << udclType << " ";//输出到函数中
}

void DynamicX86CodeGenerate::COSX86_Edcl(Node *node, edclNode *u, int offset)
{

	string edclType = u->type->name;
	if (isCGGlobalVar)
	{
		globalvarbuf << "enum ";
		globalvarbuf << edclType << "{\n";
		isEnumVar = true;//控制变量的输出
		COSX86_List(u->type->fields, 0);
		isEnumVar = false;

		/*删除最后一个多余的分号*/
		string orignal = globalvarbuf.str();
		//清空declInitList
		globalvarbuf.str("");
		orignal = orignal.substr(0, orignal.size() - 2);
		globalvarbuf << orignal << "\n};\n";
	}
	else
		declInitList << edclType << " ";//输出到函数中
}

void DynamicX86CodeGenerate::COSX86_Decl(Node *node, declNode *u, int offset)
{
	if (extractDecl)
	{
		ExtractDeclVariables(node);
		return;
	}
	else
	{
		if (strcmp(node->u.decl.name, "SPLExternType") == 0 && node->u.decl.type->typ == Sdcl)
		{
			isExternType = TRUE;
			needExternType = TRUE;
			ExternTypeBuf << "typedef struct \n";
			ExternTypeBuf << "{ \n";
			COSX86_Node(u->type, offset);
			ExternTypeBuf << "}" << node->u.decl.name << ";\n";
			isExternType = FALSE;
		}
		else {
			OutputCRSpaceAndTabs(offset);
			if (isCGGlobalVar && STORAGE_CLASS(node->u.decl.tq) == T_EXTERN)
			{
				declInitList << "extern ";
				globalvarbuf << "extern ";
			}
			COSX86_Node(u->type, offset);
			if (u->type->typ == Adcl)	//多维数组
			{
				Node *tempNode = u->type;
				globalvarbuf << node->u.decl.name;
				if (flag_Global)			//输出在GlobalVar中，为整个流程序的全局变量
					declInitList << node->u.decl.name;
				else
					declInitList_temp << node->u.decl.name;
				while (tempNode->typ == Adcl)
				{
					if (STORAGE_CLASS(node->u.decl.tq) == T_EXTERN) {
						if (flag_Global)
							declInitList << "[" << "];";
						else
							declInitList_temp << "[" << "];";
						globalvarbuf << "[" << "]";
					}
					else {
						string dim = GetArrayDim(tempNode->u.adcl.dim);
						if (flag_Global)
							declInitList << "[" << dim << "]";
						else
							declInitList_temp << "[" << dim << "]";
						globalvarbuf << "[" << dim << "]";
					}
					tempNode = tempNode->u.adcl.type;
				}
				temp_declInitList << declInitList.str() << ";\n";
				globalvarbuf << ";\n";
			}
			else		//标量
			{
				if (isExternType)
					ExternTypeBuf << node->u.decl.name << ";\n";//声明变量
				else {
					declInitList << " " << node->u.decl.name;   //变量名
					temp_declInitList << declInitList.str() << ";\n";
					globalvarbuf << node->u.decl.name << ";\n";//声明变量
				}
			}

			if (u->type->typ == Adcl)
			{
				if (STORAGE_CLASS(node->u.decl.tq) != T_EXTERN)
					AdclInit(node, offset);		//初始化数组
			}
			else
			{
				if (node->u.decl.init)
				{
					if (u->type->typ == Prim && u->type->u.prim.basic == Char)//如果是个字符声明
					{
						declInitList << " = " << u->init->u.implicitcast.expr->u.Const.text;
					}
					else
					{
						declInitList << " = ";			//初始化变量 如int a = 2;
						COSX86_Node(u->init, offset);
					}
				}
			}
			if (u->type->typ != Adcl)
				declInitList << ";";
		}
	}

	///*添加isstructVar部分用于控制Sturct变量在除GlobalVar.h的其他文件中生成*/
	//if (extractDecl)
	//{
	//	ExtractDeclVariables(node);
	//	return;
	//}
	//else
	//{
	//	if (!isstructVar && !isUnionVar &&!isEnumVar&&!isCGGlobalVar)
	//		OutputCRSpaceAndTabs(offset);
	//	if (isCGGlobalVar && STORAGE_CLASS(node->u.decl.tq) == T_EXTERN)
	//	{
	//		declInitList << "extern ";
	//		globalvarbuf << "extern ";
	//	}
	//	COSX86_Node(u->type, offset);
	//	if (u->type->typ == Adcl)	//多维数组
	//	{
	//		Node *tempNode = u->type;
	//		globalvarbuf << node->u.decl.name;
	//		if (!isstructVar && !isUnionVar && !isEnumVar)
	//		{
	//			if (flag_Global)			//输出在GlobalVar中，为整个流程序的全局变量
	//				declInitList << node->u.decl.name;
	//			else
	//				declInitList_temp << node->u.decl.name;
	//		}
	//		while (tempNode->typ == Adcl)
	//		{
	//			if (STORAGE_CLASS(node->u.decl.tq) == T_EXTERN){
	//				if (!isstructVar && !isUnionVar &&!isEnumVar)
	//				{

	//					if (flag_Global)
	//						declInitList << "[" << "];";
	//					else
	//						declInitList_temp << "[" << "];";
	//				}
	//				globalvarbuf << "[" << "]";
	//			}
	//			else{
	//				string dim = GetArrayDim(tempNode->u.adcl.dim);
	//				if (!isstructVar && !isUnionVar && !isEnumVar)
	//				{

	//					if (flag_Global)
	//						declInitList << "[" << dim << "]";
	//					else
	//						declInitList_temp << "[" << dim << "]";
	//				}
	//				globalvarbuf << "[" << dim << "]";
	//			}
	//			tempNode = tempNode->u.adcl.type;
	//		}
	//		temp_declInitList << declInitList.str() << ";\n";
	//		globalvarbuf << ";\n";
	//	}
	//	else if (u->type->typ == Fdcl){ //2015-05-08 添加 
	//		int count = 0;
	//		//获取全局函数名,将函数名插入到"("前面
	//		string temp = globalvarbuf.str();
	//		int i, begin, end;
	//		for (i = 0; i < temp.size(); i++)
	//		{
	//			if (temp[i] == '(')
	//			{
	//				begin = i - 1;
	//			}
	//			if (temp[i] == ')')
	//			{
	//				end = i - 2;
	//				count++;
	//			}
	//			//判断是否处理的当前函数声明
	//			if (count == n_fdcl)
	//				break;
	//			else
	//				continue;
	//		}
	//		//最终的声明信息
	//		string declInfo(temp);
	//		//获得函数声明的标准形式在（之前插入函数名，在）之前删除多余的，号
	//		declInfo = temp.substr(0, begin + 1) + node->u.decl.name + temp.substr(begin + 1, end - begin - 1) + temp.substr(end + 1, temp.size() - end);
	//		globalvarbuf.str("");
	//		globalvarbuf << declInfo;
	//	}

	//	else if (node->u.decl.type->typ == Sdcl || node->u.decl.type->typ == Udcl || node->u.decl.type->typ == Edcl&&isCGGlobalVar)//处理SUE类型的节点
	//	{
	//		if (isCGGlobalVar)
	//		{/*删除多余的一个分号，类型命名*/
	//			string orignal = globalvarbuf.str();
	//			//清空declInitList
	//			globalvarbuf.str("");
	//			orignal = orignal.substr(0, orignal.size() - 1);
	//			globalvarbuf << orignal;
	//			globalvarbuf << node->u.decl.name << ";\n";
	//		}
	//		else
	//		{
	//			declInitList << " " << node->u.decl.name << ";";   //变量名

	//			temp_declInitList << declInitList.str() << ";\n";
	//		}
	//	}
	//	else{
	//		if (!isstructVar && !isUnionVar &&!isEnumVar)//防止SUE类型的变量在Globalvar.cpp文件中
	//			declInitList << " " << node->u.decl.name;   //变量名

	//		temp_declInitList << declInitList.str() << ";\n";
	//		globalvarbuf << node->u.decl.name;
	//		if (isEnumVar == false)
	//			globalvarbuf << ";\n";//声明变量				
	//	}//if
	//}

	///*初始化部分*/
	//if (u->type->typ == Adcl)
	//{
	//	if (STORAGE_CLASS(node->u.decl.tq) != T_EXTERN)
	//	if (!isstructVar && !isUnionVar)
	//		AdclInit(node, offset);		//初始化数组
	//}
	//else
	//{
	//	if (node->u.decl.init)
	//	{
	//		if (u->type->typ == Prim && u->type->u.prim.basic == Char)//如果是个字符声明
	//		{
	//			globalvarbuf << " = " << u->init->u.implicitcast.expr->u.Const.text;
	//		}
	//		else
	//		{
	//			if (isEnumVar == true)
	//				globalvarbuf << " = ";			//初始化变量 如int a = 2;

	//			else
	//				declInitList << " = ";
	//			COSX86_Node(u->init, offset);

	//		}
	//	}

	//}
	//if (u->type->typ != Adcl && u->type->typ != Fdcl && !isstructVar && !isUnionVar&&!isEnumVar&&node->u.decl.type->typ != Sdcl&&node->u.decl.type->typ != Udcl&&node->u.decl.type->typ != Edcl)
	//{
	//	declInitList << ";\n";
	//}
	//if (isEnumVar == true)   //带有初始化时，需先输出初始化值再输出符号
	//	globalvarbuf << ",\n";
}
void DynamicX86CodeGenerate::COSX86_Attrib(Node *node, attribNode *u, int offset)
{
}

//2015-05-08添加函数定义代码生成模块实现      
void DynamicX86CodeGenerate::COSX86_Proc(Node *node, procNode *u, int offset)
{
	//（1）若程序运行到此处，更改标志flag值为true，并为declInitLen赋值
	if (!flag)
	{
		flag = 1;
		declInitLen = declInitList.str().size();
	}
	//(2)获取函数名
	string funName = u->decl->u.decl.name;
	//(3)获取函数返回类型
	string returnType = "";
	if (u->decl->u.fdcl.returns->typ == Prim)
	{
		returnType = GetPrimDataType(u->decl->u.fdcl.returns->u.decl.type);
	}
	else{
		returnType = GetArrayDataType(u->decl->u.fdcl.returns->u.decl.type);
	}
	//(4)写入字符流
	declInitList << returnType << " " << funName << "(";
	ListMarker argList;
	IterateList(&argList, node->u.proc.decl->u.decl.type->u.fdcl.args);
	Node *item = NULL;
	while (NextOnList(&argList, (GenericREF)&item))
	{
		string argType = "";
		if (item->u.decl.type->typ == Prim)
		{
			argType = GetPrimDataType(item->u.decl.type);
		}
		else{
			argType = GetArrayDataType(item->u.decl.type);
		}
		string argName = item->u.decl.name;
		declInitList << argType << " " << argName << " ,";
	}
	//(5)删除最后一个多余的逗号
	string orignal = declInitList.str();
	declInitList.str("");
	orignal = orignal.substr(0, orignal.size() - 1);
	declInitList << orignal << ")";
	temp_declInitList << declInitList.str() << "\n";
	//(6)处理并输出函数体代码生成
	declInitList << "\t{ \n";
	if (u->body->typ != Block)//如果是非block结点，则需要换行对齐
	{
		declInitList << "\n\t";
	}
	OutputStmt(u->body, offset + 1);
	declInitList << "\n\t} \n";
}

void DynamicX86CodeGenerate::COSX86_Text(Node *node, textNode *u, int offset)
{
}