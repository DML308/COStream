#include "schedulerSSG.h"
#include <list>
#include <cmath>

using namespace std;

int SchedulerSSG::GetInitCount(FlatNode *node)
{
	std::map<FlatNode *, int> ::iterator pos;
	pos = mapInitCount2FlatNode.find(node);
	assert(pos!= mapInitCount2FlatNode.end());
	return pos->second;
}

int SchedulerSSG::GetSteadyCount(FlatNode *node)
{
	std::map<FlatNode *, int> ::iterator pos;
	pos = mapSteadyCount2FlatNode.find(node);
	assert(pos!=mapSteadyCount2FlatNode.end());
	return pos->second;
}

SchedulerSSG::SchedulerSSG(StaticStreamGraph *ssg)
{
	comName = ssg->GetName();
	flatNodes = ssg->GetFlatNodes();
	mapEdge2UpFlatNode = ssg->GetMapEdge2UpFlatNode();
	mapEdge2DownFlatNode = ssg->GetMapEdge2DownFlatNode();
	topLevel = ssg->GetTopLevel();
	mapSteadyWork2FlatNode = ssg->GetSteadyWorkMap();
	mapInitWork2FlatNode = ssg->GetInitWorkMap();
}

bool SchedulerSSG::SteadyScheduling2()
{
	list<FlatNode *> flatNodeList;
	std::map<FlatNode *,int>::iterator pos;
	// Ĭ�ϵ�һ���ڵ���Դ��Ҳ����˵peek��pop��Ϊ0,��ͼ�ı�ʾ���ݲ������ж��Դ���������ж��peek = pop = 0�ڵ�
	FlatNode *up = topLevel, *down = NULL;
	int nPush = 0, nPop = 0, nLcm = 0;
	int x, y, i, j;
	int size = 0;
	// ���ڿ��ǵ���ֻ��һ������ڵ����
	//while (1)
	while (size<=(flatNodes.size()-1))
	{
		// ��̬����ϵ�г�ʼϵ��Ϊ1
		mapSteadyCount2FlatNode.insert(make_pair(up, 1));
		// �����ýڵ����������ڵ㣨������ȱ�����
		for (i = 0; i < up->nOut; ++i)
		{
			nPush = up->outPushWeights[i]; // �϶˽ڵ��pushֵ
			//if(!nPush)continue;
			down = up->outFlatNodes[i]; // �ҵ��¶˽ڵ�

			for (j = 0; down->inFlatNodes[j] != up; j++); // �¶˽ڵ��ҵ����϶˽ڵ��Ӧ�ı��
			nPop = down->inPopWeights[j]; // �¶˽ڵ�ȡ����Ӧ��popֵ

			// ���ýڵ��Ƿ��ѽ�����̬���ȣ�ÿ��ֻ����һ����̬����
			pos = mapSteadyCount2FlatNode.find(down);
			// �ýڵ�δ������̬����
			if (pos == mapSteadyCount2FlatNode.end())
			{
				// �õ��϶˽ڵ����̬����ϵ��
				pos = mapSteadyCount2FlatNode.find(up);
				x = pos->second;
				nPush *= x; // Ϊʲô��x*nPush�أ������̬���ȵĸ���--�ڵ�����ˮ���ȶ�������ִ�е����ٴ���
				if(nPush != 0)
				{
					// nPush, nPop����С������;
					nLcm = lcm(nPush, nPop);
					int temp = nLcm/nPush;
					if( temp != 1) // ��һ���жϣ����Ч�ʣ���1�ǲ���Ҫ��
					{
						// ���ݼ�����������
						for (pos = mapSteadyCount2FlatNode.begin(); pos != mapSteadyCount2FlatNode.end(); ++pos)
							pos->second *= temp;
					}
					mapSteadyCount2FlatNode.insert(make_pair(down, nLcm/nPop));
				}
				else // ��push(0)������ lxx.2012.02.22
				{
					//assert(nPop == 0);
					// ȡ 1 ֵ lxx.2012.02.22
					mapSteadyCount2FlatNode.insert(make_pair(down, 1)); 
				}
				// ��down����listNode��Ϊ�˶�down������ڵ���е���
				flatNodeList.push_back(down);
			}
			else //�ýڵ��ѽ�����̬���ȣ����SDFͼ�Ƿ������̬����ϵ�У�һ�㲻���ڵĻ�������������
			{
				y = pos->second;
				pos = mapSteadyCount2FlatNode.find(up);
				x = pos->second;

				//nPop == 0 ˵���ڽ���join 0 ����
				if((nPop != 0 ) && (nPush * x) != (nPop * y))
				{
					cout<<"��������̬����..."<<endl;
					system("pause");
					exit(1); // ��ʾ��������̬���� 
				}
			}
		}
		if(flatNodeList.size() == 0) break; // ����Ϊ�գ�˵�����нڵ��ѵ������
		up = flatNodeList.front();
		flatNodeList.pop_front();
		size++;
	}

	return true;
}
bool SchedulerSSG::SteadyScheduling()
{
	list<FlatNode *> flatNodeList;
	std::map<FlatNode *, int>::iterator pos;
	// Ĭ�ϵ�һ���ڵ���Դ��Ҳ����˵peek��pop��Ϊ0,��ͼ�ı�ʾ���ݲ������ж��Դ���������ж��peek = pop = 0�ڵ�
	FlatNode *up = topLevel, *down = NULL;
	int nPush = 0, nPop = 0, nLcm = 0;
	int x, y, i, j;

	// ���ڿ��ǵ���ֻ��һ������ڵ����
	while (1)
	{
		// ��̬����ϵ�г�ʼϵ��Ϊ1
		mapSteadyCount2FlatNode.insert(make_pair(up, 1));
		// �����ýڵ����������ڵ㣨������ȱ�����
		for (i = 0; i < up->nOut; ++i)
		{
			nPush = up->outPushWeights[i]; // �϶˽ڵ��pushֵ
										   //if(!nPush)continue;
			down = up->outFlatNodes[i]; // �ҵ��¶˽ڵ�

			for (j = 0; down->inFlatNodes[j] != up; j++); // �¶˽ڵ��ҵ����϶˽ڵ��Ӧ�ı��
			nPop = down->inPopWeights[j]; // �¶˽ڵ�ȡ����Ӧ��popֵ

										  // ���ýڵ��Ƿ��ѽ�����̬���ȣ�ÿ��ֻ����һ����̬����
										  //Map��find����Ѱ��Ԫ��elem���ֵ�λ�ã����ض�Ӧλ�õĵ���������û�г����򷵻�βԪ�ص���һ��λ��
			pos = mapSteadyCount2FlatNode.find(down);
			// �ýڵ�δ������̬����
			if (pos == mapSteadyCount2FlatNode.end())
			{
				// �õ��϶˽ڵ����̬����ϵ���������̬����ϵ������һ������������ָ��Map�е�ָ���ڵ㡶first��second�� ����㣬ִ�д�������
				pos = mapSteadyCount2FlatNode.find(up);
				x = pos->second;

				nPush *= x; // Ϊʲô��x*nPush�أ������̬���ȵĸ���--�ڵ�����ˮ���ȶ�������ִ�е����ٴ��� = ÿ��push * ��ִ̬�д���

				if (nPush != 0)
				{
					// nPush, nPop����С������;
					nLcm = lcm(nPush, nPop);

					int temp = nLcm / nPush; //���½ڵ�ﵽƽ��ʱ����Ҫ��ִ�д���

					if (temp != 1) // ��һ���жϣ����Ч�ʣ���1�ǲ���Ҫ��
					{
						// ���ݼ�����������
						for (pos = mapSteadyCount2FlatNode.begin(); pos != mapSteadyCount2FlatNode.end(); ++pos)
							pos->second *= temp;
					}
					mapSteadyCount2FlatNode.insert(make_pair(down, nLcm / nPop));
				}
				else // ��push(0)������ lxx.2012.02.22
				{
					//assert(nPop == 0);
					// ȡ 1 ֵ lxx.2012.02.22
					mapSteadyCount2FlatNode.insert(make_pair(down, 1));
				}
				// ��down����listNode��Ϊ�˶�down������ڵ���е���,�൱�ڱ���
				flatNodeList.push_back(down);
			}
			else //�ýڵ��ѽ�����̬���ȣ����SDFͼ�Ƿ������̬����ϵ�У�һ�㲻���ڵĻ�������������
			{
				y = pos->second; //�·��ڵ��ִ�д���
				pos = mapSteadyCount2FlatNode.find(up);
				x = pos->second; //�϶˽ڵ��ִ�д���

								 //nPop == 0 ˵���ڽ���join 0 ����
				if ((nPop != 0) && (nPush * x) != (nPop * y))
				{
					cout << "��������̬����..." << endl;
					system("pause");
					exit(1); // ��ʾ��������̬���� 
				}//if
			}//else
		}//for

		if (flatNodeList.size() == 0) break; // ����Ϊ�գ�˵�����нڵ��ѵ������
		up = flatNodeList.front();
		flatNodeList.pop_front(); //���Ѿ�������Ľڵ��Ƴ�list
	}//while
	return true;
}

