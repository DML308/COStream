#include "X86DynamicCodeGeneration.h"
static bool isInComma = false;//��ʾ��ǰ���ڶ��ű��ʽ��
static bool isEnumVar = false;
static bool isstructVar = false;//��ʾ�ñ���Ϊ�ṹ���ڱ���
static bool isUnionVar = false;
static bool isCGGlobalVar = false;//��ʾĿǰ����������ȫ�ֱ���
static int flag_Global = 0;//��ʶ�Ƿ������GlobalVarCpp�ļ�����ʼ��Ϊ0
static bool Filerw = 0;//��ʶ�Ƿ����ļ���дactor
static bool FileR = 0; // ��ʶ�Ƿ����ļ��� actor
static bool FileW = 0; // ��ʶ�Ƿ����ļ�д actor
static bool isInFileReader = false;//��ʾ��ǰ����FileReader��
static bool isInFileWriter = false;//��ʾ��ǰ����FileWriter��
static int batch = 100;		//batching
static bool isExternType = false; //��ʾĿǰ����������Ƕ��ʽ�Ľṹ�ļ�ExternType��
static bool needExternType = false; //��ʾʱ����Ҫ���ɽӿ��ļ�
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
//	pEdgeInfo = new ActorEdgeInfo(Sssg);//���edgeinfoһ���ٸ�
	nCpucore_ = nCpucore;
	nActors_ = flatNodes_.size();
	readerActor = writerActor = NULL;
	dir_ = currentDir;
	pEdgeInfo = new ActorEdgeInfo(dsg);
	//������������Ŀ���ļ���Ŀ¼

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
	
	//����mapActor2InEdge��mapActor2OutEdge�Լ�readActor��writeActor
	vector<FlatNode*>::iterator iter1, iter2, iter3;		//iter1��ʾ��ǰ��㣬iter2���ڱ���������ڵ㣬iter3���ڱ���������ڵ�
	for (iter1 = flatNodes_.begin(); iter1 != flatNodes_.end(); iter1++)
	{
		//���ýڵ��������Ϣ

		
		for (iter2 = (*iter1)->inFlatNodes.begin(); iter2 != (*iter1)->inFlatNodes.end(); ++iter2)
		{
			string tmpString = (*iter2)->name + "_" + (*iter1)->name;		//�õ�����A_B�ı�
			mapActor2InEdge.insert(make_pair((*iter1), tmpString));
		}
		for (iter3 = (*iter1)->outFlatNodes.begin(); iter3 != (*iter1)->outFlatNodes.end(); ++iter3)
		{
			string tmpString = (*iter1)->name + "_" + (*iter3)->name; 
			mapActor2OutEdge.insert(make_pair((*iter1), tmpString));
		}

		if ((*iter1)->name.find("FileReader") != -1)		//��ǰ��������г���FileReader
		{
			readerActor = (*iter1);
			FileR = 1;					//�����ļ�������
		}
		if ((*iter1)->name.find("FileWriter") != -1)		//��ǰ��������г���FileWriter
		{
			writerActor = (*iter1);
			FileW = 1;				//�����ļ�д
		}
	}
	//����ÿ���߳��ϵ�stage�����б�
	//����ͼ��Ŀ����
	vector<FlatNode*>tempactors;
	vector<FlatNode*>::iterator iter;
	for (int i = 0; i < dsg->getCount(); i++)		//��ͼѭ��
	{
		//ÿһ����ͼ����һ��map��Ȼ�����vector
		map<int, set<int> > tmp;	
		for (int j = 0; j < nCpucore; j++)			//��ͼ�ں�ѭ��,һ�������½׶�
		{
			set<int>tempstageset;		//��ʱset
			tempstageset.clear();
			tempactors = mplist[i]->findNodeSetInPartition(j);//�ҵ���Ӧ���ϵ��������
			for (iter = tempactors.begin(); iter != tempactors.end(); iter++)
			{
				tempstageset.insert(psalist[i]->FindStage(*iter));
			}
			tmp.insert(make_pair(j, tempstageset));
		}
		mapNum2StageList.push_back(tmp);//һ��ÿ����ͼ��ÿ���̶߳�Ӧ��stage�б�
	}
	//������������֪��ʲô����
	extractDecl = false;
	isInParam = false;

	threadNum = 0;
	vector<CombineState>::iterator iterThreadNum = dsg_->combineList.begin();
	while (iterThreadNum != dsg_->combineList.end())
	{
		if ((*iterThreadNum) == FULLCORE || (*iterThreadNum) == NOTMERGE)	//ʹ��8���˻��ߺϲ�Ч�ʵ�
			threadNum += nCpucore_;
		else if ((*iterThreadNum) == MERGEONE || (*iterThreadNum) == MERGEDOWNONE || (*iterThreadNum) == MERGEUPONE)
		{
			threadNum += nCpucore_;
			iterThreadNum++;//������һ��
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
		buf << "#include \"lock_free_barrier.h\"	//����barrier����\n";
		buf << "#include \"global.h\"\n";
		if (gfrtaCallList != NULL){
			buf << "#include \"frta.h\"\n";
			buf << "#include \"GlobalVar.h\"\n";
		}
		buf << "using namespace std;\n";
	}
	buf << Tab << "int MAX_ITER=1;//Ĭ�ϵ�ִ�д�����1\n";

	//�߳���������Ҫ�������¼����֣�1������̬��ͼ���߳�������2.ѹ��/�ϲ���ͼ���߳�����
	//���ϲ�����ͼÿ����Ҫ8���̣߳��ϲ�����ͼ����ֻ��Ҫ8���߳�

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

		//�����߳���Ŀ�㹻�࣬����setCpu��Ҫ�޸�
		buf << i%nCpucore_;
		buf<<");\n\t" << Tab << "thread_" << i/nCpucore_<<"_"<<i%nCpucore_<< "_fun();\n\t" << Tab << "return 0;\n" << Tab << "}\n";
	}
	if (CallModelEmbed){}
	else				//����ģʽ������main
	{
		buf << "int main(int argc,char **argv)\n{\n";
		buf << "\tvoid setRunIterCount(int,char**);\n";
		if (gfrtaCallList != NULL){
			COSX86_List(gfrtaCallList, 0);
			buf << "\t";
			buf << declInitList.str();
			buf << "\n";
			declInitList.str(""); // ���declInitList����
			CGFrta();
		}
		buf << "\tsetRunIterCount(argc,argv);\n";
		//buf << "\tset_cpu(0);\n";
		//buf << "\tallocBarrier(" << nCpucore_ << ");\n";
		//�߳�����ǰ���ȫ�ֱ�����ʼ��
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

		//���Ĵ���
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
					//���ö�Ӧi��ͼ����
					buf << "pthread_mutex_init(&mutex" << i << ",NULL);\n";
				}
			}
		}


		buf << "\tpthread_t tid[" << threadNum << "];\n";
		for (int i = 0; i < threadNum; i++)
		{
			buf << "\tpthread_create (&tid[" << i << "], NULL, thread_" << i/nCpucore_<<"_"<<i%nCpucore_ << "_fun_start, (void*)NULL);\n";
		}
		//�˴���Ҫ�޸�
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

		//mainҪ�ȴ�����ͼ���߳̽���
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








