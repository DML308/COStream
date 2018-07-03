#include "MetisPartiton.h"
#include "3rdpart/include/metis.h"

using namespace std;

MetisPartiton::MetisPartiton(int objtype,int contig):Partition()
{
	options[0] = 1;//METIS_OPTION_PTYPE              Multilevel k-way partitioning.
	options[1] = objtype;//METIS_OPTION_OBJTYPE**    0 means Edge-cut minimization.
	options[2] = 1;//METIS_OPTION_CTYPE**            Sorted heavy-edge matching
	options[3] = 4;//METIS_OPTION_IPTYPE             Grow a bisection using a greedy node-based strategy
	options[4] = 1;//METIS_OPTION_RTYPE              Greedy-based cut and volume refinement.
	options[5] = 0;//METIS_OPTION_DBGLVL**           The default value is 0 (no debugging/progress information).
	options[6] = 10;//METIS_OPTION_NITER**
	options[7] = 1;//METIS_OPTION_NCUTS**
	options[8] = -1;//METIS_OPTION_SEED**            Specifies the seed for the random number generator.
	options[9] = 0;//METIS_OPTION_MINCONN**          0 Does not explicitly minimize the maximum connectivity.
	options[10] = contig;//METIS_OPTION_CONTIG**     0 means Does not force contiguous partitions.
	//options[11]=1;//
	options[15] = 30;//METIS_OPTION_UFACTOR
	options[16] = 0;//METIS_OPTION_NUMBERING
	options[20] = 0;//NOOutPut
	options[21] = 0;
    tpwgts=NULL;
    ubvec=NULL;
	mncon=1;
}

 void  MetisPartiton::SssgPartition(SchedulerSSG *sssg , int level)
{
	assert(level==1);
	nvtxs=sssg->GetFlatNodes().size();
	if(this->mnparts ==1 ){//如果只有一个place则不作划分
		if (X86Backend)
		{
			for (int i=0;i<nvtxs;i++)
				FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i],0));//建立节点到划分编号的映射
			for (int i=0;i<nvtxs;i++)
				PartitonNum2FlatNode.insert(make_pair(0,sssg->GetFlatNodes()[i]));
		}
		else if (GPUBackend)
		{
			for (int i=0;i<nvtxs;i++)
			{
				if (!DetectiveActorState(sssg->GetFlatNodes()[i]))
				{
					FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i],0));//建立节点到划分编号的映射
				}
				else
				{
					FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i],1));//建立节点到划分编号的映射
				}
			}
			for (int i=0;i<nvtxs;i++)
			{
				if (!DetectiveActorState(sssg->GetFlatNodes()[i]))
				{
					PartitonNum2FlatNode.insert(make_pair(0,sssg->GetFlatNodes()[i]));
				}
				else
				{
					PartitonNum2FlatNode.insert(make_pair(1,sssg->GetFlatNodes()[i]));
				}
			}
		}
		

#if 1 //打印图
        // delete by mobinsheng for  BSPartitionGraph.dot generate path error!
        // DumpStreamGraph(sssg,this,"BSPartitionGraph.dot",NULL);//zww_20120605添加第四个参数
        // delete by mobinsheng for  BSPartitionGraph.dot generate path error!
