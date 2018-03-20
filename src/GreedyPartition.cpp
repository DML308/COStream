/* Begin ���� wangliang */
#include "GreedyPartition.h"

void GreedyPartition::SssgPartition(SchedulerSSG *sssg, int level){
	assert(level == 1);
	if (this->mnparts == 1)
	{//���ֻ��һ��place��������
		if (X86Backend || DynamicX86Backend)
		{
			for (int i = 0; i<nvtxs; i++)
				FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], 0));//�����ڵ㵽���ֱ�ŵ�ӳ��
			for (int i = 0; i<nvtxs; i++)
				PartitonNum2FlatNode.insert(make_pair(0, sssg->GetFlatNodes()[i]));
		}
		else if (GPUBackend)
		{
			for (int i = 0; i<nvtxs; i++)
			{
				if (!DetectiveActorState(sssg->GetFlatNodes()[i]))
				{
					FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], 0));//�����ڵ㵽���ֱ�ŵ�ӳ��
				}
				else
				{
					FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], 1));//�����ڵ㵽���ֱ�ŵ�ӳ��
				}
			}
			for (int i = 0; i<nvtxs; i++)
			{
				if (!DetectiveActorState(sssg->GetFlatNodes()[i]))
				{
					PartitonNum2FlatNode.insert(make_pair(0, sssg->GetFlatNodes()[i]));
				}
				else
				{
					PartitonNum2FlatNode.insert(make_pair(1, sssg->GetFlatNodes()[i]));
				}
			}
		}
		return;
	}

	nvtxs = sssg->GetFlatNodes().size();//����ĸ���
	int k = getParts();//���ֵķ���
	X.resize(k, vector<FlatNode *>());//k����ͼ
	w.resize(k, 0);//���ֵ�K����ͼÿ����ͼ���ܹ�������ʼ��Ϊ0
	edge.resize(k, 0);//���ֵ�K����ͼÿ����ͼ��ͨ�ű߳�ʼ��Ϊ0
	vwgt.resize(nvtxs, 0);//ÿ�������Ȩ��(�ܹ�����)��ʼ��Ϊ0
	part.resize(nvtxs, 0);//ÿ���ڵ��Ӧ�ĺ˺ų�ʼ��Ϊ0

	setActorWorkload(sssg);//����ÿ���ڵ���ܹ�����

	doPartition(sssg, k);//������������Ļ����㷨

	vector<vector<FlatNode *>> tmp_X = X;//��ʱ���滮�ֽ��
	vector<int> tmp_w = w;//��ʱ���滮�ֽ��

	int wmax = 0;//�õ�������ͼ������
	for (int i = 0; i < w.size(); i++){
		if (w[i] > wmax){
			wmax = w[i];
		}
	}
	int total_edge = getTotalEdge(sssg, k);//�õ���ͨ������С

	//cout << "------------------------" << endl << wmax << endl << total_edge << "------------------------" << endl;
	errorDecrease(sssg, k);//���ֵ�����½��㷨
	//����½��㷨���ܻ���ּ��ٱȲ��䣬ͨ�����������������ٳ��֣�
	//ͨ���Ƚ�����½��㷨ǰ��ļ��ٱȺ�ͨ��������ѡ��������½��㷨ǰ�Ļ��ֽ����������½��㷨��Ļ��ֽ��
	vector<vector<FlatNode *>> tmp_X1 = X;//��ʱ���滮�ֽ��
	vector<int> tmp_w1 = w;//��ʱ���滮�ֽ��
	int wmax_1 = 0;
	for (int i = 0; i < w.size(); i++){
		if (w[i] > wmax_1){
			wmax_1 = w[i];
		}
	}
	int total_edge_1 = getTotalEdge(sssg, k);

	X = tmp_X;
	w = tmp_w;
	errorDecrease2(sssg, k);
	vector<vector<FlatNode *>> tmp_X2 = X;//��ʱ���滮�ֽ��
	vector<int> tmp_w2 = w;//��ʱ���滮�ֽ��
	int wmax_2 = 0;
	for (int i = 0; i < w.size(); i++){
		if (w[i] > wmax_2){
			wmax_2 = w[i];
		}
	}
	int total_edge_2 = getTotalEdge(sssg, k);

	if (total_edge_2 <= total_edge_1){
		X = tmp_X2;
		w = tmp_w2;
	}
	else {
		X = tmp_X1;
		w = tmp_w1;
	}
	doTabuSearch(sssg, k);//��Сͨ�űߵĽ��������㷨

	getTotalEdge(sssg, k);//���õ�ͨ�űߵ���Ϣ
	int s = orderPartitionResult();//�Ի��ֵĽ����������

	for (int i = 0; i < X.size(); i++){//����ڵ���֮��Ӧ�ĺ˺�
		for (int j = 0; j < X[i].size(); j++){
			part[findID(sssg, X[i][j])] = i;
		}
	}

	if (X86Backend || DynamicX86Backend)
	{
		for (int i = 0; i<nvtxs; i++){
			FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], part[i]));//�����ڵ㵽���ֱ�ŵ�ӳ��
		}
		for (int i = 0; i<nvtxs; i++)
			PartitonNum2FlatNode.insert(make_pair(part[i], sssg->GetFlatNodes()[i]));
	}
	else if (GPUBackend)
	{
		int Num = this->mnparts;
		for (int i = 0; i<nvtxs; i++){
			if (DetectiveActorState(sssg->GetFlatNodes()[i]))
			{
				part[i] = Num;
			}
			FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], part[i]));//�����ڵ㵽���ֱ�ŵ�ӳ��
		}
		for (int i = 0; i<nvtxs; i++)
		{
			PartitonNum2FlatNode.insert(make_pair(part[i], sssg->GetFlatNodes()[i]));
		}
	}

}