bool SchedulerSSG::InitScheduling()
{
	list<FlatNode *> flatNodeList;
	std::map<FlatNode *, int>::iterator pos;
	FlatNode *down = NULL, *up = NULL;
	int nPush = 0, nPop = 0, nPeek = 0;
	int x, y, i, j, n, num = 0;

	//���ڿ��ǵ���ֻ��һ������ڵ����
	//�ҵ�sink�ڵ�
	for (i = 0; i < flatNodes.size(); ++i)
	{
		if (!flatNodes[i]->nOut)
		{
			down = flatNodes[i];
			num++;
		}
	}

	if (num > 1)
	{
		fprintf(stdout, "FATAL ERROR: ������ڶ������ڣ�\n");
		system("pause");
		exit(1);
	}

	if (num == 0)
	{
		fprintf(stdout, "FATAL ERROR: ����������ڣ�\n");
		system("pause");
		exit(1);
	}

	//ÿ���ڵ�ĳ�ʼ�����ȴ�����ʼֵΪ0
	mapInitCount2FlatNode.insert(make_pair(down, 0));
	while (1)
	{
		//�����ýڵ����������ڵ�
		for (i = 0; i < down->nIn; i++)
		{
			//�ҵ��¶˽ڵ��peek��popֵ
			nPeek = down->inPeekWeights[i];
			nPop = down->inPopWeights[i];
			up = down->inFlatNodes[i];

			//�ҵ���Ӧ�϶˽ڵ��popֵ
			for (j = 0; up->outFlatNodes[j] != down; j++);

			nPush = up->outPushWeights[j];

			pos = mapInitCount2FlatNode.find(down);
			//�¶˽ڵ����еĳ�ʼ�����ȴ���
			x = pos->second;

			//�¶˽ڵ�����һ����Ҫ�Ķ���������
			y = nPeek - nPop;
			if (y <= 0 || nPeek <= nPush)
				y = 0;
			if (nPush != 0)
				//ceil����Ϊ��ȡ������
				n = ceil((x * nPop + y) / float(nPush));
			else
				n = 0;

			pos = mapInitCount2FlatNode.find(up);

			if (pos == mapInitCount2FlatNode.end())//zww��20120322��Ϊ����û������Ľڵ���޸�
			{
				mapInitCount2FlatNode.insert(make_pair(up, n));
				flatNodeList.push_back(up);
			}
			else
			{
				if (pos->second < n)
				{
					pos->second = n;
					//�ýڵ�ĳ�ʼ�����ȴ����Ѹı䣬�������¼�����ж����϶˽ڵ���е���
					flatNodeList.push_back(up);
				}
			}
		}//for
		if (flatNodeList.size() == 0) break;//����Ϊ�գ�˵�����нڵ��ѵ������
		down = flatNodeList.front();
		flatNodeList.pop_front();
	}
	return true;
}

