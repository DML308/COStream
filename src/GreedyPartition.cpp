/* Begin —— wangliang */
#include "GreedyPartition.h"

void GreedyPartition::SssgPartition(SchedulerSSG *sssg, int level){
	assert(level == 1);
	if (this->mnparts == 1)
	{//如果只有一个place则不作划分
		if (X86Backend || DynamicX86Backend)
		{
			for (int i = 0; i<nvtxs; i++)
				FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], 0));//建立节点到划分编号的映射
			for (int i = 0; i<nvtxs; i++)
				PartitonNum2FlatNode.insert(make_pair(0, sssg->GetFlatNodes()[i]));
		}
		else if (GPUBackend)
		{
			for (int i = 0; i<nvtxs; i++)
			{
				if (!DetectiveActorState(sssg->GetFlatNodes()[i]))
				{
					FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], 0));//建立节点到划分编号的映射
				}
				else
				{
					FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], 1));//建立节点到划分编号的映射
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

	nvtxs = sssg->GetFlatNodes().size();//顶点的个数
	int k = getParts();//划分的份数
	X.resize(k, vector<FlatNode *>());//k个子图
	w.resize(k, 0);//划分的K个子图每个子图的总工作量初始化为0
	edge.resize(k, 0);//划分的K个子图每个子图的通信边初始化为0
	vwgt.resize(nvtxs, 0);//每个顶点的权重(总工作量)初始化为0
	part.resize(nvtxs, 0);//每个节点对应的核号初始化为0

	setActorWorkload(sssg);//设置每个节点的总工作量

	doPartition(sssg, k);//基于拓扑排序的划分算法

	vector<vector<FlatNode *>> tmp_X = X;//暂时保存划分结果
	vector<int> tmp_w = w;//暂时保存划分结果

	int wmax = 0;//得到最大的子图工作量
	for (int i = 0; i < w.size(); i++){
		if (w[i] > wmax){
			wmax = w[i];
		}
	}
	int total_edge = getTotalEdge(sssg, k);//得到总通信量大小

	//cout << "------------------------" << endl << wmax << endl << total_edge << "------------------------" << endl;
	errorDecrease(sssg, k);//划分的误差下降算法
	//误差下降算法可能会出现加速比不变，通信量增大的情况（很少出现）
	//通过比较误差下降算法前后的加速比和通信量，来选择是误差下降算法前的划分结果还是误差下降算法后的划分结果
	vector<vector<FlatNode *>> tmp_X1 = X;//暂时保存划分结果
	vector<int> tmp_w1 = w;//暂时保存划分结果
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
	vector<vector<FlatNode *>> tmp_X2 = X;//暂时保存划分结果
	vector<int> tmp_w2 = w;//暂时保存划分结果
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
	doTabuSearch(sssg, k);//减小通信边的禁忌搜索算法

	getTotalEdge(sssg, k);//最后得到通信边的信息
	int s = orderPartitionResult();//对划分的结果进行重排

	for (int i = 0; i < X.size(); i++){//保存节点与之对应的核号
		for (int j = 0; j < X[i].size(); j++){
			part[findID(sssg, X[i][j])] = i;
		}
	}

	if (X86Backend || DynamicX86Backend)
	{
		for (int i = 0; i<nvtxs; i++){
			FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], part[i]));//建立节点到划分编号的映射
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
			FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i], part[i]));//建立节点到划分编号的映射
		}
		for (int i = 0; i<nvtxs; i++)
		{
			PartitonNum2FlatNode.insert(make_pair(part[i], sssg->GetFlatNodes()[i]));
		}
	}

}