void GreedyPartition::doPartition(SchedulerSSG *sssg, int k){
	int edgenum = sssg->GetMapEdge2DownFlatNode().size();//ͼ�ı���
	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg���ж����vector
	X[0] = V;//��һ����ͼ��ʼ��Ϊȫͼ

	sssg->edge_work = 0.0;//�ܵı�ͨ�ų�ʼ��Ϊ0
	for (int i = 0; i < V.size(); i++){//����ԭͼ��ͨ����
		for (int j = 0; j < V[i]->nIn; j++){//�������ֺ�ÿһ���ֵĽڵ�
			sssg->edge_work += (V[i]->inPopWeights[j]) * (sssg->GetSteadyCount(V[i]));
		}
		for (int j = 0; j < V[i]->nOut; j++){//�������ֺ�ÿһ���ֵĽڵ�
			sssg->edge_work += (V[i]->outPushWeights[j]) * (sssg->GetSteadyCount(V[i]));
		}
	}
	w[0] = sssg->total_work;
	//cout << "test************" <<endl<< sssg->total_work << endl << k << endl;
	double we = sssg->total_work / k;//ÿ����ͼ��ƽ��������
	double e = 2 - ee;//����ϵ����2-ee������

	int choose = 0;//��ѡ�ڵ���X[0]�е����
	int index = 0;//��ѡ�ڵ���ԭͼ�е����
	for (int i = 1; i < k; i++){//������ͼX[1] ~ X[k-1]
		S.clear();
		while (w[i] < we * e && X[0].size() != 0){//һ��Ҫ��X[0].size() != 0������137�п��ܻ��bug
			choose = 0;
			index = 0;
			if (S.size() == 0){//�����ѡ����Ϊ��
				if (X[0].size() == 0)choose = 0;
				else {//choose = rand() % (X[0].size());//��X[0]�����ѡ��һ���ڵ�

					//ѡ��X[0]�ж���Ȩ�����Ľڵ�
					int tmax = findID(sssg, X[0][0]);
					for (int j = 1; j < X[0].size(); j++){
						int t = findID(sssg, X[0][j]);//�ҵ��ýڵ���ԭͼ�е����
						if (vwgt[t] > vwgt[tmax])tmax = t;
					}
					choose = find(X[0].begin(), X[0].end(), V[tmax]) - X[0].begin();
				}
				index = findID(sssg, X[0][choose]);//�ҵ��ýڵ���ԭͼ�е����

				X[i].push_back(X[0][choose]);//���ýڵ����X[i]
				w[i] += vwgt[index];//������ͼ�Ĺ�����

				X[0].erase(X[0].begin() + choose);//���ýڵ��Ƴ�X[0]
				w[0] -= vwgt[index];//������ͼ�Ĺ�����
			}
			else{//ѡ���ѡ���������溯��ֵ���Ľڵ�
				int maxGain = chooseMaxGain(sssg, S, X[i], X[0]);//�ҵ��ýڵ���S�е����
				choose = find(X[0].begin(), X[0].end(), S[maxGain]) - X[0].begin();//�ҵ��ýڵ���X[0]�е����
				index = findID(sssg, X[0][choose]);//�ҵ��ýڵ���ԭͼ�е����

				X[i].push_back(X[0][choose]);//���ýڵ����X[i]
				w[i] += vwgt[index];//������ͼ�Ĺ�����

				X[0].erase(X[0].begin() + choose);//���ýڵ��Ƴ�X[0]
				w[0] -= vwgt[index];//������ͼ�Ĺ�����

				S.erase(S.begin() + maxGain);//���ýڵ�Ӻ�ѡ������ɾ��
			}
			updateCandidate(sssg, X[0], S, index);//���º�ѡ�ڵ㼯��
		}
	}

}