//public ���ɻ��������������������Ҫ�޸�,��Ҫ�Ѷ�̬�����������ֱ�Ӷ������
void DynamicX86CodeGenerate::DYGlobalHeader()
{
	stringstream buf;
	string edgename;
	ListMarker marker;
	Node *node;
	vector<FlatNode *>::iterator iter_1, iter_2;
	buf << "#ifndef _GLOBAL_H\n";
	buf << "#define _GLOBAL_H\n";
	buf << "/*ȫ�ֱ��������ڴ洢�ߵ���Ϣ*/\n";
	buf << "/*�ߵ���������A_B,����A->B*/\n\n";
	buf << "/*ѹ�����Ӷ�̬������2��*/\n";
	buf << "/*��̬��ȫ�������ﶨ��*/\n";

	if (FileR || FileW)		//�����ļ���д����
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
	if (Win)//WinĬ����false
	{
		buf << "#define MAX_ITER 10\n";
	}
	if (CallModelEmbed)
		buf << "namespace COStream{\n";//���ֿռ�
	//���stream�ṹ�����Ͷ��� 
	pEdgeInfo->DeclEdgeType(buf);

	//���ö��״̬����
	buf << "enum State\n";
	buf << "{\n";
	buf << "\tRUNNING,STOP,WAITING,FULL\n";
	buf << "};\n";

	//�ź�������
	//���ڿ���
	buf << "extern sem_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "zu"<<i <<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//ȥ�����һ������
	buf << ";\n";

	//������
	buf << "extern sem_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "exchange" <<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//ȥ�����һ������
	buf << ";\n";

	//����
	buf << "extern pthread_barrier_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "barrier"<< i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//ȥ�����һ������
	buf << ";\n";

	//״̬��ǩ
	buf << "extern int ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "label"<<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//ȥ�����һ������
	buf << ";\n";

	//��ˮ��ֹͣ����
	buf << "extern volatile bool ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "pipeline"<<i <<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//ȥ�����һ������
	buf << ";\n";

	//ѹ������������
	//�ж���ѹ������
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
		buf.seekp((int)buf.tellp() - 1);		//ȥ�����һ������
		buf << ";\n";
	}
	

	//���ȫ�ֱ���Buffer������
	vector<FlatNode *>::iterator iter;
	for (iter = flatNodes_.begin(); iter != flatNodes_.end(); ++iter)//�������н��,ֻ�������
	{
		//�����ѹ����ͼ��������2��Buffer����Ҫһ������flatNode���������ͼ�±�ĺ���
		for (iter_2 = (*iter)->outFlatNodes.begin(); iter_2 != (*iter)->outFlatNodes.end(); iter_2++)	//iter_2Ϊ�����ӵ������
		{
			int graphindex = dsg_->flatNode2graphindex.find(*iter)->second;
			if (dsg_->combineList[graphindex] == MERGEONE || dsg_->combineList[graphindex] == MERGEDOWNONE || dsg_->combineList[graphindex] == MERGEUPONE)
			{
				//ѹ����������߶�̬��
				//����һ�鶯̬�ߵĶ���,����2����̬��

				//һ��ѹ��������������Buffer������������̬��
				if ((dsg_->getActorPosition(*iter) == LAST || dsg_->getActorPosition((*iter)) == BOTH) && !dsg_->isSink(*iter))
				{
					//�����Ӵ�����ͼ������������Ϊ��̬��
					string edgename1 = (*iter)->name + "_" + (*iter_2)->name + "_w";//���ζ�̬Buffer
					string edgename2 = (*iter)->name + "_" + (*iter_2)->name + "_r";//���ζ�̬Buffer
					buf << "extern Buffer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << edgename1 << ";\n";
					buf << "extern Buffer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << edgename2 << ";\n";

					//����ȫ��Producer��Consumer
					string dsgconsumerName = (*iter)->name + "_" + (*iter_2)->name + "_READ";
					string dsgproducerName = (*iter)->name + "_" + (*iter_2)->name + "_WRITE";
					buf << "extern DSGProducer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << dsgproducerName << ";\n";
					buf << "extern DSGConsumer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << dsgconsumerName << ";\n";

				}
				else
				{
					//�����Ӵ���ѹ����ͼ�ڣ����ǲ��Ƕ�̬������
					string edgename = (*iter)->name + "_" + (*iter_2)->name;
					buf << "extern Buffer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << ">" << edgename << ";\n";
				}
			}
			else
			{
				//����Ҫ�ж�actor�ǲ���LAST�ڵ�
				if ((dsg_->getActorPosition(*iter) == LAST ||dsg_->getActorPosition((*iter))==BOTH)&& !dsg_->isSink(*iter))
				{
					//��̬������
					string edgename = (*iter)->name + "_" + (*iter_2)->name;
					buf << "extern Buffer<";
					buf << pEdgeInfo->getEdgeInfo((*iter), (*iter_2)).typeName << "> " << edgename << ";\n";
					//����һ�鶯̬�ߵĶ��壬����2����̬��
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

	

	if (readerActor)		//���ļ�Actor
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
		buf << "}\n";//���ֿռ����

	buf << "\n";
	buf << "#endif\n";
	//������ļ�
	stringstream ss;
	ss << dir_ << "global.h";
	OutputToFile(ss.str(), buf.str());
}
void DynamicX86CodeGenerate::DYGlobalCpp()
{
	//�������л�����

	//������ͼ�仺��������Ϊ���֣�1.��ͨ��ͼ�䶯̬����������С��ʼΪ������ͼ��̬������������10��
	//2.ѹ����ͼ�䶯̬����������Ϊread��write���ֱ����consumer��producer����С��ʼΪ������ͼ��̬������������10��������������������������ͬ
	stringstream buf;
	buf << "/*cpp�ļ���ȫ�ֱ��������ڴ洢�ߵ���Ϣ*/\n";

	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"global.h\"\n";
	buf << "#include \"DSG.h\"\n";
	buf << "#include <vector>\n";
	buf << "using namespace std;\n";
	if (CallModelEmbed)
		buf << "namespace COStream{\n";//���ֿռ�
	//ȫ�ֿ��Ʊ�����ʼ��

	//���ڿ����ź���
	buf << "sem_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "zu" <<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//ȥ�����һ������
	buf << ";\n";
	
	

	//�������ź�
	buf << "sem_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "exchange"<<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);		//ȥ�����һ������
	buf << ";\n";


	//����
	buf << "pthread_barrier_t ";
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "barrier"<<i<<",";
	}
	buf.seekp((int)buf.tellp() - 1);
	buf << ";\n";
	
	//�̱߳�ǩ
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "int label"<<i<<"=WAITING;\n";
	}

	//��ˮ���ſ�
	for (int i = 0; i < dsg_->scheStaticChildGraph.size(); ++i)
	{
		buf << "volatile bool pipeline"<<i<<"=true;\n";
	}

	//��
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
				//���ö�Ӧi��ͼ����
				buf << "pthread_mutex_t mutex"<<i<<";\n";
				
			}
		}
	}



	int init1, init2;//����actor�ͽ���actor��̬���Ȳ����ͽ��ܵ�������
	for (vector<FlatNode*>::iterator iter1 = flatNodes_.begin(); iter1 != flatNodes_.end(); ++iter1)
	{
		//�������ӣ�Ҫ�ж����Ƿ���ѹ����ͼ��
		//���Ҫ�ж����Ƿ���LAST���ж�̬��
		int graphindex = dsg_->flatNode2graphindex.find(*iter1)->second;		//�����ͼ���
		//ʹ�������
		if (dsg_->combineList[graphindex] == MERGEONE || dsg_->combineList[graphindex] == MERGEUPONE
			|| dsg_->combineList[graphindex] == MERGEDOWNONE)
		{
			//ֻ��������ѹ����ͼ�������˫Buffer���������ǵ�Buffer
			if ((dsg_->getActorPosition(*iter1) == LAST || dsg_->getActorPosition((*iter1)) == BOTH) && !dsg_->isSink(*iter1))
			{
				//��ͼ�����½ڵ�
				for (vector<FlatNode*>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); iter2++)
				{
					//ѭ����������ߣ�ÿ������߹���Buffer
					int size;
					int copySize = 0;
					int copyStartPos = 0;
					string edgenameRead = (*iter1)->name + "_" + (*iter2)->name + "_r";//������ͼ��ȡbuffer
					string edgenameWrite = (*iter1)->name + "_" + (*iter2)->name + "_w";//������ͼд��buffer
					string dsgproducerName = (*iter1)->name + "_" + (*iter2)->name + "_WRITE";

					string dsgconsumerName = (*iter1)->name + "_" + (*iter2)->name + "_READ";

					//�����ȡ�������ӵ�popֵ,Ҫ����һ����̬��������ݣ���Ҫ���ֵ������̬����
					int bianhao;
					for (int j = 0; j < (*iter2)->inFlatNodes.size(); j++)
					{
						if ((*iter2)->inFlatNodes.at(j) == (*iter1))//�ҵ����
						{
							bianhao = j;
						}
					}
					int perWorkPopCount = (*iter2)->inPopWeights[bianhao];//iter2������������
					//����û�ȡiter2���¸���ͼ�е���̬���ȴ���
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
				//ѹ����ͼ��actor���������Ϊ��ͨͨ�ű�
				for (vector<FlatNode*>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); iter2++)
				{
					//ѭ����������ߣ�ÿ������߹���Buffer
					int stageminus;//��ͨͨ�ű�����actor�н׶β�
					int size;		//��������С
					string edgename = (*iter1)->name + "_" + (*iter2)->name;//A_B
					stageminus = psalist[graphindex]->FindStage(*iter2) - psalist[graphindex]->FindStage(*iter1);//����������������ӽ׶β�
					
					int edgePos = iter2 - (*iter1)->outFlatNodes.begin();//iter2��iter1������е����
					int perSteadyPushCount = dsg_->scheStaticChildGraph[graphindex]->GetSteadyCount(*iter1)*(*iter1)->outPushWeights.at(edgePos);
					//�����̬push����
					if (NoCheckBuffer)
					{
						int copySize = 0, copyStartPos = 0;//���������ݴ�С��copy��ŵĿ�ʼλ��
						for (int inEdgeIndex = 0; inEdgeIndex < (*iter2)->inFlatNodes.size(); inEdgeIndex++)//�������ӵ��������Ŀ����
						{
							int perWorkPeekCount = (*iter2)->inPeekWeights[inEdgeIndex];//���ܱ�peekֵ
							int perWorkPopCount = (*iter2)->inPopWeights[inEdgeIndex];//���ܱ�popֵ
							//���û�г�̬��������ֵӦ�����
							init1 = dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter1)*(*iter1)->outPushWeights.at(edgePos);
							//��������һ��initwork������������Ŀǰ���������0
							init2 = dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter2)*perWorkPopCount;//���ܷ���̬������Ҫ���ĵ�
							//��������ĿǰҲ����Ϊ0
							size = init1 + perSteadyPushCount*(stageminus + 2);
							if (perWorkPopCount == perWorkPeekCount)//û�г�̬����
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
					else//ʹ�ñ߽���.Ŀǰ��֧�֣����ܴ�
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
		else	//��ѹ����ͼ
		{
			if ((dsg_->getActorPosition(*iter1) == LAST || dsg_->getActorPosition((*iter1)) == BOTH) && !dsg_->isSink(*iter1))
			{
				for (vector<FlatNode*>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); iter2++)
				{
					string edgename = (*iter1)->name + "_" + (*iter2)->name;
					string dsgconsumerName = (*iter1)->name + "_" + (*iter2)->name + "_C";
					string dsgproducerName = edgename + "_P";
					//ѭ����������ߣ�ÿ������߹���Buffer,��ѹ�����Ӷ�̬�ߣ�ֻ����һ��Buffer��һ��dsgConsumerһ��dsgproducer
					int copySize = 0;
					int copyStartPos = 0;
					int size;//��������С
					//�����ȡ�������ӵ�popֵ,Ҫ����һ����̬��������ݣ���Ҫ���ֵ������̬����
					int bianhao;
					for (int j = 0; j < (*iter2)->inFlatNodes.size(); j++)
					{
						if ((*iter2)->inFlatNodes.at(j) == (*iter1))//�ҵ����
						{
							bianhao = j;
						}
					}
					int perWorkPopCount = (*iter2)->inPopWeights[bianhao];//iter2������������
					//����û�ȡiter2���¸���ͼ�е���̬���ȴ���
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
				//��ѹ����ͼ��actor���������Ϊ��ͨͨ�ű�
				//ѹ����ͼ��actor���������Ϊ��ͨͨ�ű�
				//�����sink��ֱ�Ӳ�����iter2
				for (vector<FlatNode*>::iterator iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); iter2++)
				{
					//ѭ����������ߣ�ÿ������߹���Buffer
					int stageminus;//��ͨͨ�ű�����actor�н׶β�
					int size;		//��������С
					string edgename = (*iter1)->name + "_" + (*iter2)->name;//A_B
					stageminus = psalist[graphindex]->FindStage(*iter2) - psalist[graphindex]->FindStage(*iter1);//����������������ӽ׶β�

					int edgePos = iter2 - (*iter1)->outFlatNodes.begin();//iter2��iter1������е����
					int perSteadyPushCount = dsg_->scheStaticChildGraph[graphindex]->GetSteadyCount(*iter1)*(*iter1)->outPushWeights.at(edgePos);
					//�����̬push����
					if (NoCheckBuffer)
					{
						int copySize = 0, copyStartPos = 0;//���������ݴ�С��copy��ŵĿ�ʼλ��
						for (int inEdgeIndex = 0; inEdgeIndex < (*iter2)->inFlatNodes.size(); inEdgeIndex++)//�������ӵ��������Ŀ����
						{
							if ((*iter2)->inFlatNodes.at(inEdgeIndex) == (*iter1))
							{

								int perWorkPeekCount = (*iter2)->inPeekWeights[inEdgeIndex];//���ܱ�peekֵ
								int perWorkPopCount = (*iter2)->inPopWeights[inEdgeIndex];//���ܱ�popֵ
								//���û�г�̬��������ֵӦ�����
								init1 = dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter1)*(*iter1)->outPushWeights.at(edgePos);
								//��������һ��initwork������������Ŀǰ���������0
								init2 = dsg_->scheStaticChildGraph[graphindex]->GetInitCount(*iter2)*perWorkPopCount;//���ܷ���̬������Ҫ���ĵ�
								//��������ĿǰҲ����Ϊ0
								size = init1 + perSteadyPushCount*(stageminus + 2);
								if (perWorkPopCount == perWorkPeekCount)//û�г�̬����
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
					else//ʹ�ñ߽���.Ŀǰ��֧�֣����ܴ�
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
	//������ļ�
	stringstream ss;
	ss << dir_ << "global.cpp";
	OutputToFile(ss.str(), buf.str());

}
void DynamicX86CodeGenerate::DYGlobalVarExtern()		//ȫ�ֱ���������
{
	stringstream ss;
	ss << dir_ << "GlobalVar.h";
	OutputToFile(ss.str(), globalvarbuf.str());
}

void DynamicX86CodeGenerate::DYGlobalVar()		//ȫ�ֱ����Ķ���
{
	isCGGlobalVar = true;
	stringstream buf;
	// ��ȫ�ֱ������д�������
	flag_Global = 1;
	COSX86_List(gDeclList, 1);
	buf << declInitList.str() << "\n\n";
	declInitList.str("");
	flag_Global = 0;
	//������ļ�
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
	buf << "\n\t}\n"; // initVarAndState��������

	declInitList.str(""); // ���
	stateInit.str(""); // ���
}
void DynamicX86CodeGenerate::DYlogicInit(FlatNode*actor, OperatorType ot, stringstream&buf)
{
	declInitList.str(""); // ���
	buf << "\t// init\n";
	buf << "\tvoid init()";
	Node *init = actor->contents->body->u.operBody.init;
	if (init)
	{
		COSX86_Node(init, 2);
		buf << declInitList.str(); // init�ṹ�����"{}"
	}
	else
	{
		buf << " {\n";
		buf << "\t}\n"; // '}'��ռһ��
	}
	declInitList.str(""); // ���
}
void DynamicX86CodeGenerate::DYdeclList(FlatNode*actor, OperatorType ot, stringstream&buf)
{
	List *state = NULL;
	Node  *param = NULL;
	assert(actor);
	buf << "\t// AST Variables\n";
	//state��var��param��������������
	state = actor->contents->body->u.operBody.state;


	extractDecl = true;//��־����decl
	declList.str(""); // ���declList����
	buf << "\tint steadyScheduleCount;\t//��̬ʱһ�ε�����ִ�д���\n";
	buf << "\tint initScheduleCount;\n";
	

	//state
	COSX86_List(state, 0);//���state
	buf << "\t/* *****logic state***** */\n\t" << declList.str();
	
	extractDecl = false;
	//state init
	stateInit << declInitList.str();
	declInitList.str(""); // ���declInitList����
}
void DynamicX86CodeGenerate::DYinitPeek(stringstream &buf, string initPeekBuf)		//������ʹ��
{
	buf << "\t// initPeek\n";
	buf << "\tdef initPeek() {\n";

	buf << initPeekBuf;

	buf << "\t}\n"; // initPeek��������
}


void DynamicX86CodeGenerate::DYinitWork(stringstream &buf)		//��ʼ������
{
	buf << "\t// initWork\n";
	buf << "\tvoid initWork() {\n";
	buf << "\t\tinitVarAndState();\n";
	buf << "\t\tinit();\n";

	buf << "\t}\n"; // initWork��������
}


void DynamicX86CodeGenerate::DYwork(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	vector<string>::iterator iter;
	Node *work = actor->contents->body->u.operBody.work;
	string actorname = actor->name;
	stringstream tmpBuf;
	buf << "\t// work\n";
	buf << "\tvoid work() {\n";
	COSX86_Node(work, 2);		//����work��work�е����������declInitList
	buf << declInitList.str();
	buf << "\n\t\tpushToken();\n";
	buf << "\n\t\tpopToken();\n";
	buf << "\t}\n"; // work��������
	declInitList.str(""); // ���
}



/*-------------------------

���ߺ���

----------------------------*/
//����dsg��SSSGͼ��index��combinelist�����ʹ��mp���ֵõ�ʹ�ú���
void DynamicX86CodeGenerate::setGraphindex2coreNum()
{
	//��ֵgraphIndex2coreNum���ó�Ա��getThreadIndex2SSSGindexʹ��
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
			//ѹ��������������ѹ��������ѹ������ʹ��nCpucore-��������ʹ�ú���
			graphIndex2coreNum.insert(pair<int, int>(i, nCpucore_ - graphIndex2coreNum.find(i - 1)->second));
		}
		else if (dsg_->combineList[i] == MERGETWO)
		{
			//������ʹ�õĺ�����nCpucore-��������ʹ�ú���
			graphIndex2coreNum.insert(pair<int, int>(i, nCpucore_-graphIndex2coreNum.find(i-1)->second));
		}
		else if (dsg_->combineList[i] == MERGEONE || dsg_->combineList[i] == MERGEUPONE || dsg_->combineList[i]==MERGEDOWNONE)
		{
			//��Щ����ʹ�õĺ�����Ҳ�ǻ��ֵ�����ż�1�����ʹ��5���ˣ���maxPartition=4��insert��ֵΪmaxPartition+1
			graphIndex2coreNum.insert(pair<int, int>(i, maxPartition + 1));
		}
		else
		{
			//��ѹ������ʹ���߳���ĿΪnCpucore
			graphIndex2coreNum.insert(pair<int, int>(i, nCpucore_));
		}
		
	}

}

//�����̱߳�Ż�����ڱ���ͼ�еı��
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

//�����̵߳�index��ö�ӦSSSGͼ��index
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
//�����̱߳���ж��Ƿ�����ͼ�����߳�
bool DynamicX86CodeGenerate::isMainThreadInGraph(int index)
{
	int bianhao = getindexinGraph(index);		//����߳�����ͼ�ڱ��
	/*int graphindex = getThreadIndex2SSSGindex(index);	//����ڵ���ͼ���
	if (dsg_->combineList[graphindex] == MERGETWO || dsg_->combineList[graphindex] == MERGEUPTWO ||
		dsg_->combineList[graphindex] == MERGEDOWNTWO)
	{
		//ֻ��ѹ�����ӵ��������������̲߳�Ϊ�����ı���
		int UseCorePos = graphIndex2coreNum.find(graphindex - 1)->second;
		//�����������ʹ��5������ʵ���̱߳��Ϊ0-4��������ͼ�����̱߳��Ϊ5
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

//���һ����ͼ������ߵ�����
string DynamicX86CodeGenerate::getGraphInName(int graphindex)
{
	assert(graphindex != 0);//����ͼû�������
	string inName="";
	FlatNode*firstNode = dsg_->staticChildGraph[graphindex]->flatNodes[0];

	pair<multimap<FlatNode*, string>::iterator, multimap<FlatNode*, string>::iterator>pos1;

	pos1 = mapActor2InEdge.equal_range(firstNode); //[pos1.first,pos1.second)������ÿ��key������firstNode��������key

	while (pos1.first != pos1.second)
	{
		inName= pos1.first->second;
		++pos1.first;
	}
	//��������A_B��Ҫ��ô���A_B_C����A_B_P
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
//���һ����ͼ������ߵ�����
string DynamicX86CodeGenerate::getGraphOutName(int graphindex)
{
	if (graphindex == (dsg_->scheStaticChildGraph.size() - 1))//β��ͼû�������
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


void DynamicX86CodeGenerate::DYThreads()		//�߳������ļ����
{
	string threadname;
	stringstream buf;
	//�����nCpucore���ˣ�N����ͼ������Ҫ�ж���ͼ��MERGE���
	//�����FULLCORE������nCpucore�������MERGE����MERGEUP,MERGEDOWN�ͺϲ�Ϊһ��
	
	
	for (int i = 0; i < threadNum; ++i)
	{
		buf << "/*�����̺߳����ڴ˶���*/\n";
		stringstream ss;
		ss << dir_ << "thread_" << i / nCpucore_ << "_" << i%nCpucore_ << ".cpp";
		DYThread(i, buf);	//
		OutputToFile(ss.str(), buf.str());
		ss.str("");
		buf.str("");//�������
	}


}
void DynamicX86CodeGenerate::DYThreadFaker(int index, stringstream&buf)		//�߳����ɴ���
{
	//����ʹ��pSAlist����ȡÿ����ͼ�Ľ׶λ���
	//����dsg�ڲ���SSSGͼ�±���psalist��mplist�е��±��MetisPartition��StageAssignment��Ӧ
	//����������index����˱�����һ��ͨ��index��ȡ���Ӧ��̬��ͼ�±�ĺ���
	int graphIndex = getThreadIndex2SSSGindex(index);//���ͼ�±�
	int bigstagenum = psalist[graphIndex]->MaxStageNum();//��ý׶λ��ֱ�����ֵ
	//���3�����ӣ�8�ˣ����ڱ��Ϊ1�Ļ��֣�����indexֵΪ14���ڵ�ǰ��ͼ�б��Ϊ1��graphNum��Ϊ1
	int graphNum = getindexinGraph(index);		//��ø�index�ڸ���ͼ�еı��
	//���һ��ѹ����ͼʹ��4���ˣ���׶λ��ֱ�����ֵ����4
	pair<multimap<FlatNode*, string>::iterator, multimap<FlatNode*, string>::iterator>pos1, pos2;
	//���ͷ�ļ�
	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"Producer.h\"\n";
	buf << "#include \"DSG.h\"\n";
	buf << "#include \"Consumer.h\"\n";
	
	buf << "#include \"global.h\"\n";
	buf << "#include \"AllActorHeader.h\"\t//��������actor��ͷ�ļ�\n";
	buf << "#include \"lock_free_barrier.h\"\t\n";
	buf << "#include \"pthread.h\"\n";
	buf << "#include \"rdtsc.h\"\n";
	buf << "#include <fstream>\n";
	if (MakeProfile)		//����Ҫ����profile�ļ������Ǽ��Ϻ���
	{
		buf << "#include <sys/stat.h>\n";
		buf << "#include <sys/types.h>\n";
	}
	if (CHECKEACHACTORTIME || MakeProfile)//������̬ÿ��actor��steadyworkʱ��
	{
		buf << "#include <sstream>";
	}
	if (CallModelEmbed)	//Ƕ��ʽ
	{
		buf << "namespace COStream{\n";
	}
	if (Linux)
	{
		buf << "extern int MAX_ITER;\n";		//linux�µĵ�������
	}
	if (TRACE)
	{
		buf << "extern double (*deltatime)[" << threadNum << "][2];\n";//���Ϊtrace�Ļ������¼ÿ���߳�ÿ�ε�����ͬ���ͼ���ʱ�䳤��
		buf << "#define MEASURE_RUNS 100000\n";//MEASURE_RUNSΪʱ�ӳ��ȵ�λ��Ϊ10Mʱ������
	}
	else if (CALRATIO)		//����ÿ���̼߳���ͬ����
	{
		buf << "#define MEASURE_RUNS 100000\n";
	}

	

	buf << "void thread_" << index / nCpucore_ << "_" << index%nCpucore_ << "_fun()\n{\n";
	
	if (TRACE || CALRATIO)//����ÿ���̼߳���ͬ����
	{
		buf << "\ttsc_counter c0,c1;\n";
	}
	if (CALRATIO)
		buf << "\tdouble ca1=0,total = 0;\n";
	if (CHECKEACHACTORTIME)
	{
		buf << "\tpfstream txtfw;\n\tstringstream ss;\n";
		buf << "\tss<<\"���ļ���¼���߳���ÿ��actor����ִ̬�е�ʱ�䣬��һ��Ϊactor�����ƣ��ڶ���Ϊ�ô���ִ̬�е�ʱ������������λΪ10^6��\"<<endl;\n";
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

	//���̲߳���һ�����������Ӿ��Ǹ���ͼ�ĵ�һ�����Ӻ����һ������
	//������������̺߳������м���ͼ���Ҹ�indexΪ���̵߳Ļ����������һ�����������Ƿ�ִֹͣ���ж�
	//��Ҫ������ʼ���ݲ�������
	if (graphIndex != 0&&isMainThreadInGraph(index))
	{
		//���߳�
		buf << "\tif(label" << graphIndex - 1 << "==STOP)\n";
		buf << "\t{\n";
		//������Ҫ��ȡ����ͼ��Ӧ�Ķ�̬��������֣�
		//����Ҫ�ж��Ƿ���ѹ����ͼ��ѹ����ͼʹ�ò�ͬ��ͨ�ű�����
		if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
			dsg_->combineList[graphIndex] != NOTJUDGE)
		{
			buf << "\t\t" << getGraphInName(graphIndex) << ".append(" << getGraphOutName(graphIndex - 1) << ");\n";

		}
		
		buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
		buf << "\t\t{";
		buf << "\t\t//����������\n";
		
		buf << "\t\t\tlabel" << graphIndex << "=STOP;\n";
		buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
		buf << "\t\t\treturn ;\n";
		buf << "\t\t}\n";
		buf << "\t}\n\n";


	}

	//������Ҫ��ȡ��Ӧindex��ȫ������
	//����Ҫ�жϵ�ǰindex��Ӧ�߳�������ͼ��Ȼ�������ͼ�±�����ȡ��Ӧmetis����
	//ͬʱ����metis���ֻ��ڻ��ֱ�ţ����Ա������index��ȡ����index����ͼ�еĻ��ֱ��
	//����ѹ�����ӵ�������ͼ����index������index%nCpucore������
	vector<FlatNode*> tmpactorset;//��Ӧ��ǰ���ֵ����Ӽ���
	if (dsg_->combineList[graphIndex] == MERGETWO || dsg_->combineList[graphIndex] == MERGEUPTWO
		|| dsg_->combineList[graphIndex] == MERGEDOWNTWO)
	{
		//�����������ڱ���ͼ�еı��
		
		tmpactorset = mplist[graphIndex]->findNodeSetInPartition(graphNum);
	}
	else
		tmpactorset = mplist[graphIndex]->findNodeSetInPartition(index%nCpucore_);
	

	//����ȷ�����߳�������߳�
	//ֻ���ɵ���һ��ͬ��
	//��Ҫһ�����ߺ��������ݴ����index���ж��Ƿ���ĳ����ͼ�����̺߳���Ϊ0��8������ѹ�����ӵ�ĳһ��
	if (isMainThreadInGraph(index))
	{
		//���߳�

		buf << "\t//�����߳��ź�����������\n";
		if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
			dsg_->combineList[graphIndex] != NOTJUDGE)
		{
			if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEUPONE
				|| dsg_->combineList[graphIndex] == MERGEDOWNONE)
			{
				//����ѹ�����ӵ��������ӣ����ѹ����ͼ�ܺ���С��nCpucore�����������Ӳ���
				//��������������5��������bigstagenum�͵���5����0��4
				buf << "\tfor(int i = 0;i<"<<bigstagenum<<";++i)\n";
				buf << "\t{\n" << "sem_post(zu" << graphIndex << ");\n";
				buf << "\t}\n";
				buf << "masterSync(" << bigstagenum << ");\n";
			}
			else
			{
				//ѹ�����ӵ��������Ӽ�����2��������������3��Ӧ����012
				buf << "\tfor(int i = 0;i<" << (nCpucore_ - psalist[graphIndex - 1]->MaxStageNum()) << ";++i)\n";
				buf << "\t{\n" << "sem_post(zu" << graphIndex << ");\n";
				buf << "\t}\n";
				buf << "masterSync(" << (nCpucore_ - psalist[graphIndex - 1]->MaxStageNum()) << ");\n";
			}
			
		}
		else
		{
			//��ѹ���������߳�
			buf << "\tfor(int i = 0;i<" << nCpucore_ << ";++i)\n";
			buf << "\t{\n" << "sem_post(zu" << graphIndex << ");\n";
			buf << "\t}\n";
			buf << "masterSync(" << nCpucore_ << ");\n";
		}
		
	}
	else
	{
		//�����߳�
		buf << "sem_wait(&zu" << graphIndex << ");\n";
		buf << "workerSync(" << getindexinGraph(index) << ");\n";
	}



	//���캯������
	for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
	{
		buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
		//mapActor2InEdge��mapActor2OutEdge��Ÿ�actor����Ӧ�����������buffer������
		//��ȡ��Ӧstring����
		pos1 = mapActor2InEdge.equal_range(*iter);
		pos2 = mapActor2OutEdge.equal_range(*iter);
		while (pos2.first != pos2.second)		//�����������
		{
			buf << pos2.first->second << ",";
			++pos2.first;
		}
		while (pos1.first != pos1.second)		//�����
		{
			buf << pos1.first->second << ",";
			++pos1.first;
		}
		buf.seekp((int)buf.tellp() - 1);		//ȥ�����һ������
		buf << "):\n";
		if (MakeProfile)		//��profile���ۼ�ִ��ʱ��
		{
			buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
		}
	}
	//stage��ʾ�׶κ����飬��ʼ��0�ⶼΪ0
	buf << "\tchar stage[" << bigstagenum << "]={0};\n";
	buf << "\tstage[0]=1;\n";
	//����Ҫ������ͼ�жϣ������һ����ˮ���ſտ��Ʊ���
	if (graphIndex != 0 && isMainThreadInGraph(index))
	{
		//������ͼ���������߳�
		buf << "\tbool out = true;\n";
		//�����̲߳���Ҫ��ˮ�߽�����ֻ��Ҫ����Ȼ��ȴ�main������������
	}
	
	//��ʼ��
	//ֱ�ӵ���runInitScheduleWork
	//ѭ�����л��ֱ��߳�����
	for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
	{
		buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
	}


	
	//��̬����ǰͬ��
	if (!isMainThreadInGraph(index))
	{
		//�������̣߳������߳�
		buf << "\t\n\t\tworkerSync(" <<getindexinGraph(index) << ");\n";
	}
	else
	{
		//���masterSync��Ҫ��ѹ����ͼ�����жϣ���Ϊѹ����ͼ��ͬ�����߳�����������nCpucore
		
		if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
			dsg_->combineList[graphIndex] != NOTJUDGE)
		{
			//ѹ����ͼ���߳�
			if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEUPONE
				|| dsg_->combineList[graphIndex] == MERGEDOWNONE)
			{
				//��������
				buf << "\t\n\t\tmasterSync(" << bigstagenum << ");\n";
			}
			else
			{
				//��������
				buf << "\t\n\t\tmasterSync(" << (nCpucore_ - psalist[graphIndex - 1]->MaxStageNum()) << ");\n";
			}
			
		}
		else
			buf << "\t\n\t\tmasterSync(" << nCpucore_ << ");\n";
	}


	//��̬����
	//Ҫ��ö�Ӧindex��ŵĽ׶κţ�����һ���߳̿���ִ�ж���׶�
	set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second; 
	//���mapNum2Stage�������ʼ���ģ�
	set<int>::iterator endIter = ptempstagenum.end();
	//����ͼ��������ͼ��ͬ����������ȷ��
	if (isMainThreadInGraph(index))//���̱߳�����nCpucore�ı���
	{
		if (graphIndex == 0)
		{
			//����ͼ,Ҳ���ж��Ƿ���ѹ����ͼ
			//�ֳ�ѹ����ͼ���ѹ����ͼ
			if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
				dsg_->combineList[graphIndex] != NOTJUDGE)
			{
				//ѹ�����ӵ�����ͼ����һ������Ϊѹ����������
			}
			else
			{
				buf << "\tfor(int _stageNum = " << bigstagenum << ";_stageNum<2*" << bigstagenum << "+MAX_ITER-1;_stageNum++)\n";
				buf << "\t{\n";
				for (int i = bigstagenum - 1; i >= 0; i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenumΪ���߳���Ҫִ�еĽ׶�
					//�����ҵ����׶�ִ�е�����
					if (stageiter != endIter)
					{
						buf << "\t\tif(stage[" << i << "])\n\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
						vector<FlatNode*>::iterator iter1;
						for (iter1 = flatVec.begin(); iter1 != flatVec.end(); iter1++)
						{
							if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
							{
								//��ǰ�߳���ŵ��ڸ����ӻ��ֵı�ţ���ʾ������ȷ���ڸ��߳���ִ��
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
							}
						}
						buf << "\t\t}\n";
					}

				}
				//��ʼ����ع�
				buf << "\t\tfor(int index = " << bigstagenum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";
				buf << "\t\tif(_stageNum==(MAX_ITER-1+" << bigstagenum << "))\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0]=0;\n";
				buf << "\t\t}\n";
				buf << "\t\n\t\tmasterSync(" << nCpucore_ << ");\n";

				//�����ж�
				buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())//���������\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");//������������\n";
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");//����ȴ�\n";
				//��̬��ʼ��һ��ѭ��,�ַ��ź���
				//���������Ҫÿһ�����߳�һ���ź�����������Ժ��ٿ���
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
			//������ͼ
			//�ֳ�ѹ����ͼ���ѹ����ͼ
			if (dsg_->combineList[graphIndex] != FULLCORE&&dsg_->combineList[graphIndex] != NOTMERGE&&
				dsg_->combineList[graphIndex] != NOTJUDGE)
			{
				//ѹ������
				//��Ϊ���κ�����
				if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEUPONE
					|| dsg_->combineList[graphIndex]);
			}
			else
			{
				//��ѹ������
				//��̬����
				buf << "while(1)\n";
				buf << "\t{\n";
				for (int i = bigstagenum - 1; i >= 0; i++)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenum���ڸ��߳���ִ�е����н׶μ���
					if (stageiter != endIter)//�ҵ���ʾ��stage�ڸ�thread��
					{
						buf << "\t\tif(stage[" << i << "])\n";
						buf << "\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);//��Ӧ�ý׶ε�����
						vector<FlatNode*>::iterator iter1;
						for (iter1 == flatVec.begin(); iter1 != flatVec.end(); iter1++)
						{
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadySchuduleWork();\n";
						}
						buf << "\t\t}\n";
					}

				}
				//����ع�
				buf << "\t\tfor(int index=" << bigstagenum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";
				buf << "\t\tmasterSync(" << nCpucore_ << ");\n";


				//�������ӽ���
				buf << "\t\t//��ʼ�����߳�ͬ�������ȴ������������ӵĽ���\n";
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&label" << graphIndex - 1 <<
					"==STOP&&stage[0]!=0)\n";
				buf << "\t\t//��ˮ�߿�ʼ�ſ�\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0]=0;\n";
				buf << "\t\t}\n";
				
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t//��̬���ݲ���\n";
				buf << "\t\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t\t\t//�ȴ�����\n";
				buf << "\t\t}\n";

				//���Ѻ���
				buf << "\t\t//���Ѻ�Ҫһ�μ�飬�ж������Ƿ�һ����̬��������ʾ���������Ѿ���������ʼ�ſ�\n";
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0] = 0;\n";
				buf << "\t\t}\n";

				//�������
				if (graphIndex == dsg_->scheStaticChildGraph.size() - 1)//���һ����ͼ
				{
					;
				}
				else
				{
					buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
					buf << "\t\t{\n";
					buf << "\t\t//�����\n";
					buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
					buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
					buf << "\t\t}\n";
				}

				//֪ͨ���������߳̿�ʼ��ˮ���ſ�
				buf << "\t\tif(stage[0]==0&&pipeline" << graphIndex << "==true)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpipeline" << graphIndex << "=false\n";
				buf << "\t\t}\n";

				//��ˮ��ֹͣ�ж�
				buf << "\t\tfor(int i= 0;i<" << bigstagenum << ";i++)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tif(stage[i]==1)\n";
				buf << "\t\t\t{\n";
				buf << "out = false;\n";
				buf << "\t\t\t}\n";
				buf << "\t\t}\n";
				buf << "\t\tif(out==false);\n";
				buf << "\t\telse break;\n";
				
				//��̬ѭ���ź���
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
		//�����߳�
		if (graphIndex == 0)
		{
			//���̵߳Ĵ����̣߳����������ƾ�̬���߳���
			buf << "\tfor(int _stageNum=" << bigstagenum << ";_stageNum<2*" << bigstagenum << "+MAX_ITER-1;_stageNum++)\n\t{\n";
			for (int i = bigstagenum - 1; i >= 0; i--)
			{
				set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenumΪ���߳���Ҫִ�еĽ׶μ���
				//����ҵ��ͱ�ʾ��Ӧ�׶�i�ڱ��߳���ִ��
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
			//��ʼ����ع�
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
			buf << "\t\t//�����̲߳���Ҫ������ֻҪ��������main�����ȴ����߳�ִ����Ϻ��Զ�����������������Ѿ�";
			buf << "ֹͣ�ͻ�һֱ����";
			//������Ҫ����ڸ�index���߳���ִ�еĽ׶�
			for (int i = bigstagenum - 1; i >= 0; i--)	//����stage
			{
				set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenum��һ�����߳�
				//��Ҫִ�еĽ׶μ��ϡ�
				//���stageiter���ҵ���˵���ý׶��ڱ��߳���ִ��
				if (stageiter != endIter)
				{
					buf << "\t\tif(stage[" << i << "])\n";
					buf << "\t\t{\n";

					vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
					vector<FlatNode*>::iterator iter1;
					for (iter1 = flatVec.begin(); iter1 != flatVec.end(); iter1++)
					{
						//��ʼ��̬��������
						if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
						{
							//��ʾ�ڻ����У��ý׶ζ�Ӧ�ĸ����ӻ��ֵ����߳�
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
						}
					}
					buf << "\t\t}\n";
				}
			}
			


			//��ʼ��������ع�
			buf << "\t\tfor(int index=" << bigstagenum - 1 << ";index>=1;--index)\n";
			buf << "\t\t\tstage[index] = stage[index-1];\n";

			//��̬��������ʼͬ��
			buf << "\t\tworkerSync(" << graphNum << ")\n";

			//�ȴ���һ��ִ�е��ź���
			buf << "\t\tsem_wait(&zu" << graphIndex << ");\n";


			//��ˮ���ſ�

			buf << "\t\tif(pipeline" << graphIndex << "==false&&stage[0]!=0)\n";
			buf << "\t\t{\n";
			buf << "\t\t//���߳�������pipeline��ǣ������̻߳�ȡ�ź�����ֻ�ܶ�ȡ�ñ��\n";
			buf << "\t\t//���������ʱstage[0]Ϊ1������Ϊ0��ʼ�ſգ����Ϊ0������";
			buf << "\t\t\tstage[0] = 0;\n";
			buf << "\t\t}\n";
			//�ٴ�ͬ��
			buf << "\t\tworkerSync(" << graphNum << ")\n";

			buf << "\t}\n";
			buf << "}\n";
		}
	}
	//������ˮ���ſս׶�
	//�����Ϊѹ����ͼ���κ�������ͼ���������ͼ
	//����ֻ�����߳���Ҫ������ˮ���ſմ���
	if (isMainThreadInGraph(index))
	{
		buf << "��̬���Ƚ�������ˮ���ſ�\n\t���ý���״̬������������ͼ\n";
		if (graphIndex == dsg_->scheStaticChildGraph.size() - 1)	//�����ͼ
		{
			buf << "\tlabel" << graphIndex << "=STOP";
		}
		else
		{
			buf << "\tlabel" << graphIndex << "=STOP\n";
			buf << "\t//���ݵ�ʣ�ಿ�ֿ����������������ӣ���Ȼ�������ӿ�����Ҫ�����������״̬\n";
			buf << "\t//�����������߳�ͬ����\n";
			buf << "\tsem_post(&exchange" << graphIndex + 1 << ");\n";
			buf << "\t}\n";
		}
	}


}
void DynamicX86CodeGenerate::DYThread(int index, stringstream&buf)
{
	//����ʹ��pSAlist����ȡÿ����ͼ�Ľ׶λ���
	//����dsg�ڲ���SSSGͼ�±���psalist��mplist�е��±��MetisPartition��StageAssignment��Ӧ
	//����������index����˱�����һ��ͨ��index��ȡ���Ӧ��̬��ͼ�±�ĺ���
	int graphIndex = getThreadIndex2SSSGindex(index);//���ͼ�±�
	int stageMaxNum = psalist[graphIndex]->MaxStageNum();
	int graphNum = getindexinGraph(index);//index����ͼ�б��
	int useCoreNum = graphIndex2coreNum.find(graphIndex)->second;
	//dsg���ssg2coreNum��һ��FlatNode2Corenum�ĳ�Ա�������graphIndex2coreNum��index2coreNum�ĳ�Ա
	//���һ��ѹ����ͼʹ��4���ˣ���metis���ֱ�����ֵ����4
	//�����ļ�
	pair<multimap<FlatNode*, string>::iterator, multimap<FlatNode*, string>::iterator>pos1, pos2;
	//���ͷ�ļ�
	buf << "#include \"Buffer.h\"\n";
	buf << "#include \"Producer.h\"\n";
	
	buf << "#include \"Consumer.h\"\n";
	buf << "#include \"DSG.h\"\n";
	buf << "#include \"global.h\"\n";
	buf << "#include \"AllActorHeader.h\"\t//��������actor��ͷ�ļ�\n";
	buf << "#include \"lock_free_barrier.h\"\t\n";
	buf << "#include \"pthread.h\"\n";
	buf << "#include \"rdtsc.h\"\n";
	buf << "#include <fstream>\n";
	if (MakeProfile)		//����Ҫ����profile�ļ������Ǽ��Ϻ���
	{
		buf << "#include <sys/stat.h>\n";
		buf << "#include <sys/types.h>\n";
	}
	if (CHECKEACHACTORTIME || MakeProfile)//������̬ÿ��actor��steadyworkʱ��
	{
		buf << "#include <sstream>";
	}
	if (CallModelEmbed)	//Ƕ��ʽ
	{
		buf << "namespace COStream{\n";
	}
	if (Linux)
	{
		buf << "extern int MAX_ITER;\n";		//linux�µĵ�������
	}
	if (TRACE)
	{
		buf << "extern double (*deltatime)[" << threadNum << "][2];\n";//���Ϊtrace�Ļ������¼ÿ���߳�ÿ�ε�����ͬ���ͼ���ʱ�䳤��
		buf << "#define MEASURE_RUNS 100000\n";//MEASURE_RUNSΪʱ�ӳ��ȵ�λ��Ϊ10Mʱ������
	}
	else if (CALRATIO)		//����ÿ���̼߳���ͬ����
	{
		buf << "#define MEASURE_RUNS 100000\n";
	}
	buf << "void thread_" << index / nCpucore_ << "_" << index%nCpucore_ << "_fun()\n{\n";

	if (TRACE || CALRATIO)//����ÿ���̼߳���ͬ����
	{
		buf << "\ttsc_counter c0,c1;\n";
	}
	if (CALRATIO)
		buf << "\tdouble ca1=0,total = 0;\n";
	if (CHECKEACHACTORTIME)
	{
		buf << "\tpfstream txtfw;\n\tstringstream ss;\n";
		buf << "\tss<<\"���ļ���¼���߳���ÿ��actor����ִ̬�е�ʱ�䣬��һ��Ϊactor�����ƣ��ڶ���Ϊ�ô���ִ̬�е�ʱ������������λΪ10^6��\"<<endl;\n";
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

	//�������������Ƿ������̣߳��ǲ��ǵ�һ��ֿ�
	if (graphIndex == 0)
	{
		//����ͼ
		if (isMainThreadInGraph(index))
		{
			//���߳�
			if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEUPONE
				|| dsg_->combineList[graphIndex] == MERGEDOWNONE)
			{
				//ѹ����ͼ
				//����ͼ��ѹ����ͼû��WAITING״̬
				buf << "\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";
				buf << "\tlabel" << graphIndex << "=RUNNING;\n";
				//����
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" <<nCpucore_<< ",true,true,"<<dsg_->ssg2coreNum[graphIndex].second<<");\n";
				//���캯������
				vector<FlatNode*> tmpactorset;
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(graphIndex);

				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//����ͼû�ж�̬�����
					if (dsg_->getActorPosition(*iter) == LAST)
					{
						//���һ�������
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
						//��ͼ����������
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter��������б�
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter��������б�
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
					if (MakeProfile)		//��profile���ۼ�ִ��ʱ��
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}
			
				//��ˮ���ȿ�������
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//������ִ�н׶���Ŀ
				//stage��ʾ�׶κ����飬��ʼ��0�ⶼΪ0
				buf << "\tstage[0]=1;\n";

				//��ʼ��
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name + "_obj.runInitScheduleWork();\n";
				}

				//��̬����
				//��ȡ���߳�ʹ�õĽ׶���Ŀ
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
								//��ǰ�߳���ŵ��ڸ����ӱ����ֵ����
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";

							}
						}
						buf << "\t\t}\n";
					}
				}
				//����ع�
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";
				buf << "\t\tif(_stageNum==(MAX_ITER-1+" << stageMaxNum << "))\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0]=0;\n";
				buf << "\t\t}\n";

				//��������
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,true," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//û������,������ѹ�����ӣ����Ա�Ȼ�������
				//���ν���
				buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
					//�����
				buf << "\t\t{\n";
				buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//����
				buf << "\t\t\tlabel" << graphIndex << "=FULL;\n";	//����״̬
				buf << "\t\t\tif(label" << graphIndex + 1 << "==WAITING||label" << graphIndex + 1 << "==FULL)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t//������ͼ�Ѿ�����\n";
				buf << "\t\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "\t\t\t}\n";

				buf << "\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";//����
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//�������޸�״̬
				buf << "\t\t\tlabel" << graphIndex << "=RUNNING;\n";
				buf << "\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";
				buf << "\t\t\t" << getGraphOutName(graphIndex) << ".resetWrite();\n";
				buf << "\t\t}\n";
				
				//���ƴ����߳�
				buf << "\t\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";

				//��������
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\t}\n";

				//ֹͣ��־
				buf << "\tlabel" << graphIndex << "=STOP;\n";
				buf << "\tsem_post(&exchange" << graphIndex + 1 << ");\n";

				buf << "}\n";



			}
			else
			{
				//����ͼ��ѹ����ͼ���߳�

				
				buf << "\tfor(int i = 1;i<" << nCpucore_ << ";++i)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";

				//����
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ");\n";
				//���캯������
				vector<FlatNode*> tmpactorset;
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(graphIndex);

				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter!= tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//����ͼû�ж�̬�����
					if (dsg_->getActorPosition(*iter) == LAST)
					{
						//���һ�������
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
						//��ͼ����������
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter��������б�
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter��������б�
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
					if (MakeProfile)		//��profile���ۼ�ִ��ʱ��
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}

				//��ˮ���ȿ�������
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//������ִ�н׶���Ŀ
				//stage��ʾ�׶κ����飬��ʼ��0�ⶼΪ0
				buf << "\tstage[0]=1;\n";

				//��ʼ��
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name + "_obj.runInitScheduleWork();\n";
				}
				//��̬����
				//����ڸ��߳���ִ�еĽ׶�
				set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
				set<int>::iterator endIter = ptempstagenum.end();

				buf << "\tfor(int _stageNum = " << stageMaxNum << ";_stageNum<2*" << stageMaxNum << "+MAX_ITER-1;_stageNum++)\n";
				buf << "\t{\n";
				for (int i = stageMaxNum - 1; i >= 0; i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);//ptempstagenumΪ���߳���Ҫִ�еĽ׶�
					//�����ҵ����׶�ִ�е�����
					if (stageiter != endIter)
					{
						buf << "\t\tif(stage[" << i << "])\n\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
						vector<FlatNode*>::iterator iter1;
						for (iter1 = flatVec.begin(); iter1 != flatVec.end(); iter1++)
						{
							if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
							{
								//��ǰ�߳���ŵ��ڸ����ӻ��ֵı�ţ���ʾ������ȷ���ڸ��߳���ִ��
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
							}
						}
						buf << "\t\t}\n";
					}
				}

				//����ع�
				//��ʼ����ع�
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";
				buf << "\t\tif(_stageNum==(MAX_ITER-1+" << stageMaxNum << "))\n";
				buf << "\t\t{\n";
				buf << "\t\t\tstage[0]=0;\n";
				buf << "\t\t}\n";

				//��������
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ");\n";
				//������ͼ����
				if (graphIndex != (dsg_->scheStaticChildGraph.size() - 1))
				{
					buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
					buf << "\t\t{\n";
					buf << "\t\t\tsem_post(&exchange" << graphIndex+1 << ");\n";
					buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
					
					buf << "\t\t"<<getGraphOutName(graphIndex) << ".resetTail();\n";
					buf << "\t\t}\n";
				}

				//�����߳�����ִ����̬
				buf << "\t\tfor(int i = 1;i<" << nCpucore_ << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";

				//��������
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\t}\n";

				//����STOP
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
			//����ͼ�����߳�

			buf << "\tsem_wait(&zu" << graphIndex << ");\n";

			//����
			//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\tworkerSync(" << index%nCpucore_ << ");\n";
			//���캯������
			vector<FlatNode*> tmpactorset;
			tmpactorset = mplist[graphIndex]->findNodeSetInPartition(index%nCpucore_);

			for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
			{
				buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
				//����ͼû�ж�̬�����
				if (dsg_->getActorPosition(*iter) == LAST)
				{
					//���һ�������
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
					//��ͼ����������
					pos1 = mapActor2InEdge.equal_range(*iter);		//iter��������б�
					pos2 = mapActor2OutEdge.equal_range(*iter);		//iter��������б�
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
				if (MakeProfile)		//��profile���ۼ�ִ��ʱ��
				{
					buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
				}
			}
			
			//��ˮ��������
			buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//������ִ�н׶���Ŀ
			//stage��ʾ�׶κ����飬��ʼ��0�ⶼΪ0
			buf << "\tstage[0]=1;\n";
			
			//��ʼ��
			for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
			{
				buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
			}

			//��̬����
			set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;//����׶εĹ�ϵ
			//ptempstagenum����ĳ�����Ͻ׶εļ���
			set<int>::iterator enditer = ptempstagenum.end();


			buf << "\tfor(int _stageNum =" << stageMaxNum << ";_stageNum<2*" << stageMaxNum << "+MAX_ITER-1;_stageNum++)\n";
			buf << "\t{\n";
			for (int i = stageMaxNum - 1; i >= 0; i--)
			{
				set<int>::iterator stageiter = ptempstagenum.find(i);
				//�׶δ�StageMaxNum��0������ȷ����Щ�׶����������ִ��
				if (stageiter != enditer)
				{
					//�ڸú���
					buf << "\t\tif(stage[" << i << "])\n";
					buf << "\t\t{\n";
					vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
					//�ҵ���Ӧ�ý׶ε���������
					vector<FlatNode*>::iterator iter1;

					for (iter1 = flatVec.begin(); iter1 != flatVec.end(); ++iter1)
					{
						if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
						{
							//����ж��б�Ҫ��
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
						}
					}
					buf << "\t\t}\n";
				}
			}

			//����ع�
			buf << "\t\tfor(int index=" << stageMaxNum - 1 << ";index>=1;--index)\n";
			buf << "\t\t\tstage[index]=stage[index-1];\n";
			buf << "\t\tif(_stageNum==(MAX_ITER-1+" << stageMaxNum << "))\n";
			buf << "\t\t{\n";
			buf << "\t\t\tstage[0] = 0;\n";
			buf << "\t\t}\n";

			//��������
			//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\tworkerSync(" << index%nCpucore_ << ");\n";
			//�ȴ��ź���
			buf << "\tsem_wait(&zu" << graphIndex << ");\n";

			//���̵߳Ĵ����̲߳���Ҫ�ж�pipeline�������Ƿ��ſ�
			//��Ϊ�й̶��ĵ�������

			//��������
			//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\t}\n";
			buf << "}";
		}
	}
	else
	{
		//������ͼ
		if (isMainThreadInGraph(index))
		{
			//���߳�
			if (dsg_->combineList[graphIndex] == MERGEONE || dsg_->combineList[graphIndex] == MERGEDOWNONE || dsg_->combineList[graphIndex]
				== MERGEUPONE)
			{
				//ѹ����ͼ
				//������ͼ���߳�
				//������ͼ����
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

				//���������߳�,�����ӳ�Ա����graphIndex2CoreNum�л�ȡ
				buf << "\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";

				//����
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,true," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//���캯��,���ȫ������
				vector<FlatNode*> tmpactorset;
				//������ڱ��
				int bianhao = getindexinGraph(index);
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(bianhao);

				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//�ж��Ƿ�Ҫ��DSG����
					if (dsg_->getActorPosition(*iter) == FIRST)
					{
						//ֻ��һ�������
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
						//���һ�������
						string outName = getGraphOutName(graphIndex);
						pos1 = mapActor2InEdge.equal_range(*iter);
						if (outName != "")
						{
							buf << outName << ",";//û����߾Ͳ���ִ����һ��
						}
						while (pos1.first != pos1.second)
						{
							buf << pos1.first->second << ",";
							++pos1.first;
						}


					}
					else
					{
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter��������б�
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter��������б�
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
					buf.seekp((int)buf.tellp() - 1);		//ȥ��һ������
					buf << ");\n";
					if (MakeProfile)		//��profile���ۼ�ִ��ʱ��
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}


				//��ˮ��������
				//stage��ʾ�׶κ����飬��ʼ��0�ⶼΪ0
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//������ִ�н׶���Ŀ
				buf << "\tstage[0]=1;\n";
				buf << "\tbool out = true;\n";

				//��ʼ��
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
				}


				//��̬����
				set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
				set<int>::iterator endIter = ptempstagenum.end();

				buf << "\twhile(1)\n";
				buf << "\t{\n";
				for (int i = stageMaxNum - 1; i >= 0; i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);//�ڸ��߳�ִ�еĽ׶���Ѱ��
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
								//��ǰ�߳�Ϊ�����и����ӱ����ֵ��߳�
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
							}
						}
						buf << "\t\t}\n";
					}
				}

				
				//��ʼ����ع�
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";

				//��������
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,true," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//����STOP
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&label" << graphIndex - 1 << "==STOP&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t//�������ݲ��㣬��ʼ�ſ�\n";
				buf << "\t\t\tstage[0] = 0;\n";
				buf << "\t\t}\n";

				//���ν���
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t//����������һ��ֹͣ���ܻ�������\n";
				buf << "\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//����
				buf << "\t\t\tif(label" << graphIndex + 1 << "==WAITING||label" << graphIndex + 1 << "==FULL)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t//����ֹͣ\n";
				buf << "\t\t\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t\t\t}\n";
				buf << "\t\t\tlabel" << graphIndex << "=WAITING;\n";
				buf << "\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t\t\t//���Ѻ��ж�\n";
				buf << "\t\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t\t{\n";
				
				buf << "\t\t\t\tstage[0] = 0;\n";
				buf << "\t\t\t}\n";
				buf << "\t\t\telse\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//�������޸�״̬Ϊrunning
				buf << "\t\t\t\tlabel" << graphIndex << "=RUNNING;\n";
				buf << "\t\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";//����
				buf << "\t\t\t\t" << getGraphInName(graphIndex) << ".resetHead();\n";
				buf << "\t\t\t}\n";
				buf << "\t\t}\n";

				//���ν���
				//����MERGEONEһ����������ͼ
				buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex << ");\n";//����
				buf << "\t\t\tlabel" << graphIndex << "=FULL;\n";
				buf << "\t\t\tif(label" << graphIndex + 1 << "==WAITING||label" << graphIndex + 1 << "==FULL)\n";
				buf <<"\t\t\t{\n";
				buf << "\t\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "\t\t\t}\n";
				buf << "\t\t\tpthread_mutex_unlock(&mutex" << graphIndex << ");\n";	//����
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

				//������ˮ���жϿ�
				buf << "\t\tif(stage[0]==0&&pipeline" << graphIndex << "==true)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpipeline" << graphIndex << "=false;\n";
				buf << "\t\t}\n";

				//��ˮ��ֹͣ�ж�
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

				//���������߳�
				buf << "\t\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";

				//��������
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
				//������ͼ���߳�
				//ѹ����ͼ
				//������ͼ����
				buf << "\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\tlabel" << graphIndex<< "=RUNNING;\n";
				buf << "\tif(label" << graphIndex-1 << "!=STOP)\n";
				buf << "\t{\n";
				buf << "\t\t" << getGraphInName(graphIndex) << ".copy(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t}\n";
				buf << "\telse\n";
				buf << "\t{\n";
				buf << "\t\t//������ͼ���ݲ���,����ʣ������\n";
				buf << "\t\t" << getGraphInName(graphIndex) << ".append(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t{\n";
				buf << "\t\t\tlabel" << graphIndex << "=STOP;\n";
				if (graphIndex != (dsg_->staticChildGraph.size() - 1))
					buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
				buf << "\t\t\treturn ;\n";
				buf << "\t\t}\n";
				buf << "\t}\n";


				//���������߳�
				buf << "\tfor(int i = 1;i<" << graphIndex2coreNum.find(graphIndex)->second << ";++i)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";

				//����
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,false," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//���캯������
				//�ҵ�ȫ������
				vector<FlatNode*> tmpactorset;
				//��ȡ���
				int bianhao = getindexinGraph(index);
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(bianhao);

				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//�ж��Ƿ�Ҫ��DSG����
					if (dsg_->getActorPosition(*iter) == FIRST)
					{
						//ֻ��һ�������
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
						//���һ�������
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
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter��������б�
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter��������б�
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
					buf.seekp((int)buf.tellp() - 1);		//ȥ��һ������
					buf << ");\n";
					if (MakeProfile)		//��profile���ۼ�ִ��ʱ��
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}


				//��ˮ��������
				//stage��ʾ�׶κ����飬��ʼ��0�ⶼΪ0
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//������ִ�н׶���Ŀ
				buf << "\tstage[0]=1;\n";
				buf << "\tbool out = true;\n";


				//��ʼ��
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
				{
					buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
				}

				//��̬����
				//��ȡ�׶�
				set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
				set<int>::iterator endIter = ptempstagenum.end();

				buf << "\twhile(1)\n";
				buf << "\t{\n";
				for (int i = stageMaxNum - 1; i >= 0; i--)
				{
					set<int>::iterator stageiter = ptempstagenum.find(i);

					//�ҵ��ڸú���ִ�еĽ׶�
					if (stageiter != endIter)
					{
						buf << "\t\tif(stage[" << i << "])\n";
						buf << "\t\t{\n";
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
						//�ҵ��ڸý׶���ִ�е�����
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

				////��ʼ����ع�
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";

				//��������
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ",true,false," << dsg_->ssg2coreNum[graphIndex].second << ");\n";
				//����STOP����
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&label" << graphIndex - 1 << "==STOP&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t\t" << getGraphInName(graphIndex) << ".append(" << getGraphOutName(graphIndex - 1) << ");\n";
				buf << "\t\t\tif(" << getGraphInName(graphIndex) << ".notEnough())\n";
				buf << "\t\t\t\tstage[0] = 0;\n";
				buf << "\t\t}\n";


				//���ν���
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0&&label"<<graphIndex-1<<"!=STOP)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex-1<< ");\n";//����
				//FULL
				buf << "\t\t\tif(label" << graphIndex - 1 << "==FULL)\n";
				buf << "\t\t\t{\n";
				buf << "\t\t\t\t//����ֹͣ����\n";
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

				//���ν���
				if (graphIndex == (dsg_->scheStaticChildGraph.size() - 1))
				{
					//û������;
				}
				else
				{
					buf << "\t\tif(" << getGraphOutName(graphIndex) << "_obj.full())\n";
					buf << "\t\t{\n";
					buf << "\t\t\tpthread_mutex_lock(&mutex" << graphIndex - 1 << ");\n";//����
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

				//�����߳̿���
				buf << "\t\tif(stage[0]==0&&pipeline" << graphIndex << "==true)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpipeline" << graphIndex << "=false;\n";
				buf << "\t\t}\n";

				//��ˮ��ֹͣ�ж�
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

				//�����߳��ź���
				buf << "\t\tfor(int i = 0;i<" << stageMaxNum << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";
				
				//��������
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\t}\n";

				//ֹͣ
				buf << "\tlabel" << graphIndex << "=STOP;\n";
				buf << "\tcout<<\"" << graphIndex << "_" << getindexinGraph(index) << "\"<<endl;\n";
				buf << "}\n";

			}
			else
			{
				//��ѹ����ͼ���߳�
				//������ͼ����
				buf << "\tsem_wait(&exchange" << graphIndex << ");\n";
				buf << "\t//�ж�������ͼ�Ƿ��Ѿ�ֹͣ\n";
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


				//���������߳�
				buf << "\tfor(int i = 1;i<" << nCpucore_ << ";i++)\n";
				buf << "\t{\n";
				buf << "\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t}\n";

				//����
				//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ");\n";
				//���캯������
				//�����first����last������Ҫ�������캯��


				//�ҵ���Ӧ��ǰindex��ȫ������
				vector<FlatNode*> tmpactorset;
				tmpactorset = mplist[graphIndex]->findNodeSetInPartition(index%nCpucore_);
				
				//���캯������
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
				{
					buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
					//�ж��Ƿ�Ҫ��DSG����
					if (dsg_->getActorPosition(*iter) == FIRST)
					{
						//ֻ��һ�������
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
						//���һ�������
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
						pos1 = mapActor2InEdge.equal_range(*iter);		//iter��������б�
						pos2 = mapActor2OutEdge.equal_range(*iter);		//iter��������б�
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
					buf.seekp((int)buf.tellp() - 1);		//ȥ��һ������
					buf << ");\n";
					if (MakeProfile)		//��profile���ۼ�ִ��ʱ��
					{
						buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
					}
				}


				//��ˮ��������
				//stage��ʾ�׶κ����飬��ʼ��0�ⶼΪ0
				buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//������ִ�н׶���Ŀ
				buf << "\tstage[0]=1;\n";
				buf << "\tbool out = true;\n";


				//��ʼ��
				for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
				{
					buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
				}

				//��̬����ǰͬ��������
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
						vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);//��Ӧ�׶ε�����
						vector<FlatNode*>::iterator iter1;//����ѭ���ý׶�����
						for (iter1 = flatVec.begin(); iter1 != flatVec.end(); ++iter1)
						{
							if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
							{
								//��ǰ�߳���ŵ��ڸ����ӻ��ֵı�ţ���ʾ������ȷ���ڸ��߳���ִ��
								buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
							}
						}
						buf << "\t\t}\n";
					}
				}

				//��ʼ����ع�
				buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
				buf << "\t\t\tstage[index]=stage[index-1];\n";

				//��������
				//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
				buf << "\tmasterSync(" << nCpucore_ << ");\n";
				//������ͼͬ��
				//stop�ж�
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&label" << graphIndex - 1 << "==STOP&&stage[0]!=0)\n";
				buf << "\t\t\tstage[0] = 0;\n";
				

				//������ͼ�ж�
				buf << "\t\tif(" << getGraphInName(graphIndex) << ".notEnough()&&stage[0]!=0)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&exchange" << graphIndex - 1 << ");\n";
				buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";

				buf << "\t\t\t//�ȴ�����\n";
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

				//������ͼ�жϣ����������
				if (graphIndex == (dsg_->scheStaticChildGraph.size() - 1))
				{
					;
				}
				else
				{
					//������
					buf << "\t\tif(" << getGraphOutName(graphIndex) << ".full())\n";
					buf << "\t\t{\n";
					buf << "\t\t\tsem_post(&exchange" << graphIndex + 1 << ");\n";
					buf << "\t\t\tsem_wait(&exchange" << graphIndex << ");\n";
					buf << "\t\t\t" << getGraphOutName(graphIndex) << ".resetTail();\n";
					buf << "\t\t}\n";
				}

				//��ˮ���ſ�֪ͨ
				buf << "\t\tif(stage[0]==0&&pipeline" << graphIndex << "==true)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tpipeline" << graphIndex << "=false;\n";
				buf << "\t\t}\n";
					
				//��ˮ��ֹͣ�ж�
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


				//�߳�ͬ��
				buf << "\t\tfor(int i = 1;i<" << nCpucore_ << ";++i)\n";
				buf << "\t\t{\n";
				buf << "\t\t\tsem_post(&zu" << graphIndex << ");\n";
				buf << "\t\t}\n";

				//��������
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
			//������ͼ�����߳�
			buf << "\tsem_wait(&zu" << graphIndex << ");\n";

			//����
			//buf << "\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\tworkerSync(" << index%nCpucore_ << ");\n";
			//���캯��
			//���������̬�ߣ�����Ҫע�⣬�����߳�������Ϊ����ͼ��first����last
			//�ҵ���Ӧ��ǰindex��ȫ������
			vector<FlatNode*> tmpactorset;
			//tmpactorset = mplist[graphIndex]->findNodeSetInPartition(index%nCpucore_);
			tmpactorset = mplist[graphIndex]->findNodeSetInPartition(getindexinGraph(index));

			for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); ++iter)
			{
				buf << "\t" << (*iter)->name << " " << (*iter)->name << "_obj(";
				if (dsg_->getActorPosition(*iter) == FIRST)
				{
					//ֻ��һ�������
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
					//���һ�������
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
					pos1 = mapActor2InEdge.equal_range(*iter);		//iter��������б�
					pos2 = mapActor2OutEdge.equal_range(*iter);		//iter��������б�
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
				buf.seekp((int)buf.tellp() - 1);		//ȥ��һ������
				buf << ");\n";

				if (MakeProfile)		//��profile���ۼ�ִ��ʱ��
				{
					buf << "\tdouble " << (*iter)->name << "_time = 0;" << endl;
				}
			}
			//��ˮ��������
			//stage��ʾ�׶κ����飬��ʼ��0�ⶼΪ0
			buf << "\tchar stage[" << stageMaxNum << "]={0};\n";//������ִ�н׶���Ŀ
			buf << "\tstage[0]=1;\n";
			buf << "\tbool out = true;\n";


			//��ʼ��
			for (vector<FlatNode*>::iterator iter = tmpactorset.begin(); iter != tmpactorset.end(); iter++)
			{
				buf << "\t" << (*iter)->name << "_obj.runInitScheduleWork();\n";
			}

			

			//����ڸú���ִ�еĽ׶μ���
			set<int> ptempstagenum = mapNum2StageList[graphIndex].find(graphNum)->second;
			set<int>::iterator endIter = ptempstagenum.end();
			buf << "\twhile(1)\n";
			buf << "\t{\n";
			for (int i = stageMaxNum - 1; i >= 0; i--)
			{
				//���жϸý׶��Ƿ��ڸú���
				set<int>::iterator stageiter = ptempstagenum.find(i);
				if (stageiter != endIter)
				{

					//�ҵ��ڸý׶���ִ�е�����
					buf << "\t\tif(stage[" << i << "])\n";
					buf << "\t\t{\n";
					vector<FlatNode*> flatVec = psalist[graphIndex]->FindActor(i);
					vector<FlatNode*>::iterator iter1;
					for (iter1 = flatVec.begin(); iter1 != flatVec.end(); ++iter1)
					{

						if (graphNum == mplist[graphIndex]->findPartitionNumForFlatNode(*iter1))
						{
							//��ǰ�߳���ŵ��ڸ����ӻ��ֵı�ţ���ʾ������ȷ���ڸ��߳���ִ��
							buf << "\t\t\t" << (*iter1)->name << "_obj.runSteadyScheduleWork();\n";
						}
					}
					buf << "\t\t}\n";
					}
			}

			//����ع�
			buf << "\t\tfor(int index = " << stageMaxNum - 1 << ";index>=1;--index)\n";
			buf << "\t\t\tstage[index]=stage[index-1];\n";
				
			
			//��������
			//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			buf << "\tworkerSync(" << index%nCpucore_ << ");\n";
			//��ˮ��ֹͣ�ж�
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

			//��ˮ���ſ��ж�
			buf << "\t\tif(pipeline" << graphIndex << "==false&&stage[0]!=0)\n";
			buf << "\t\t{\n";
			buf << "\t\t\tstage[0] = 0;\n";
			buf << "\t\t}\n";
			//��������
			//buf << "\t\tpthread_barrier_wait(&barrier" << graphIndex << ");\n";
			
			buf << "\t}\n";
			buf << "\tcout<<\"" << graphIndex << "_" << getindexinGraph(index) << "\"<<endl;\n";
			buf << "}\n";
		}
		

	}
}
void DynamicX86CodeGenerate::DYAllActorHeader()		//�������������ļ�ͷ�ļ��ļ����ļ�
{
	vector<FlatNode*>::iterator iter;
	stringstream buf, ss;
	buf << "/*��������actor��ͷ�ļ�����Ҫ��Ϊ�˷������ļ�����*/\n\n";
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
		int len = ListLength(flatNodes_[i]->oldContents->decl->u.decl.type->u.operdcl.outputs);		//�������Ŀ
		int nOut = flatNodes_[i]->nOut;
		//������һ�������жϣ�������len�ǵ���nOut��
		OperatorType ot = flatNodes_[i]->oldContents->ot;
		//���ot���������ӵ�����
		//ԭʼ�������pos��ȡû������
		string name = flatNodes_[i]->name;
		int index = name.find_first_of("_");
		string tmp = name.substr(0, index);//�����������

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
void DynamicX86CodeGenerate::DYEdgeParam(FlatNode *actor, stringstream &buf)		//�ú������ɻ�������Ա����
{
	buf << "\t// make output datamember\n";
	vector<FlatNode*>::iterator iter, end;	//����producer��consumer

	//��ȡ�����
	iter = actor->outFlatNodes.begin();
	end = actor->outFlatNodes.end();

	//��ȡ���������
	List* outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;		//����ߵ��б�
	ListMarker output_maker;
	Node*outputNode = NULL;
	IterateList(&output_maker, outputList);
	while (iter != end)
	{
		NextOnList(&output_maker, (GenericREF)&outputNode);		//ת��Ϊvoid**
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl)outputString = outputNode->u.decl.name;
		if (dsg_->getActorPosition(actor) == LAST)//ʹ�ö�̬����
		{
			//����First����last��λ�ã����������ֻ��LAST��Ҫʹ�ö�̬���壬������Ҫ�ж��Ƿ���Sink
			//�����First����Ҫ�ж��ǲ���ѹ�����ӵ����Σ������ж��Ƿ���sink
			//Ϊlast�ж��Ƿ���Sink
			if (dsg_->isSink(actor))	//��sink
				buf << "\tProducer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << ">" << outputString << ";\n";
			else
				buf << "\tDSGProducer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << "> &" << outputString << ";\n";
		}
		else
			buf << "\tProducer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << ">" << outputString << ";\n";
		iter++;
	}


	buf << "\t// make input datamember\n";
	//��ȡ�����
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

		if (dsg_->getActorPosition(actor) == FIRST)		//��ͼ��������
		{
			//�ж��Ƿ���source
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
	//����actor�仯������CombineState������λ�����ж���Ҫʹ��ʲô���Ļ�����
	//ͬʱinclude�Ŀ�ҲҪ����
	stringstream buf;//��������actorͷ�ļ����ַ�����
	stringstream srcBuf;//��������actorԴ�ļ����ַ�����ʵ�ʻ����ò���
	vector<string>::iterator iter;
	string className = actor->name;
	assert(actor);
	cout << actor->name << "      " << endl;
	actor->SetIOStreams();		//�ú�������nPeek/nPop/nPushString

	//ͷ�ļ�
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
		buf << "namespace COStream{\n";//������ֿռ�,�ⲿ��Ŀǰ������
	//�ඨ�忪ʼ
	buf << "class "<< className << "{\n";

	//ͨ�ű߶���
	DYEdgeParam(actor, buf);
	if (actor == readerActor)
	{

	}
	else if (actor == writerActor)
	{

	}
	else
	{
		//��ʼд����˽�г�Ա
		parameterBuf.str("");		//���parammeterBuf,thisBuf
		thisBuf.str("");
		buf << "private:\n";

		DYdeclList(actor, ot, buf);		//д���﷨�������Ϣ����ʵ��Ҫ�Ƕ������̬���ȴ���
		DYinitVarAndState(actor, ot, buf);//д����work���һЩ����������

		DYlogicInit(actor, ot, buf);	//init������ûinit�����ᴴ��һ���պ���	

		DYpopToken(actor, buf);			//дpop������������

		DYpushToken(actor, buf);		//дpush������������

		//д��initwork����
		DYinitWork(buf);

		//д��work����
		DYwork(actor, ot, buf);

	}
	
	//����ÿ��actor�ĳ�Ա�������������캯������̬���ȣ���̬����
	buf << "public:\n";
	//���캯��
	DYthis(actor, ot, buf);
	//��̬���Ⱥ�������
	DYrunInitScheduleWork(actor,buf);
	//��̬���Ⱥ�������
	DYrunSteadyScheduleWork(actor, buf);
	buf << "};\n";

	
	if (CallModelEmbed)
		buf << "}\n";//���ֿռ�����
	//����Դ�ļ�����
	srcBuf << "#include \"" << actor->name << ".h\"\n";
	for (iter = staticNameInit.begin(); iter != staticNameInit.end(); ++iter)
		srcBuf << (*iter);
	//������ļ�
	stringstream headerFileName, srcFileName;	//�ֱ𱣴�ͷ�ļ�����Դ�ļ�����Դ�ļ������˸�actor�ľ�̬��Ա
	headerFileName << dir_ << className << ".h";
	srcFileName << dir_ << className << ".cpp";
	OutputToFile(headerFileName.str(), buf.str());	//ͷ�ļ�
	if (staticNameInit.size() != 0)		//��������̬��Ա��������Դ�ļ���ʼ�����Ա
		OutputToFile(srcFileName.str(), srcBuf.str());
	staticNameInit.clear();
}

void DynamicX86CodeGenerate::DYpopToken(FlatNode*actor, stringstream&buf)
{
	//������
	//���actorλ�ò���first������������poptoken�������ж��Ƿ�source��Ȼ��͵���DSGComsumer�ĺ���
	buf << "\t//popToken\n";
	buf << "\tvoid popToken(){";
	//ע�⣬���ڶ�̬����ͨ�űߣ���������Ψһ������ߣ�������ֶ�������Ǵ�ġ�
	List*inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	

	ListMarker input_maker;
	
	Node *inputNode = NULL;
	
	//����ߵ������ǹ̶��ģ����Բ����ڱ仯

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
	//������
	//���actorλ�ò���last������������pushtoken�������ж��Ƿ�sink��Ȼ��͵���DSGProducer�ĺ���
	buf << "\t// pushToken\n";
	buf << "\tvoid pushToken(){";
	List *outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;		//����б�
	ListMarker output_maker;
	Node* outputNode = NULL;
	IterateList(&output_maker, outputList);
	vector<int>::iterator iter = actor->outPushWeights.begin();		//����ߵĴ��ڵ�����

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

	/*if (dsg_->getActorPosition(actor) == LAST)		//Ϊ��ͼ�������
	{
		if (dsg_->isSink(actor))		//����sink���������������Producer
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
			assert(ListLength(outputList) == 1);//��Ϊ��̬��ͼ��LAST���뱣ֻ֤��һ�������
			//DSGProducer

		}
	}
	else	//��ͼ�з�LAST������actor
	{
		while (NextOnList(&output_maker, (GenericREF)&outputNode))
		{
			string outputString;
			if (outputNode->typ == Id)outputString = outputNode->u.id.text;
			else if (outputNode->typ == Decl)outputString = outputNode->u.decl.name;
			buf << "\n\t" << outputString << ".updatetail" << "(" << *iter << ");\n";
			iter++;//��һ������
		}
	}
		
		*/
}

void DynamicX86CodeGenerate::DYthis(FlatNode *actor, OperatorType ot, stringstream &buf)
{
	//�ж�actorλ�ã������FIRST����LAST���Ҳ���source����sink����Ҫ������Ӧ�»��������캯������Ӳ���
	//���캯����ͬ
	//DSGProducer��DSGConsumer����ʹ��ԭʼBuffer������ʹ�õĲ�����ȫ�ֱ���
	buf << "\t //Constructor\n";
	buf << "\t" << actor->name << "(";//���캯������
	vector<FlatNode*>::iterator iter, end;

	//���캯������
	//���������
	iter = actor->outFlatNodes.begin();
	end = actor->outFlatNodes.end();

	List*outputList = actor->contents->decl->u.decl.type->u.operdcl.outputs;

	
	ListMarker output_maker;
	Node *outputNode = NULL;
	IterateList(&output_maker, outputList);
	while (iter != end)		//ѭ�������
	{
		NextOnList(&output_maker, (GenericREF)&outputNode);
		string outputString;
		if (outputNode->typ == Id) outputString = outputNode->u.id.text;
		else if (outputNode->typ == Decl)outputString = outputNode->u.decl.name;//�������ߵ�����
		
		//��Ҫ���ж��Ƿ�����ͼ��β����������
		if (dsg_->getActorPosition(actor) == LAST||dsg_->getActorPosition(actor)==BOTH)
		{
			buf << "DSGProducer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << ">&" << outputString << ",";
		}
		else
			buf << "Buffer<" << pEdgeInfo->getEdgeInfo(actor, (*iter)).typeName << ">&" << outputString << ",";
		iter++;
	}


		//���������
	iter = actor->inFlatNodes.begin();
	end = actor->inFlatNodes.end();

	//��ȡ���������
	List*inputList = actor->contents->decl->u.decl.type->u.operdcl.inputs;
	ListMarker input_maker;
	Node* inputNode = NULL;
	IterateList(&input_maker, inputList);
	
	while (iter != end)
	{
		NextOnList(&input_maker, (GenericREF)&inputNode);
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl)inputString = inputNode->u.decl.name;//���������PStream1_06_0���ֵ�
		if (dsg_->getActorPosition(actor) == FIRST || dsg_->getActorPosition(actor) == BOTH)
		{
			buf << "DSGConsumer<" << pEdgeInfo->getEdgeInfo((*iter),actor).typeName << ">&" << inputString << ",";
		}
		else
			buf << "Buffer<"<<pEdgeInfo->getEdgeInfo((*iter), actor).typeName << ">&" << inputString << ",";
		iter++;
	}
	//���ϲ��������˹��캯���ģ����ڵ��βΣ�����Ҫ����Ĭ�ϳ�ʼ���б�

	//�ⲿ����Ҫ����
	buf << parameterBuf.str();
	if (parameterBuf.str() == "")
	{
		buf.seekp((int)buf.tellp() - 1);//��������ļ�����ָ��λ�ã�tellp�����������������ָ��λ��
		//����仯��ԭ������Ϊǰ���������ߵ�ʱ�򣬶�����һ����
	}
	else
	{
		buf.seekp((int)buf.tellp() - 2);
	}
	buf << "):";

	//��ʼ����Ĭ�ϳ�ʼ���б�
	//����ߣ�����LAST�ڵ�Ҫ��������
	IterateList(&output_maker, outputList);
	while (NextOnList(&output_maker, (GenericREF)&outputNode))
	{
		string outputString;
		if (outputNode->typ == Id)outputString = outputNode->u.id.text;
		else if (outputNode->typ = Decl) outputString = outputNode->u.decl.name;
		//�ж�//�ؼ��ǣ�����LAST�ڵ㣬������sink�����������������ط�������Ϊ�˷�ֹ���˹���д������д�Ϻ���
		if (dsg_->getActorPosition(actor) == LAST || dsg_->getActorPosition(actor) == BOTH)
		{
			if (dsg_->isSink(actor))		//SDF�����һ��
			{
				buf << outputString << "(" << outputString << "),";
			}
			else
			{
				assert(ListLength(outputList) == 1);
				//������Ƕ�̬����Ҫ��ȡ��һ��ͼ����ʼactor������������
				int dataInUse = dsg_->getNextGraphFirstNodePop(actor);
				//buf << outputString << "(" << outputString << "," << dataInUse << "," << batch << "),";
				buf << outputString << "(" << outputString << "),";
			}

		}
		else
			buf << outputString << "("<<outputString << "),";
	}
	
	//����ߣ�����FIRST�ڵ�Ҫ��������,����source��û�п��ܲ�������ߣ��������������ѭ��
	IterateList(&input_maker, inputList);
	while (NextOnList(&input_maker,(GenericREF)&inputNode))
	{
		string inputString;
		if (inputNode->typ == Id)inputString = inputNode->u.id.text;
		else if (inputNode->typ == Decl)inputString = inputNode->u.decl.name;
		//�ж�,����ߣ������FIRST������source����Ҫʹ���»�����
		if (dsg_->getActorPosition(actor) == FIRST || dsg_->getActorPosition(actor) == BOTH)
		{
			//����Ҫ��ȡ�����ӵ�������������
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
		//û�������
		buf.seekp((int)buf.tellp() - 1);
		//ȥһ������
	}*/
	buf << "{\n";
	buf << thisBuf.str();

	buf << "\t\tsteadyScheduleCount = " << dsg_->getSteadyCount(actor) << ";\n";
	buf << "\t}\n";


}
void DynamicX86CodeGenerate::DYrunInitScheduleWork(FlatNode*actor, stringstream &buf)
{
	//�ж�actorλ�ã������FIRST����LAST���Ҳ���source����sink���������⻺�������̬��һ��'
	//���ڶ�̬�����̬���������һЩ�����ĳ�ʼ������������work
	buf << "\t//runInitScheduleWork\n";
	buf << "\tvoid runInitScheduleWork(){\n";
	buf << "\t\tinitWork();\n";
	buf << "\t}\n"; // CGrunInitScheduleWork��������

}
//���ڶ�̬��Producer��ÿ��work����[]��updatetail����̬һ�β���ִ��resetTail��������������ʱ�����[]������
//���ڶ�̬��Consumer��ÿ����ִ̬��N��updatehead��������������������̺߳�����ִ�У�����Ҳû��Ҫִ��resetHead��ֻҪ����
//���ӱ����ѷ��������ݶ����Ż�resethead
void DynamicX86CodeGenerate::DYrunSteadyScheduleWork(FlatNode*actor, stringstream &buf)
{
	//�ж�actorλ�ã������FIRST����LAST���Ҳ���source����sink���������⻺��������̬��һ��
	buf << "\t//runSteadyScheduleWork\n";
	buf << "\tvoid runSteadyScheduleWork(){\n";
	if (AmplifySchedule)
	{
		buf << "\t\tfor(int j=0;i<AMPLIFYFACTOR;j++)\n\t\t{\n";
	}
	buf << "\t\tfor(int i = 0;i<steadyScheduleCount;i++)\n\t\twork();\n";
	if (NoCheckBuffer)
	{
		//����
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
				//��̬�߲���ҪresetTail;
			}
			else
				buf << "\t\t" << outputString << ".resetTail();\n";
		}

		//���
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
	buf << "\t}\n"; // // CGrunSteadyScheduleWork��������
}