void GreedyPartition::doPartition(SchedulerSSG *sssg, int k){
	int edgenum = sssg->GetMapEdge2DownFlatNode().size();//图的边数
	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg所有顶点的vector
	X[0] = V;//第一个子图初始化为全图

	sssg->edge_work = 0.0;//总的边通信初始化为0
	for (int i = 0; i < V.size(); i++){//计算原图的通信量
		for (int j = 0; j < V[i]->nIn; j++){//遍历划分后每一部分的节点
			sssg->edge_work += (V[i]->inPopWeights[j]) * (sssg->GetSteadyCount(V[i]));
		}
		for (int j = 0; j < V[i]->nOut; j++){//遍历划分后每一部分的节点
			sssg->edge_work += (V[i]->outPushWeights[j]) * (sssg->GetSteadyCount(V[i]));
		}
	}
	w[0] = sssg->total_work;
	//cout << "test************" <<endl<< sssg->total_work << endl << k << endl;
	double we = sssg->total_work / k;//每个子图的平均工作量
	double e = 2 - ee;//满足系数（2-ee）即可

	int choose = 0;//所选节点在X[0]中的序号
	int index = 0;//所选节点在原图中的序号
	for (int i = 1; i < k; i++){//构造子图X[1] ~ X[k-1]
		S.clear();
		while (w[i] < we * e && X[0].size() != 0){//一定要加X[0].size() != 0，否则137行可能会出bug
			choose = 0;
			index = 0;
			if (S.size() == 0){//如果候选集合为空
				if (X[0].size() == 0)choose = 0;
				else {//choose = rand() % (X[0].size());//从X[0]中随机选择一个节点

					//选择X[0]中顶点权重最大的节点
					int tmax = findID(sssg, X[0][0]);
					for (int j = 1; j < X[0].size(); j++){
						int t = findID(sssg, X[0][j]);//找到该节点在原图中的序号
						if (vwgt[t] > vwgt[tmax])tmax = t;
					}
					choose = find(X[0].begin(), X[0].end(), V[tmax]) - X[0].begin();
				}
				index = findID(sssg, X[0][choose]);//找到该节点在原图中的序号

				X[i].push_back(X[0][choose]);//将该节点加入X[i]
				w[i] += vwgt[index];//调整子图的工作量

				X[0].erase(X[0].begin() + choose);//将该节点移出X[0]
				w[0] -= vwgt[index];//调整子图的工作量
			}
			else{//选择候选集合中收益函数值最大的节点
				int maxGain = chooseMaxGain(sssg, S, X[i], X[0]);//找到该节点在S中的序号
				choose = find(X[0].begin(), X[0].end(), S[maxGain]) - X[0].begin();//找到该节点在X[0]中的序号
				index = findID(sssg, X[0][choose]);//找到该节点在原图中的序号

				X[i].push_back(X[0][choose]);//将该节点加入X[i]
				w[i] += vwgt[index];//调整子图的工作量

				X[0].erase(X[0].begin() + choose);//将该节点移出X[0]
				w[0] -= vwgt[index];//调整子图的工作量

				S.erase(S.begin() + maxGain);//将该节点从候选集合中删除
			}
			updateCandidate(sssg, X[0], S, index);//更新候选节点集合
		}
	}

}

int GreedyPartition::chooseMaxGain(SchedulerSSG *sssg, vector<FlatNode *>& S, vector<FlatNode *>& Xi, vector<FlatNode *>& X0){
	int result = 0;
	int maxGain = INT_MIN;
	for (int i = 0; i < S.size(); i++){//遍历所有候选节点
		int wi = 0;
		int w0 = 0;
		for (int j = 0; j < S[i]->nIn; j++){//遍历所有输入边
			FlatNode *p = (S[i]->inFlatNodes)[j];//得到输入边对应的上端节点
			if (find(Xi.begin(), Xi.end(), p) != Xi.end()){//如果上端节点在Xi中
				wi += (S[i]->inPopWeights[j]) * (sssg->GetSteadyCount(S[i]));
			}
			else if (find(X0.begin(), X0.end(), p) != X0.end()){//如果上端节点在X0中
				w0 += (S[i]->inPopWeights[j]) * (sssg->GetSteadyCount(S[i]));
			}
		}
		for (int j = 0; j < S[i]->nOut; j++){//遍历所有输出边
			FlatNode *p = (S[i]->outFlatNodes)[j];//得到输出边对应的下端端节点
			if (find(Xi.begin(), Xi.end(), p) != Xi.end()){//如果下端节点在Xi中
				wi += (S[i]->outPushWeights[j]) * (sssg->GetSteadyCount(S[i]));
			}
			else if (find(X0.begin(), X0.end(), p) != X0.end()){//如果下端节点在X0中
				w0 += (S[i]->outPushWeights[j]) * (sssg->GetSteadyCount(S[i]));
			}
		}
		if (wi - w0 > maxGain){
			maxGain = wi - w0;
			result = i;//更新最大收益节点
		}
	}
	return result;
}

