#include "dumpGraph.h"
using namespace std;
static stringstream buf;
static SchedulerSSG *Ssg;
static Partition *mpp;

static stringstream vertexBuf;
static stringstream edgeBuf;
static stringstream  *vertexBufInPlace = NULL;//用于记录各place对应的buf

//146种颜色，划分部分大于给定颜色种类则无法表示
static string color[]={"aliceblue", "antiquewhite", "yellowgreen", "aquamarine", "azure",
	"magenta", "maroon", "mediumaquamarine", "mediumblue", "mediumorchid",
	"mediumpurple", "mediumseagreen", "mediumslateblue", "mediumspringgreen", "mediumturquoise",
	"mediumvioletred", "midnightblue", "mintcream", "mistyrose", "moccasin",
	"navajowhite", "navy", "oldlace", "olive", "olivedrab",
	"orange", "orangered", "orchid", "palegoldenrod", "palegreen",
	"paleturquoise", "palevioletred", "papayawhip", "peachpuff", "peru",
	"pink", "plum", "powderblue", "purple", "red",
	"rosybrown", "royalblue", "saddlebrown", "salmon", "sandybrown",
	"seagreen", "seashell", "sienna", "silver", "skyblue",
	"slateblue", "slategray", "slategrey", "snow", "springgreen",
	"steelblue", "tan", "teal", "thistle", "tomato",
	"deeppink", "deepskyblue", "dimgray", "dimgrey", "dodgerblue",
	"firebrick", "floralwhite", "forestgreen", "fuchsia", "gainsboro",
	"ghostwhite", "gold", "goldenrod", "gray", "grey",
	"green", "greenyellow", "honeydew", "hotpink", "indianred",
	"indigo", "ivory", "khaki", "lavender", "lavenderblush",
	"lawngreen", "lemonchiffon", "lightblue", "lightcoral", "lightcyan",
	"lightgoldenrodyellow", "lightgray", "lightgreen", "lightgrey", "lightpink",
	"lightsalmon", "lightseagreen", "lightskyblue", "lightslategray", "lightslategrey",
	"lightsteelblue", "lightyellow", "lime&nbsp", "limegreen", "linen",
	"beige", "bisque", "yellow", "blanchedalmond", "blue",
	"blueviolet", "brown", "burlywood", "cadetblue", "chartreuse",
	"chocolate", "coral", "cornflowerblue", "cornsilk", "crimson",
	"cyan", "darkblue", "darkcyan", "darkgoldenrod", "darkgray",
	"darkgreen", "darkgrey", "darkkhaki", "darkmagenta", "darkolivegreen",
	"darkorange", "darkorchid", "darkred", "darksalmon", "darkseagreen",
	"darkslateblue", "darkslategray", "darkslategrey", "darkturquoise", "darkviolet",
	"turquoise", "violet", "wheat", "white", "whitesmoke"};

void MyVisitNode(FlatNode *node)/*访问该结点并打印相关的信息*/
{
	node->VisitNode();

	if (node->contents!=NULL ) {
		//we are visiting a flatnode

		//print the name and multiplicities and some other crap
		//buf <<node->name << "[ label = \"" <<	node->GetOperatorName() << "\\n";//20120619
		buf << "\n" << node->name << "[ label = \"" <<	node->name << "\\n";
		buf<<"init Mult: " <<Ssg->GetInitCount(node) << 
			" steady Mult: "<<Ssg->GetSteadyCount(node) <<"\\n";
		buf<<"init work: " <<Ssg->GetInitWork(node)<<              
			" steady work:" <<Ssg->GetSteadyWork(node)<<"\\n";
		for (int i = 0; i < node->nIn; i++)
			buf<<" peek: " << node->inPeekWeights[i]; 
		if(node->nIn!=0)buf<<"\\n";
		for (int j = 0; j < node->nIn; j++)
			buf<<" pop: " << node->inPopWeights[j];
		if(node->nIn!=0)buf<<"\\n";
		for (int k = 0; k < node->nOut; k++)
			buf<<" push: " << node->outPushWeights[k];
		if(node->nOut!=0)buf<<"\\n";
		buf<<"\"";
		if(mpp != NULL)//Partition后则对节点着色
		{		
			buf<<" color=\""<<color[mpp->findPartitionNumForFlatNode(node)]<<"\""; 
			buf<<" style=\"filled\" "; 	
		}
		buf<<"]";
	}
	//假如当前遇到的node是一个多输入的结点，该结点的链接边如下处理
	if (node->nIn > 1)
	{
		for (int i = 0; i < node->nIn; i++) {
			//joiners may have null upstream neighbors
			if (node->inFlatNodes[i] == NULL)
				continue;
			buf<< node->inFlatNodes[i]->name << " -> " << node->name;
			buf<< "[label=\"1\"];\n";//输出权重固定为1
		}
	}

	//create the arcs for the outgoing edges
	for (int i = 0; i < node->nOut; i++) {
		if (node->outFlatNodes[i] == NULL)
			continue;
		if(node->outFlatNodes[i]->nIn > 1)
			continue;
		buf<<node->name << " -> " 
			<< node->outFlatNodes[i]->name;
		buf<<"[label=\"" << node->outPushWeights[i] << "\"];\n";
	}
}