bool SchedulerSSG::InitScheduling2()
{
	list<FlatNode *> flatNodeList;
	std::map<FlatNode *,int>::iterator pos;
	FlatNode *down = NULL, *up = NULL;
	int nPush = 0, nPop = 0, nPeek = 0;
	int x, y, i, j, n, num = 0;

	//���ڿ��ǵ���ֻ��һ������ڵ����
	//�ҵ�sink�ڵ�
	for (i = 0; i < flatNodes.size(); ++i)
	{
		//if(!flatNodes[i]->nOut)
		if (i == flatNodes.size()-1)
		{
			down = flatNodes[i];
			num ++;
		}
	}
	//down = flatNodes[flatNodes.size() - 1];
	int size = 0;
	if(num > 1)
	{
		fprintf(stdout, "FATAL ERROR: ������ڶ������ڣ�\n");
		system("pause");
		exit(1);
	}

	if(num == 0)
	{
		fprintf(stdout, "FATAL ERROR: ����������ڣ�\n");
		system("pause");
		exit(1);
	}

	//ÿ���ڵ�ĳ�ʼ�����ȴ�����ʼֵΪ0
	mapInitCount2FlatNode.insert(make_pair(down, 0));
	while (size<=(flatNodes.size()-1))
	{
		//�����ýڵ����������ڵ�
		for (i = 0; i < down->nIn; i++)
		{
			//�ҵ��¶˽ڵ��peek��popֵ
			nPeek = down->inPeekWeights[i];
			nPop  = down->inPopWeights[i];
			up = down->inFlatNodes[i];

			//�ҵ���Ӧ�϶˽ڵ��popֵ
			for (j = 0; up->outFlatNodes[j] != down; j++);
			nPush = up->outPushWeights[j];

			pos = mapInitCount2FlatNode.find(down);
			//�¶˽ڵ����еĳ�ʼ�����ȴ���
			x = pos->second;

			//�¶˽ڵ�����һ����Ҫ�Ķ���������
			y = nPeek - nPop;
			if(y <= 0 || nPeek <= nPush)
				y = 0;
			if(nPush != 0)	
				n = ceil((x * nPop + y)/float(nPush));
			else
				n = 0;
			pos = mapInitCount2FlatNode.find(up);
			if (pos == mapInitCount2FlatNode.end())//zww��20120322��Ϊ����û������Ľڵ���޸�
			{
				mapInitCount2FlatNode.insert(make_pair(up, n));
				flatNodeList.push_back(up);
			}
			else
			{
				if(pos->second < n) 
				{
					pos->second = n;
					//�ýڵ�ĳ�ʼ�����ȴ����Ѹı䣬�������¼�����ж����϶˽ڵ���е���
					flatNodeList.push_back(up);
				}
			}
		}
		if(flatNodeList.size() == 0) break;//����Ϊ�գ�˵�����нڵ��ѵ������
		down = flatNodeList.front();
		flatNodeList.pop_front();
		size++;
	}
	return true;
}