void GreedyPartition::updateCandidate(SchedulerSSG *sssg, vector<FlatNode *>& X0, vector<FlatNode *>& S, int index)
{
	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg所有顶点的vector
	for (int j = 0; j < V[index]->nIn; j++){//遍历所有输入边
		FlatNode *p = (V[index]->inFlatNodes)[j];//得到输入边对应的上端节点
		if (find(X0.begin(), X0.end(), p) != X0.end() && find(S.begin(), S.end(), p) == S.end()){//如果上端节点在X0中并且不在S中
			S.push_back(p);//将它加入S中
		}
	}
	for (int j = 0; j < V[index]->nOut; j++){//遍历所有输出边
		FlatNode *p = (V[index]->outFlatNodes)[j];//得到输出边对应的下端节点
		if (find(X0.begin(), X0.end(), p) != X0.end() && find(S.begin(), S.end(), p) == S.end()){//如果上端节点在X0中并且不在S中
			S.push_back(p);//将它加入S中
		}
	}
}

int GreedyPartition::getTotalEdge(SchedulerSSG *sssg, int k){//得到各核间通信量的大小和总大小
	edge.clear();
	edge.resize(k, 0);
	int total_edge = 0;
	for (int i = 0; i < k; i++){//计算此时的通信量
		for (int j = 0; j < X[i].size(); j++){//遍历划分后每一部分的节点
			for (int m = 0; m < X[i][j]->nIn; m++){//遍历所有输入边
				FlatNode *p = (X[i][j]->inFlatNodes)[m];//得到输入边对应的上端节点
				if (find(X[i].begin(), X[i].end(), p) == X[i].end()){//如果上端节点不在Xi中
					edge[i] += (X[i][j]->inPopWeights[m]) * (sssg->GetSteadyCount(X[i][j]));
				}
			}
			for (int m = 0; m < X[i][j]->nOut; m++){//遍历所有输出边
				FlatNode *p = (X[i][j]->outFlatNodes)[m];//得到输出边对应的下端端节点
				if (find(X[i].begin(), X[i].end(), p) == X[i].end()){//如果下端节点不在Xi中
					edge[i] += (X[i][j]->outPushWeights[m]) * (sssg->GetSteadyCount(X[i][j]));
				}
			}
		}
		total_edge += edge[i];
	}
	return total_edge;
}

