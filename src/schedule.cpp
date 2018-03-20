#include "schedulerSSG.h"

using namespace std;


GLOBAL SchedulerSSG *SchedulingSSG(StaticStreamGraph *ssg)
{
	SchedulerSSG *sssg = new SchedulerSSG(ssg);

	if(sssg->SteadyScheduling())
	{
		sssg->InitScheduling();
		/*std::map<FlatNode *,int>::iterator pos;
		sssg->total_work = 0;
		for (pos = sssg->mapSteadyCount2FlatNode.begin(); pos != sssg->mapSteadyCount2FlatNode.end(); ++pos)
		{
			pos->first->work_estimate *= pos->second;//���½ڵ�ľ�̬������
			sssg->total_work += pos->first->work_estimate;
		}
		*/
#if 0
		std::map<FlatNode *,int>::iterator pos;
		cout<<"��̬��������:"<<endl;
		for (pos = sssg->mapSteadyCount2FlatNode.begin(); pos != sssg->mapSteadyCount2FlatNode.end(); ++pos)
			cout<<pos->first->name<<"\t"<<pos->second<<endl;
		cout<<"��ʼ����������:"<<endl;
		for (pos = sssg->mapInitCount2FlatNode.begin(); pos != sssg->mapInitCount2FlatNode.end(); ++pos)
			cout<<pos->first->name<<"\t"<<pos->second<<endl;

#endif
	}
	else
	{
		fprintf(stdout, "���򲻴�����̬���ȣ��޷����д������ɣ�\n");
		system("pause");
		exit(1);
	}

	return sssg;
}
