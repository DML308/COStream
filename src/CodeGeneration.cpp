#include "CodeGeneration.h"
#include "X86CodeGenerate.h"
using namespace std;
#ifndef WIN32
#include <sys/stat.h> 
#endif
#ifdef WIN32
#define mkdir(tmp) _mkdir(tmp.c_str());

#else
#define mkdir(tmp) mkdir(tmp.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
char* objName;
//����Ϊ����ɵ��ȵľ�̬������ͼ
GLOBAL void CodeGeneration(char *currentDir, SchedulerSSG *sssg, string substring, StageAssignment *psa, Partition *Mp, MAFLPartition *maflp)
{//20121127�ո���lihe�°汾�����޸�
	assert(currentDir);
	assert(sssg);
	//ѡ��������ɺ������
	if (X10Backend == TRUE)//x10
	{
		string dir = currentDir;
		dir += "\\X10DistCode\\";
		dir += substring;
		cout<<"-----------����x10����-----------"<<endl;
		//ȡ��������ָ����place����������ָ�������ó������actor����һ��
		int nCpucore = CpuCoreNum > 0 ? CpuCoreNum : sssg->GetFlatNodes().size();

		cout<<"�򵥹���ģʽ...";
		X10CodeGenerate *X10Code = new X10CodeGenerate(sssg, nCpucore, Buffer_Size, dir.c_str());

		// ���ɽӿ��ļ�, �����������͵�����
		X10Code->CGinterface();
		// ���ɸ������ļ�
		X10Code->CGactors();
		// ���������ļ�
		X10Code->SimpleScheduler();
		X10Code->CGmain();

		// ����lib�ļ�
		X10LibCopy tmp;
		tmp.Run(dir.c_str());

		cout<<"done!"<<endl;

	}
	else if (X86Backend == TRUE)
	{
		string dir = currentDir;
#ifdef WIN32
		if (Win)
		{
			dir += "\\X86StaticDistCode_Win\\";
		}
		else dir +="\\X86StaticDistCode_Linux\\";
		dir += substring;
		dir += "\\";
		objName = new char[substring.size()+1];
		strcpy(objName,substring.c_str());

#else
		if(dir == "./")	//����û��ָ��Ŀ¼�����ڵ�ǰĿ¼���½��ļ��У����ļ��е�����Ϊ��ǰspl
		{
			dir += substring;
			dir += "/";
			mkdir(dir);

		}
		int first,end;
		first = 0;
		first = dir.find_first_of('/',first);
		end = dir.find_first_of('/',first+1);
		while(end !=-1)
		{
			mkdir(dir.substr(0,end));
			first = end;
			end =  dir.find_first_of('/',first+1);
		}
		cout<<"dir_______"<<dir<<endl;
		if(dir.at(dir.size()-1) != '/')
		{
			mkdir(dir);
		}
		objName = new char[substring.size()+1];
		strcpy(objName,substring.c_str());
#endif

		cout<<dir<<"*******************"<<endl;
		cout<<"-----------����x86����-----------"<<endl;
		//ȡ��������ָ����place����������ָ�������ó������actor����һ��
		int nCpucore = CpuCoreNum > 0 ? CpuCoreNum : sssg->GetFlatNodes().size();
		
		//Begin ����wangliang
		//���洦��������˵ĵ�������
		if (realnCpucore){
			int k = Mp->getParts();//���ֵķ���
			//	assert(nCpucore == k);

			nCpucore = Mp->orderPartitionResult();
		}
		//End ����wangliang
		cout<<"�򵥹���ģʽ...\n";
		X86CodeGenerate *X86Code = new X86CodeGenerate(sssg, nCpucore, dir.c_str(),psa,Mp);
		X86Code->CGGlobalvar();//���������������ȫ�ֱ��������ļ�	GlobalVar.cpp
		X86Code->CGGlobalvarextern();//���������������ȫ�ֱ����������ļ� GlobalVar.h
		X86Code->CGglobalHeader();	//����stream�����ͺ�ȫ��������������������
		X86Code->CGglobalCpp();		//�������������Ķ���
		X86Code->CGThreads();		//���������߳�
		X86Code->CGactors();		//���������ʾ�ļ��㵥Ԫactor
		X86Code->CGMain();//���������̵߳�main�ļ�
#ifdef WIN32
		X86LibCopy tmp;
		tmp.Run(dir.c_str());
#else		
		cout<<"cgmakefile"<<endl;
		X86Code->CGMakefile();
#endif

		cout<<"done!"<<endl;
		//X86Code->CGAllActorHeader();
	}
	else if (GPUBackend)
	{
		string dir = currentDir;
		if (Win)
		{
			dir += "\\GPUDistCode_Win\\";
		}
		else dir +="\\GPUDistCode_Linux\\";
		dir += substring;
		dir += "\\";
		cout<<dir<<"*******************"<<endl;
		cout<<"-----------����GPU(OpenCL)����-----------"<<endl;
		//ȡ��������ָ����place����������ָ�������ó������actor����һ��
		int ngpu = GpuNum > 0 ? GpuNum : sssg->GetFlatNodes().size();

		
		//cout << nCpucore << endl;
		cout<<"�򵥹���ģʽ...";
		GPUCodeGenerate *GPUCode = new GPUCodeGenerate(sssg, ngpu, Buffer_Size, dir.c_str(),psa,maflp);
		GPUCode->CGGlobalvar();//����ȫ�ֱ���
		GPUCode->CGGlobalvarextern();
		GPUCode->CGThreads();
		GPUCode->CGglobalHeader();
		GPUCode->CGactors();
		GPUCode->CGAllKernel();
		GPUCode->CGglobalCpp();
		GPUCode->CGMain();//���������̵߳�main�ļ�
		GPULibCopy tmp;
		tmp.Run(dir.c_str());
		cout<<"done!"<<endl;
	}
}