int GreedyPartition::chooseMaxGain(SchedulerSSG *sssg, vector<FlatNode *>& S, vector<FlatNode *>& Xi, vector<FlatNode *>& X0){
	int result = 0;
	int maxGain = INT_MIN;
	for (int i = 0; i < S.size(); i++){//�������к�ѡ�ڵ�
		int wi = 0;
		int w0 = 0;
		for (int j = 0; j < S[i]->nIn; j++){//�������������
			FlatNode *p = (S[i]->inFlatNodes)[j];//�õ�����߶�Ӧ���϶˽ڵ�
			if (find(Xi.begin(), Xi.end(), p) != Xi.end()){//����϶˽ڵ���Xi��
				wi += (S[i]->inPopWeights[j]) * (sssg->GetSteadyCount(S[i]));
			}
			else if (find(X0.begin(), X0.end(), p) != X0.end()){//����϶˽ڵ���X0��
				w0 += (S[i]->inPopWeights[j]) * (sssg->GetSteadyCount(S[i]));
			}
		}
		for (int j = 0; j < S[i]->nOut; j++){//�������������
			FlatNode *p = (S[i]->outFlatNodes)[j];//�õ�����߶�Ӧ���¶˶˽ڵ�
			if (find(Xi.begin(), Xi.end(), p) != Xi.end()){//����¶˽ڵ���Xi��
				wi += (S[i]->outPushWeights[j]) * (sssg->GetSteadyCount(S[i]));
			}
			else if (find(X0.begin(), X0.end(), p) != X0.end()){//����¶˽ڵ���X0��
				w0 += (S[i]->outPushWeights[j]) * (sssg->GetSteadyCount(S[i]));
			}
		}
		if (wi - w0 > maxGain){
			maxGain = wi - w0;
			result = i;//�����������ڵ�
		}
	}
	return result;
}

void GreedyPartition::updateCandidate(SchedulerSSG *sssg, vector<FlatNode *>& X0, vector<FlatNode *>& S, int index)
{
	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg���ж����vector
	for (int j = 0; j < V[index]->nIn; j++){//�������������
		FlatNode *p = (V[index]->inFlatNodes)[j];//�õ�����߶�Ӧ���϶˽ڵ�
		if (find(X0.begin(), X0.end(), p) != X0.end() && find(S.begin(), S.end(), p) == S.end()){//����϶˽ڵ���X0�в��Ҳ���S��
			S.push_back(p);//��������S��
		}
	}
	for (int j = 0; j < V[index]->nOut; j++){//�������������
		FlatNode *p = (V[index]->outFlatNodes)[j];//�õ�����߶�Ӧ���¶˽ڵ�
		if (find(X0.begin(), X0.end(), p) != X0.end() && find(S.begin(), S.end(), p) == S.end()){//����϶˽ڵ���X0�в��Ҳ���S��
			S.push_back(p);//��������S��
		}
	}
}