void GreedyPartition::errorDecrease(SchedulerSSG *sssg, int k){//误差下降算法
	double we = sssg->total_work / k;
	int imax = 0;
	for (int i = 0; i < w.size(); i++){
		if (w[i] > w[imax]){
			imax = i;
		}
	}

	int cnt = X[imax].size();
	while (w[imax] > we && cnt > 0){//大于平衡因子时进行误差下降算法
		int t = w[imax];

		int choose = 0;
		int partChoose = 0;
		for (int i = 0; i < X[imax].size(); i++){
			int index = findID(sssg, X[imax][i]);//找到该节点在原图中的序号

			for (int m = 0; m < X[imax][i]->nIn; m++){//遍历所有输入边
				FlatNode *p = (X[imax][i]->inFlatNodes)[m];//得到输入边对应的上端节点
				int partNum = getPart(p);
				if (partNum != imax){//如果上端节点对应的子图不是imax
					if (w[partNum] + vwgt[index] < w[imax]){//如果移动后最大子图工作量减小
						int tt = max(w[partNum] + vwgt[index], w[imax] - vwgt[index]); //最大子图工作量

						if (tt < t){ //以工作量为首要指标
							t = tt;
							choose = i;
							partChoose = partNum;
						}
						else if (tt == t){//看总的通信量是否减少
							int total_edge = getTotalEdge(sssg, k);

							FlatNode *q = X[imax][i];
							//尝试移动
							X[partNum].push_back(q);//将该节点加入X[imin]
							w[partNum] += vwgt[index];//调整子图的工作量

							X[imax].erase(X[imax].begin() + i);//将该节点移出X[max]
							w[imax] -= vwgt[index];//调整子图的工作量

							int total_edge_2 = getTotalEdge(sssg, k);

							if (total_edge_2 < total_edge){
								choose = i;//如果通信量减小了，就选择此节点
								partChoose = partNum;
							}

							X[imax].push_back(q);//将该节点加入X[imax]
							w[imax] += vwgt[index];//调整子图的工作量

							X[partNum].pop_back();//将该节点移出X[imin]
							w[partNum] -= vwgt[index];//调整子图的工作量
						}
					}
				}
			}

			for (int m = 0; m < X[imax][i]->nOut; m++){//遍历所有输入边
				FlatNode *p = (X[imax][i]->outFlatNodes)[m];//得到输入边对应的上端节点
				int partNum = getPart(p);
				if (partNum != imax){//如果上端节点对应的子图不是imax
					if (w[partNum] + vwgt[index] < w[imax]){//如果移动后最大子图工作量减小
						int tt = max(w[partNum] + vwgt[index], w[imax] - vwgt[index]); //最大子图工作量

						if (tt < t){ //以工作量为首要指标
							t = tt;
							choose = i;
							partChoose = partNum;
						}
						else if (tt == t){//看总的通信量是否减少
							int total_edge = getTotalEdge(sssg, k);

							FlatNode *q = X[imax][i];
							//尝试移动
							X[partNum].push_back(q);//将该节点加入X[imin]
							w[partNum] += vwgt[index];//调整子图的工作量

							X[imax].erase(X[imax].begin() + i);//将该节点移出X[max]
							w[imax] -= vwgt[index];//调整子图的工作量

							int total_edge_2 = getTotalEdge(sssg, k);

							if (total_edge_2 < total_edge){
								choose = i;//如果通信量减小了，就选择此节点
								partChoose = partNum;
							}

							X[imax].push_back(q);//将该节点加入X[imax]
							w[imax] += vwgt[index];//调整子图的工作量

							X[partNum].pop_back();//将该节点移出X[imin]
							w[partNum] -= vwgt[index];//调整子图的工作量
						}
					}
				}
			}
		}

		int index = findID(sssg, X[imax][choose]);
		if (w[partChoose] + vwgt[index] < w[imax]){

			X[partChoose].push_back(X[imax][choose]);//将该节点加入X[imin]
			w[partChoose] += vwgt[index];//调整子图的工作量

			X[imax].erase(X[imax].begin() + choose);//将该节点移出X[max]
			w[imax] -= vwgt[index];//调整子图的工作量
		}

		int tmax = 0;
		for (int i = 0; i < w.size(); i++){
			if (w[i] > w[tmax]){
				tmax = i;
			}
		}
		if (tmax == imax){
			cnt--; //如果原子图的工作量依然最大，则减少循环次数
		}
		else{
			imax = tmax;
			cnt = X[imax].size();
		}
	}
}