#endif

		return;
	}
	/************************************************************************/
	/* metis 任务调度，结合边通信以及负载均衡                               */
	/************************************************************************/
	vector<int>xadj(nvtxs+1,0);//动态定义xadj数组
	vector<int>vwgt(nvtxs);
	vector<int>part(nvtxs);
	vector<int>vsize(nvtxs,0);
	//xadj[0]=0;
	int edgenum=sssg->GetMapEdge2DownFlatNode().size();//图的边数
	vector<int>adjncy(edgenum*2);
	vector<int>adjwgt(edgenum*2);//用于存储边的权重
	int k=0;//k用于记录flagnode的相邻节点数
	map<FlatNode *, int>::iterator iter;
	typedef multimap<int,FlatNode *>::iterator iter1;
	map<FlatNode *, int> tmp = sssg->GetSteadyWorkMap();

	for(int i=0;i<nvtxs;i++){
		// vsize[i]=0;
		int flag=0;//保证sum只加一次
		int sum=0;
		sum+=xadj[i];             
		if (sssg->GetFlatNodes()[i]->nOut!=0){  
			flag=1;
			xadj[i+1]=sum+sssg->GetFlatNodes()[i]->nOut;
			for (int j=0;j<sssg->GetFlatNodes()[i]->nOut;j++){
			  adjncy[k]=findID(sssg,sssg->GetFlatNodes()[i]->outFlatNodes[j]);
			  adjwgt[k]=sssg->GetFlatNodes()[i]->outPushWeights[j]*sssg->GetSteadyCount(sssg->GetFlatNodes()[i]);
			  vsize[i]+=adjwgt[k];
			  k++;
			}
		}
		// cout<<"here"<<endl;
		if (sssg->GetFlatNodes()[i]->nIn!=0){
			if (flag==0){
				xadj[i+1]=sum+sssg->GetFlatNodes()[i]->nIn;
			}else{
				xadj[i+1]+=sssg->GetFlatNodes()[i]->nIn;
			}
			for (int j=0;j<sssg->GetFlatNodes()[i]->nIn;j++)
			{
			  adjncy[k]=findID(sssg,sssg->GetFlatNodes()[i]->inFlatNodes[j]);
			  adjwgt[k]=sssg->GetFlatNodes()[i]->inPopWeights[j]*sssg->GetSteadyCount(sssg->GetFlatNodes()[i]);//cout<<"here"<<endl;
			  vsize[i]+=adjwgt[k];
			  k++;
			}
		}

		iter=tmp.find(sssg->GetFlatNodes()[i]);
		//iter=tmp.begin();
		//iter = ((map<FlatNode *,int>)(sssg->GetWorkMap())).begin();

		//cout<<(sssg->GetFlatNodes())[i]->name<<endl;
		 // cout<<iter->second<<endl;;
		if (X86Backend)
		{
			vwgt[i]=sssg->GetSteadyCount(sssg->GetFlatNodes()[i])*iter->second;
		}
		else if (GPUBackend)
		{
			if (DetectiveActorState(sssg->GetFlatNodes()[i]))
			{
				vwgt[i] = 0;
			}
			else
			{
				vwgt[i]=sssg->GetSteadyCount(sssg->GetFlatNodes()[i])*iter->second;
			}
		}
		
	} 

	mxadj=&xadj[0];
	madjncy=&adjncy[0];
	//madjwgt=&adjwgt[0];
	madjwgt=NULL;
	mvsize=&vsize[0];
	mpart=&part[0];
	mvwgt=&vwgt[0];

	METIS_PartGraphKway(&nvtxs,&mncon,mxadj,madjncy,mvwgt,mvsize,madjwgt,&mnparts,tpwgts,ubvec,options,&objval,mpart);
	if (X86Backend)
	{
		for (int i=0;i<nvtxs;i++){
			//cout<<part[i]<<endl;
			FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i],part[i]));//建立节点到划分编号的映射
		}

		for (int i=0;i<nvtxs;i++)
			PartitonNum2FlatNode.insert(make_pair(part[i],sssg->GetFlatNodes()[i]));
	}
	else if (GPUBackend)
	{
		int Num = this->mnparts;
		for (int i=0;i<nvtxs;i++){
			//cout<<part[i]<<endl;
			if (DetectiveActorState(sssg->GetFlatNodes()[i]))
			{
				part[i] = Num;
			}
			FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i],part[i]));//建立节点到划分编号的映射
		}

		for (int i=0;i<nvtxs;i++)
		{
			PartitonNum2FlatNode.insert(make_pair(part[i],sssg->GetFlatNodes()[i]));
		}
	}

	cout<<"The total communication volume or edge-cut of the solution is:"<<objval<<endl;

#if 1 //打印图
	// delete by mobinsheng for  BSPartitionGraph.dot generate path error!
	//DumpStreamGraph(sssg,this,"BSPartitionGraph.dot",NULL);//zww_20120605添加第四个参数
#endif
}


 //************************************20120924 zww 添加
 // Method:    metisPartition
 // FullName:  MetisPartiton::metisPartition
 // Access:    public 
 // Returns:   void
 // Qualifier:
 // Parameter: int nvtx 图的顶点数目
 // Parameter: int mncon 划分的限制条件的数目（至少为1）
 // Parameter: int * mxadj，int * madjncy 存储的是图的结构（顶点以及顶点间的连接关系）
 // Parameter: int * mvwgt 顶点的权值(工作量)
 // Parameter: int * mvsize 节点发送和接受的数据量
 // Parameter: int * madjwgt 边的权重
 // Parameter: int mnparts 最终划分的份数
 // Parameter: float * tpwgts 每一个划分与每一个限制条件的关联的权值，为空表示均匀划分
 // Parameter: float * ubvec 记录划分不同限制条件在划分过程中占得权值，为空代表只考虑负载
 // Parameter: int options[40] 划分的各种选项
 // Parameter: int objval 划分完成后记录图的通信量
 // Parameter: int *mpart 存储的是节点对应的划分编号
 //************************************
 void MetisPartiton::metisPartition(int nvtx,int mncon,int *mxadj,int *madjncy,int *mvwgt,int *mvsize,int *madjwgt,int mnparts,float *tpwgts,float *ubvec,int objval,int *mpart)
 {
	 METIS_PartGraphKway(&nvtx,&mncon,mxadj,madjncy,mvwgt,mvsize,madjwgt,&mnparts,tpwgts,ubvec,options,&objval,mpart);
 }


 MetisPartiton::~MetisPartiton()
 {
	 delete []mxadj;mxadj=NULL;
	 delete []madjncy;madjncy=NULL;
	 delete []mobjval;mobjval=NULL;
	 delete []mpart;mpart=NULL;
	 delete []mvwgt;mvwgt=NULL;
	 delete []madjwgt;madjwgt=NULL;
	 delete []tpwgts;tpwgts=NULL;
	 delete []mvsize;mvsize=NULL;
	 delete []ubvec;ubvec=NULL;
 }