int GreedyPartition::getTotalEdge(SchedulerSSG *sssg, int k){//�õ����˼�ͨ�����Ĵ�С���ܴ�С
	edge.clear();
	edge.resize(k, 0);
	int total_edge = 0;
	for (int i = 0; i < k; i++){//�����ʱ��ͨ����
		for (int j = 0; j < X[i].size(); j++){//�������ֺ�ÿһ���ֵĽڵ�
			for (int m = 0; m < X[i][j]->nIn; m++){//�������������
				FlatNode *p = (X[i][j]->inFlatNodes)[m];//�õ�����߶�Ӧ���϶˽ڵ�
				if (find(X[i].begin(), X[i].end(), p) == X[i].end()){//����϶˽ڵ㲻��Xi��
					edge[i] += (X[i][j]->inPopWeights[m]) * (sssg->GetSteadyCount(X[i][j]));
				}
			}
			for (int m = 0; m < X[i][j]->nOut; m++){//�������������
				FlatNode *p = (X[i][j]->outFlatNodes)[m];//�õ�����߶�Ӧ���¶˶˽ڵ�
				if (find(X[i].begin(), X[i].end(), p) == X[i].end()){//����¶˽ڵ㲻��Xi��
					edge[i] += (X[i][j]->outPushWeights[m]) * (sssg->GetSteadyCount(X[i][j]));
				}
			}
		}
		total_edge += edge[i];
	}
	return total_edge;
}

void GreedyPartition::errorDecrease(SchedulerSSG *sssg, int k){//����½��㷨
	double we = sssg->total_work / k;
	int imax = 0;
	for (int i = 0; i < w.size(); i++){
		if (w[i] > w[imax]){
			imax = i;
		}
	}

	int cnt = X[imax].size();
	while (w[imax] > we && cnt > 0){//����ƽ������ʱ��������½��㷨
		int t = w[imax];

		int choose = 0;
		int partChoose = 0;
		for (int i = 0; i < X[imax].size(); i++){
			int index = findID(sssg, X[imax][i]);//�ҵ��ýڵ���ԭͼ�е����

			for (int m = 0; m < X[imax][i]->nIn; m++){//�������������
				FlatNode *p = (X[imax][i]->inFlatNodes)[m];//�õ�����߶�Ӧ���϶˽ڵ�
				int partNum = getPart(p);
				if (partNum != imax){//����϶˽ڵ��Ӧ����ͼ����imax
					if (w[partNum] + vwgt[index] < w[imax]){//����ƶ��������ͼ��������С
						int tt = max(w[partNum] + vwgt[index], w[imax] - vwgt[index]); //�����ͼ������

						if (tt < t){ //�Թ�����Ϊ��Ҫָ��
							t = tt;
							choose = i;
							partChoose = partNum;
						}
						else if (tt == t){//���ܵ�ͨ�����Ƿ����
							int total_edge = getTotalEdge(sssg, k);

							FlatNode *q = X[imax][i];
							//�����ƶ�
							X[partNum].push_back(q);//���ýڵ����X[imin]
							w[partNum] += vwgt[index];//������ͼ�Ĺ�����

							X[imax].erase(X[imax].begin() + i);//���ýڵ��Ƴ�X[max]
							w[imax] -= vwgt[index];//������ͼ�Ĺ�����

							int total_edge_2 = getTotalEdge(sssg, k);

							if (total_edge_2 < total_edge){
								choose = i;//���ͨ������С�ˣ���ѡ��˽ڵ�
								partChoose = partNum;
							}

							X[imax].push_back(q);//���ýڵ����X[imax]
							w[imax] += vwgt[index];//������ͼ�Ĺ�����

							X[partNum].pop_back();//���ýڵ��Ƴ�X[imin]
							w[partNum] -= vwgt[index];//������ͼ�Ĺ�����
						}
					}
				}
			}

			for (int m = 0; m < X[imax][i]->nOut; m++){//�������������
				FlatNode *p = (X[imax][i]->outFlatNodes)[m];//�õ�����߶�Ӧ���϶˽ڵ�
				int partNum = getPart(p);
				if (partNum != imax){//����϶˽ڵ��Ӧ����ͼ����imax
					if (w[partNum] + vwgt[index] < w[imax]){//����ƶ��������ͼ��������С
						int tt = max(w[partNum] + vwgt[index], w[imax] - vwgt[index]); //�����ͼ������

						if (tt < t){ //�Թ�����Ϊ��Ҫָ��
							t = tt;
							choose = i;
							partChoose = partNum;
						}
						else if (tt == t){//���ܵ�ͨ�����Ƿ����
							int total_edge = getTotalEdge(sssg, k);

							FlatNode *q = X[imax][i];
							//�����ƶ�
							X[partNum].push_back(q);//���ýڵ����X[imin]
							w[partNum] += vwgt[index];//������ͼ�Ĺ�����

							X[imax].erase(X[imax].begin() + i);//���ýڵ��Ƴ�X[max]
							w[imax] -= vwgt[index];//������ͼ�Ĺ�����

							int total_edge_2 = getTotalEdge(sssg, k);

							if (total_edge_2 < total_edge){
								choose = i;//���ͨ������С�ˣ���ѡ��˽ڵ�
								partChoose = partNum;
							}

							X[imax].push_back(q);//���ýڵ����X[imax]
							w[imax] += vwgt[index];//������ͼ�Ĺ�����

							X[partNum].pop_back();//���ýڵ��Ƴ�X[imin]
							w[partNum] -= vwgt[index];//������ͼ�Ĺ�����
						}
					}
				}
			}
		}

		int index = findID(sssg, X[imax][choose]);
		if (w[partChoose] + vwgt[index] < w[imax]){

			X[partChoose].push_back(X[imax][choose]);//���ýڵ����X[imin]
			w[partChoose] += vwgt[index];//������ͼ�Ĺ�����

			X[imax].erase(X[imax].begin() + choose);//���ýڵ��Ƴ�X[max]
			w[imax] -= vwgt[index];//������ͼ�Ĺ�����
		}

		int tmax = 0;
		for (int i = 0; i < w.size(); i++){
			if (w[i] > w[tmax]){
				tmax = i;
			}
		}
		if (tmax == imax){
			cnt--; //���ԭ��ͼ�Ĺ�������Ȼ��������ѭ������
		}
		else{
			imax = tmax;
			cnt = X[imax].size();
		}
	}
}

