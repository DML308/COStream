#include "Speedup.h"
#include "schedulerSSG.h"
#include <limits>
#include <time.h>
#include <iomanip>// add by wangliang
using namespace std;

string do_fraction(double v, int decplaces=3)
{
	ostringstream out;
	int prec=numeric_limits<double>::digits10;// 18
	out.precision(prec);//����Ĭ�Ͼ���
	out<<v;
	string str= out.str(); //������ȡ���ַ����ִ�8
	char DECIMAL_POINT='.'; // ŷ���÷�Ϊ','
	size_t n=str.find(DECIMAL_POINT);
	if ((n!=string::npos) && (str.size()> n+decplaces)) //�������ٻ���decplacesλ��
	{
		str[n+decplaces]='\0';//���ǵ�һ���������
	}
	//str.swap(string(str.c_str()));//ɾ��nul֮��Ķ����ַ�
	str.swap(*(new string(str.c_str())));//ɾ��nul֮��Ķ����ַ�
	return str;
} 

/* Begin ���� wangliang */
GLOBAL void ComputeSpeedup(SchedulerSSG *sssg, Partition *mp, string sourceFileName, const char *fileName, string pSelected)
{

	stringstream buf, buff;
	double total = 0.0;
	double totalCommunication = 0.0;
	double maxWorkLoad = 0;
	double maxActorWorkload = 0.0;
	string maxActorWorkloadName = "";
	for (int i = 0; i<sssg->GetFlatNodes().size(); i++)
	{
		total += sssg->GetSteadyCount(sssg->GetFlatNodes()[i])*sssg->GetSteadyWorkMap().find(sssg->GetFlatNodes()[i])->second;
	}
	string t = do_fraction(total);
	time_t curtime = time(0);
	tm tim = *localtime(&curtime);
	int day, mon, year;
	day = tim.tm_mday;
	mon = tim.tm_mon + 1;
	year = tim.tm_year + 1900;
	int min = tim.tm_min; /*��,0-59*/
	int hour = tim.tm_hour; /*ʱ,0-23*/

	buf << "----------------------" << sourceFileName << " - " << pSelected << "(" << mp->getParts() << ") " << year << "-" << mon << "-" << day << "," << hour << "." << min << "---------------------\n";
	buf << "#######################  Partition info  ##########################\n";
	buf << "part" << setw(20) << "workload" << setw(20) << "percent" << setw(20) << "communication\n";
	buff << "\n######################## Detail ###################################\n";
	buff << "part" << setw(25) << "actor" << setw(25) << "workload" << setw(20) << "percent\n";
	for (int i = 0; i<mp->getParts(); i++)//����ÿ��place
	{
		vector<FlatNode *> tmp = mp->findNodeSetInPartition(i);
		double total_inplace = 0.0;
		double edgeCommunication = 0.0;
		for (int j = 0; j<tmp.size(); j++){
			double tmpd = sssg->GetSteadyCount(tmp[j]) * sssg->GetSteadyWorkMap().find(tmp[j])->second;
			total_inplace += tmpd;
			if (tmpd > maxActorWorkload)
			{
				maxActorWorkload = tmpd;
				maxActorWorkloadName = tmp[j]->name;
			}

			for (int m = 0; m < tmp[j]->nIn; m++){//�������������
				FlatNode *p = (tmp[j]->inFlatNodes)[m];//�õ�����߶�Ӧ���϶˽ڵ�
				if (find(tmp.begin(), tmp.end(), p) == tmp.end()){//����϶˽ڵ㲻��Xi��
					edgeCommunication += (tmp[j]->inPopWeights[m]) * (sssg->GetSteadyCount(tmp[j]));
				}
			}
			for (int m = 0; m < tmp[j]->nOut; m++){//�������������
				FlatNode *p = (tmp[j]->outFlatNodes)[m];//�õ�����߶�Ӧ���¶˶˽ڵ�
				if (find(tmp.begin(), tmp.end(), p) == tmp.end()){//����¶˽ڵ㲻��Xi��
					edgeCommunication += (tmp[j]->outPushWeights[m]) * (sssg->GetSteadyCount(tmp[j]));
				}
			}

			buff << i << setw(30) << tmp[j]->name << setw(20) << tmpd << setw(18) << do_fraction(tmpd / total * 100) << "%\n";
		}
		totalCommunication += edgeCommunication;
		string inp = do_fraction(total_inplace);
		buf << i << setw(23) << total_inplace << setw(18) << do_fraction(total_inplace / total * 100) << "%" << setw(20) << edgeCommunication << "\n";
		if (total_inplace>maxWorkLoad) maxWorkLoad = total_inplace;
	}

	string tt = do_fraction(totalCommunication);
	buff << "\nmax actor workload:" << "\t\t" << maxActorWorkloadName << "\t\t" << do_fraction(maxActorWorkload) << "\t\t" << do_fraction(maxActorWorkload / total * 100) << "%\n";
	string m = do_fraction(maxWorkLoad);
	buf << buff.str();
	buf << "\n##################### total info ###############################\n";
	buf << "total workload \t= " << t << "\n";
	buf << "total Comunication \t= " << tt << "\n";
	buf << "max workload \t= " << maxWorkLoad << "\n";
	buf << "max speedup \t= " << do_fraction(total / maxWorkLoad) << "\n";
	buf << "-----------------------------------------------------------------------------------------------\n\n";
	ofstream fw;
	fw.open(fileName, ios::out);
	fw << buf.str();
	fw.close();
}

/* End ���� wangliang */