//��a,b�����Լ��
int SchedulerSSG::gcd(int a, int b)
{
	int r = 0;
	if(a < b)
	{
		r = a;
		a = b; 
		b = r;
	}
	assert(b);
	while(a % b)
	{
		assert(b);
		r = a % b;
		a = b;
		b = r;
	}

	return b;
}


//��a,b����С������
int SchedulerSSG::lcm(int a, int b)
{
	int product = a * b;

	return product/gcd(a,b);
}

std::map<FlatNode *, int> SchedulerSSG::SteadySchedulingGroup(std::vector<FlatNode *>flatNodeVec)
{//����һ���ֲ�����̬


	list<FlatNode *> flatNodeList;
	std::map<FlatNode *,int>::iterator pos;
	std::map<FlatNode *, int> flatNode2SteadyCount;
	assert(flatNodeVec.size() > 0);
	map<FlatNode * ,Bool> flatNodesTag;//���ڱ�ʾflatNodeVec�еĽڵ��Ƿ񱻵���
	for (int indexNode = 0; indexNode != flatNodeVec.size(); indexNode++)
	{
		flatNodesTag.insert(make_pair(flatNodeVec[indexNode],FALSE));
	}
	// Ĭ�ϵ�һ���ڵ���Դ��Ҳ����˵peek��pop��Ϊ0,��ͼ�ı�ʾ���ݲ������ж��Դ���������ж��peek = pop = 0�ڵ�
	FlatNode *up = flatNodeVec[0], *down = NULL, *parent = NULL;
	int nPush = 0, nPop = 0, nLcm = 0;
	int x, y, i, j;
	Bool flag = FALSE;//ֻ�е�flatNode��flatNodeVec�вŽ��е���
	while (!flatNodesTag.empty())
	{	
		up = flatNodesTag.begin()->first;
		while(1)
		{		
			// ��̬����ϵ�г�ʼϵ��Ϊ1
			flatNode2SteadyCount.insert(make_pair(up, 1));
			flatNodesTag.erase(up);
			// �����ýڵ����������ڵ�up��child�ڵ���еĵ��ȣ���ѭ��ִ���꣬��up��parent�ڵ���е���
			/*����up��child�ڵ�*/
			for (i = 0; i < up->nOut; ++i)
			{
				flag = FALSE;
				for (int k = 0; k != flatNodeVec.size(); ++k)
				{
					if(up->outFlatNodes[i] == flatNodeVec[k]) 
					{
						flag = TRUE;break;
					}
				}
				if(flag)
				{
					nPush = up->outPushWeights[i]; // �϶˽ڵ��pushֵ
					down = up->outFlatNodes[i]; // �ҵ��¶˽ڵ�

					for (j = 0; down->inFlatNodes[j] != up; j++); // �¶˽ڵ��ҵ����϶˽ڵ��Ӧ�ı��
					nPop = down->inPopWeights[j]; // �¶˽ڵ�ȡ����Ӧ��popֵ

					// ���ýڵ��Ƿ��ѽ�����̬���ȣ�ÿ��ֻ����һ����̬����
					pos = flatNode2SteadyCount.find(down);
					// �ýڵ�δ������̬����
					if (pos == flatNode2SteadyCount.end())
					{
						// �õ��϶˽ڵ����̬����ϵ��
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;
						nPush *= x; // Ϊʲô��x*nPush�أ������̬���ȵĸ���--�ڵ�����ˮ���ȶ�������ִ�е����ٴ���
						if(nPush != 0)
						{
							// nPush, nPop����С������;
							nLcm = lcm(nPush, nPop);
							int temp = nLcm/nPush;
							if( temp != 1) // ��һ���жϣ����Ч�ʣ���1�ǲ���Ҫ��
							{
								// ���ݼ�����������
								for (pos = flatNode2SteadyCount.begin(); pos != flatNode2SteadyCount.end(); ++pos)
									pos->second *= temp;
							}

							flatNode2SteadyCount.insert(make_pair(down, nLcm/nPop));
							flatNodesTag.erase(down);
							// ��down����listNode��Ϊ�˶�down������ڵ���е���
							flatNodeList.push_back(down);
						}

					}
					else //�ýڵ��ѽ�����̬���ȣ����SDFͼ�Ƿ������̬����ϵ�У�һ�㲻���ڵĻ�������������
					{
						y = pos->second;
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;

						//nPop == 0 ˵���ڽ���join 0 ����
						if((nPop != 0 ) && (nPush * x) != (nPop * y)) 
						{
							cout<<"��������̬����1"<<endl;
							system("pause");
							exit(1); // ��ʾ��������̬����
						}
					}
				}
			}
			/*����up��parent�ڵ�*/
			for (i = 0; i < up->nIn; ++i)
			{
				flag = FALSE;
				for (int k = 0; k != flatNodeVec.size(); ++k)
				{//�ж�parent�ڵ��ڲ���flatNodeVec��
					if(up->inFlatNodes[i] == flatNodeVec[k]) 
					{
						flag = TRUE;break;
					}
				}

				if(flag)
				{
					nPop = up->inPopWeights[i]; // ��ǰ�ڵ��popֵ
					parent = up->inFlatNodes[i]; // �ҵ���ǰ�ڵ�ĸ��ڵ�

					for (j = 0; parent->outFlatNodes[j] != up; j++); // up�ڵ���parent������ڵ��ж�Ӧ�ı��
					nPush = parent->outPushWeights[j]; // parent�ڵ�ȡ����Ӧ��pushֵ

					// ���ýڵ��Ƿ��ѽ�����̬���ȣ�ÿ��ֻ����һ����̬����
					pos = flatNode2SteadyCount.find(parent);
					// �ýڵ�δ������̬����
					if (pos == flatNode2SteadyCount.end())
					{
						// �õ��϶˽ڵ����̬����ϵ��
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;
						nPop *= x; // Ϊʲô��x*nPush�أ������̬���ȵĸ���--�ڵ�����ˮ���ȶ�������ִ�е����ٴ���
						if(nPop != 0)
						{
							// nPush, nPop����С������;
							nLcm = lcm(nPush, nPop);
							int temp = nLcm/nPop;
							if( temp != 1) // ��һ���жϣ����Ч�ʣ���1�ǲ���Ҫ��
							{
								// ���ݼ�����������
								for (pos = flatNode2SteadyCount.begin(); pos != flatNode2SteadyCount.end(); ++pos)
									pos->second *= temp;
							}

							flatNode2SteadyCount.insert(make_pair(parent, nLcm/nPush));
							flatNodesTag.erase(parent);
							// ��down,parent����listNode��Ϊ�˶�down������ڵ���е���
							flatNodeList.push_back(parent);
						}

					}
					else //�ýڵ��ѽ�����̬���ȣ����SDFͼ�Ƿ������̬����ϵ�У�һ�㲻���ڵĻ�������������
					{
						y = pos->second;
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;

						//nPop == 0 ˵���ڽ���join 0 ����
						if((nPop != 0 ) && (nPop * x) != (nPush * y))
						{
							cout<<"��������̬����2"<<endl;
							system("pause");
							exit(1); // ��ʾ��������̬����
						}
					}
				}

			}
			if(flatNodeList.size() == 0) break; // ����Ϊ�գ�˵�����нڵ��ѵ������
			up = flatNodeList.front();
			flatNodeList.pop_front();

		}
	}

	return flatNode2SteadyCount;
}