void GreedyPartition::errorDecrease2(SchedulerSSG *sssg, int k){//����½��㷨
	double we = sssg->total_work / k;
	int imax = 0;
	int imin = 0;
	for (int i = 0; i < w.size(); i++){
		if (w[i] > w[imax]){
			imax = i;
		}
		if (w[i] < w[imin]){
			imin = i;
		}
	}

	int cnt = X[imax].size();
	while (w[imax] > we && cnt > 0){//����ƽ������ʱ��������½��㷨
		int t = w[imax];
		int choose = 0;
		for (int i = 0; i < X[imax].size(); i++){
			int index = findID(sssg, X[imax][i]);//�ҵ��ýڵ���ԭͼ�е����
			if (w[imin] + vwgt[index] < w[imax]){//���С�������ͼ������
				int tt = max(w[imin] + vwgt[index], w[imax] - vwgt[index]); //�����ͼ������
				if (tt < t){ //�Թ�����Ϊ��Ҫָ��
					t = tt;
					choose = i;
				}
				else if (tt == t){//���ܵ�ͨ�����Ƿ����
					int total_edge = getTotalEdge(sssg, k);
					FlatNode *q = X[imax][i];
					//�����ƶ�
					X[imin].push_back(q);//���ýڵ����X[imin]
					w[imin] += vwgt[index];//������ͼ�Ĺ�����

					X[imax].erase(X[imax].begin() + i);//���ýڵ��Ƴ�X[max]
					w[imax] -= vwgt[index];//������ͼ�Ĺ�����

					int total_edge_2 = getTotalEdge(sssg, k);;

					if (total_edge_2 < total_edge)
						choose = i;//���ͨ������С�ˣ���ѡ��˽ڵ�

					X[imax].push_back(q);//���ýڵ����X[imax]
					w[imax] += vwgt[index];//������ͼ�Ĺ�����

					X[imin].pop_back();//���ýڵ��Ƴ�X[imin]
					w[imin] -= vwgt[index];//������ͼ�Ĺ�����
				}
			}

		}

		int index = findID(sssg, X[imax][choose]);
		if (w[imin] + vwgt[index] < w[imax]){

			X[imin].push_back(X[imax][choose]);//���ýڵ����X[imin]
			w[imin] += vwgt[index];//������ͼ�Ĺ�����

			X[imax].erase(X[imax].begin() + choose);//���ýڵ��Ƴ�X[max]
			w[imax] -= vwgt[index];//������ͼ�Ĺ�����
		}

		int tmax = 0;
		int tmin = 0;
		for (int i = 0; i < w.size(); i++){
			if (w[i] > w[tmax]){
				tmax = i;
			}
			if (w[i] < w[tmin]){
				tmin = i;
			}
		}
		if (tmax == imax){
			imin = tmin;
			cnt--; //���ԭ��ͼ�Ĺ�������Ȼ��������ѭ������
		}
		else{
			imax = tmax;
			imin = tmin;
			cnt = X[imax].size();
		}
	}
}