void MyClusterVisitNode(FlatNode *node, ClusterPartition *cp)/*访问该结点并打印相关的信息*/
{//二级划分的结果
	node->VisitNode();
	stringstream tmp;
	assert(cp);
	if (node->contents!=NULL ) {
		//we are visiting a flatnode

		//print the name and multiplicities and some other crap
		tmp << "\n" << node->name << "[ label = \"" <<	node->name << "\\n";
		tmp<<"init Mult: " << Ssg->GetInitCount(node) << " steady Mult: "<< cp->GetSteadyCount(node) << "\\n";
		int tmpInit, tmpSteady;
		tmpInit = Ssg->GetInitWork(node);
		tmpSteady = Ssg->GetSteadyWork(node);

#if 1
		tmp<<"init work: " << tmpInit << " steady work:" << tmpSteady <<"\\n";
#endif

		for (int i = 0; i < node->nIn; i++)
			tmp<<" peek: " << node->inPeekWeights[i]; 
		if(node->nIn!=0)tmp<<"\\n";
		for (int j = 0; j < node->nIn; j++)
			tmp<<" pop: " << node->inPopWeights[j];
		if(node->nIn!=0) tmp<<"\\n";
		for (int k = 0; k < node->nOut; k++)
			tmp <<" push: " << node->outPushWeights[k];
		if(node->nOut!=0)tmp<<"\\n";
		tmp<<"\"";
		std::pair<int, int> cluster2Core = cp->GetClusterCoreNum(node);
		tmp<<" color=\""<<color[cluster2Core.first * cp->GetCores() + cluster2Core.second]<<"\""; 
		//cout<<cluster2Core.first<< " * "<<cp->GetCores()<<" + "<<cluster2Core.second<<" = "<<cluster2Core.first * cp->GetCores() + cluster2Core.second<<endl;
		tmp<<" style=\"filled\""; 	
		tmp<<"]";

		vertexBuf << tmp.str();
		vertexBufInPlace[cluster2Core.first] << tmp.str();
	}
	//假如当前遇到的node是一个多输入的结点，该结点的链接边如下处理
	if (node->nIn > 1)
	{
		for (int i = 0; i < node->nIn; i++) {
			//joiners may have null upstream neighbors
			if (node->inFlatNodes[i] == NULL)
				continue;
			edgeBuf<< node->inFlatNodes[i]->name << " -> " << node->name;
			edgeBuf<< "[label=\"1\"];\n";//输出权重固定为1
		}
	}

	//create the arcs for the outgoing edges
	for (int i = 0; i < node->nOut; i++) {
		if (node->outFlatNodes[i] == NULL)
			continue;
		if(node->outFlatNodes[i]->nIn > 1)
			continue;
		edgeBuf<<node->name << " -> " 
			<< node->outFlatNodes[i]->name;
		edgeBuf<<"[label=\"" << cp->GetSteadyCount(node) * node->outPushWeights[i] << "\"];\n";
	}
}

void toBuildOutPutString(FlatNode *node)
{
	MyVisitNode(node);
	for (int i = 0; i < node->nOut; i++) {/*深度优先遍历*/
		if (node->outFlatNodes[i] == NULL || node->outFlatNodes[i]->GetVisitTimes() != 0)//该结点的后续结点还未被访问过
			continue;
		toBuildOutPutString(node->outFlatNodes[i]);
	}
}


void toBuildClusterOutPutString(FlatNode *node, ClusterPartition *cp)
{
	MyClusterVisitNode(node, cp);
	for (int i = 0; i < node->nOut; i++) {/*深度优先遍历*/
		if (node->outFlatNodes[i] == NULL || node->outFlatNodes[i]->GetVisitTimes() != 0)//该结点的后续结点还未被访问过
			continue;
		toBuildClusterOutPutString(node->outFlatNodes[i],cp);
	}

}

GLOBAL void DumpStreamGraph(SchedulerSSG *sssg,Partition *mp,const char *fileName, ClusterPartition* cp)
{	

	if ( cp )
	{
		vertexBuf.str("");
		edgeBuf.str("");
		stringstream buf;
		Ssg = sssg;
		mpp = mp;
		buf<<"digraph COStream {";
		vertexBufInPlace = new stringstream[cp->GetClusters()];
		toBuildClusterOutPutString(Ssg->GetTopLevel(), cp);
		for (int i = 0; i < cp->GetClusters(); i++)
		{
			buf << "\nsubgraph cluster_" << i << " {";
			buf << "\nlabel = \"place_" << i << "\";";
			buf <<vertexBufInPlace[i].str() << "}" ;
		}

		buf<<"\n\n";  
		buf<<edgeBuf.str();
		buf<<"}\n";  
		Ssg->ResetFlatNodeVisitTimes();//将flatnode的visttimes置0
		ofstream fw;
		fw.open(fileName);
		fw<<buf.str();
		fw.close();
	}
	else
	{
		Ssg = sssg;
		mpp = mp;
		buf.str("");
		buf<<"digraph Flattend {\n";
		toBuildOutPutString(Ssg->GetTopLevel());
		buf<<"}\n";  
		Ssg->ResetFlatNodeVisitTimes();//将flatnode的visttimes置0
		ofstream fw;
		fw.open(fileName);
		fw<<buf.str();
		fw.close();
	}

}






