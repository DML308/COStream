#include "DSGCodeGeneration.h"
using namespace std;
#ifndef  WIN32
#include <sys/stat.h>
#endif
#ifdef WIN32
#define mkdir(tmp) _mkdir(tmp.c_str());


#else
#define mkdir(tmp) mkdir(tmp.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
#include <string.h>

char*DSGobjName;

GLOBAL void DSGCodeGeneration(char*currentDir, DividedStaticGraph*dsg, string substring, vector<StageAssignment*> pSAList, vector<MetisPartiton*>mplist)
{
	assert(currentDir);
	assert(dsg);
	//��Ժ��
	if (DynamicX86Backend == TRUE)
	{
		string dir = currentDir;
#ifdef WIN32
		if (Win)
		{
			dir += "\\X86DynamicDistCode_Win\\";
		}
		else dir += "\\X86DynamicDistCode_Linux\\";
		dir += substring;
		dir += "\\";
		DSGobjName = new char[substring.size() + 1];
		strcpy(DSGobjName, substring.c_str());

#else
		if (dir == "./")	//����û��ָ��Ŀ¼�����ڵ�ǰĿ¼���½��ļ��У����ļ��е�����Ϊ��ǰspl
		{
			dir += substring;
			dir += "/";
			mkdir(dir);

		}
		int first, end;
		first = 0;
		first = dir.find_first_of('/', first);
		end = dir.find_first_of('/', first + 1);
		while (end != -1)
		{
			mkdir(dir.substr(0, end));
			first = end;
			end = dir.find_first_of('/', first + 1);
		}
		cout << "dir_______" << dir << endl;
		if (dir.at(dir.size() - 1) != '/')
		{
			mkdir(dir);
		}
		DSGobjName = new char[substring.size() + 1];
		strcpy(DSGobjName, substring.c_str());
#endif
		cout << dir << "*******************" << endl;
		cout << "-----------����x86��̬����-----------" << endl;

		int nCpucore = CpuCoreNum;//����ָ��

		DynamicX86CodeGenerate *X86code = new DynamicX86CodeGenerate(dsg, nCpucore, dir.c_str(), pSAList, mplist);
		X86code->DYGlobalVar();
		X86code->DYGlobalVarExtern();
		X86code->DYGlobalHeader();
		X86code->DYGlobalCpp();
		X86code->DYThreads();
		X86code->DYactors();
		X86code->DYMain();
#ifdef  WIN32
		X86LibCopy tmp;
		tmp.Run(dir.c_str());
#else
		cout<<"cgmakefile"<<endl;
		X86code->CGMakefile();
#endif //  WIN32

		cout << "done!" << endl;
	}
}