#ifndef _FLAT_NODE_H_
#define _FLAT_NODE_H_

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <math.h>

#ifdef WIN32
#include <direct.h> 
#else  /* UNIX */
#include <unistd.h>
#endif /* WIN32 */

extern "C" {
#include "ast.h"
};


/*************************************************************************/
/*                                                                       */
/*                          SDF nodes                                    */
/*                                                                       */
/*************************************************************************/

class  FlatNode
{
public:
	std::string name; // opeator名字
	std::string PreName;//cwb记录Operator被重命名前的名字
	int visitTimes; // 表示该结点是否已经被访问过

	operatorNode *contents; // 指向operator(经常量传播后的)
	compositeNode *composite; // 指向operator 所在的 composite （实际展开的）
	operatorNode *oldContents; // 指向原始operator
	compositeNode *oldComposite; // 指向原始operator 所在的 composite  （为operator提供composite中的param，var中的参数）

	int nOut; // 输 出 边个数
	int nIn; // 输 入 边个数


	//两级划分算法中,actor所在的place号、thread号、thread中的序列号
	int place_id;
	int thread_id, post_thread_id;
	int serial_id;

	std::vector<FlatNode *> outFlatNodes; // 输 出 边各operator
	std::vector<FlatNode *> inFlatNodes; // 输 入 边各operator
	
	//std::vector<int> AddPopAtCodeGen;//	zww:20120313,一般情况下为零，但采用Horizontalfission时会产生额外的pop值，用此数据来记录
	
	std::vector<int> outPushWeights; // 输 出 边各权重
	std::vector<int> inPopWeights; // 输 入 边各权重
	std::vector<int> inPeekWeights; // 输 入 边各权重

	// 为类模板代码生成方案添加
	std::vector<std::string> outPushString; 
	std::vector<std::string> inPopString; 
	std::vector<std::string> inPeekString;

	std::vector<std::string> pushString; 
	std::vector<std::string> peekString; 

	//节点work函数的静态工作量
	long work_estimate;

	// opeator在ssg的flatnodes中的顺序编号
	int num;

	// opeator是否生成缓存化类型
	bool memorizedNode;

	//GPU节点划分时专用变量 cwb
	int GPUPart;
	bool BorderFlag;

	// 以下参数供今后扩展用
	int currentIn;
	int currentOut;
	int schedMult;
	int schedDivider;
	int uin;
	int label;
	

public:
	FlatNode(operatorNode *node, compositeNode *com, compositeNode *newCom);
	FlatNode(operatorNode *node);//重载构造函数，为了以后在构造新的节点的时候使用
	void AddOutEdges(FlatNode *dest);
	void AddInEdges(FlatNode *src);
	std::string GetOperatorName(); // 获取该flatnode结点的operator的name
	void VisitNode(); // 访问该结点
	void ResetVistTimes(); // 重置visitTimes信息
	int GetVisitTimes(); // 获取该结点的访问信息
	void SetIOStreams();
};

#endif /* _FLAT_NODE_H_ */