/*����Ӧ����SDFͼ  2016/01/19 yrr*/
void SchedulerSSG::hClusterAdjust()
{
	for (auto iter = mapUpBordNode2Edge.begin(); iter != mapUpBordNode2Edge.end(); ++iter)
	{
		for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
		{
			mapEdge2UpFlatNode[*iter2] = iter->first;
		}//for
	}//for

	for (auto iter = mapDownBordNode2Edge.begin(); iter != mapDownBordNode2Edge.end(); ++iter)
	{
		for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
		{
			mapEdge2DownFlatNode.insert(make_pair(*iter2, iter->first));
		}//for
	}//for

	 /*Ȼ���޸�FlatNode�ڲ��ṹ*/
	for (auto iter = mapUpBordNode2Edge.begin(); iter != mapUpBordNode2Edge.end(); ++iter)
	{
		FlatNode *oldNode = mapNewNode2OldNode[iter->first];
		FlatNode *newNode = iter->first;

		newNode->nIn = oldNode->nIn;
		newNode->nOut = oldNode->nOut;
		newNode->outFlatNodes.clear();
		newNode->inFlatNodes.clear();
		newNode->outPushWeights = oldNode->outPushWeights;
		newNode->inPopWeights = oldNode->inPopWeights;
		newNode->inPeekWeights = oldNode->inPeekWeights;
		newNode->outPushString = oldNode->pushString;
		newNode->inPopString = oldNode->inPopString;
		newNode->inPeekString = oldNode->inPeekString;
		newNode->pushString = oldNode->pushString;
		newNode->peekString = oldNode->peekString;

		for (vector<FlatNode *>::iterator outIter = oldNode->outFlatNodes.begin(); outIter != oldNode->outFlatNodes.end(); ++outIter) {
			newNode->outFlatNodes.push_back(mapOldNode2NewNode[*outIter]);
		}//for
		for (vector<FlatNode *>::iterator inIter = oldNode->inFlatNodes.begin(); inIter != oldNode->inFlatNodes.end(); ++inIter) {
			newNode->inFlatNodes.push_back(mapOldNode2NewNode[*inIter]);
		}//for
	}//for

	for (map<FlatNode *, vector<Node *> >::iterator iter = mapDownBordNode2Edge.begin(); iter != mapDownBordNode2Edge.end(); ++iter) {
		FlatNode *oldNode = mapNewNode2OldNode[iter->first];
		FlatNode *newNode = iter->first;

		newNode->nIn = oldNode->nIn;
		newNode->nOut = oldNode->nOut;
		newNode->outFlatNodes.clear();
		newNode->inFlatNodes.clear();
		newNode->outPushWeights = oldNode->outPushWeights;
		newNode->inPopWeights = oldNode->inPopWeights;
		newNode->inPeekWeights = oldNode->inPeekWeights;
		newNode->outPushString = oldNode->pushString;
		newNode->inPopString = oldNode->inPopString;
		newNode->inPeekString = oldNode->inPeekString;
		newNode->pushString = oldNode->pushString;
		newNode->peekString = oldNode->peekString;
		for (vector<FlatNode *>::iterator outIter = oldNode->outFlatNodes.begin(); outIter != oldNode->outFlatNodes.end(); ++outIter) {
			newNode->outFlatNodes.push_back(mapOldNode2NewNode[*outIter]);
		}//for
		for (vector<FlatNode *>::iterator inIter = oldNode->inFlatNodes.begin(); inIter != oldNode->inFlatNodes.end(); ++inIter) {
			newNode->inFlatNodes.push_back(mapOldNode2NewNode[*inIter]);
		}//for
	}//for

}