//���ߺ���
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
		if (isprint(val)) //�ж�val�Ƿ�Ϊ�ɴ�ӡ�ַ�
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


	//������ļ�
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
//string DynamicX86CodeGenerate::GetPrimDataType(Node *from)//���Ͷ���
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
	Node *initNode = from->u.decl.init;//������ʼ��node
	string name = from->u.decl.name;
	string arrayType = GetArrayDataType(tmpNode->u.adcl.type);
	string dim = GetArrayDim(tmpNode->u.adcl.dim);
	bool isGlobal = FALSE;
	if (initNode == NULL) //���û�г�ʼ�������������ͽ��г�ʼ��
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
	else//������ڳ�ʼ�����ʼ��Ϊָ��ֵ
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
		if (n == 1) // �����ʼ������1����һֵʱ�����������г�Ա��ʼ��Ϊ��ֵ
		{
			Node *item = (Node *)FirstItem(tmp);
			if (item->typ == Const)
			{
				declInitList << " = ";
				RecursiveAdclInit(tmp);
				declInitList << ";\n";
			}
		}
		else //��ʼ�ĸ���������ά��һ�£�����Բ�ȡ�����ĸ�ֵ��ʽ��val pp:Array[Int](1) = [1,2,3,4,5];
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
	//�������node
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
		// declInitList<< c->u.Const.value.u;//splûָ�룬���Ժ���
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
//	if (node->typ == Prim) //��������
//	{
//		type = GetPrimDataType(node);
//	}
//	else if (node->typ == Adcl) // Ҳ�Ǹ�������ݹ��������
//	{
//		stringstream ss;
//		ss << GetArrayDataType(node->u.adcl.type);
//		type = ss.str();
//	}
//	else // �������ĳ�Ա�Ǹ������ͣ����д���չ
//	{
//		Warning(1, "this arrayDataType can not be handle!");
//		type = "Any";// ��ʱ����һ��ͨ������
//		UNREACHABLE;
//	}
//	return type;
//}