void GreedyPartition::errorDecrease2(SchedulerSSG *sssg, int k){//误差下降算法
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
	while (w[imax] > we && cnt > 0){//大于平衡因子时进行误差下降算法
		int t = w[imax];
		int choose = 0;
		for (int i = 0; i < X[imax].size(); i++){
			int index = findID(sssg, X[imax][i]);//找到该节点在原图中的序号
			if (w[imin] + vwgt[index] < w[imax]){//如果小于最大子图工作量
				int tt = max(w[imin] + vwgt[index], w[imax] - vwgt[index]); //最大子图工作量
				if (tt < t){ //以工作量为首要指标
					t = tt;
					choose = i;
				}
				else if (tt == t){//看总的通信量是否减少
					int total_edge = getTotalEdge(sssg, k);
					FlatNode *q = X[imax][i];
					//尝试移动
					X[imin].push_back(q);//将该节点加入X[imin]
					w[imin] += vwgt[index];//调整子图的工作量

					X[imax].erase(X[imax].begin() + i);//将该节点移出X[max]
					w[imax] -= vwgt[index];//调整子图的工作量

					int total_edge_2 = getTotalEdge(sssg, k);;

					if (total_edge_2 < total_edge)
						choose = i;//如果通信量减小了，就选择此节点

					X[imax].push_back(q);//将该节点加入X[imax]
					w[imax] += vwgt[index];//调整子图的工作量

					X[imin].pop_back();//将该节点移出X[imin]
					w[imin] -= vwgt[index];//调整子图的工作量
				}
			}

		}

		int index = findID(sssg, X[imax][choose]);
		if (w[imin] + vwgt[index] < w[imax]){

			X[imin].push_back(X[imax][choose]);//将该节点加入X[imin]
			w[imin] += vwgt[index];//调整子图的工作量

			X[imax].erase(X[imax].begin() + choose);//将该节点移出X[max]
			w[imax] -= vwgt[index];//调整子图的工作量
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
			cnt--; //如果原子图的工作量依然最大，则减少循环次数
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
	for (int i = 0; i < p->nIn; i++){//遍历所有输入边
		q = (p->inFlatNodes)[i];//得到输入边对应的上端节点
		upNum = getPart(q);//得到上端节点对应的划分号
		if (upNum == partNum){
			cnt1++;
		}
		else cnt2++;
	}
	for (int i = 0; i < p->nOut; i++){//遍历所有输出边
		q = (p->outFlatNodes)[i];//得到输出边对应的下端端节点
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
	for (int i = 0; i < p->nIn; i++){//遍历所有输入边
		q = (p->inFlatNodes)[i];//得到输入边对应的上端节点
		FlatNodeToState[q] = getFlatNodeState(q);
	}
	for (int i = 0; i < p->nOut; i++){//遍历所有输出边
		q = (p->outFlatNodes)[i];//得到输出边对应的下端端节点
		FlatNodeToState[q] = getFlatNodeState(q);
	}
}

void GreedyPartition::doTabuSearch(SchedulerSSG *sssg, int k){//禁忌搜索优化算法
	int total_edge = getTotalEdge(sssg, k);//得到通信量
	int imax = 0;
	for (int i = 0; i < w.size(); i++){//得到最大的子图工作量
		if (w[i] > imax){
			imax = w[i];
		}
	}

	vector<FlatNode*> aloneVec;
	for (int i = 0; i < X.size(); i++){//得到每个节点的状态
		for (int j = 0; j < X[i].size(); j++){
			FlatNodeState p = getFlatNodeState(X[i][j]);
			FlatNodeToState.insert({ X[i][j], p });
			if (p == ALONE){
				aloneVec.push_back(X[i][j]);
			}
		}
	}

	//保持最大负载不变的情况下，使孤零节点变为边界节点
	int cnt = 0;
	while (aloneVec.size() > 0){
		FlatNode *p = aloneVec[cnt];
		int part = getPart(p);
		int index = findID(sssg, p);
		FlatNode *q;

		FlatNode *choose = NULL;
		int choosePart = 0;
		int minEdge = total_edge;
		for (int i = 0; i < p->nIn; i++){//遍历所有输入边
			q = (p->inFlatNodes)[i];//得到输入边对应的上端节点

			int partNum = getPart(q);
			X[partNum].push_back(p);
			w[partNum] += vwgt[index];

			auto iter = find(X[part].begin(), X[part].end(), p);
			X[part].erase(iter);
			w[part] -= vwgt[index];

			int total_edge2 = getTotalEdge(sssg, k);
			int imax2 = 0;
			for (int i = 0; i < w.size(); i++){//得到最大的子图工作量
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
		for (int i = 0; i < p->nOut; i++){//遍历所有输出边
			q = (p->outFlatNodes)[i];//得到输入边对应的上端节点

			int partNum = getPart(q);
			X[partNum].push_back(p);
			w[partNum] += vwgt[index];

			auto iter = find(X[part].begin(), X[part].end(), p);
			X[part].erase(iter);
			w[part] -= vwgt[index];

			int total_edge2 = getTotalEdge(sssg, k);
			int imax2 = 0;
			for (int i = 0; i < w.size(); i++){//得到最大的子图工作量
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
			for (int i = 0; i < X.size(); i++){//得到每个节点的状态
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

int GreedyPartition::orderPartitionResult(){//按子图负载由大到小排序,选择排序算法
	int k = getParts();//划分的分数
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
	vector<FlatNode *> V = sssg->GetFlatNodes();//sssg所有顶点的vector
	map<FlatNode *, int> stadyWork = sssg->GetSteadyWorkMap();//存放各个operator的稳态工作量估计
	map<FlatNode *, int> steadyCount = sssg->mapSteadyCount2FlatNode; // SDF图所有节点稳定状态调度序列<节点，执行次数>
	sssg->total_work = 0.0;//计算SDF总的节点工作量
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

/* End —— wangliang */