FlatNodeState GreedyPartition::getFlatNodeState(FlatNode *p){
	int partNum = getPart(p);
	FlatNodeState res = INSIDE;
	int cnt1 = 0;
	int cnt2 = 0;
	int upNum = 0;
	int downNum = 0;
	FlatNode *q;
	for (int i = 0; i < p->nIn; i++){//�������������
		q = (p->inFlatNodes)[i];//�õ�����߶�Ӧ���϶˽ڵ�
		upNum = getPart(q);//�õ��϶˽ڵ��Ӧ�Ļ��ֺ�
		if (upNum == partNum){
			cnt1++;
		}
		else cnt2++;
	}
	for (int i = 0; i < p->nOut; i++){//�������������
		q = (p->outFlatNodes)[i];//�õ�����߶�Ӧ���¶˶˽ڵ�
		downNum = getPart(q);
		if (downNum == partNum){
			cnt1++;
		}
		else cnt2++;
	}
	if (cnt1 == 0)res = ALONE;
	else if (cnt2 == 0)res = INSIDE;
	else res = BORDER;
	return res;
}

void GreedyPartition::upDateFlatNodeState(FlatNode *p){
	FlatNodeToState[p] = getFlatNodeState(p);
	FlatNode *q;
	for (int i = 0; i < p->nIn; i++){//�������������
		q = (p->inFlatNodes)[i];//�õ�����߶�Ӧ���϶˽ڵ�
		FlatNodeToState[q] = getFlatNodeState(q);
	}
	for (int i = 0; i < p->nOut; i++){//�������������
		q = (p->outFlatNodes)[i];//�õ�����߶�Ӧ���¶˶˽ڵ�
		FlatNodeToState[q] = getFlatNodeState(q);
	}
}