void DynamicX86CodeGenerate::ExtractDeclVariables(Node *from)
{
	stringstream tempdeclList, tempdeclInitList;
	if (from->u.decl.type->typ == Prim) // ��������
	{
		Node *typeNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string type = GetPrimDataType(typeNode);
		string name = from->u.decl.name;
		char tempvalude[20];
		declList << "\t" << type << " " << name << ";\n";
		if (initNode) // ���ڳ�ʼ������г�ʼ��
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
			sdim << dim;
			int dimNum;
			bool dynamicArray = false;

			if (sdim >> dimNum)		//Ϊ����
				declList << "\t" << arrayType << " " << name << "[" << dim << "]";
			else		//����ά����ȷ����ʹ��new��̬����
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
		else//������ڳ�ʼ�����ʼ��Ϊָ��ֵ
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
			if (n == 1) // �����ʼ������1����һֵʱ�����������г�Ա��ʼ��Ϊ��ֵ
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
			else //��ʼ�ĸ���������ά��һ�£�����Բ�ȡ�����ĸ�ֵ��ʽ��val pp:Array[Int](1) = [1,2,3,4,5];
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
	else if (from->u.decl.type->typ == Ptr) // ָ�룬ֻ�ܳ�����param��
	{
		Node *typeNode = from->u.decl.type;
		Node *initNode = from->u.decl.init;
		string type;
		if (typeNode->u.ptr.type->typ == Prim)
			type = GetPrimDataType(typeNode->u.ptr.type);
		else if (typeNode->u.ptr.type->typ == Tdef)
			type = typeNode->u.ptr.type->u.tdef.name;
		else UNREACHABLE;			//�������͵��ݲ�֧��

		string name = from->u.decl.name;
		char tempvalude[20];
		declList << "\t" << type << " " << "*" << name << ";\n";
		if (initNode) // ���ڳ�ʼ������г�ʼ��
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
//��ά�����ʼ�����̣��ݹ飩
void DynamicX86CodeGenerate::RecursiveAdclInit(List *init)
{
	//��ά������init��һ����ά������
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
		else if (item->typ == Const) // ��������Ա�ǻ�������
		{
			if (i != 1) declInitList << ",";
			COSX86_Node(item, 0);
		}
		else if (item->typ == Initializer)//��������Ա��һ��������ݹ�
		{
			RecursiveAdclInit(item->u.initializer.exprs);
			if (i != len)
			{
				declInitList << ",";
				OutputCRSpaceAndTabs(4);
			}
		}
		else if (item->typ == ImplicitCast)//�������͵���ʽת��
		{
			if (i != 1) declInitList << ",";
			COSX86_Node(item->u.implicitcast.value, 0);
		}
		i++;
	}
	declInitList << "}";
}


//ȡ�����ά��
string DynamicX86CodeGenerate::GetArrayDim(Node *from)
{
	string dim;
	if (from->typ == Const)//���ά���ڵ�����Ϊ����������a[10]
	{
		if (from->u.Const.text)dim = from->u.Const.text;
		else
		{
			char *tmpdim = (char *)malloc(20);
			sprintf(tmpdim, "%d", from->u.Const.value.l);//20120322 zww���
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
		string tmp = declInitList.str(); // ����
		stringstream tmp2;

		declInitList.str("");
		COSX86_Node(from, 0);
		tmp2 << "(" << declInitList.str() << ")";
		dim = tmp2.str();
		declInitList.str("");
		declInitList << tmp; // �ָ� 
	}
	else
		UNREACHABLE;

	return dim;
}













//\----------------------------------------------------------------------

void DynamicX86CodeGenerate::COSX86_Node(Node *node, int offset)
{
	if (node == NULL) return;

	if (node->parenthesized == TRUE) declInitList << "("; //�����ű�֤�߼���

#define CODE(name, node, union) COSX86_##name(node, union, offset)
	ASTSWITCH(node, CODE)
#undef CODE

	if (node->parenthesized == TRUE) declInitList << ")"; //�����ű�֤�߼���
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
void DynamicX86CodeGenerate::COSX86_Binop(Node *node, binopNode *u, int offset)//�Էǽṹ�����͵�Stream���м�,lihe,2012-09-04
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
	if (node->u.cast.type->typ == Ptr&&node->u.cast.expr->typ == Const){//��Ӷ�NULL���ж�
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
	isInComma = true;//�����ڶ��ű��ʽ��
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
	while (tmp != NULL)//�����Ƕ�ά���飬��Ҫ����dim���list
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
		int flag = 1;//��ʶ�Ƿ������
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
			declInitList << u->name->u.id.text;//���������
		}
		if (flag == 1)
		{
			declInitList << "(";
			OutputArgList(u->args, offset);//����
			declInitList << ")";
			if (strcmp(ident, "frta") == 0)
				declInitList << ";";
		}
		else if (flag == 2)
		{
			OutputArgList(u->args, offset);//����
			declInitList << "<<endl";
		}
		else
		{
			OutputArgList(u->args, offset);//����
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
		/*ɾ��Enum����δ��ʼ��ʱ�ġ�=��*/
		string orignal = globalvarbuf.str();
		//���declInitList
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
	if (u->stmt->typ != Block) // ����Ƿ�block��㣬����Ҫ���ж���
	{
		OutputCRSpaceAndTabs(offset + 1);
	}
	COSX86_Node(u->stmt, offset + 1);
	if (u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//����Ǳ��ʽ�������Ҫ��ĩλ��ӷֺű�ʾ����
		declInitList << ";";
}

void DynamicX86CodeGenerate::COSX86_IfElse(Node *node, IfElseNode *u, int offset)
{
	declInitList << "if (";
	COSX86_Node(u->expr, offset);
	declInitList << ")";
	if (u->true_->typ != Block)//����Ƿ�block��㣬����Ҫ���ж���
	{
		OutputCRSpaceAndTabs(offset + 1);
	}
	COSX86_Node(u->true_, offset);
	if (u->true_->typ == Binop || u->true_->typ == Unary || u->true_->typ == Ternary || u->true_->typ == Call || u->true_->typ == Decl)//����Ǳ��ʽ�������Ҫ��ĩλ��ӷֺű�ʾ����
		declInitList << ";";
	OutputCRSpaceAndTabs(offset);
	declInitList << "else ";
	if (u->false_->typ != Block && u->false_->typ != IfElse)//����Ƿ�block������ifelse��㣬����Ҫ���ж���
	{
		OutputCRSpaceAndTabs(offset + 1);
	}
	COSX86_Node(u->false_, offset);
	if (u->false_->typ == Binop || u->false_->typ == Unary || u->false_->typ == Ternary || u->false_->typ == Call || u->false_->typ == Decl)//����Ǳ��ʽ�������Ҫ��ĩλ��ӷֺű�ʾ����
		declInitList << ";";
}

void DynamicX86CodeGenerate::COSX86_While(Node *node, WhileNode *u, int offset)
{
	declInitList << "while (";
	COSX86_Node(u->expr, offset);
	declInitList << ")";
	if (u->stmt->typ != Block)//����Ƿ�block��㣬����Ҫ���ж���
	{
		OutputCRSpaceAndTabs(offset + 1);
	}
	COSX86_Node(u->stmt, offset);
	if (u->stmt->typ == Binop || u->stmt->typ == Unary || u->stmt->typ == Ternary || u->stmt->typ == Call || u->stmt->typ == Decl)//����Ǳ��ʽ�������Ҫ��ĩλ��ӷֺű�ʾ����
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

//�ú�������ԴCOStream�������漰��forѭ���Ĳ��֣�ת��ΪC++�ļ��е�forѭ��
void DynamicX86CodeGenerate::COSX86_For(Node *node, ForNode *u, int offset)
{
	declInitList << "for (";
	COSX86_Node(u->init, offset);
	declInitList << " ; ";
	COSX86_Node(u->cond, offset);
	declInitList << " ; ";
	COSX86_Node(u->next, offset);
	declInitList << ")";
	if (u->stmt->typ != Block)//����Ƿ�block��㣬����Ҫ���ж���
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
	cout << "return ����������*********************************************" << endl;
}

void DynamicX86CodeGenerate::COSX86_Block(Node *node, BlockNode *u, int offset)
{
	declInitList << "\t\t{\n";
	//���decl
	COSX86_List(u->decl, offset);
	declInitList << "\n"; // ����һ��

	OutputStmtList(u->stmts, offset);

	//OutputCRSpaceAndTabs(offset);
	declInitList << "\t\t}\n"; // '}'��ռһ��
}

void DynamicX86CodeGenerate::COSX86_Prim(Node *node, primNode *u, int offset)
{
	if (!isstructVar && !isUnionVar && !isEnumVar)
		declInitList << GetPrimDataType(node);
	if (!isstructVar && !isUnionVar && !isEnumVar)//ʹ�ṹ����������extern
		globalvarbuf << "extern ";
	if (isEnumVar == false)
		globalvarbuf << GetPrimDataType(node) << " ";
}

void DynamicX86CodeGenerate::COSX86_Tdef(Node *node, tdefNode *u, int offset)
{
	cout << "typedef ����������*********************************************" << endl;

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
	if (!isstructVar && !isUnionVar && !isEnumVar)//ʹ�ṹ����������extern
		globalvarbuf << "extern ";
	globalvarbuf << arrayType << " ";
}

//2015-05-08��Ӻ���������������ģ��ʵ��
void DynamicX86CodeGenerate::COSX86_Fdcl(Node *node, fdclNode *u, int offset)
{
	//��1�����º�������������¼
	n_fdcl++;
	//��2����ȡ������������ֵ����
	string funType = GetArrayDataType(node->u.fdcl.returns);
	//��3����ȡ�������������б�
	ListMarker argList;
	IterateList(&argList, node->u.fdcl.args);
	Node *item = NULL;
	//��4���������������Ϣд��globalvarbuf��
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

		isstructVar = true;//���Ʊ��������

		COSX86_List(u->type->fields, 0);

		globalvarbuf << "};";
		//declInitList<<sdclType<<" ";//�����������
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
		isUnionVar = true;//���Ʊ��������
		COSX86_List(u->type->fields, 0);
		isUnionVar = false;
		globalvarbuf << "};";
	}
	else
		declInitList << udclType << " ";//�����������
}

void DynamicX86CodeGenerate::COSX86_Edcl(Node *node, edclNode *u, int offset)
{

	string edclType = u->type->name;
	if (isCGGlobalVar)
	{
		globalvarbuf << "enum ";
		globalvarbuf << edclType << "{\n";
		isEnumVar = true;//���Ʊ��������
		COSX86_List(u->type->fields, 0);
		isEnumVar = false;

		/*ɾ�����һ������ķֺ�*/
		string orignal = globalvarbuf.str();
		//���declInitList
		globalvarbuf.str("");
		orignal = orignal.substr(0, orignal.size() - 2);
		globalvarbuf << orignal << "\n};\n";
	}
	else
		declInitList << edclType << " ";//�����������
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
			if (u->type->typ == Adcl)	//��ά����
			{
				Node *tempNode = u->type;
				globalvarbuf << node->u.decl.name;
				if (flag_Global)			//�����GlobalVar�У�Ϊ�����������ȫ�ֱ���
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
			else		//����
			{
				if (isExternType)
					ExternTypeBuf << node->u.decl.name << ";\n";//��������
				else {
					declInitList << " " << node->u.decl.name;   //������
					temp_declInitList << declInitList.str() << ";\n";
					globalvarbuf << node->u.decl.name << ";\n";//��������
				}
			}

			if (u->type->typ == Adcl)
			{
				if (STORAGE_CLASS(node->u.decl.tq) != T_EXTERN)
					AdclInit(node, offset);		//��ʼ������
			}
			else
			{
				if (node->u.decl.init)
				{
					if (u->type->typ == Prim && u->type->u.prim.basic == Char)//����Ǹ��ַ�����
					{
						declInitList << " = " << u->init->u.implicitcast.expr->u.Const.text;
					}
					else
					{
						declInitList << " = ";			//��ʼ������ ��int a = 2;
						COSX86_Node(u->init, offset);
					}
				}
			}
			if (u->type->typ != Adcl)
				declInitList << ";";
		}
	}

	///*���isstructVar�������ڿ���Sturct�����ڳ�GlobalVar.h�������ļ�������*/
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
	//	if (u->type->typ == Adcl)	//��ά����
	//	{
	//		Node *tempNode = u->type;
	//		globalvarbuf << node->u.decl.name;
	//		if (!isstructVar && !isUnionVar && !isEnumVar)
	//		{
	//			if (flag_Global)			//�����GlobalVar�У�Ϊ�����������ȫ�ֱ���
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
	//	else if (u->type->typ == Fdcl){ //2015-05-08 ��� 
	//		int count = 0;
	//		//��ȡȫ�ֺ�����,�����������뵽"("ǰ��
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
	//			//�ж��Ƿ���ĵ�ǰ��������
	//			if (count == n_fdcl)
	//				break;
	//			else
	//				continue;
	//		}
	//		//���յ�������Ϣ
	//		string declInfo(temp);
	//		//��ú��������ı�׼��ʽ�ڣ�֮ǰ���뺯�������ڣ�֮ǰɾ������ģ���
	//		declInfo = temp.substr(0, begin + 1) + node->u.decl.name + temp.substr(begin + 1, end - begin - 1) + temp.substr(end + 1, temp.size() - end);
	//		globalvarbuf.str("");
	//		globalvarbuf << declInfo;
	//	}

	//	else if (node->u.decl.type->typ == Sdcl || node->u.decl.type->typ == Udcl || node->u.decl.type->typ == Edcl&&isCGGlobalVar)//����SUE���͵Ľڵ�
	//	{
	//		if (isCGGlobalVar)
	//		{/*ɾ�������һ���ֺţ���������*/
	//			string orignal = globalvarbuf.str();
	//			//���declInitList
	//			globalvarbuf.str("");
	//			orignal = orignal.substr(0, orignal.size() - 1);
	//			globalvarbuf << orignal;
	//			globalvarbuf << node->u.decl.name << ";\n";
	//		}
	//		else
	//		{
	//			declInitList << " " << node->u.decl.name << ";";   //������

	//			temp_declInitList << declInitList.str() << ";\n";
	//		}
	//	}
	//	else{
	//		if (!isstructVar && !isUnionVar &&!isEnumVar)//��ֹSUE���͵ı�����Globalvar.cpp�ļ���
	//			declInitList << " " << node->u.decl.name;   //������

	//		temp_declInitList << declInitList.str() << ";\n";
	//		globalvarbuf << node->u.decl.name;
	//		if (isEnumVar == false)
	//			globalvarbuf << ";\n";//��������				
	//	}//if
	//}

	///*��ʼ������*/
	//if (u->type->typ == Adcl)
	//{
	//	if (STORAGE_CLASS(node->u.decl.tq) != T_EXTERN)
	//	if (!isstructVar && !isUnionVar)
	//		AdclInit(node, offset);		//��ʼ������
	//}
	//else
	//{
	//	if (node->u.decl.init)
	//	{
	//		if (u->type->typ == Prim && u->type->u.prim.basic == Char)//����Ǹ��ַ�����
	//		{
	//			globalvarbuf << " = " << u->init->u.implicitcast.expr->u.Const.text;
	//		}
	//		else
	//		{
	//			if (isEnumVar == true)
	//				globalvarbuf << " = ";			//��ʼ������ ��int a = 2;

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
	//if (isEnumVar == true)   //���г�ʼ��ʱ�����������ʼ��ֵ���������
	//	globalvarbuf << ",\n";
}
void DynamicX86CodeGenerate::COSX86_Attrib(Node *node, attribNode *u, int offset)
{
}

//2015-05-08��Ӻ��������������ģ��ʵ��      
void DynamicX86CodeGenerate::COSX86_Proc(Node *node, procNode *u, int offset)
{
	//��1�����������е��˴������ı�־flagֵΪtrue����ΪdeclInitLen��ֵ
	if (!flag)
	{
		flag = 1;
		declInitLen = declInitList.str().size();
	}
	//(2)��ȡ������
	string funName = u->decl->u.decl.name;
	//(3)��ȡ������������
	string returnType = "";
	if (u->decl->u.fdcl.returns->typ == Prim)
	{
		returnType = GetPrimDataType(u->decl->u.fdcl.returns->u.decl.type);
	}
	else{
		returnType = GetArrayDataType(u->decl->u.fdcl.returns->u.decl.type);
	}
	//(4)д���ַ���
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
	//(5)ɾ�����һ������Ķ���
	string orignal = declInitList.str();
	declInitList.str("");
	orignal = orignal.substr(0, orignal.size() - 1);
	declInitList << orignal << ")";
	temp_declInitList << declInitList.str() << "\n";
	//(6)��������������������
	declInitList << "\t{ \n";
	if (u->body->typ != Block)//����Ƿ�block��㣬����Ҫ���ж���
	{
		declInitList << "\n\t";
	}
	OutputStmt(u->body, offset + 1);
	declInitList << "\n\t} \n";
}

void DynamicX86CodeGenerate::COSX86_Text(Node *node, textNode *u, int offset)
{
}