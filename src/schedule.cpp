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
			pos->first->work_estimate *= pos->second;//更新节点的静态工作量
			sssg->total_work += pos->first->work_estimate;
		}
		*/
#if 0
		std::map<FlatNode *,int>::iterator pos;
		cout<<"稳态调度序列:"<<endl;
		for (pos = sssg->mapSteadyCount2FlatNode.begin(); pos != sssg->mapSteadyCount2FlatNode.end(); ++pos)
			cout<<pos->first->name<<"\t"<<pos->second<<endl;
		cout<<"初始化调度序列:"<<endl;
		for (pos = sssg->mapInitCount2FlatNode.begin(); pos != sssg->mapInitCount2FlatNode.end(); ++pos)
			cout<<pos->first->name<<"\t"<<pos->second<<endl;

#endif
	}
	else
	{
		fprintf(stdout, "程序不存在稳态调度，无法进行代码生成！\n");
		system("pause");
		exit(1);
	}

	return sssg;
}