void GreedyPartition::doTabuSearch(SchedulerSSG *sssg, int k){//���������Ż��㷨
	int total_edge = getTotalEdge(sssg, k);//�õ�ͨ����
	int imax = 0;
	for (int i = 0; i < w.size(); i++){//�õ�������ͼ������
		if (w[i] > imax){
			imax = w[i];
		}
	}

	vector<FlatNode*> aloneVec;
	for (int i = 0; i < X.size(); i++){//�õ�ÿ���ڵ��״̬
		for (int j = 0; j < X[i].size(); j++){
			FlatNodeState p = getFlatNodeState(X[i][j]);
			FlatNodeToState.insert({ X[i][j], p });
			if (p == ALONE){
				aloneVec.push_back(X[i][j]);
			}
		}
	}

	//��������ز��������£�ʹ����ڵ��Ϊ�߽�ڵ�
	int cnt = 0;
	while (aloneVec.size() > 0){
		FlatNode *p = aloneVec[cnt];
		int part = getPart(p);
		int index = findID(sssg, p);
		FlatNode *q;

		FlatNode *choose = NULL;
		int choosePart = 0;
		int minEdge = total_edge;
		for (int i = 0; i < p->nIn; i++){//�������������
			q = (p->inFlatNodes)[i];//�õ�����߶�Ӧ���϶˽ڵ�

			int partNum = getPart(q);
			X[partNum].push_back(p);
			w[partNum] += vwgt[index];

			auto iter = find(X[part].begin(), X[part].end(), p);
			X[part].erase(iter);
			w[part] -= vwgt[index];

			int total_edge2 = getTotalEdge(sssg, k);
			int imax2 = 0;
			for (int i = 0; i < w.size(); i++){//�õ�������ͼ������
				if (w[i] > imax2){
					imax2 = w[i];
				}
			}
			if (imax2 < imax * 1.01  && total_edge2 < total_edge){
				if (total_edge2 < minEdge){
					minEdge = total_edge2;
					choose = q;
					choosePart = partNum;
				}
			}

			auto iter2 = find(X[partNum].begin(), X[partNum].end(), p);
			X[partNum].erase(iter2);
			w[partNum] -= vwgt[index];

			X[part].push_back(p);
			w[part] += vwgt[index];
		}
		for (int i = 0; i < p->nOut; i++){//�������������
			q = (p->outFlatNodes)[i];//�õ�����߶�Ӧ���϶˽ڵ�

			int partNum = getPart(q);
			X[partNum].push_back(p);
			w[partNum] += vwgt[index];

			auto iter = find(X[part].begin(), X[part].end(), p);
			X[part].erase(iter);
			w[part] -= vwgt[index];

			int total_edge2 = getTotalEdge(sssg, k);
			int imax2 = 0;
			for (int i = 0; i < w.size(); i++){//�õ�������ͼ������
				if (w[i] > imax2){
					imax2 = w[i];
				}
			}
			if (imax2 < imax * 1.01 && total_edge2 < total_edge){
				if (total_edge2 < minEdge){
					minEdge = total_edge2;
					choose = q;
					choosePart = partNum;
				}
			}

			auto iter2 = find(X[partNum].begin(), X[partNum].end(), p);
			X[partNum].erase(iter2);
			w[partNum] -= vwgt[index];

			X[part].push_back(p);
			w[part] += vwgt[index];
		}

		if (choose != NULL){
			X[choosePart].push_back(p);
			w[choosePart] += vwgt[index];

			auto iter = find(X[part].begin(), X[part].end(), p);
			X[part].erase(iter);
			w[part] -= vwgt[index];

			upDateFlatNodeState(p);
			aloneVec.clear();
			for (int i = 0; i < X.size(); i++){//�õ�ÿ���ڵ��״̬
				for (int j = 0; j < X[i].size(); j++){
					FlatNodeState state = getFlatNodeState(X[i][j]);
					if (state == ALONE){
						aloneVec.push_back(X[i][j]);
					}
				}
			}
			total_edge = getTotalEdge(sssg, k);
			cnt = 0;
		}
		else {
			cnt++;
		}

		if (cnt == aloneVec.size())break;
	}
}

int GreedyPartition::orderPartitionResult(){//����ͼ�����ɴ�С����,ѡ�������㷨
	int k = getParts();//���ֵķ���
	int index = 0;
	for (int i = 0; i < k; i++){
		if (w[i] != 0){
			index++;
		}
	}
	vector<FlatNode *>t;
	int tw;
	int tedge;
	for (int i = 0; i < index; i++){
		int choose = i;
		for (int j = i + 1; j < k; j++){
			if (w[j] > w[choose]){
				choose = j;
			}
		}
		t = X[i];
		tw = w[i];
		tedge = edge[i];

		X[i] = X[choose];
		w[i] = w[choose];
		edge[i] = edge[choose];

		X[choose] = t;
		w[choose] = tw;
		edge[choose] = tedge;
	}
	return index;
}

void GreedyPartition::setActorWorkload(SchedulerSSG *sssg){
	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg���ж����vector
	map<FlatNode *, int> stadyWork = sssg->GetSteadyWorkMap();//��Ÿ���operator����̬����������
	map<FlatNode *, int> steadyCount = sssg->mapSteadyCount2FlatNode; // SDFͼ���нڵ��ȶ�״̬��������<�ڵ㣬ִ�д���>
	sssg->total_work = 0.0;//����SDF�ܵĽڵ㹤����
	for (int i = 0; i < nvtxs; i++){
		if (X86Backend || DynamicX86Backend)
		{
			vwgt[i] = stadyWork[V[i]] * steadyCount[V[i]];
		}
		else if (GPUBackend)
		{
			if (DetectiveActorState(sssg->GetFlatNodes()[i]))
			{
				vwgt[i] = 0;
			}
			else
			{
				vwgt[i] = stadyWork[V[i]] * steadyCount[V[i]];
			}
		}
		//	cout << i << " " << vwgt[i] << endl;
		sssg->total_work += vwgt[i];
	}
}

/* End ���� wangliang */